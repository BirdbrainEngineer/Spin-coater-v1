#include <SpinnerMenu.h>

void* runProgrammed(){
    pidEnabled = true;
    return nullptr;
}

void* runAnalog(){
    lcd.clear();
    lcd.print("Analog control");
    lcd.setCursor(0, 1);
    lcd.print("RPM: ");
    SWtimer* timer = new SWtimer(500000, true);
    SWtimer* stopTimer = new SWtimer(2000000, false);
    panicIfOOM(timer, "@:01->timer");
    panicIfOOM(timer, "@:01->stopTimer");
    pidEnabled = false;
    enableMotor();
    timer->start();
    while(true){
        if(timer->poll()){
            lcd.setCursor(5, 1);
            lcd.print(currentRPM, 0);
            lcd.print("        ");
        }
        if(pollKeypadForSpecificCharPressed(BACK)){
            disableMotor();
            pidEnabled = true;
            lcd.setCursor(0, 0);
            lcd.print("Stopping...     ");
            stopTimer->start();
        }
        if(stopTimer->poll()){
            delay(500);
            delete timer;
            delete stopTimer;
            return nullptr;
        }
    }
}

void* automaticCalibration(){
    return nullptr;
}

void* setKp(){
    float newKp;
    if(setUserVariable("Set Kp...", &newKp)){
        config.Kp = newKp;
        saveConfiguration(config);
    }
    return nullptr;
}
void* setKi(){
    float newKi;
    if(setUserVariable("Set Ki...", &newKi)){
        config.Ki = newKi;
        saveConfiguration(config);
    }
    return nullptr;
}
void* setKd(){
    float newKd;
    if(setUserVariable("Set Kd...", &newKd)){
        config.Kd = newKd;
        saveConfiguration(config);
    }
    return nullptr;
}
void* setAnalogAlpha(){
    float newAlpha;
    if(setUserVariable("Analog-in alpha...", &newAlpha)){
        config.analogAlpha = newAlpha;
        saveConfiguration(config);
    }
    return nullptr;
}
void* setRpmAlpha(){
    float newAlpha;
    if(setUserVariable("Set RPM alpha...", &newAlpha)){
        config.rpmAlpha = newAlpha;
        saveConfiguration(config);
    }
    return nullptr;
}

void* doNothing(char* caller){
    return nullptr;
}

