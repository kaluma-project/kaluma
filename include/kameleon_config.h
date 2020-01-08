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

#ifndef __KAMELEON_CONFIG_H
#define __KAMELEON_CONFIG_H

/**
 * Condensed version string to be appears in USB serial number string
 * 5-Digits (e.g. 100B1 = 1.0.0-beta.1)
 * - 1st (1) : Major version number in HEX
 * - 2nd (0) : Minor version number in HEX
 * - 3rd (0) : Patch version number in HEX
 * - 4th (B) : Prerelease (A = alpha, B = beta, R = rc)
 * - 5th (1) : Prerelease number in HEX
 */

#define KAMELEON_VERSION "1.0.0-beta.1"
#define KAMELEON_VERSION_CONDENSED "100B1"

#endif /* __KAMELEON_CONFIG_H */
