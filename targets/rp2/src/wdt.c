/* Copyright (c) 2017 Kaluma
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

#include "wdt.h"

#include <stdint.h>

#include "board.h"
#include "err.h"
#include "hardware/watchdog.h"
#define MAX_TIMEOUT 0x7fffff

/**
 * Enable watchdog timer
 *
 * @param {uint32_t} timeout, milliseconds
 * @return error code
 */
int km_wdt_enable(bool en, uint32_t timeout_ms) {
    (void) en;
    if (timeout_ms > MAX_TIMEOUT) {
        timeout_ms = MAX_TIMEOUT;
    }
    watchdog_enable(timeout_ms, true);
    return 0;
}

/**
 * Feed (Kick) watch dog reset timer
 *
 */
void km_wdt_feed(void) {
    watchdog_update();
}
