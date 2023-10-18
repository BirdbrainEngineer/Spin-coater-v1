#include <main.h>


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
const Config defaultConfig = {0.3, 0.004, 0.0, 0.01, 0.2};
const char jobsPath[] = "/jobs/";
const char jobsFolderPath[] = "/jobs";
const char configFilePath[] = "/config";
const u8_t maxFileNameLength = 16;
const u8_t maxJobNameLength = 9;


// =========================Inter-Core variables==============================
volatile float analogDutyCycle = 0.0;
volatile bool pidEnabled = false;
volatile float rpmTarget = 3333.0;
volatile double currentRPM = 0.0;
volatile float pidDutyCycle = 0.0;
volatile Config config = {0.0, 0.0, 0.0, 0.0, 0.0};
volatile Config storedConfig = {0.0, 0.0, 0.0, 0.0, 0.0};
volatile bool motorEnabled = false;


// ========================Core-specific variables===========================

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  Core-0  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
File myFile;
LiquidCrystal lcd = LiquidCrystal(lcd_rs, lcd_en, lcd_d4, lcd_d5, lcd_d6, lcd_d7);
BBkeypad* keypad;
MenuController* menuController;
JobTable* jobTable;
bool cursorBlinker = false;
unsigned long inputBufferIndex = 0;
bool memoryGood = true;



//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  Core-1  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

RP2040_PWM* motorDriver;
PID_controller pidController;
RPI_PICO_Timer ITimer3(3);
SWtimer* coreLoop;
unsigned long core1Timer = 0;
unsigned long core1Loops = 0;

// =======================Interrupt variables=================================
volatile unsigned long rpmTimer = 0;
volatile float analogAverage = 0.01;
volatile unsigned long microsSinceLastRpmCount = 0;
volatile bool newRpmData = false;



// ========================= Core 0 ========================================
void setup() {
  lcd.begin(16, 2);
  lcd.print("Initializing...");
  lcd.setCursor(0, 1);
  lcd.print("Init keypad...   ");

  keypad = new BBkeypad((char*)keys, keypadCols, keypadRows, colPins, rowPins);


  lcd.setCursor(0, 1);
  lcd.print("Init memory...   ");
  if (!SD.begin(17)) {
    printlcdErrorMsg("SD card not\nfound!");
    memoryGood = false;
    printlcdErrorMsg(" Functionality\nhighly limited!");
  }
  else {
    // Checks whether Jobs directory exists, if not, creates it. If a file with the same name exists, deletes it
    if(SD.exists(jobsFolderPath)){
      myFile = SD.open(jobsFolderPath);
      if(myFile.isDirectory()){
        myFile.close();
      }
      else{
        myFile.close();
        SD.remove(jobsFolderPath);
        SD.mkdir(jobsFolderPath);
      }
    }
    else{
      SD.mkdir(jobsFolderPath);
    }
    loadConfiguration(config);
  }

  lcd.setCursor(0, 1);
  lcd.print("Init menu...     ");
  if(memoryGood){
    menuController = new MenuController(mainMenuConstructor);
    menuController->init(10, reboot);
    lcd.setCursor(0, 1);
    lcd.print("Init jobs...     ");
    jobTable = new JobTable(jobsFolderPath);
    auto jobsDirectory = SD.open(jobsFolderPath);
    while(true){
      auto file = jobsDirectory.openNextFile();
      if(!file){ break; }
      SpinnerJob* job = loadJob(file);
      if(job == nullptr){ printlcdErrorMsg("Error loading\na job file!"); }
      else{
        if(!jobTable->addJob(job)){ printlcdErrorMsg("Too many jobs\nsaved to memory!"); }
      }
    }
  }
  else{
    //todo: init reduced menu
  }
}

void loop() {
  lcd.clear();
  MenuControlSignal menuSignal = IDLE;
  MenuContext menuContext = menuController->update(menuSignal);
  renderMenuContext(menuContext);
  while(true){
    keypad->pollStateBlocking(KeyState::KEY_DOWN);
    char key = keypad->buffer[0].key;
    if(key == ENTER){ menuSignal = SELECT; }
    else if(key == BACK){ menuSignal = RETRACT; }
    else if(key == UP){ menuSignal = PREVITEM; }
    else if(key == DOWN){ menuSignal = NEXTITEM; }
    else{ menuSignal = IDLE; }
    menuContext = menuController->update(menuSignal);
    renderMenuContext(menuContext);
  }
}


