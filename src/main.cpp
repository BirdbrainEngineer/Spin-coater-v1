#include <Arduino.h>
#include <LiquidCrystal.h>
#include <SD.h>
#include <SPI.h>
#include <RP2040_PWM.h>
#include <RPi_Pico_TimerInterrupt.h>
#include <PID_controller.h>
#include <BBkeypad.h>

class PID_controller;

// =====================Device information constants========================
const char DEVICE_NAME[] = "BB Spin Coater";
const char HARDWARE_REVISION[] = "0.1.0";
const char FIRMWARE_VERSION[] = "0.1.0";
const char HARDWARE[] = "Birdbrain";
const char SOFTWARE[] = "Birdbrain";
const char LICENSE[] = "MIT";


// ============================GPIO PINOUT==================================
// = 0;
// = 1;
const int lcd_d4 = 2;
const int lcd_d5 = 3;
const int lcd_d6 = 4;
const int lcd_d7 = 5;
const int membrane0_pin = 6;
const int membrane1_pin = 7;
const int membrane2_pin = 8;
const int membrane3_pin = 9;
const int membrane4_pin = 10;
const int membrane5_pin = 11;
const int membrane6_pin = 12;
const int membrane7_pin = 13;
const int lcd_rs = 14;
const int lcd_en = 15;
//=====================
const int SPI0_RX = 16;  //Arduino-pico core default pin
const int SPI0_SC = 17;  //Arduino-pico core default pin
const int SPI0_CLK = 18; //Arduino-pico core default pin
const int SPI0_TX = 19;  //Arduino-pico core default pin
const int spinner_power_enable_pin = 20;
const int motor_pwm_pin = 21;
const int tachometer_pin = 22;
// = 23;
// = 24;
// = 25;
// = 26;
const int manual_rpm_fine_adjust_pin = 27;
const int manual_rpm_coarse_adjust_pin = 28;

// ===========================Declarations==================================
void tachInterrupt();
bool analogInterrupt(struct repeating_timer *t);
void processInput(Key inputKey);
void(* resetFunc) (void) = 0;


// ========================Program constants================================
const float motorPWMFrequency = 25000.0;
enum CoreMsg {
  NONE,
  RECEIVED_FAULTY,
  START_MOTOR,
  STOP_MOTOR,
  KICK_MOTOR,
};
const int analogInterruptInterval = 100;
const float controllerMinOutput = 12.0;
const float controllerMaxOutput = 100.0;
const unsigned long inputBufferSize = 16;
const long tachometerDebounceInterval = 800;
const u8_t keyPadRows = 4;
const u8_t keyPadCols = 4;
char keys[keyPadRows][keyPadCols] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
const u8_t rowPins[keyPadRows] = {membrane0_pin, membrane1_pin, membrane2_pin, membrane3_pin};
const u8_t colPins[keyPadCols] = {membrane4_pin, membrane5_pin, membrane6_pin, membrane7_pin};
const unsigned int keypadDebounceInterval = 10;


// =========================Inter-Core variables==============================
volatile float manualDutyCycle = 0.0;
volatile float analogAlpha = 0.01;
volatile bool PID_enabled = false;
volatile float rpmTarget = 3333.0;
volatile double currentRpm = 0.0;
volatile float dutyCycle = 0.0;
volatile CoreMsg coreMsgBuf[2] = {NONE, NONE};
volatile float Kp = 0.3;
volatile float Ki = 0.004;
volatile float Kd = 0.0;


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  Core-0  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

// ========================Core-specific variables===========================
File myFile;
LiquidCrystal lcd = LiquidCrystal(lcd_rs, lcd_en, lcd_d4, lcd_d5, lcd_d6, lcd_d7);
BBkeypad* keypad;
bool cursorBlinker = false;
char inputBuffer[inputBufferSize];
unsigned long inputBufferIndex = 0;
bool memoryGood = true;


// ========================= Code ========================================
void setup() {
  keypad = new BBkeypad((char*)keys, keyPadCols, keyPadRows, colPins, rowPins);
  lcd.begin(16, 2);
  lcd.print("Initializing...");

  if (!SD.begin(17)) {
    lcd.print("Memory failure!");
    lcd.setCursor(0, 1);
    lcd.print("#:Cont.  Reset:*");
    while(true){
      keypad->pollBlocking();
      if(keypad->getKeysWithState(KEY_DOWN) > 0){
        switch(keypad->buffer[0].key){
          case '#':   memoryGood = false;
                      lcd.clear();
                      lcd.print("Loading and saving");
                      lcd.setCursor(0, 1);
                      lcd.print("not available.");
                      delay(2000);
                      break;
          case '*':   lcd.clear();
                      lcd.print("Restarting...");
                      delay(2000);
                      resetFunc();
                      break;
        }
      }
    }
  }

  myFile = SD.open("test.txt", FILE_WRITE);
  if (myFile) {
    myFile.println("PID tuning");
    myFile.close();
  }
  coreMsgBuf[1] = START_MOTOR;
}

void loop() {
  cursorBlinker = !cursorBlinker;
  cursorBlinker ? lcd.cursor() : lcd.noCursor();
  if(keypad->poll()){
    if(keypad->getKeysWithState(KEY_DOWN) > 0){
      processInput(keypad->buffer[0]);
    }
  }
  lcd.clear();
  lcd.print(dutyCycle, 2);
  lcd.print(" rpm:");
  lcd.print(currentRpm, 2);
  lcd.setCursor(0, 1);
  lcd.print(inputBuffer);
  delay(200);
}


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  Core-1  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

