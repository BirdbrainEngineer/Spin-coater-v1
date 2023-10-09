#include <main.h>

#define cstr(x) const_cast<char*>(x)

MenuItem mainMenuItems[] = {
    {cstr("Quick start"), MENU, quickStartMenuConstructor, askStoreProgrammed},
    {cstr("Jobs"), MENU, jobsMenuConstructor, nullptr},
    {cstr("Test"), MENU, testMenuConstructor, nullptr},
    {cstr("Calibration"), MENU, calibrationMenuConstructor, nullptr},
    {cstr("Information"), MENU, informationMenuConstructor, nullptr},
};

MenuItem QuickstartMenuItems[] = {
      {cstr("Programmed"), FUNC, runProgrammed, nullptr},
      {cstr("Analog"), FUNC, runAnalog, nullptr},
};

MenuItem JobsMenuItems[] = {
      {cstr("Load job")},
      {cstr("Create job")},
      {cstr("Delete job")},
};

MenuItem TestMenuItems[] = {
      {cstr("Combined")},
      {cstr("Acceleration")},
      {cstr("Speed")},
      {cstr("PID")},
};

MenuItem CalibrationMenuItems[] = {
      {cstr("Automatic"), FUNC, &automaticCalibration, nullptr},
      {cstr("Set Kp"), FUNC, &setKp, nullptr},
      {cstr("Set Ki"), FUNC, &setKi, nullptr},
      {cstr("Set Kd"), FUNC, &setKd, nullptr},
};

MenuItem InformationMenuItems[] = {
      {cstr("Firmware\nversion")},
      {cstr("Hardware\nversion")},
      {cstr("LICENSE")},
};

//@@@@@@@@@@@@@@@@@@@@@@@@@@@ Menus @@@@@@@@@@@@@@@@@@@@@@@@

void* mainMenuConstructor() { return (void*)new Menu(mainMenuItems); }
void* quickStartMenuConstructor(){ return (void*)new Menu(QuickstartMenuItems); }
void* JobsMenuConstructor() { return (void*)new Menu(JobsMenuItems); }
void* TestMenuConstructor() { return (void*)new Menu(TestMenuItems); }
void* CalibrationMenuConstructor() { return (void*)new Menu(CalibrationMenuItems); }
void* InformationMenuconstructor() { return (void*)new Menu(InformationMenuItems); }