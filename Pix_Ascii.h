#include <sys/_stdint.h>
#ifndef Pix_Ascii
#define Pix_Ascii
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

#include "chars.h"
#define ROWS 8

const uint8_t (&getByteMap(char character))[ROWS] {


  switch (character) {
    // Uppercase letters
    case 'A':
    case 'a':
        return pix_A;
    case 'B':
    case 'b':
        return pix_B;
    case 'C':
    case 'c':
        return pix_C;
    case 'D':
    case 'd':
        return pix_D;
    case 'E':
    case 'e':
        return pix_E;
    case 'F':
    case 'f':
        return pix_F;
    case 'G':
    case 'g':
        return pix_G;
    case 'H':
    case 'h':
        return pix_H;
    case 'I':
    case 'i':
        return pix_I;
    case 'J':
    case 'j':
        return pix_J;
    case 'K':
    case 'k':
        return pix_K;
    case 'L':
    case 'l':
        return pix_L;
    case 'M':
    case 'm':
        return pix_M;
    case 'N':
    case 'n':
        return pix_N;
    case 'O':
    case 'o':
        return pix_O;
    case 'P':
    case 'p':
        return pix_P;
    case 'Q':
    case 'q':
        return pix_Q;
    case 'R':
    case 'r':
        return pix_R;
    case 'S':
    case 's':
        return pix_S;
    case 'T':
    case 't':
        return pix_T;
    case 'U':
    case 'u':
        return pix_U;
    case 'V':
    case 'v':
        return pix_V;
    case 'W':
    case 'w':
        return pix_W;
    case 'X':
    case 'x':
        return pix_X;
    case 'Y':
    case 'y':
        return pix_Y;
    case 'Z':
    case 'z':
        return pix_Z;
    case 'Ö':
    case 'ö':
        return pix_OE;
    case 'Ä':
    case 'ä':
        return pix_AE;

    // Numbers
    case '0':
        return pix_0;
    case '1':
        return pix_1;
    case '2':
        return pix_2;
    case '3':
        return pix_3;
    case '4':
        return pix_4;
    case '5':
        return pix_5;
    case '6':
        return pix_6;
    case '7':
        return pix_7;
    case '8':
        return pix_8;
    case '9':
        return pix_9;

    // Special characters
    case '!':
        return pix_Exclamation;
    case '#':
        return pix_Hash;
    case '$':
        return pix_Dollar;
    case '%':
        return pix_Percent;
    case '^':
        return pix_Caret;
    case '&':
        return pix_Ampersand;
    case '*':
        return pix_Asterisk;
    case '(':
        return pix_LeftParenthesis;
    case ')':
        return pix_RightParenthesis;
    case '_':
        return pix_Underscore;
    case '+':
        return pix_Plus;
    case '-':
        return pix_Minus;
    case '/':
        return pix_Slash;
    case ':':
        return pix_Colon;
    case ';':
        return pix_Semicolon;
    case '<':
        return pix_LessThan;
    case '=':
        return pix_Equals;
    case '>':
        return pix_GreaterThan;
    case '?':
        return pix_Question;
    case ' ':
        return pix_Space;
    case '█':
        return pix_Block;
    case '▄':
        return pix_HalfBlock;
    case '\'':
        return pix_SingleQuote;
    case '\\':
        return pix_BackSlash;

    default:
        printf("Character not supported: %c\n", character);
        return pix_Question;  // Return question mark for unsupported characters
  }
}

uint8_t** getPixelMatrix(char character){
  uint8_t** arr = new uint8_t*[ROWS];
  for(int i = 0; i < ROWS; i++){
    arr[i] = new uint8_t[5];
  }

  const uint8_t (&bmap)[ROWS] = getByteMap(character);

  for (int i = 0; i < ROWS; ++i) {
    for (int j = 0; j < 5; ++j) {
      arr[i][j] = (bmap[i] >> (5 - 1 - j)) & 1;
    }
  }

  return arr;
}

#endif