// ============================== Core 1 =====================================
void setup1() {
  motorDriver = new RP2040_PWM(motor_pwm_pin, motorPWMFrequency, 0.0);
  pidController = PID_controller(config.Kp, config.Ki, config.Kd, controllerMinOutput, controllerMaxOutput);
  coreLoop = new SWtimer(coreLoopInterval, true);

  pinMode(spinner_power_enable_pin, OUTPUT);
  pinMode(spinner_running_led_pin, OUTPUT);
  pinMode(tachometer_pin, INPUT_PULLUP);

  disableMotor();

  attachInterrupt(tachometer_pin, tachInterrupt, CHANGE);
  rpmTimer = micros();

  pinMode(manual_rpm_coarse_adjust_pin, INPUT);
  pinMode(manual_rpm_fine_adjust_pin, INPUT);
  analogReadResolution(16);
  ITimer3.attachInterruptInterval(analogInterruptInterval, analogInterrupt);
  core1Timer = micros();
  interrupts();
  coreLoop->start();
}

void loop1() {
  while(true){
    if(coreLoop->poll()){
      if(motorEnabled){
        float currentDutyCycle;
        if(pidEnabled){
          currentDutyCycle = pidDutyCycle;
        }
        else{
          currentDutyCycle = analogDutyCycle;
        }
        motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, currentDutyCycle);
      }
    }
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
  analogDutyCycle = analogNewAverage * 0.0015259;
  return true;
}

// @@@@@@@@@@@@@@@@@@@@@@@@@  Functions @@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void reboot(uint32_t delay){
  watchdog_reboot(0, 0, delay);
  while(true){}
}
void reboot(){
  reboot(10);
}
void rebootWithText(uint32_t delay, char* text){
  printlcdErrorMsg(text);
  reboot(delay);
}
void rebootWithText(uint32_t delay, const char* text){
  rebootWithText(delay, const_cast<char*>(text));
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
  File file = SD.open(configFilePath);
  if (!file) {
    printlcdErrorMsg("Missing the\nconfig file!");
    saveConfiguration(config);
    printlcdErrorMsg("Using default\nconfiguration.");
    return;
  }

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error){
    printlcdErrorMsg("Failed to\ndeserialize data!");
    printlcdErrorMsg("Using default\nconfiguration.");
    file.close();
    return;
  }
  config.Kp = doc["Kp"];
  config.Ki = doc["Ki"];
  config.Kd = doc["Kd"];
  file.close();
}

