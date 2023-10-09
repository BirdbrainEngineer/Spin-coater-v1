#include <main.h>

// =====================Device information constants========================
const char DEVICE_NAME[] = "BB Spin Coater\nv0.1.0";
const char HARDWARE_VERSION[] = "0.1.0";
const char FIRMWARE_VERSION[] = "0.1.0";
const char HARDWARE[] = "Birdbrain";
const char SOFTWARE[] = "Birdbrain";
const char LICENSE[] = "MIT";


// ============================GPIO PINOUT==================================
// 0 used for usb
// 1 used for usb
const int lcd_rs = 2;
const int lcd_en = 3;
const int lcd_d4 = 5;
const int lcd_d5 = 4;
const int lcd_d6 = 6;
const int lcd_d7 = 7;
const int membrane_row_0_pin = 8;
const int membrane_row_1_pin = 9;
const int membrane_row_2_pin = 10;
const int membrane_row_3_pin = 11;
const int membrane_col_0_pin = 12;
const int membrane_col_1_pin = 13;
const int membrane_col_2_pin = 14;
const int membrane_col_3_pin = 15;
//=====================
const int SPI0_RX = 16;  //Arduino-pico core default pin
const int SPI0_SC = 17;  //Arduino-pico core default pin
const int SPI0_CLK = 18; //Arduino-pico core default pin
const int SPI0_TX = 19;  //Arduino-pico core default pin
const int spinner_power_enable_pin = 20;
const int tachometer_pin = 21;
const int motor_pwm_pin = 22;
// 23 not available
// 24 not available
// 25 not available
const int spinner_running_led_pin = 26;
const int manual_rpm_fine_adjust_pin = 27;
const int manual_rpm_coarse_adjust_pin = 28;


// ========================Program constants================================
const float motorPWMFrequency = 25000.0;
const int analogInterruptInterval = 100;
const float controllerMinOutput = 12.0;
const float controllerMaxOutput = 100.0;
const u16_t inputBufferSize = 16;
const long tachometerDebounceInterval = 800;
const u8_t keypadRows = 4;
const u8_t keypadCols = 4;
const char keys[keypadRows][keypadCols] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
char ENTER = '#';
char BACK = '*';
char UP = 'C';
char DOWN = 'D';
char YES = 'A';
char NO = 'B';
char DOT = 'A';
const u8_t rowPins[keypadRows] = {membrane_row_0_pin, membrane_row_1_pin, membrane_row_2_pin, membrane_row_3_pin};
const u8_t colPins[keypadCols] = {membrane_col_0_pin, membrane_col_1_pin, membrane_col_2_pin, membrane_col_3_pin};
const unsigned int keypadDebounceInterval = 10;
const unsigned long coreLoopInterval = 500; //in micros
volatile float analogAlpha = 0.01;
volatile float rpmAlpha = 0.2;
const Config defaultConfig = {0.3, 0.004, 0.0};



// =========================Inter-Core variables==============================
volatile float manualDutyCycle = 0.0;
volatile bool PID_enabled = false;
volatile float rpmTarget = 3333.0;
volatile double currentRPM = 0.0;
volatile float dutyCycle = 0.0;
volatile Config config = {0.0, 0.0, 0.0};
volatile Config storedConfig = {0.0, 0.0, 0.0};



// ========================Core-specific variables===========================

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  Core-0  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
File myFile;
LiquidCrystal lcd = LiquidCrystal(lcd_rs, lcd_en, lcd_d4, lcd_d5, lcd_d6, lcd_d7);
BBkeypad* keypad;
TextBuffer* buffer;
MenuController menuController;
char inputBuffer[inputBufferSize];
bool cursorBlinker = false;
unsigned long inputBufferIndex = 0;
bool memoryGood = true;



//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  Core-1  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

RP2040_PWM* motorDriver;
PID_controller pidController;
RPI_PICO_Timer ITimer3(3);
bool motorEnabled = false;
unsigned long core1Timer = 0;
unsigned long core1Loops = 0;

// =======================Interrupt variables=================================
volatile unsigned long rpmTimer = 0;
volatile float analogAverage = 0.01;
volatile unsigned long microsSinceLastRpmCount = 0;
volatile bool newRpmData = false;



