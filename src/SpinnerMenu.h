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
extern volatile Config config;
extern JobTable* jobTable;



void* deleteJob(char* jobName);
void* runJob(char* jobName);

void* runProgrammed(char* caller);
void* runAnalog(char* caller);

void* automaticCalibration(char* caller);
void* setKp(char* caller);
void* setKi(char* caller);
void* setKd(char* caller);
void* setAnalogAlpha(char* caller);
void* setRpmAlpha(char* caller);
void* doNothing(char*caller);
void* searchJobForRunning(char* caller);
void* searchJobForDeletion(char* caller);

void* askStoreProgrammed(char* caller);

void* createJob(char* caller);

SpinnerJob* jobCreation(bool askForName);




