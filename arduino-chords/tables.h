/*Quantizer

  Creative Commons License

  Quantizer by Pantala Labs is licensed under a
  Creative Commons Attribution 4.0 International License.
  Based on a work at https://www.muffwiggler.com/forum/viewtopic.php?t=50137&highlight=

  Gibran Curtiss SalomÃ£o. MAY/2017 - CC-BY

  Original work:
  Author : SATINDAS
  Code: https://www.muffwiggler.com/forum/viewtopic.php?t=50137&highlight=
*/

//scale00 Chromatic
int  chromaTable[] = {
  0,
  1,  2,  3,  4,  5,  6,  7,  8, 9,  10, 11, 12,
  13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
  25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
  37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
  49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
  60
};

//scale01 Major
int  majTable[] = {
  0,  2,  4,  5,  7,  9, 11,
  12, 14, 16, 17, 19, 21, 23,
  24, 26, 28, 29, 31, 33, 35,
  36, 38, 40, 41, 43, 45, 47,
  48, 50, 52, 53, 55, 57, 59,
  60
};

//scale02 Minor
int  minTable[] = {
  0,  2,  3,  5,  7,  8, 10,
  12, 14, 15, 17, 19, 20, 22,
  24, 26, 27, 29, 31, 32, 34,
  36, 38, 39, 41, 43, 44, 46,
  48, 50, 51, 53, 55, 56, 58,
  60
};

//scale03 Pentatonic
int  pentaTable[] = {
  0,  2,  3,  5,  7, 10,
  12, 14, 15, 17, 19, 22,
  24, 26, 27, 29, 31, 34,
  36, 38, 39, 41, 43, 46,
  48, 50, 51, 53, 55, 58,
  60
};

//scale04 Dorian
int  dorianTable[] = {
  0,  2,  3,  5, 7,  9,  10,
  12, 14, 15, 17, 19, 21, 22,
  24, 26, 27, 29, 31, 33, 34,
  36, 38, 39, 41, 43, 45, 46,
  48, 50, 51, 53, 55, 57, 58,
  60
};

//scale05 Maj7(9)
int  maj3rdTable[] = {
  0, 4,  7, 11,
  12, 16, 19, 23,
  24, 26, 31, 35,
  36, 40, 43, 47,
  48, 50, 54, 55,  59,
  60
};

//scale06 Minor7(9,11)
int  min3rdTable[] = {
  0,   3,   7,  10,
  14,  15,  19,  22,
  24,  26,  27,  31,  34,
  36,  39,  41,  46,
  48,  50,  51,  55,  58,
  60
};

//scale07 (WholeTone)
int  whTable[] = {
  0,   2,   4,   6,   8,  10,
  12,  14,  16,  18,  20,  22,
  24,  26,  28,  30,  32,  34,
  36,  38,  40,  42,  44,  46,
  48,  50,  52,  54,  56,  58,
  60
};


