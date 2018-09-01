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
int  octaveSelect(int  input) {
  int  value = map(input, 0, 1023, -4, 4);
  return (value);
}

int  semitoneSelect(int  input, int  semitonesPerOctave) {
  int  value = map(input, 0, 1023, -(semitonesPerOctave - 1) , semitonesPerOctave - 1);
  return (value);
}

int noteSelect(int note, int table[], int tableSize) {
  int noteIndex;
  for (int i = 0; i < tableSize; i++) {
    if (note == table[i]) {
      noteIndex = i;
      break;
    }
  }
  if (noteIndex > tableSize) {
    noteIndex = tableSize;
  }
  if (noteIndex < 0) {
    noteIndex = 0;
  }
  return(noteIndex);
}

int  shiftNotes(int note, int shifts, int table[], int tableSize) {
  int noteIndex;
  for (int i = 0; i < tableSize; i++) {
    if (note == table[i]) {
      noteIndex = i + shifts;
      break;
    }
  }
  if (noteIndex > tableSize) {
    noteIndex = tableSize;
  }
  if (noteIndex < 0) {
    noteIndex = 0;
  }
  return (table[noteIndex]);
}


int  mapMaj(int  input, int offset) {
  int  value = (input / 36) + offset;
  return (majTable[value]);
}

int  mapMin(int  input, int offset) {
  int  value = (input / 36) + offset;
  return (minTable[value]);
}

int  mapPenta(int  input, int offset) {
  int  value = (input / 42) + offset;
  return (pentaTable[value]);
}

int  mapDorian(int  input, int offset) {
  int  value = (input / 36) + offset;
  return (dorianTable[value]);
}

int  mapMaj3rd(int  input, int offset) {
  int  value = (input / 61) + offset;
  return (maj3rdTable[value]);
}

int  mapMin3rd(int  input, int offset) {
  int  value = (input / 59) + offset;
  return (min3rdTable[value]);
}

int  mapWh(int  input, int offset) {
  int  value = (input / 42) + offset;
  return (whTable[value]);
}

int  mapChromatic(int  input, int offset) {
  int  value = (input / 21) + offset;
  return (chromaTable[value]);
}

