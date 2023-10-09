#include <main.h>
#include <SpinnerMenuItems.h>
#include <SpinnerMenu.h>

#define cstr(x) const_cast<char*>(x)

MenuItem mainMenuItems[5] = {
    {cstr("Quick start"), MENU, quickStartMenuConstructor},
    {cstr("Jobs"), MENU, jobsMenuConstructor},
    {cstr("Test"), MENU, testMenuConstructor},
    {cstr("Calibration"), MENU, calibrationMenuConstructor},
    {cstr("Information"), MENU, informationMenuConstructor},
};

MenuItem QuickstartMenuItems[2] = {
      {cstr("Programmed"), FUNC, runProgrammed},
      {cstr("Analog"), FUNC, runAnalog},
};

MenuItem JobsMenuItems[3] = {
      {cstr("Load job"), FUNC, nullptr},
      {cstr("Create job"), FUNC, nullptr},
      {cstr("Delete job"), FUNC, nullptr},
};

MenuItem TestMenuItems[4] = {
      {cstr("Combined"), FUNC, nullptr},
      {cstr("Acceleration"), FUNC, nullptr},
      {cstr("Speed"), FUNC, nullptr},
      {cstr("PID"), FUNC, nullptr},
};

MenuItem CalibrationMenuItems[4] = {
      {cstr("Automatic"), FUNC, automaticCalibration},
      {cstr("Set Kp"), FUNC, setKp},
      {cstr("Set Ki"), FUNC, setKi},
      {cstr("Set Kd"), FUNC, setKd},
};

MenuItem InformationMenuItems[3] = {
      {cstr("Firmware\nversion"), FUNC, nullptr},
      {cstr("Hardware\nversion"), FUNC, nullptr},
      {cstr("LICENSE"), FUNC, nullptr},
};

//@@@@@@@@@@@@@@@@@@@@@@@@@@@ Static Menus @@@@@@@@@@@@@@@@@@@@@@@@
Menu mainMenu = Menu(mainMenuItems);
Menu quickStartMenu = Menu(QuickstartMenuItems);
Menu jobsMenu = Menu(JobsMenuItems);
Menu testMenu = Menu(TestMenuItems);
Menu calibrationMenu = Menu(CalibrationMenuItems);
Menu informationMenu = Menu(InformationMenuItems);


//@@@@@@@@@@@@@@@@@@@@@@@@@@@ Menu constructors @@@@@@@@@@@@@@@@@@@@@@@@

Menu* mainMenuConstructor() { return &mainMenu; }
void* quickStartMenuConstructor(){ return &quickStartMenu; }
void* jobsMenuConstructor() { return &jobsMenu; }
void* testMenuConstructor() { return &testMenu; }
void* calibrationMenuConstructor() { return &calibrationMenu; }
void* informationMenuConstructor() { return &informationMenu; }