SpinnerJob* jobCreator(){ //DELICIOUS spaghetti!
      TextBuffer* input = new TextBuffer(8);
      panicIfOOM(input, "@jobCreation-0");
      SpinnerJob* job = new SpinnerJob(const_cast<char*>("temp"));
      panicIfOOM(job, "@jobCreation-1");
      SpinnerAction* sequence = (SpinnerAction*)malloc(sizeof(SpinnerAction) * UINT8_MAX);
      panicIfOOM(sequence, "@jobCreation-2");
      u8_t sequenceIndex = 0;
      u8_t queryIndex = 0;
      SpinnerTask task = RAMP;
      while(sequenceIndex < UINT8_MAX){
            lcd.clear();
            lcd.print(sequenceIndex);
            lcd.print(": ");
            switch(queryIndex){
                  case 0: {
                        lcd.print("Task?");
                        keypad->pollStateBlocking(KeyState::KEY_DOWN);
                        char key = keypad->buffer[0].key;
                        if(key == ENTER){
                              sequence[sequenceIndex].task = task;
                              sequenceIndex++;
                              if(task == END){ goto finalize; }
                              queryIndex = 1;
                        }
                        else if(key == BACK){
                              if(sequenceIndex > 0){
                                    sequenceIndex--;
                                    queryIndex = 2;
                              }
                              else {
                                    delete input;
                                    delete job;
                                    free(sequence);
                                    return nullptr;
                              }
                        }
                        else if(key == DOWN){
                              switch(task){
                                    case RAMP: task = END; break;
                                    case HOLD: task = RAMP; break;
                                    case END: task = HOLD; break;
                                    default: 
                                          printlcdErrorMsg("Firmware error!\n@jobCreation-3"); 
                                          task = RAMP; 
                                          break;
                              }
                        }
                        else{
                              switch(task){
                                    case RAMP: task = HOLD; break;
                                    case HOLD: task = END; break;
                                    case END: task = RAMP; break;
                                    default: 
                                          printlcdErrorMsg("Firmware error!\n@jobCreation-4"); 
                                          task = RAMP; 
                                          break;
                              }
                        }
                        break;
                  }
                  case 1: { 
                        lcd.print("RPM?");
                        if(getUserInput(input, true)){
                              sequence[sequenceIndex].rpm = atof(input->buffer);
                              queryIndex++;
                              input->clear();
                        }
                        else{
                              input->clear();
                              queryIndex--;
                        }
                        break;
                  }
                  case 2: { 
                        lcd.print("Duration?");
                        if(getUserInput(input, true)){
                              sequence[sequenceIndex].duration = (u32_t)atoi(input->buffer);
                              queryIndex = 0;
                              sequenceIndex++;
                              input->clear();
                        }
                        else{
                              input->clear();
                              queryIndex--;
                        }
                        break;
                  }
                  default: 
                        printlcdErrorMsg("Firmware error!\n@jobCreation-5");
                        queryIndex = 0;
                        break;
            }
      }
      sequence[UINT8_MAX].task = END;
      sequence[UINT8_MAX].rpm = 0;
      sequence[UINT8_MAX].duration = 0;
      finalize:
      job->init(sequenceIndex);
      for(int i = 0; i < sequenceIndex; i++){
            job->sequence[i].task = sequence[i].task;
            job->sequence[i].rpm = sequence[i].rpm;
            job->sequence[i].duration = sequence[i].duration;
      }
      auto jobConfig = Config{config.Kp, config.Ki, config.Kd, config.analogAlpha, config.rpmAlpha};
      if(askYesNo("Use current PID\nsettings?")){
            job->addConfig(jobConfig);
      }
      else {
            printlcd("Enter Kp:");
            input->clear();
            if(getUserInput(input, true)){ jobConfig.Kp = atof(input->buffer); }
            printlcd("Enter Ki:");
            input->clear();
            if(getUserInput(input, true)){ jobConfig.Ki = atof(input->buffer); }
            printlcd("Enter Kd:");
            input->clear();
            if(getUserInput(input, true)){ jobConfig.Kd = atof(input->buffer); }
            job->addConfig(jobConfig);
      }
      delete input;
      free(sequence);
      return job;
}

