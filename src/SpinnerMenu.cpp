#include <SpinnerMenu.h>

void* runAnalog(char* caller){
    lcd.clear();
    lcd.print("Analog control");
    lcd.setCursor(0, 1);
    lcd.print("RPM: ");
    SWtimer* timer = new SWtimer(500000, true);
    panicIfOOM(timer, "@runAnalog-0");
    panicIfOOM(timer, "@runAnalog-1");
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
            stopTest("Exiting analog\ncontrol mode.");
            disableMotor();
            delete timer;
            return nullptr;
        }
    }
}

void* setKp(char* caller){
    lcd.clear();
    lcd.print("Set Kp (");
    lcd.print(config.Kp, 4);
    lcd.print(")");
    TextBuffer* input = new TextBuffer(8);
    panicIfOOM(input, "@setKp-0");
    if(getUserInput(input, true)){
        config.Kp = atof(input->buffer);
        saveConfiguration(config);
        pidController->setPID(config.Kp, config.Ki, config.Kd);
    }
    return nullptr;
}
void* setKi(char* caller){
    lcd.clear();
    lcd.print("Set Ki (");
    lcd.print(config.Ki, 4);
    lcd.print(")");
    TextBuffer* input = new TextBuffer(8);
    panicIfOOM(input, "@setKi-0");
    if(getUserInput(input, true)){
        config.Ki = atof(input->buffer);
        saveConfiguration(config);
        pidController->setPID(config.Kp, config.Ki, config.Kd);
    }
    return nullptr;
}
void* setKd(char* caller){
    lcd.clear();
    lcd.print("Set Kd (");
    lcd.print(config.Kd, 4);
    lcd.print(")");
    TextBuffer* input = new TextBuffer(8);
    panicIfOOM(input, "@setKd-0");
    if(getUserInput(input, true)){
        config.Kd = atof(input->buffer);
        saveConfiguration(config);
        pidController->setPID(config.Kp, config.Ki, config.Kd);
    }
    return nullptr;
}
void* setAnalogAlpha(char* caller){
    lcd.clear();
    lcd.print("Set Kp (");
    lcd.print(config.analogAlpha, 4);
    lcd.print(")");
    TextBuffer* input = new TextBuffer(8);
    panicIfOOM(input, "@setAnalogAlpha-0");
    if(getUserInput(input, true)){
        config.analogAlpha = atof(input->buffer);
        saveConfiguration(config);
    }
    return nullptr;
}
void* setRpmAlpha(char* caller){
    lcd.clear();
    lcd.print("Set Kp (");
    lcd.print(config.rpmAlpha, 4);
    lcd.print(")");
    TextBuffer* input = new TextBuffer(8);
    panicIfOOM(input, "@setKp-0");
    if(getUserInput(input, true)){
        config.rpmAlpha = atof(input->buffer);
        saveConfiguration(config);
    }
    return nullptr;
}

void* updateMinDutyCycle(char* caller){
    saveConfiguration(config);
    pidController->setOutputRange(config.minDutyCycle, 100.0);
    printlcdErrorMsg("Minimum duty\ncycle updated!");
    return nullptr;
}