// Saves the configuration to a file
void saveConfiguration(volatile Config &config) {
  SD.remove(configFilePath);
  File file = SD.open(configFilePath, FILE_WRITE);
  if (!file) {
    printlcdErrorMsg("Failed to make\nconfig file!");
    return;
  }

  StaticJsonDocument<256> doc;
  doc["Kp"] = config.Kp;
  doc["Ki"] = config.Ki;
  doc["Kd"] = config.Kd;
  if (serializeJsonPretty(doc, file) == 0) {
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

void renderMenuContext(MenuContext menuContext){
  lcd.clear();
  lcd.print(menuContext.name);
  lcd.setCursor(0, 1);
  lcd.print(menuContext.itemSelected);
  lcd.print("/");
  lcd.print(menuContext.numItems);
}

void enableMotor(){
  digitalWrite(spinner_running_led_pin, HIGH);
  digitalWrite(spinner_power_enable_pin, HIGH);
  motorEnabled = true;
}

void disableMotor(){
  motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 0);
  digitalWrite(spinner_running_led_pin, LOW);
  digitalWrite(spinner_power_enable_pin, LOW);
  motorEnabled = false;
}

void panicIfOOM(void* pointer, const char* errorText){
  if(pointer == nullptr){
    char errorMsg[40] = "Out of memory!\n";
    printlcdErrorMsg(strcat(errorMsg, errorText));
    printlcdErrorMsg("Reboot imminent!");
    reboot(100);
    while(true);
  }
}

SpinnerJob* loadJob(File file){
  SpinnerJob* job = new SpinnerJob(file.name());
  if(rp2040.getFreeHeap() < 15000 || job == nullptr){ rebootWithText(100, "Out of memory!\n@:_loadJob"); }
  DynamicJsonDocument doc(15000);
  DeserializationError error = deserializeJson(doc, file);
  if(error){
    delete job;
    doc.~BasicJsonDocument();
    printlcdErrorMsg("Failed to load\nthe job!");
    return nullptr;
  }
  int length = doc["length"];
  job->init(length);
  Config jobConfig = {
    .Kp = doc["Kp"],
    .Ki = doc["Ki"],
    .Kd = doc["Kd"],
    .analogAlpha = doc["analogAlpha"],
    .rpmAlpha = doc["rpmAlpha"]
  };
  job->addConfig(jobConfig);
  for(int i = 0; i < length; i++){
    job->sequence[i].duration = doc["sequence"][i]["duration"];
    job->sequence[i].task = doc["sequence"][i]["task"];
    job->sequence[i].rpm = doc["sequence"][i]["rpm"];
  }
  doc.~BasicJsonDocument();
  return job;
}

bool askYesNo(char* text){
  lcd.clear();
  printlcd(text);
  while(true){
    keypad->pollBlocking();
    if(keypad->getKeysWithState(KeyState::KEY_DOWN) > 0){
      if(keypad->buffer[0].key == YES){
        return true;
      }
      else if(keypad->buffer[0].key == NO || keypad->buffer[0].key == BACK){
        return false;
      }
    }
  }
}

bool askYesNo(){
  while(true){
    keypad->pollBlocking();
    if(keypad->getKeysWithState(KeyState::KEY_DOWN) > 0){
      if(keypad->buffer[0].key == YES){
        return true;
      }
      else if(keypad->buffer[0].key == NO || keypad->buffer[0].key == BACK){
        return false;
      }
    }
  }
}

bool askYesNo(const char* text){
  return askYesNo(const_cast<char*>(text));
}

bool pollKeypadForSpecificCharPressed(char c){
  keypad->poll();
  if(keypad->getKeysWithState(KeyState::KEY_DOWN)){
    if(keypad->buffer[0].key == c){
      return true;
    }
  }
  return false;
}

bool getUserInput(TextBuffer* buffer, bool numeric){
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.cursor();
  while(true){
    keypad->pollStateBlocking(KeyState::KEY_DOWN);
    char key = keypad->buffer[0].key;
    if(key == ENTER){
      lcd.noCursor();
      return buffer->isEmpty()? false : true;
    }
    else if(key == BACK){
      if(buffer->isEmpty()){ lcd.noCursor(); return false; }  
      else{ buffer->popBack(); }
    }
    else{
      if(numeric){
        if(key == DOT){
          buffer->pushBack('.');
        }
        else if(key >= '0' && key <= '9'){
          buffer->pushBack(key);
        }
      }
      else {
        buffer->pushBack(keypad->buffer[0].key);
      }
    }
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(buffer->buffer);
  }
}

bool setUserVariable(const char* displayText, float* variable){
    TextBuffer* textBuffer = new TextBuffer(17);
    lcd.clear();
    lcd.print(displayText);
    while(true){
        lcd.setCursor(0, 1);
        if(keypad->pollStateBlocking(KeyState::KEY_DOWN) > 0){
            char key = keypad->buffer[0].key;
            if(key == ENTER){
                if(textBuffer->isEmpty()){ 
                    delete textBuffer;
                    return false; 
                }
                *variable = (float)atof(textBuffer->buffer);
                delete textBuffer;
                return true;
            }
            else if(key == BACK){
                if(textBuffer->isEmpty()){ 
                    delete textBuffer;
                    return false; 
                }
                textBuffer->popBack();
            }
            else if(key == DOT){
                textBuffer->pushBack('.');
            }
            else{
                if(key >= '0' && key <= '9'){
                    textBuffer->pushBack(key);
                }
            }
        }
        lcd.print(textBuffer->buffer);
    }
}