void* runJob(char* caller){
    int jobIndex = jobTable->exists(caller);
    if(jobIndex == -1){ 
        printlcdErrorMsg("Firmware error!\n@runJob-0");
        return nullptr;
    }
    SpinnerJob* job = jobTable->getJob(jobIndex);
    SWtimer* updateTimer = new SWtimer(1000, true);
    SWtimer* displayTimer = new SWtimer(200000, true);
    panicIfOOM(&updateTimer, "@runJob-1");
    panicIfOOM(&displayTimer, "@runJob-2");
    char startText[36] = "Run job:\n\"";
    strcat(startText, caller);
    strcat(startText, "\"?");
    if(!askYesNo(startText)){
        delete updateTimer;
        delete displayTimer;
        return nullptr;
    }
    lcd.clear();
    lcd.print(caller);
    lcd.setCursor(14, 0);
    lcd.print(" |");
    lcd.setCursor(0, 1);
    digitalWrite(spinner_running_led_pin, HIGH);
    const char animationGraphic[4] = {'|', '/', '-', '\\'};
    int animationIndex = 0;
    storedConfig.Kp = config.Kp;
    storedConfig.Ki = config.Ki;
    storedConfig.Kd = config.Kd;
    config.Kp = job->getConfig()->Kp;
    config.Ki = job->getConfig()->Ki;
    config.Kd = job->getConfig()->Kd;
    job->start();
    while(true){
        if(updateTimer->poll()){
            if(!job->update()){
                job->stop();
                lcd.clear();
                lcd.print("Finishing...");
                delete updateTimer;
                delete displayTimer;
                config.Kp = storedConfig.Kp;
                config.Ki = storedConfig.Ki;
                config.Kd = storedConfig.Kd;
                delay(2000);
                job->reset();
                digitalWrite(spinner_running_led_pin, LOW);
                return nullptr;
            }
        }
        if(displayTimer->poll()){
            if(keypad->pollState(KeyState::KEY_DOWN) > 0){
                if(keypad->buffer[0].key == BACK){
                    job->stop();
                    lcd.clear();
                    lcd.print("Stopping...");
                    delete updateTimer;
                    delete displayTimer;
                    config.Kp = storedConfig.Kp;
                    config.Ki = storedConfig.Ki;
                    config.Kd = storedConfig.Kd;
                    delay(2000);
                    job->reset();
                    digitalWrite(spinner_running_led_pin, LOW);
                    return nullptr;
                }
            }
            lcd.setCursor(15, 0);
            lcd.print(animationGraphic[animationIndex]);
            if(animationIndex < 3){ animationIndex++; }
            else { animationIndex = 0; }
            lcd.setCursor(0, 1);
            lcd.print(job->index + 1);
            lcd.print('/');
            lcd.print(job->sequenceLength);
            lcd.print(" RPM:");
            lcd.print(currentRPM, 0);
        }
    }
}

void* displayDeviceInformation(char* caller){
    int pointer = 0;
    lcd.clear();
    lcd.print(deviceInfo[0]);
    lcd.setCursor(0, 1);
    lcd.print(deviceInfo[1]);
    while(true){
        keypad->pollStateBlocking(KeyState::KEY_DOWN);
        if(keypad->buffer[0].key == BACK){
            return nullptr;
        }
        else if(keypad->buffer[0].key == UP){
            if(pointer > 0){ pointer--; }
        }
        else {
            if(pointer < DEVICEINFOLEN - 1) { pointer++; }
        }
        lcd.clear();
        lcd.print(deviceInfo[pointer]);
        lcd.setCursor(0, 1);
        lcd.print(deviceInfo[pointer + 1]);
    }
}

void* displayLicenseCC(char* caller){
    int pointer = 0;
    lcd.clear();
    lcd.print(licenseCC[0]);
    lcd.setCursor(0, 1);
    lcd.print(licenseCC[1]);
    while(true){
        keypad->pollStateBlocking(KeyState::KEY_DOWN);
        if(keypad->buffer[0].key == BACK){
            return nullptr;
        }
        else if(keypad->buffer[0].key == UP){
            if(pointer > 0){ pointer--; }
        }
        else {
            if(pointer < CCLICENSELEN - 1) { pointer++; }
        }
        lcd.clear();
        lcd.print(licenseCC[pointer]);
        lcd.setCursor(0, 1);
        lcd.print(licenseCC[pointer + 1]);
    }
}

void* displayLicenseMIT(char* caller){
    int pointer = 0;
    lcd.clear();
    lcd.print(licenseMIT[0]);
    lcd.setCursor(0, 1);
    lcd.print(licenseMIT[1]);
    while(true){
        keypad->pollStateBlocking(KeyState::KEY_DOWN);
        if(keypad->buffer[0].key == BACK){
            return nullptr;
        }
        else if(keypad->buffer[0].key == UP){
            if(pointer > 0){ pointer--; }
        }
        else {
            if(pointer < MITLICENSELEN - 1) { pointer++; }
        }
        lcd.clear();
        lcd.print(licenseMIT[pointer]);
        lcd.setCursor(0, 1);
        lcd.print(licenseMIT[pointer + 1]);
    }
}