void* doNothing(char* caller){
    printlcdErrorMsg("Functionality\nnot available!");
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
            lcd.print(sequenceIndex + 1);
            lcd.print(": ");
            switch(queryIndex){
                  case 0: {
                        lcd.print("Task?");
                        lcd.setCursor(0, 1);
                        lcd.print(task == RAMP ? "Ramp" : task == HOLD ? "Hold" : "End");
                        keypad->pollStateBlocking(KeyState::KEY_DOWN);
                        char key = keypad->buffer[0].key;
                        if(key == ENTER){
                              sequence[sequenceIndex].task = task;
                              if(task == END){ 
                                    sequence[sequenceIndex].rpm = 1;
                                    sequence[sequenceIndex].duration = 1;
                                    sequenceIndex++;
                                    goto finalize;
                              }
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
      auto jobConfig = Config{config.Kp, config.Ki, config.Kd};
      job->addConfig(jobConfig);
      if(askYesNo("Embed current\nPID settings?")){
         job->useEmbeddedPIDConstants = true;   
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
    SWtimer* updateTimer = new SWtimer(coreLoopInterval, true);
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
    lcd.setCursor(15, 0);
    lcd.print("|");
    lcd.setCursor(0, 1);
    const char animationGraphic[4] = {'|', '/', '-', 0x01};
    int animationIndex = 0;
    storedConfig.Kp = config.Kp;
    storedConfig.Ki = config.Ki;
    storedConfig.Kd = config.Kd;
    config.Kp = job->getConfig()->Kp;
    config.Ki = job->getConfig()->Ki;
    config.Kd = job->getConfig()->Kd;
    if(job->useEmbeddedPIDConstants){ pidController->setPID(config.Kp, config.Ki, config.Kd); }
    job->start();
    displayTimer->start();
    updateTimer->start();
    while(true){
        if(updateTimer->poll()){
            if(!job->update()){
                job->stop();
                delete updateTimer;
                delete displayTimer;
                config.Kp = storedConfig.Kp;
                config.Ki = storedConfig.Ki;
                config.Kd = storedConfig.Kd;
                if(job->useEmbeddedPIDConstants){ pidController->setPID(config.Kp, config.Ki, config.Kd); }
                job->reset();
                return nullptr;
            }
            rpmTarget = job->currentTargetRpm;
        }
        if(displayTimer->poll()){
            if(keypad->pollState(KeyState::KEY_DOWN) > 0){
                if(keypad->buffer[0].key == BACK){
                    stopTest("Job stopped by\nthe user!");
                    job->stop();
                    delete updateTimer;
                    delete displayTimer;
                    config.Kp = storedConfig.Kp;
                    config.Ki = storedConfig.Ki;
                    config.Kd = storedConfig.Kd;
                    if(job->useEmbeddedPIDConstants){ pidController->setPID(config.Kp, config.Ki, config.Kd); }
                    job->reset();
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
            lcd.print(job->sequenceLength - 1);
            lcd.print(" RPM:");
            lcd.print(currentRPM, 0);
            lcd.print("     ");
        }
    }
}

bool runJob(SpinnerJob* job){
    SWtimer* updateTimer = new SWtimer(coreLoopInterval, true);
    SWtimer* displayTimer = new SWtimer(200000, true);
    panicIfOOM(&updateTimer, "@runJob-1");
    panicIfOOM(&displayTimer, "@runJob-2");
    char startText[36] = "Run job:\n\"";
    strcat(startText, job->name);
    strcat(startText, "\"?");
    if(!askYesNo(startText)){
        delete updateTimer;
        delete displayTimer;
        return false;
    }
    lcd.clear();
    lcd.print(job->name);
    lcd.setCursor(15, 0);
    lcd.print("|");
    lcd.setCursor(0, 1);
    const char animationGraphic[4] = {'|', '/', '-', 0x01};
    int animationIndex = 0;
    storedConfig.Kp = config.Kp;
    storedConfig.Ki = config.Ki;
    storedConfig.Kd = config.Kd;
    config.Kp = job->getConfig()->Kp;
    config.Ki = job->getConfig()->Ki;
    config.Kd = job->getConfig()->Kd;
    job->start();
    displayTimer->start();
    updateTimer->start();
    while(true){
        if(updateTimer->poll()){
            if(!job->update()){
                job->stop();
                disableMotor();
                delete updateTimer;
                delete displayTimer;
                config.Kp = storedConfig.Kp;
                config.Ki = storedConfig.Ki;
                config.Kd = storedConfig.Kd;
                job->reset();
                return true;
            }
            rpmTarget = job->currentTargetRpm;
        }
        if(displayTimer->poll()){
            if(keypad->pollState(KeyState::KEY_DOWN) > 0){
                if(keypad->buffer[0].key == BACK){
                    stopTest("Job stopped by\nthe user!");
                    job->stop();
                    delete updateTimer;
                    delete displayTimer;
                    config.Kp = storedConfig.Kp;
                    config.Ki = storedConfig.Ki;
                    config.Kd = storedConfig.Kd;
                    job->reset();
                    return false;
                }
            }
            lcd.setCursor(15, 0);
            lcd.print(animationGraphic[animationIndex]);
            if(animationIndex < 3){ animationIndex++; }
            else { animationIndex = 0; }
            lcd.setCursor(0, 1);
            lcd.print(job->index + 1);
            lcd.print('/');
            lcd.print(job->sequenceLength - 1);
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
            if(pointer < DEVICEINFOLEN - 2) { pointer++; }
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
            if(pointer < CCLICENSELEN - 2) { pointer++; }
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
            if(pointer < MITLICENSELEN - 2) { pointer++; }
        }
        lcd.clear();
        lcd.print(licenseMIT[pointer]);
        lcd.setCursor(0, 1);
        lcd.print(licenseMIT[pointer + 1]);
    }
}

void* searchJobForRunning(char* caller){
    TextBuffer* input = new TextBuffer(maxJobNameLength);
    panicIfOOM(input, "@searchJobRun-0");
    lcd.clear();
    lcd.print("Job name:");
    if(getUserInput(input, false)){
        if(jobTable->exists(input->buffer) >= 0){
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
    printlcdErrorMsg("Job deleted!");
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
            if(jobTable->exists(name->buffer) >= 0){
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
                }
            }
            printlcd("Use name:\n\"");
            lcd.print(name->buffer);
            lcd.print("\"?");
            if(askYesNo()){ break; }
            else{ 
                overwrite = false;
                name->clear();
                continue; 
            }
        }
        else {
            delete name;
            return nullptr;
        }
    }
    SpinnerJob* job = jobCreator();
    if(job == nullptr){
        delete name;
        return nullptr;
    }
    job->changeName(name->buffer);
    if(!askYesNo("Save the\ncreated job?")){
        delete name;
        delete job;
        return nullptr;
    }
    if(overwrite){
        jobTable->removeJob(jobTable->exists(name->buffer));
        char jobPath[40] = "";
        strcat(jobPath, jobsPath);
        strcat(jobPath, name->buffer);
        SD.remove(jobPath);
    }
    jobTable->addJob(job);
    if(!saveJob(job, jobsPath)){
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
    SpinnerJob* job = jobCreator();
    if(job == nullptr){ return nullptr; }
    job->changeName(const_cast<char*>("quick"));
    if(memoryGood){
        if(SD.exists("/quick")){ SD.remove("/quick"); }
        saveJob(job, "/");
    }

    do{
        if(!runJob(job)){ return nullptr; }
    } while(askYesNo("Run job again?"));
    
    if(memoryGood && askYesNo("Save job\npermanently?")){
        TextBuffer* name = new TextBuffer(maxJobNameLength);
        panicIfOOM(name, "@runProg.Job-2");

        askForName:
        lcd.clear();
        lcd.print("New job name:");
        name->clear();
        if(getUserInput(name, false)){
            int jobIndex = jobTable->exists(name->buffer);
            if(jobIndex >= 0){
                printlcdErrorMsg("Name already\ntaken!");
                lcd.clear();
                lcd.print("Try again: ");
                lcd.print(YES);
                lcd.setCursor(0, 1);
                lcd.print("Override: ");
                lcd.print(NO);
                if(askYesNo()){ goto askForName; }
                else{
                    deleteJob(name->buffer);
                    if(!jobTable->removeJob(jobIndex)){
                        printlcdErrorMsg("Firmware error\n@runProg.Job-3");
                    }
                }
            }
            else{
                printlcd("Use name:\n\"");
                lcd.print(name->buffer);
                lcd.print("\"?");
                if(!askYesNo()){ goto askForName; }
            }
            job->changeName(name->buffer);
            jobTable->addJob(job);
            saveJob(job, jobsPath);
            delete name;
            return nullptr;
        }
        else {
            delete name;
            delete job;
            return nullptr;
        }
    }
    delete job;
    return nullptr;
}

void stopTest(const char* text){
    float previousRPM = 0.0;
    int animation = 8;
    motorEnabled = false;
    motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 100.0 - config.minDutyCycle);
    lcd.clear();
    lcd.print("Stopping");
    while(abs(currentRPM - previousRPM) > 10.0){
        previousRPM = currentRPM;
        animation++;
        lcd.print(".");
        if(animation == 16){lcd.setCursor(0, 1); }
        delay(200);
    }
    digitalWrite(spinner_power_enable_pin, LOW);
    motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 100.0);
    printlcdErrorMsg(text);
    digitalWrite(spinner_running_led_pin, LOW);
}

void* motorTest(char* caller){
    struct Results{
        float maxSpeed;
        float minSpeed;
        float acceleration;
        float deceleration;
    };
    Results results = {0.0, 0.0, 0.0, 0.0};
    digitalWrite(spinner_power_enable_pin, HIGH);
    motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 100.0);
    digitalWrite(spinner_running_led_pin, HIGH);
    if(!speedTest(&results.maxSpeed, &results.minSpeed)){ 
        digitalWrite(spinner_running_led_pin, LOW);
        return nullptr; 
    }
    if(!accelerationTest(&results.acceleration, &results.deceleration, results.maxSpeed, results.minSpeed)){ 
        digitalWrite(spinner_running_led_pin, LOW);
        return nullptr; 
    }
    motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 100.0);
    digitalWrite(spinner_power_enable_pin, LOW);
    
    lcd.clear();
    printlcd("Max speed:\n");
    lcd.print(results.maxSpeed);
    lcd.print(" rpm");
    keypad->pollStateBlocking(KeyState::KEY_DOWN);
    
    lcd.clear();
    printlcd("Min speed:\n");
    lcd.print(results.minSpeed);
    lcd.print(" rpm");
    keypad->pollStateBlocking(KeyState::KEY_DOWN);

    lcd.clear();
    printlcd("Acceleration\n");
    lcd.print(results.acceleration);
    lcd.print(" rpm/s");
    keypad->pollStateBlocking(KeyState::KEY_DOWN);

    lcd.clear();
    printlcd("Deceleration\n");
    lcd.print(results.deceleration);
    lcd.print(" rpm/s");
    keypad->pollStateBlocking(KeyState::KEY_DOWN);

    digitalWrite(spinner_running_led_pin, LOW);
    currentRPM = 0.0;
    return nullptr;
}

bool speedTest(float* maxSpeed, float* minSpeed){
    const char animationGraphic[4] = {'|', '/', '-', 0x01};
    int animationIndex = 0;
    float minDutyCycle = 0.0;
    float dutyCycle = 0.0;
    float previousRPM = 0.0;
    int testIndex = 0;
    currentRPM = 0.0;
    SWtimer* timer = new SWtimer(200000, true);
    panicIfOOM(&timer, "@speedTest-0");
    printlcd("Speed test...   \n1/2");
    timer->start();
    while(testIndex < 2){
        if(timer->poll()){
            if(keypad->pollState(KeyState::KEY_DOWN)){
                if(keypad->buffer[0].key == BACK){
                    delete timer;
                    stopTest("Test stopped\nearly by user.");
                    return false;
                }
            }
            switch(testIndex){
                case 0:{
                    if(previousRPM == currentRPM){
                        dutyCycle += 0.1;
                        motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 100.0 - dutyCycle);
                    }
                    else {
                        lcd.setCursor(15, 0);
                        lcd.print('?');
                        timer->pause();
                        delay(2000);
                        *minSpeed = currentRPM;
                        minDutyCycle = dutyCycle + 0.1;
                        motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 0.0);
                        testIndex++;
                        lcd.setCursor(0, 1);
                        lcd.print("2/2");
                        delay(500);
                        timer->resume();
                    }
                    break;
                }
                case 1:{
                    if((abs(currentRPM - previousRPM) * 5.0) < 20.0){
                        *maxSpeed = currentRPM;
                        testIndex++;
                    }
                    break;
                }
                default:{
                    delete timer;
                    stopTest("Firmware error\n@speedTest-1");
                    return false;
                }
            }
            previousRPM = currentRPM;

            lcd.setCursor(15, 0);
            lcd.print(animationGraphic[animationIndex]);
            animationIndex++;
            if(animationIndex == 4){ animationIndex = 0; }
        }
    }
    motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 100.0 - minDutyCycle);
    config.minDutyCycle = minDutyCycle;
    delete timer;
    lcd.clear();
    printlcd("Finishing speed\ntest...");
    while(currentRPM > *minSpeed * 2.0);
    motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 100.0);
    delay(500);
    return true;
}