// ========================= Core 0 ========================================
void setup() {
  pinMode(spinner_running_led_pin, OUTPUT);

  keypad = new BBkeypad((char*)keys, keypadCols, keypadRows, colPins, rowPins);
  lcd.begin(16, 2);
  lcd.print("Initializing...");

  if (!SD.begin(17)) {
    printlcdErrorMsg("SD card not\nfound!");
    memoryGood = false;
    printlcdErrorMsg(" Functionality\nhighly limited!");
    // todo: enable limited menu
  }
  else {
    // Checks whether Jobs directory exists, if not, creates it. If a file with the same name exists, deletes it
    if(SD.exists("/jobs")){
      myFile = SD.open("/jobs");
      if(myFile.isDirectory()){
        myFile.close();
      }
      else{
        myFile.close();
        SD.remove("/jobs");
        SD.mkdir("/jobs");
      }
    }
    else{
      SD.mkdir("/jobs");
    }
    loadConfiguration(config);
    // todo: enable main menu
  }

  digitalWrite(spinner_running_led_pin, LOW);
}

void loop() {
  
}


// ============================== Core 1 =====================================
void setup1() {
  motorDriver = new RP2040_PWM(motor_pwm_pin, motorPWMFrequency, 0.0);
  pidController = PID_controller(config.Kp, config.Ki, config.Kd, controllerMinOutput, controllerMaxOutput);

  pinMode(spinner_power_enable_pin, OUTPUT);
  pinMode(tachometer_pin, INPUT_PULLUP);

  attachInterrupt(tachometer_pin, tachInterrupt, CHANGE);
  rpmTimer = micros();

  pinMode(manual_rpm_coarse_adjust_pin, INPUT);
  pinMode(manual_rpm_fine_adjust_pin, INPUT);
  analogReadResolution(16);
  ITimer3.attachInterruptInterval(analogInterruptInterval, analogInterrupt);
  core1Timer = micros();
  interrupts();
}

void loop1() {
  unsigned int loopInterval = coreLoopInterval;
  while(true){
    while((micros() - core1Timer) < loopInterval){}
    core1Timer = micros();
    core1Loops++;

  }
}


// @@@@@@@@@@@@@@@@@@@@@@@@@  Interrupts  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void tachInterrupt(){
  unsigned long currentTime = micros();
  unsigned long microsElapsed = currentTime - rpmTimer;
  if (microsElapsed < tachometerDebounceInterval) { return; }
  microsSinceLastRpmCount = microsElapsed;
  double rpmSample = 1000000.0 / (double)microsElapsed * 15.0; // four transitions per revolution
  currentRPM = (rpmSample * rpmAlpha) + (currentRPM * (1.0 - rpmAlpha));
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

void reboot(uint32_t delay){
  watchdog_reboot(0, 0, delay);
  while(true){}
}

void printlcd(char* text){
  u8_t pointer = 0;
  u8_t bufPointer = 0;
  u8_t cursorPointer = 0; 
  char buf[50];
  lcd.clear();
  for(int i = 0; i < 50; i++){
    switch(text[i]){
      case '\n':  buf[bufPointer] = '\0';
                  lcd.print(buf);
                  cursorPointer++;
                  lcd.setCursor(0, cursorPointer);
                  bufPointer = 0;
                  pointer++;
                  break;
      case '\0':  buf[bufPointer] = '\0';
                  lcd.print(buf);
                  return;
      default:    buf[bufPointer] = text[i];
                  bufPointer++;
                  pointer++;
                  break;
    }
  }
}

void printlcd(const char* text){
  printlcd(const_cast<char*>(text));
}

void loadConfiguration(volatile Config &config) {
  File file = SD.open("/config.txt");
  if (!file) {
    printlcdErrorMsg("Missing the\nconfig file!");
    saveConfiguration(config);
    printlcdErrorMsg("Using default\nconfig.");
    return;
  }

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error){
    printlcdErrorMsg("Failed to\ndeserialize data!");
    printlcdErrorMsg("Using default\nconfig.");
    return;
  }
  config.Kp = doc["Kp"];
  config.Ki = doc["Ki"];
  config.Kd = doc["Kd"];
  file.close();
}

// Saves the configuration to a file
void saveConfiguration(volatile Config &config) {
  SD.remove("/config.txt");
  File file = SD.open("/config.txt", FILE_WRITE);
  if (!file) {
    printlcdErrorMsg("Failed to make\nconfig file!");
    return;
  }

  StaticJsonDocument<256> doc;
  doc["Kp"] = config.Kp;
  doc["Ki"] = config.Ki;
  doc["Kd"] = config.Kd;
  if (serializeJson(doc, file) == 0) {
    printlcdErrorMsg("Failed to\nserialize data!");
  }
  file.close();
}

void printlcdErrorMsg(char* text){
  printlcd(text);
  while(true){
    keypad->pollBlocking();
    if(keypad->getPressedKeys() > 0){
      if(keypad->buffer[0].key == ENTER || keypad->buffer[0].key == BACK){
        return;
      }
    }
  }
}

void printlcdErrorMsg(const char* text){
  printlcdErrorMsg(const_cast<char*>(text));
}