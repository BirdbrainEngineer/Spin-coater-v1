#pragma once

extern JobTable* jobTable;
extern bool pidTestAvailable;

Menu* mainMenuConstructor();
void* quickStartMenuConstructor(char* caller);
void* jobsMenuConstructor(char* caller);
void* testMenuConstructor(char* caller);
void* calibrationMenuConstructor(char* caller);
void* informationMenuConstructor(char* caller);

void* runJobsMenuConstructor(char* caller);
void* createJobMenuConstructor(char* caller);
void* deleteJobMenuConstructor(char* caller);

MenuData runJobsMenuForge();
MenuData deleteJobsMenuForge();