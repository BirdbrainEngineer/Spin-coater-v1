#pragma once
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


// ===========================Declarations==================================
enum SpinnerState {
  IDLE,
  RUN_PID,
  RUN_ANALOG,
  FAULT,
};
struct Config {
  float Kp;
  float Ki;
  float Kd;
};

void tachInterrupt();
bool analogInterrupt(struct repeating_timer *t);

void startMotor();
void stopMotor();

void reboot(u32_t delay);

void printlcd(char* text);
void printlcd(const char* text);
void printlcdErrorMsg(char* text);
void printlcdErrorMsg(const char* text);

void loadConfiguration(volatile Config &config);
void saveConfiguration(volatile Config &config);
