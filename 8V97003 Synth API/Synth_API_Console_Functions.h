/*
 * Synth_API_Console_Functions.h
 *
 * Author: Mason Edgar
 *
 */

#ifndef SYNTH_API_CONSOLE_FUNCTIONS_H_
#define SYNTH_API_CONSOLE_FUNCTIONS_H_

extern void InitConsole(void);
extern void clearArray(char* input);
extern void getString(char* user_string);
extern void parseString(char* original, char* command, char* value);
extern void printString(char *string);
extern float ConvertStringToFloat(char* string);
extern unsigned long ConvertStringToNumber(char* string);
extern void printFloat(float value);
extern void printInt(int value);
extern void printSingleNumber(int num);

// global array for a quick newline write
extern char newline[] = "\n\r";


#endif /* SYNTH_API_CONSOLE_FUNCTIONS_H_ */