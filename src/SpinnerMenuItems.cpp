#include <main.h>
#include <SpinnerMenuItems.h>
#include <SpinnerMenu.h>

#define cstr(x) const_cast<char*>(x)

MenuItem mainMenuItems[] = {
    {cstr("Quick start"), MENU, quickStartMenuConstructor},
    {cstr("Jobs"), MENU, jobsMenuConstructor},
    {cstr("Test"), MENU, testMenuConstructor},
    {cstr("Calibration"), MENU, calibrationMenuConstructor},
    {cstr("Information"), MENU, informationMenuConstructor},
};

MenuItem QuickstartMenuItems[] = {
      {cstr("Programmed"), FUNC, runProgrammed},
      {cstr("Analog"), FUNC, runAnalog},
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
      {cstr("Automatic"), FUNC, automaticCalibration},
      {cstr("Set Kp"), FUNC, setKp},
      {cstr("Set Ki"), FUNC, setKi},
      {cstr("Set Kd"), FUNC, setKd},
};

MenuItem InformationMenuItems[] = {
      {cstr("Firmware\nversion")},
      {cstr("Hardware\nversion")},
      {cstr("LICENSE")},
};

//@@@@@@@@@@@@@@@@@@@@@@@@@@@ Static Menus @@@@@@@@@@@@@@@@@@@@@@@@
Menu mainMenu = Menu(mainMenuItems);
Menu quickStartMenu = Menu(QuickstartMenuItems);
Menu jobsMenu = Menu(JobsMenuItems);
Menu testMenu = Menu(TestMenuItems);
Menu calibrationMenu = Menu(CalibrationMenuItems);
Menu informationMenu = Menu(InformationMenuItems);


//@@@@@@@@@@@@@@@@@@@@@@@@@@@ Menu constructors @@@@@@@@@@@@@@@@@@@@@@@@

void* mainMenuConstructor() { return &mainMenu; }
void* quickStartMenuConstructor(){ return &quickStartMenu; }
void* jobsMenuConstructor() { return &jobsMenu; }
void* testMenuConstructor() { return &testMenu; }
void* calibrationMenuConstructor() { return &calibrationMenu; }
void* informationMenuConstructor() { return &informationMenu; }