bool accelerationTest(float* acceleration, float* deceleration, float maxSpeed, float minSpeed){
    const int numTests = 6;
    const char animationGraphic[8] = {'|', '|', '/', '/', '-', '-', 0x01, 0x01};
    int animationIndex = 0;
    float previousRPM = 0.0;
    int testIndex = 0;
    int numSamples = 0;
    float currentAcceleration = 0.0;
    float results[numTests];
    for(int i = 0; i < numTests; i++){ results[i] = 0.0; }
    SWtimer* timer = new SWtimer(100000, true);
    panicIfOOM(&timer, "@accel.Test-0");
    printlcd("Accel. test...  \n1/");
    lcd.print(numTests);
    timer->start();
    while(testIndex < numTests){
        if(timer->poll()){
            if(keypad->pollState(KeyState::KEY_DOWN)){
                if(keypad->buffer[0].key == BACK){
                    delete timer;
                    stopTest("Test stopped\nearly by user.");
                    return false;
                }
            }
            currentAcceleration = (currentRPM - previousRPM) * 10.0;
            if(testIndex % 2 == 0){
                motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 0.0);
                if(currentRPM < (maxSpeed * 0.9)){
                    results[testIndex] += currentAcceleration;
                    numSamples++;
                }
                else{
                    results[testIndex] = results[testIndex] / (float)numSamples;
                    numSamples = 0;
                    testIndex++;
                    lcd.setCursor(0, 1);
                    lcd.print(testIndex + 1);
                }
                previousRPM = currentRPM;
            }
            else{
                float dutyCycle = config.minDutyCycle;
                motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 100.0 - (dutyCycle * 2.0));
                if(currentRPM > (minSpeed * 3.0)){
                    results[testIndex] += currentAcceleration;
                    numSamples++;
                }
                else{
                    results[testIndex] = results[testIndex] / (float)numSamples;
                    numSamples = 0;
                    testIndex++;
                    lcd.setCursor(0, 1);
                    lcd.print(testIndex + 1);
                }
                previousRPM = currentRPM;
            }
            previousRPM = currentRPM;
            lcd.setCursor(15, 0);
            lcd.print(animationGraphic[animationIndex]);
            animationIndex++;
            if(animationIndex == 8){ animationIndex = 0; }
        }
    }
    motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 100.0);
    printlcd("Acceleration\ntest finishing..");
    delete timer;
    lcd.clear();
    *acceleration = 0.0;
    *deceleration = 0.0;
    for(int i = 0; i < numTests; i++){
        if(i % 2 == 0){
            *acceleration += results[i];
        }
        else{
            *deceleration += results[i];
        }
    }
    *acceleration = *acceleration / (float)(numTests / 2);
    *deceleration = *deceleration / (float)(numTests / 2);
    return true;
}

