#pragma once


extern LiquidCrystal lcd;
extern BBkeypad* keypad;
extern char ENTER;
extern char BACK;
extern char UP;
extern char DOWN;
extern char YES;;
extern char NO;
extern char DOT;


void* runProgrammed();
void* runAnalog();
void* askStoreProgrammed();



void* automaticCalibration();
void* setKp();
void* setKi();
void* setKd();


void* setUserVariable(char* displayText, float* variable);
void* setUserVariable(const char* displayText, float* variable);

//@@@@@@@@@@@@@@@@@@@@@@@ Static Menu Items @@@@@@@@@@@@@@@@@@@@@




