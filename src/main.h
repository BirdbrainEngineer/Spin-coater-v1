#pragma once

struct Config {
  float Kp;
  float Ki;
  float Kd;
  float analogAlpha;
  float rpmAlpha;
};

#include <Arduino.h>
#include <LiquidCrystal.h>
#include <SD.h>
#include <SPI.h>
#include <RP2040_PWM.h>
#include <RPi_Pico_TimerInterrupt.h>
#include <PID_controller.h>
#include <BBkeypad.h>
#include <TextBuffer.h>
#include <ArduinoJson.h>
#include <SpinnerCore.h>
#include <Menu.h>
#include <SWtimer.h>
#include <SpinnerMenu.h>
#include <SpinnerMenuItems.h>

void tachInterrupt();
bool analogInterrupt(struct repeating_timer *t);

void reboot();
void reboot(u32_t delay);
void rebootWithText(u32_t delay, char* text);
void rebootWithText(u32_t delay, const char* text);

bool askYesNo(char* text);
bool askYesNo(const char* text);
bool askYesNo();
bool pollKeypadForSpecificCharPressed(char c);

void printlcd(char* text);
void printlcd(const char* text);
void printlcdErrorMsg(char* text);
void printlcdErrorMsg(const char* text);

void loadConfiguration(volatile Config &config);
void saveConfiguration(volatile Config &config);

void enableMotor();
void disableMotor();
SpinnerJob* loadJob(File file);
bool saveJob(SpinnerJob* job);
bool getUserInput(TextBuffer* buffer, bool numeric = false);

void renderMenuContext(MenuContext menuContext);

void panicIfOOM(void* pointer, const char* errorText);

bool setUserVariable(const char* displayText, float* variable);