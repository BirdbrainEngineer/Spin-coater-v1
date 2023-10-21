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
      {cstr("Load job"), MENU, runJobsMenuConstructor},
      {cstr("Create job"), FUNC, createJob},
      {cstr("Delete job"), MENU, deleteJobMenuConstructor},
};

MenuItem TestMenuItems[3] = {
      {cstr("Acceleration"), FUNC, accelerationTest},
      {cstr("Speed"), FUNC, speedTest},
      {cstr("PID"), FUNC, pidTestAvailable ? pidTest : doNothing},
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

MenuItem emptyMenuItems[1] = {
      {cstr("Nothing here!"), FUNC, doNothing},
};

//@@@@@@@@@@@@@@@@@@@@@@@@@@@ Static Menus @@@@@@@@@@@@@@@@@@@@@@@@
Menu mainMenu = Menu(mainMenuItems);
Menu quickStartMenu = Menu(QuickstartMenuItems);
Menu jobsMenu = Menu(JobsMenuItems);
Menu testMenu = Menu(TestMenuItems);
Menu calibrationMenu = Menu(CalibrationMenuItems);
Menu informationMenu = Menu(InformationMenuItems);

Menu emptyMenu = Menu(emptyMenuItems);


//@@@@@@@@@@@@@@@@@@@@@@@@@@@ Static menu constructors @@@@@@@@@@@@@@@@@@@@@@@@

Menu* mainMenuConstructor() { return &mainMenu; }
void* quickStartMenuConstructor(char* caller){ return &quickStartMenu; }
void* jobsMenuConstructor(char* caller) { return &jobsMenu; }
void* testMenuConstructor(char* caller) { return &testMenu; }
void* calibrationMenuConstructor(char* caller) { return &calibrationMenu; }
void* informationMenuConstructor(char* caller) { return &informationMenu; }


//@@@@@@@@@@@@@@@@@@@@@@@@@@@ Dynamic menu constructors @@@@@@@@@@@@@@@@@@@@@@@@


void* runJobsMenuConstructor(char* caller){return new Menu(runJobsMenuForge); }
void* deleteJobMenuConstructor(char* caller){return new Menu(deleteJobsMenuForge); }

MenuData runJobsMenuForge(){
      MenuData data;
      if(jobTable->numJobs == 0){ 
            data.size = 1;
            data.items = new MenuItem{cstr("No jobs!"), FUNC, doNothing};
            return data;
      }
      data.size = jobTable->numJobs + 1;
      MenuItem* items = (MenuItem*)malloc(sizeof(MenuItem) * data.size);
      items[0].name = const_cast<char*>("Search for a job");
      items[0].type = FUNC;
      items[0].call = searchJobForRunning;
      for(int i = 0; i < jobTable->numJobs; i++){
            items[i + 1].name = jobTable->getJob(i)->name;
            items[i + 1].type = FUNC;
            items[i + 1].call = runJob;
      }
      return data;
}

MenuData deleteJobsMenuForge(){
      MenuData data;
      if(jobTable->numJobs == 0){ 
            data.size = 1;
            data.items = new MenuItem{cstr("No jobs!"), FUNC, doNothing};
            return data;
      }
      data.size = jobTable->numJobs + 1;
      MenuItem* items = (MenuItem*)malloc(sizeof(MenuItem) * data.size);
      items[0].name = const_cast<char*>("Search for a job");
      items[0].type = FUNC;
      items[0].call = searchJobForDeletion;
      for(int i = 0; i < jobTable->numJobs; i++){
            items[i + 1].name = jobTable->getJob(i)->name;
            items[i + 1].type = FUNC;
            items[i + 1].call = deleteJob;
      }
      return data;
}