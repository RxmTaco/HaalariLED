#ifndef SPEC_H
#define SPEC_H
/* Author: Remi / RxmTaco https://github.com/RxmTaco
MIT License

Copyright (c) 2024 Remi Dubrovics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// Current limit is implemented to prevent the display getting stuck in a boot loop due to power supply resetting
#define CLIMIT 2.0  // Amperes LIMIT  // Change this according to the capabilities of your power supply
#define CR 0.012    // Amperes RED    // These values have been measured with a 2.0A load
#define CG 0.012    // Amperes GREEN
#define CB 0.012    // Amperes BLUE
#define VL 5.0      // Volts

#endif