#pragma once
#include <main.h>


extern LiquidCrystal lcd;
extern BBkeypad* keypad;
extern char ENTER;
extern char BACK;
extern char UP;
extern char DOWN;
extern char YES;;
extern char NO;
extern char DOT;
extern volatile bool pidEnabled;
extern volatile float analogDutyCycle;
extern volatile float pidDutyCycle;
extern volatile bool motorEnabled;
extern volatile double currentRPM;

void* jobsMenuConstructor(void* (*callFunc)());
void* loadJob();
void* createJob();
void* deleteJob();
void* jobCreator();

void* runProgrammed();
void* runAnalog();

void* automaticCalibration();
void* setKp();
void* setKi();
void* setKd();

void* askStoreProgrammed();

void* setUserVariable(char* displayText, float* variable);
void* setUserVariable(const char* displayText, float* variable);

bool pollKeypadForSpecificCharPressed(char c);





