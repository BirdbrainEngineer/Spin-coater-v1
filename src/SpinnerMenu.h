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
extern volatile float currentRPM;
extern volatile unsigned long rpmTimer;
extern volatile float rpmTarget;
extern volatile Config config;
extern volatile Config storedConfig;
extern JobTable* jobTable;
extern const u8_t maxJobNameLength;
extern const char jobsPath[];
extern bool quickRunJobAvailable;
extern SpinnerJob* pidTestJob;
extern SpinnerJob* quickRunJob;
extern const unsigned long coreLoopInterval;
extern RP2040_PWM* motorDriver;
extern PID_controller* pidController;
extern const int motor_pwm_pin;
extern const float motorPWMFrequency;
extern volatile float analogAlpha;
extern volatile float rpmAlpha;

extern const int spinner_running_led_pin;
extern const int spinner_power_enable_pin;


void stopTest(const char* text);

void* deleteJob(char* caller);
void* runJob(char* caller);
bool runJob(SpinnerJob* job);

void* runProgrammed(char* caller);
void* runProgrammedJob(char* caller);
void* runAnalog(char* caller);

void* setKp(char* caller);
void* setKi(char* caller);
void* setKd(char* caller);
void* updateMinDutyCycle(char* caller);
void* setAnalogAlpha(char* caller);
void* setRpmAlpha(char* caller);
void* doNothing(char*caller);
void* searchJobForRunning(char* caller);
void* searchJobForDeletion(char* caller);

void* displayDeviceInformation(char* caller);
void* displayLicenseCC(char* caller);
void* displayLicenseMIT(char* caller);

void* askStoreProgrammed(char* caller);

void* createJob(char* caller);

SpinnerJob* jobCreator();

bool accelerationTest(float* acceleration, float* deceleration, float maxSpeed, float minSpeed);
bool speedTest(float* maxSpeed, float* minSpeed);
void* pidTest(char* caller);
void* motorTest(char* caller);


#define DEVICEINFOLEN 15
const char deviceInfo[DEVICEINFOLEN][20] = {
    " BB Spin Coater",
    "     v0.1.0",
    "Firmware version",
    "0.1.0",
    "Hardware version",
    "0.1.1",
    "Programming:",
    "Birdbrain",
    "Hardware design:",
    "Birdbrain",
    "LICENSES:",
    "Software:",
    "MIT",
    "Hardware:",
    "CC BY-SA 4.0"
};


#define MITLICENSELEN 73
const char licenseMIT[MITLICENSELEN][20] = {
    "Copyright(c)", 
    "2023 Birdbrain",
    "Permission is",
    "hereby granted,",
    "free of charge,",
    "to any person",
    "obtaining a copy",
    "of this software",
    "and associated",
    "documentation",
    "files (the",
    "\"Software\"), to", 
    "deal in the ",
    "Software without",
    "restriction, ",
    "including with-",
    "out limitation ",
    "the rights to",
    "use, copy, ",
    "modify, merge,",
    "publish, distri-",
    "bute, sub-",
    "license, and/or",
    "sell copies of",
    "the Software, ",
    "and to permit",
    "persons to whom",
    "the Software is",
    "furnished to do",
    "so, subject to",
    "the following",
    "conditions:",
    "The above copy-",
    "right notice and",
    "this permission",
    "notice shall be",
    "included in all",
    "copies or sub-",
    "stantial ",
    "portions of the",
    "Software.",
    "THE SOFTWARE",
    "IS PROVIDED ",
    "\"AS IS\", WITHOUT",
    "WARRANTY OF ANY",
    "KIND, EXPRESS OR",
    "IMPLIED,",
    "INCLUDING BUT",
    "NOT LIMITED TO",
    "THE WARRANTIES",
    "OF MERCHANT-",
    "ABILITY, FITNESS",
    "FOR A PARTICULAR",
    "PURPOSE AND NON-",
    "INFRINGEMENT.",
    "IN NO EVENT",
    "SHALL THE",
    "AUTHORS OR COPY-",
    "RIGHT HOLDERS BE",
    "LIABLE FOR ANY",
    "CLAIM, DAMAGES",
    "OR OTHER LIA-",
    "BILITY, WHETHER",
    "IN AN ACTION OF",
    "CONTRACT, TORT",
    "OR OTHERWISE,",
    "ARISING FROM,",
    "OUT OF OR IN",
    "CONNECTION WITH",
    "THE SOFTWARE OR",
    "THE USE OR OTHER",
    "DEALINGS IN THE",
    "SOFTWARE.",
};

#define CCLICENSELEN 10
const char licenseCC[CCLICENSELEN][20] = {
    "  CC BY-SA 4.0",
    "      DEED",
    "Creative Commons",
    "Attribution-",
    "ShareAlike 4.0",
    "International",
    "Visit:",
    "\"creativecommons",
    ".org\"",
    "for details."
};


