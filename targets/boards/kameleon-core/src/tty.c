/* Copyright (c) 2017 Kameleon
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "tty.h"
#include "system.h"
#include "usbd_cdc_if.h"
#include "ringbuffer.h"

#define TTY_TX_RINGBUFFER_SIZE 1024
#define TTY_RX_RINGBUFFER_SIZE 1024

static unsigned char tty_tx_buffer[TTY_TX_RINGBUFFER_SIZE];
static unsigned char tty_rx_buffer[TTY_RX_RINGBUFFER_SIZE];
static ringbuffer_t tty_tx_ringbuffer;
static ringbuffer_t tty_rx_ringbuffer;

/**
*/
void tty_init_ringbuffer() {
  ringbuffer_init(&tty_tx_ringbuffer, tty_tx_buffer, sizeof(tty_tx_buffer));
  ringbuffer_init(&tty_rx_ringbuffer, tty_rx_buffer, sizeof(tty_rx_buffer));
}

/*
* this function is called in the pendable interrupt service routine which has lowest priority
  to allow other interrupts service.
*/
void tty_transmit_data() {   
  /* if the previous data is under transmitting, just return to avoid blocking */
  if (CDC_Transmit_IsReady()) {
    uint32_t len = ringbuffer_length(&tty_tx_ringbuffer);
    if (len) {
      uint8_t buf[TTY_TX_RINGBUFFER_SIZE];
      ringbuffer_read(&tty_tx_ringbuffer, buf, len);      
      CDC_Transmit_FS(buf, len);
    }
  }
}

/**
*/
uint32_t tty_get_tx_freespace() {
  return ringbuffer_freespace(&tty_tx_ringbuffer);
}

/**
*/
uint32_t tty_get_rx_freespace() {
  return ringbuffer_freespace(&tty_rx_ringbuffer);
}

/**
*/
uint32_t tty_get_rx_data_length() {
  return ringbuffer_length(&tty_rx_ringbuffer);
}

/**
*/
uint32_t tty_get_tx_data_length() {
  return ringbuffer_length(&tty_tx_ringbuffer);
}

/**
*/
uint8_t tty_get_byte() {
  uint8_t c;
  ringbuffer_read(&tty_rx_ringbuffer, &c, 1);
  return c;
}

/**
*/
uint32_t tty_get_bytes(uint8_t * buf, uint32_t nToRead) {
  /* interrupt level masking */
  uint32_t pri_group = HAL_NVIC_GetPriorityGrouping();
  uint32_t pre_emption, sub_priority;
  HAL_NVIC_GetPriority(PendSV_IRQn, pri_group, &pre_emption, &sub_priority);
  __set_BASEPRI(pre_emption << 4);
  
  if (tty_get_rx_data_length() < nToRead) {
    nToRead = tty_get_rx_data_length();
  }
  
  __set_BASEPRI(0);
  ringbuffer_read(&tty_rx_ringbuffer, buf, nToRead);
  return nToRead;
}

/**
*/
uint32_t tty_fill_rx_bytes(uint8_t * buf, uint32_t nToWrite) {
  /* interrupt level masking */
  uint32_t pri_group = HAL_NVIC_GetPriorityGrouping();
  uint32_t pre_emption, sub_priority;
  HAL_NVIC_GetPriority(PendSV_IRQn, pri_group, &pre_emption, &sub_priority);
  __set_BASEPRI(pre_emption << 4);
  
  if (tty_get_rx_freespace() < nToWrite) {
    nToWrite = tty_get_rx_freespace();
  }
  __set_BASEPRI(0);
  
  ringbuffer_write(&tty_rx_ringbuffer, buf, nToWrite);
  return nToWrite;
}

/**
*/
void tty_put_byte(uint8_t c) {
  ringbuffer_write(&tty_tx_ringbuffer, (uint8_t *)&c, 1);
}

/**
*/
uint32_t tty_put_bytes(uint8_t * buf, uint32_t nToWrite) {
  /* interrupt level masking */
  uint32_t pri_group = HAL_NVIC_GetPriorityGrouping();
  uint32_t pre_emption, sub_priority;
  HAL_NVIC_GetPriority(PendSV_IRQn, pri_group, &pre_emption, &sub_priority);
  __set_BASEPRI(pre_emption << 4);
  
  if (tty_get_tx_freespace() < nToWrite) {
    nToWrite = tty_get_tx_freespace();
  }
  __set_BASEPRI(0);
  
  ringbuffer_write(&tty_tx_ringbuffer, buf, nToWrite);
  return nToWrite;
}

/**
*/
void tty_init() {
  tty_init_ringbuffer();
}

/**
*/
void tty_putc(char ch) {
  /* (ring)buffering the string instead of transmitting it via usb channel */
  if (tty_get_tx_freespace() > 1) {
    tty_put_byte(ch);
  }
  SetPendSV(); 
}

/**
*/
void tty_printf(const char *fmt, ...) {
  va_list ap;
  char string[256];
  
  va_start(ap,fmt);
  vsprintf(string, fmt, ap);   
  va_end(ap);
  
  /* (ring)buffering the string instead of transmitting it via usb channel */
  if (tty_get_tx_freespace() > strlen(string)) {
    tty_put_bytes((uint8_t *)string, strlen(string));
  }    
  SetPendSV();
}

/**
*/
bool tty_has_data() {
  uint32_t n = tty_get_rx_data_length();
  if (n) {
    return 1;
  } else {
    return 0;
  }
}

/**
*/
uint32_t tty_data_size() {
  return tty_get_rx_data_length();
}

/** non-blocking function
*/
uint8_t tty_getc() {
  uint8_t c = 0;
  if(tty_data_size()) {
    c = tty_get_byte();       
  } 
  return c;
}

/** blocking function
*/
uint8_t tty_getch() {
  while(tty_data_size() == 0);  
  return tty_get_byte();
}

/**
*/
void tty_getstring(char * string) {
  char *string2 = string;
  char c;
  while((c = tty_getch())!='\r') {
    if(c=='\b' || c==127) {
      if ((int)string2 < (int)string) {
        tty_printf("\b \b");
        string--;
      }
    } else {
      *string++ = c;
      tty_putc(c);
    }
  }
  *string='\0';
  tty_putc('\r');   tty_putc('\n');
}

/**
*/
int tty_getintnum() {
  char str[256];
  char *string = str;
  int base     = 10;
  int minus    = 0;
  int result   = 0;
  int lastIndex;
  int i;
  
  tty_getstring(string);
  
  if (string[0]=='-') {
    minus = 1;
    string++;
  }
  
  if (string[0]=='0' && (string[1]=='x' || string[1]=='X')) {
    base    = 16;
    string += 2;
  }
  
  lastIndex = strlen(string) - 1;
  
  if (lastIndex<0) {
    return -1;
  }
  
  if (string[lastIndex]=='h' || string[lastIndex]=='H') {
    base = 16;
    string[lastIndex] = 0;
    lastIndex--;
  }
  
  if (base==10) {
    result = atoi(string);
    result = minus ? (-1*result):result;
  } else {
    for (i=0;i<=lastIndex;i++) {
      if (isalpha(string[i])) {
        if (isupper(string[i])) {
          result = (result<<4) + string[i] - 'A' + 10;
        } else {
          result = (result<<4) + string[i] - 'a' + 10;
        }
      } else {
        result = (result<<4) + string[i] - '0';
      }
    }
    result = minus ? (-1*result):result;
  }
  return result;
}
