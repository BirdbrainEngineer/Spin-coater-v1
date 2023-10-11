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
      {cstr("Load job"), MENU, loadJobMenuConstructor},
      {cstr("Create job"), MENU, createJobMenuConstructor},
      {cstr("Delete job"), MENU, deleteJobMenuConstructor},
};

MenuItem TestMenuItems[4] = {
      {cstr("Combined"), FUNC, testCombined},
      {cstr("Acceleration"), FUNC, testAcceleration},
      {cstr("Speed"), FUNC, testSpeed},
      {cstr("PID"), FUNC, testPID},
};

MenuItem CalibrationMenuItems[6] = {
      {cstr("Automatic"), FUNC, automaticCalibration},
      {cstr("Set Kp"), FUNC, setKp},
      {cstr("Set Ki"), FUNC, setKi},
      {cstr("Set Kd"), FUNC, setKd},
      {cstr("Analog-in alpha"), FUNC, setAnalogAlpha},
      {cstr("RPM alpha"), FUNC, setRpmAlpha},
};

MenuItem InformationMenuItems[3] = {
      {cstr("Device info."), FUNC, displayDeviceInformation},
      {cstr("HARDWARE LICENSE"), FUNC, displayLicenseCC},
      {cstr("SOFTWARE LICENSE"), FUNC, displayLicenseMIT},
};


//@@@@@@@@@@@@@@@@@@@@@@@@@@@ Static Menus @@@@@@@@@@@@@@@@@@@@@@@@
Menu mainMenu = Menu(mainMenuItems);
Menu quickStartMenu = Menu(QuickstartMenuItems);
Menu jobsMenu = Menu(JobsMenuItems);
Menu testMenu = Menu(TestMenuItems);
Menu calibrationMenu = Menu(CalibrationMenuItems);
Menu informationMenu = Menu(InformationMenuItems);


//@@@@@@@@@@@@@@@@@@@@@@@@@@@ Static menu constructors @@@@@@@@@@@@@@@@@@@@@@@@

Menu* mainMenuConstructor() { return &mainMenu; }
void* quickStartMenuConstructor(){ return &quickStartMenu; }
void* jobsMenuConstructor() { return &jobsMenu; }
void* testMenuConstructor() { return &testMenu; }
void* calibrationMenuConstructor() { return &calibrationMenu; }
void* informationMenuConstructor() { return &informationMenu; }


//@@@@@@@@@@@@@@@@@@@@@@@@@@@ Dynamic menu constructors @@@@@@@@@@@@@@@@@@@@@@@@

void* programmingMenuConstructor(){
    
}

void* loadJobMenuConstructor(){return jobsMenuConstructor(loadJob);}
void* createJobMenuConstructor(){return jobsMenuConstructor(createJob);}
void* deleteJobMenuConstructor(){return jobsMenuConstructor(deleteJob);}