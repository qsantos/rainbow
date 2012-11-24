#ifndef RAINBOW_H
#define RAINBOW_H

#include <stdio.h>

// number of currently active chains
extern unsigned int n_chains;

// generation
void Rainbow_Init     (unsigned int length, char* chars, unsigned int depth, unsigned int count);
void Rainbow_Deinit   (void);
char Rainbow_FindChain(void);
void Rainbow_Sort     (void);

// save and load
void Rainbow_ToFile   (FILE* f);
void Rainbow_FromFile (FILE* f);

// use
void Rainbow_Print    (void);
char Rainbow_Reverse  (char* target, char* dest);

// useful functions
char bstrncmp   (char* a, char* b, int n);
void hex2hash   (char* hex, char* hash);
void printHash  (char* hash);
void printString(char* str);

#endif
