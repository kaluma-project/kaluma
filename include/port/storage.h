/* Copyright (c) 2017-2020 Kameleon
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

#ifndef __STORAGE_H
#define __STORAGE_H

#include <stdint.h>

/**
 * Erase all items in the storage
 * @return Return 0 on success or -1 on failture
 */
int storage_clear();

/**
 * Return the number of items in the storage
 * @return The number of items, or -1 on failture
 */
int storage_length();

/**
 * Get value of key index
 * @param key The point to key string
 * @param buf The pointer to the buffer to store value
 * @return Returns the length of value or -1 on failure (key not found)
 */
int storage_get_item(const char *key, char *buf);

/**
 * Set the value with a key string
 * @param key The point to key string
 * @param buf The pointer to the buffer to store value
 * @return Returns 0 on success or -1 on failure or -2 on sweep required or -3 on full storage or -4 on over length.
 */
int storage_set_item(const char *key, char *buf);

/**
 * Remove the key and value of key index
 * @param key The point to key string
 * @return Returns 0 on success or -1 on failure.
 */
int storage_remove_item(const char *key);

/**
 * Get key string of a given index
 * @param index The index of the key
 * @param buf The pointer to the buffer to store key string
 * @return Returns 0 on success or -1 on failure.
 */
int storage_key(const int index, char *buf);

#endif /* __STORAGE_H */