void* searchJobForRunningJob(char* caller){
    TextBuffer* input = new TextBuffer(maxJobNameLength);
    lcd.clear();
    lcd.print("Job name:");
    if(getUserInput(input, false)){
        if(jobTable->exists(input->buffer)){
            runJob(input->buffer);
            delete input;
            return nullptr;
        }
        else{
            printlcdErrorMsg("No job with\nspecified name!");
            delete input;
            return nullptr;
        }
    }
    else {
        delete input;
        return nullptr;
    }
}

void* searchJobForDeletion(char* caller){
    TextBuffer* input = new TextBuffer(maxJobNameLength);
    lcd.clear();
    lcd.print("Job name:");
    if(getUserInput(input, false)){
        if(jobTable->exists(input->buffer)){
            deleteJob(input->buffer);
            delete input;
            return nullptr;
        }
        else{
            printlcdErrorMsg("No job with\nspecified name!");
            delete input;
            return nullptr;
        }
    }
    else {
        delete input;
        return nullptr;
    }
}

void* deleteJob(char* caller){
    int jobIndex = jobTable->exists(caller);
    if(jobIndex == -1){
        printlcdErrorMsg("Firmware error!\n@deleteJob-0");
        return nullptr;
    }
    char confirmationText[36] = "Really delete:\njob \"";
    strcat(confirmationText, caller);
    strcat(confirmationText, "\"?");
    if(!askYesNo(confirmationText)){
        return nullptr;
    }
    jobTable->removeJob(jobIndex);
    char jobPath[40] = "";
    strcat(jobPath, jobsPath);
    strcat(jobPath, caller);
    SD.remove(jobPath);
    lcd.clear();
    lcd.print("Job deleted!");
    return nullptr;
}

void* createJob(char* caller){
    bool overwrite = false;
    TextBuffer* name = new TextBuffer(maxJobNameLength);
    panicIfOOM(name, "@createJob-0");
    while(true){
        lcd.clear();
        lcd.print("New job name:");
        if(getUserInput(name, false)){
            if(jobTable->exists(name->buffer)){
                printlcdErrorMsg("Name already\ntaken!");
                lcd.clear();
                lcd.print("Try again: ");
                lcd.print(YES);
                lcd.setCursor(0, 1);
                lcd.print("Override: ");
                lcd.print(NO);
                if(askYesNo()){ continue; }
                else{
                    overwrite = true;
                    break;
                }
            }
            else{
                printlcd("Use name:\n\"");
                lcd.print(name->buffer);
                lcd.print("\"?");
                if(askYesNo()){ break; }
                else{ continue; }
            }
        }
        else {
            delete name;
            return nullptr;
        }
    }
    SpinnerJob* job = jobCreator();
    job->changeName(name->buffer);
    if(overwrite){
        jobTable->removeJob(jobTable->exists(name->buffer));
        char jobPath[40] = "";
        strcat(jobPath, jobsPath);
        strcat(jobPath, name->buffer);
        SD.remove(jobPath);
    }
    jobTable->addJob(job);
    if(!saveJob(job)){
        printlcdErrorMsg("Critical error!\n@createJob-1");
        printlcdErrorMsg("File corruption\npossible!");
    }
    else{
        printlcdErrorMsg("Job successfully\ncreated!");
    }
    delete name;
    return nullptr;
}

void* runProgrammedJob(char* caller){
    SpinnerJob* job;
    char jobPath[40] = "";
    auto jobIndex = jobTable->exists(const_cast<char*>("quick"));
    if(jobIndex >= 0){
        printlcd("Load previous\nquick-job?");
        if(askYesNo()){ 
            job = jobTable->getJob(jobIndex); 
            goto skipJobCreation;
        }
    }
    job = jobCreator(); 
    job->changeName(const_cast<char*>("quick"));
    strcat(jobPath, jobsPath);
    strcat(jobPath, "quick");
    SD.remove(jobPath);
    jobTable->addJob(job);
    saveJob(job);

    skipJobCreation:
    runJob(const_cast<char*>("quick"));
    if(askYesNo("Run job again?")){ goto skipJobCreation; }
    return nullptr;
}