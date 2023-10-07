#include <Menu.h>



void runProgrammed();
void askStoreProgrammed();
void runAnalog();

void automaticCalibration();
void setKp();
void setKi();
void setKd();

//@@@@@@@@@@@@@@@@@@@@@@@ Static Menu Items @@@@@@@@@@@@@@@@@@@@@

MenuItem mainMenuItems[] = {
    {"Quick start", &QuickstartMenu, &askStoreProgrammed},
    {"Jobs", &JobsMenu, nullptr},
    {"Test", &TestMenu, nullptr},
    {"Calibration", &CalibrationMenu, nullptr},
    {"Information", &InformationMenu, nullptr},
};

MenuItem QuickstartMenuItems[] = {
      {"Programmed", &runProgrammed, nullptr},
      {"Analog", &runAnalog, nullptr},
};

MenuItem JobsMenuItems[] = {
      {"Load job"},
      {"Create job"},
      {"Delete job"},
};

MenuItem TestMenuItems[] = {
      {"Combined"},
      {"Acceleration"},
      {"Speed"},
      {"PID"},
};

MenuItem CalibrationMenuItems[] = {
      {"Automatic", &automaticCalibration, nullptr},
      {"Set Kp", &setKp, nullptr},
      {"Set Ki", &setKi, nullptr},
      {"Set Kd", &setKd, nullptr},
};

MenuItem InformationMenuItems[] = {
      {"Firmware\nversion"},
      {"Hardware\nversion"},
      {"LICENSE"},
};

//@@@@@@@@@@@@@@@@@@@@@@@@@@@ Menus @@@@@@@@@@@@@@@@@@@@@@@@

Menu mainMenu = Menu(mainMenuItems);
Menu QuickstartMenu = Menu(QuickstartMenuItems);
Menu JobsMenu = Menu(JobsMenuItems);
Menu TestMenu = Menu(TestMenuItems);
Menu CalibrationMenu = Menu(CalibrationMenuItems);
Menu InformationMenu = Menu(InformationMenuItems);