void* pidTest(char* caller){
    if(pidTestJob == nullptr){
        printlcdErrorMsg("Firmware error!\n@pidTest-0");
        return nullptr;
    }
    SWtimer* updateTimer = new SWtimer(coreLoopInterval, true);
    SWtimer* displayTimer = new SWtimer(200000, true);
    panicIfOOM(&updateTimer, "@pidTest-1");
    panicIfOOM(&displayTimer, "@pidTest-2");
    const char animationGraphic[4] = {'|', '/', '-', 0x01};
    int animationIndex = 0;
    float totalError = 0.0;
    double absoluteError = 0.0;
    int samples = 0;
    pidEnabled = true;
    pidTestJob->start();
    updateTimer->start();
    displayTimer->start();
    while(true){
        if(updateTimer->poll()){
            if(!pidTestJob->update()){
                pidTestJob->stop();
                lcd.clear();
                delete updateTimer;
                delete displayTimer;
                pidTestJob->reset();
                break;
            }
            else{
                float newError = abs(currentRPM - rpmTarget);
                totalError += newError * (updateTimer->interval / 1000000.0);
                absoluteError += newError;
                samples++;
            }
            rpmTarget = pidTestJob->currentTargetRpm;
        }
        if(displayTimer->poll()){
            if(keypad->pollState(KeyState::KEY_DOWN) > 0){
                if(keypad->buffer[0].key == BACK){
                    pidTestJob->stop();
                    delete updateTimer;
                    delete displayTimer;
                    pidTestJob->reset();
                    stopTest("Test stopped\nearly by user.");
                    return nullptr;
                }
            }
            lcd.clear();
            lcd.print("PID test ");
            lcd.print(pidTestJob->index + 1);
            lcd.print('/');
            lcd.print(pidTestJob->sequenceLength - 1);
            lcd.setCursor(15, 0);
            lcd.print(animationGraphic[animationIndex]);
            if(animationIndex < 3){ animationIndex++; }
            else { animationIndex = 0; }
            lcd.setCursor(0, 1);
            lcd.print(pidTestJob->currentTargetRpm, 0);
            lcd.setCursor(5, 1);
            lcd.print((char)(0x02));
            lcd.print(currentRPM, 0);
            lcd.setCursor(11, 1);
            lcd.print((char)(0x03));
            lcd.print(pidTestJob->currentTargetRpm - currentRPM, 0);
        }
    }
    delete updateTimer;
    delete displayTimer;
    printlcd("Total error:\n");
    lcd.print((int)totalError);
    lcd.print(" rpms");
    keypad->pollStateBlocking(KeyState::KEY_DOWN);
    printlcd("Average error:\n");
    lcd.print(absoluteError / (float)samples, 2);
    lcd.print(" rpm");
    keypad->pollStateBlocking(KeyState::KEY_DOWN);
    return nullptr;
}