// =======================Interrupt variables=================================
volatile unsigned long rpmTimer = 0;
volatile float analogAverage = 0.01;
volatile unsigned long microsSinceLastRpmCount = 0;
volatile bool newRpmData = false;

// ======================Core-specific variables==============================
const unsigned long coreLoopInterval = 500; //in micros
RP2040_PWM* motorDriver;
PID_controller pidController;
RPI_PICO_Timer ITimer2(2);
bool motorEnabled = false;
unsigned long core1Timer = 0;
unsigned long core1Loops = 0;


// ==============================Code=====================================
void setup1() {
  motorDriver = new RP2040_PWM(motor_pwm_pin, motorPWMFrequency, 0.0);
  pidController = PID_controller(Kp, Ki, Kd, controllerMinOutput, controllerMaxOutput);

  pinMode(spinner_power_enable_pin, OUTPUT);
  pinMode(tachometer_pin, INPUT_PULLUP);

  attachInterrupt(tachometer_pin, tachInterrupt, CHANGE);
  rpmTimer = micros();

  pinMode(manual_rpm_coarse_adjust_pin, INPUT);
  pinMode(manual_rpm_fine_adjust_pin, INPUT);
  analogReadResolution(16);
  ITimer2.attachInterruptInterval(analogInterruptInterval, analogInterrupt);
  core1Timer = micros();
  interrupts();
}

void loop1() {
  while((micros() - core1Timer) > 500){}
  core1Timer = micros();
  switch(coreMsgBuf[1]){
    case NONE:              break;
    case RECEIVED_FAULTY:   break;

    case START_MOTOR:       digitalWrite(spinner_power_enable_pin, HIGH); 
                            rpmTimer = micros();
                            PID_enabled ?
                              motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, controllerMaxOutput) :
                              motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, -(manualDutyCycle - 100.0));
                            motorEnabled = true;
                            coreMsgBuf[1] = NONE;
                            break;

    case STOP_MOTOR:        motorEnabled = false; 
                            motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 100);
                            digitalWrite(spinner_power_enable_pin, LOW);
                            coreMsgBuf[1] = NONE;
                            break;

    case KICK_MOTOR:        if(motorEnabled){
                              motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, controllerMaxOutput);
                              coreMsgBuf[1] = NONE;
                            }
                            break;

    default:                coreMsgBuf[0] = RECEIVED_FAULTY; break;
  }

  if(motorEnabled){
    if(PID_enabled){
      if(newRpmData){
        dutyCycle = pidController.compute(rpmTarget, currentRpm, microsSinceLastRpmCount);
        motorDriver->setPWM(motor_pwm_pin, 25000.0, -(dutyCycle - 100.0)); // physical duty cycle is inverted thanks to inverter push-pull
      }
    }
    else if((core1Loops % 20) == 0) {
      dutyCycle = manualDutyCycle;
      motorDriver->setPWM(motor_pwm_pin, 25000.0, -(dutyCycle - 100.0));
    }
  }
  core1Loops++;
}


// @@@@@@@@@@@@@@@@@@@@@@@@@  Interrupts  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void tachInterrupt(){
  if(!motorEnabled){ return; }
  unsigned long currentTime = micros();
  unsigned long microsElapsed = currentTime - rpmTimer;
  if (microsElapsed < tachometerDebounceInterval) { return; }
  microsSinceLastRpmCount = microsElapsed;
  currentRpm = 1000000.0 / (double)microsElapsed * 15.0; // four transitions per revolution
  rpmTimer = currentTime;
}

bool analogInterrupt(struct repeating_timer *t){
  // Exponential moving average
  int reading = (analogRead(manual_rpm_coarse_adjust_pin) + analogRead(manual_rpm_fine_adjust_pin) / 10);
  reading = reading > 65535 ? 65535 : (reading < 0 ? 0 : reading);
  float analogNewAverage = ((float)(reading) * analogAlpha) + (analogAverage * (1.0 - analogAlpha));
  analogAverage = analogNewAverage;
  manualDutyCycle = analogNewAverage * 0.0015259;
  return true;
}

// @@@@@@@@@@@@@@@@@@@@@@@@@  Functions @@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void processInput(Key inputKey){
  char key = inputKey.key;
  switch (key){
    case '#':   rpmTarget = (float)atoi(inputBuffer);
                inputBufferIndex = 0;
                inputBuffer[inputBufferIndex] = '\0';
                break;

    case '*':   if(inputBufferIndex > 0){
                  inputBufferIndex--;
                  inputBuffer[inputBufferIndex] = '\0';
                }
                break;

    case 'A':   PID_enabled = false;
                break;

    case 'B':   PID_enabled = true;
                coreMsgBuf[1] = KICK_MOTOR;
                break;

    case 'C':   Kp = (float)atof(inputBuffer);
                inputBufferIndex = 0;
                inputBuffer[inputBufferIndex] = '\0';
                break;

    case 'D' :  if(inputBufferIndex < (inputBufferSize - 1)){
                  inputBuffer[inputBufferIndex] = '.';
                  inputBufferIndex++; 
                  inputBuffer[inputBufferIndex] = '\0';
                }
                break;

    default:    if(inputBufferIndex < (inputBufferSize - 1)){
                  inputBuffer[inputBufferIndex] = key;
                  inputBufferIndex++; 
                  inputBuffer[inputBufferIndex] = '\0';
                }
                break;
  }
}
