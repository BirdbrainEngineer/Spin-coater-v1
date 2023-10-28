#include <main.h>
#include <SpinnerMenuItems.h>
#include <SpinnerMenu.h>

#define cstr(x) const_cast<char*>(x)

const u8_t mainMenuSize = 5;
MenuItem mainMenuItems[mainMenuSize] = {
    {cstr("Quick start"), MENU, quickStartMenuConstructor},
    {cstr("Jobs"), MENU, jobsMenuConstructor},
    {cstr("Test"), MENU, testMenuConstructor},
    {cstr("Calibration"), MENU, calibrationMenuConstructor},
    {cstr("Information"), MENU, informationMenuConstructor},
};
const u8_t quickstartMenuSize = 2;
MenuItem QuickstartMenuItems[quickstartMenuSize] = {
      {cstr("Programmed"), FUNC, runProgrammedJob},
      {cstr("Analog"), FUNC, runAnalog},
};
const u8_t jobsMenuSize = 3;
MenuItem JobsMenuItems[jobsMenuSize] = {
      {cstr("Load job"), MENU, runJobsMenuConstructor},
      {cstr("Create job"), FUNC, createJob},
      {cstr("Delete job"), MENU, deleteJobMenuConstructor},
};
const u8_t testMenuSize = 2;
MenuItem TestMenuItems[testMenuSize] = {
      {cstr("Spinner params."), FUNC, motorTest},
      {cstr("PID"), FUNC, pidTestConstructor},
};
const u8_t calibrationMenuSize = 6;
MenuItem CalibrationMenuItems[calibrationMenuSize] = {
      {cstr("Set Kp"), FUNC, setKp},
      {cstr("Set Ki"), FUNC, setKi},
      {cstr("Set Kd"), FUNC, setKd},
      {cstr("Min duty cycle"), FUNC, updateMinDutyCycle},
      {cstr("Analog-in alpha"), FUNC, setAnalogAlpha},
      {cstr("RPM alpha"), FUNC, setRpmAlpha},
};
const u8_t informationMenuSize = 3;
MenuItem InformationMenuItems[informationMenuSize] = {
      {cstr("Device info."), FUNC, displayDeviceInformation},
      {cstr("HARDWARE LICENSE"), FUNC, displayLicenseCC},
      {cstr("SOFTWARE LICENSE"), FUNC, displayLicenseMIT},
};
const u8_t emptyMenuSize = 1;
MenuItem emptyMenuItems[emptyMenuSize] = {
      {cstr("Nothing here!"), FUNC, doNothing},
};

//@@@@@@@@@@@@@@@@@@@@@@@@@@@ Static Menus @@@@@@@@@@@@@@@@@@@@@@@@
Menu mainMenu = Menu(MenuData{mainMenuSize, mainMenuItems});
Menu quickStartMenu = Menu(MenuData{quickstartMenuSize, QuickstartMenuItems});
Menu jobsMenu = Menu(MenuData{jobsMenuSize, JobsMenuItems});
Menu testMenu = Menu(MenuData{testMenuSize, TestMenuItems});
Menu calibrationMenu = Menu(MenuData{calibrationMenuSize, CalibrationMenuItems});
Menu informationMenu = Menu(MenuData{informationMenuSize, InformationMenuItems});
Menu emptyMenu = Menu(MenuData{emptyMenuSize, emptyMenuItems});


//@@@@@@@@@@@@@@@@@@@@@@@@@@@ Static menu constructors @@@@@@@@@@@@@@@@@@@@@@@@

Menu* mainMenuConstructor() { return &mainMenu; }
void* quickStartMenuConstructor(char* caller){ return &quickStartMenu; }
void* jobsMenuConstructor(char* caller) { return memoryGood ? &jobsMenu : &emptyMenu; }
void* testMenuConstructor(char* caller) { return &testMenu; }
void* calibrationMenuConstructor(char* caller) { return &calibrationMenu; }
void* informationMenuConstructor(char* caller) { return &informationMenu; }

void* pidTestConstructor(char* caller){ return (pidTestAvailable && memoryGood) ? pidTest(caller) : doNothing(caller); }


//@@@@@@@@@@@@@@@@@@@@@@@@@@@ Dynamic menu constructors @@@@@@@@@@@@@@@@@@@@@@@@


void* runJobsMenuConstructor(char* caller){ return new Menu(runJobsMenuForge); }
void* deleteJobMenuConstructor(char* caller){ return new Menu(deleteJobsMenuForge); }

MenuData* runJobsMenuForge(){
      if(jobTable->numJobs == 0){ 
            MenuItem* newItems = (MenuItem*)malloc(sizeof(MenuItem) * emptyMenuSize);
            memcpy(newItems, emptyMenuItems, sizeof(MenuItem) * emptyMenuSize);
            return new MenuData{emptyMenuSize, newItems};
      }
      MenuData* data = new MenuData;
      panicIfOOM(data, "@runJobsForge-0");
      data->size = jobTable->numJobs + 1;
      MenuItem* items = (MenuItem*)malloc(sizeof(MenuItem) * data->size);
      panicIfOOM(items, "@runJobsForge-1");
      data->items = items;
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

MenuData* deleteJobsMenuForge(){
      if(jobTable->numJobs == 0){ 
            MenuItem* newItems = (MenuItem*)malloc(sizeof(MenuItem) * emptyMenuSize);
            memcpy(newItems, emptyMenuItems, sizeof(MenuItem) * emptyMenuSize);
            return new MenuData{emptyMenuSize, newItems};
      }
      MenuData* data = new MenuData;
      panicIfOOM(data, "@deleteForge-0");
      data->size = jobTable->numJobs + 1;
      MenuItem* items = (MenuItem*)malloc(sizeof(MenuItem) * data->size);
      panicIfOOM(items, "@deleteForge-1");
      data->items = items;
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