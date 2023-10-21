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
extern volatile float rpmTarget;
extern volatile Config config;
extern volatile Config storedConfig;
extern JobTable* jobTable;
extern const u8_t maxJobNameLength;
extern const char jobsPath[];
extern bool quickRunAvailable;
extern SpinnerJob* pidTestJob;
extern SpinnerJob* quickRunJob;
extern const unsigned long coreLoopInterval;
extern RP2040_PWM* motorDriver;
extern const int motor_pwm_pin;
extern const float motorPWMFrequency;

extern const int spinner_running_led_pin;
extern const int spinner_power_enable_pin;



void* deleteJob(char* caller);
void* runJob(char* caller);

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

void* displayDeviceInformation(char* caller);
void* displayLicenseCC(char* caller);
void* displayLicenseMIT(char* caller);

void* askStoreProgrammed(char* caller);

void* createJob(char* caller);

SpinnerJob* jobCreator();

void* accelerationTest(char* caller);
void* speedTest(char* caller);
void* pidTest(char* caller);



const char projectName[] =      " BB Spin Coater";
const char projectVersion[] =   "     v0.1.0";
const char firmwareVersion[] = "0.1.0";
const char hardwareVersion[] = "0.1.0";


#define DEVICEINFOLEN 15
const char deviceInfo[DEVICEINFOLEN][20] = {
    *projectName,
    *projectVersion,
    *"Firmware version",
    *firmwareVersion,
    *"Hardware version",
    *hardwareVersion,
    *"LICENSES:",
    *"Software:",
    *"MIT",
    *"Hardware:",
    *"CC BY-SA 4.0",
    *"Programming:",
    *"Birdbrain",
    *"Hardware design:",
    *"Birdbrain"
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


