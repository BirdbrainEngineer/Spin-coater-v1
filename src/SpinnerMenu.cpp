#include <SpinnerMenu.h>

void* runProgrammed(char* caller){
    pidEnabled = true;
    return nullptr;
}

void* runAnalog(char* caller){
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

void* automaticCalibration(char* caller){
    return nullptr; //TODO
}

void* setKp(char* caller){
    float newKp;
    if(setUserVariable("Set Kp...", &newKp)){
        config.Kp = newKp;
        saveConfiguration(config);
    }
    return nullptr;
}
void* setKi(char* caller){
    float newKi;
    if(setUserVariable("Set Ki...", &newKi)){
        config.Ki = newKi;
        saveConfiguration(config);
    }
    return nullptr;
}
void* setKd(char* caller){
    float newKd;
    if(setUserVariable("Set Kd...", &newKd)){
        config.Kd = newKd;
        saveConfiguration(config);
    }
    return nullptr;
}
void* setAnalogAlpha(char* caller){
    float newAlpha;
    if(setUserVariable("Analog-in alpha...", &newAlpha)){
        config.analogAlpha = newAlpha;
        saveConfiguration(config);
    }
    return nullptr;
}
void* setRpmAlpha(char* caller){
    float newAlpha;
    if(setUserVariable("Set RPM alpha...", &newAlpha)){
        config.rpmAlpha = newAlpha;
        saveConfiguration(config);
    }
    return nullptr;
}

void* doNothing(char* caller){
    printlcdErrorMsg("Function not\navailable!");
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
    lcd.setCursor(14, 0);
    lcd.print(" |");
    lcd.setCursor(0, 1);
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

void* searchJobForRunning(char* caller){
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
    SpinnerJob* job;
    if(SD.exists("quick")){
        printlcd("Load previously\nmade quick-job?");
        if(askYesNo()){
            auto jobFile = SD.open("/quick");
            if(!jobFile){
                printlcdErrorMsg("File error!\n@runProg.Job-0");
                return nullptr;
            }
            job = loadJob(jobFile);
            jobFile.close();
            goto run;
        }
    }
    job = jobCreator(); 
    job->changeName(const_cast<char*>("quick"));
    if(SD.exists("quick")){ SD.remove("/quick"); }
    if(saveJob(job, "/")){ quickRunAvailable = true; }

    run:
    runJob(const_cast<char*>("quick"));
    if(askYesNo("Run job again?")){ goto run; }
    else{
        if(askYesNo("Save job\npermanently?")){
            TextBuffer* name = new TextBuffer(maxJobNameLength);
            panicIfOOM(name, "@runProg.Job-1");
            askForName:
            lcd.clear();
            lcd.print("New job name:");
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
                        jobTable->removeJob(jobIndex);
                        deleteJob(name->buffer);
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
            }
            else {
                delete name;
                return nullptr;
            }
        }
    }
    return nullptr;
}

void* accelerationTest(char* caller){ 
    const char animationGraphic[4] = {'|', '/', '-', '\\'};
    int animationIndex = 0;
    float results[10] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    int resultsIndex = 0;
    int indexTest = 0;
    float previousRPM = 0.0;
    float currentAcceleration = 0.0;
    SWtimer* timer = new SWtimer(200000, true);
    SWtimer* testTimer = new SWtimer(2000000, false);
    panicIfOOM(&timer, "@accelerationTest-0");
    panicIfOOM(&testTimer, "@accelerationTest-1");
    lcd.clear();
    lcd.print("Running test.. ");
    digitalWrite(spinner_power_enable_pin, HIGH);
    digitalWrite(spinner_running_led_pin, HIGH);
    timer->start();
    while(indexTest < 5){
        lcd.setCursor(0, 1);
        lcd.print(indexTest + 1);
        lcd.print("/5");
        testTimer->stop();
        if(resultsIndex % 2 == 0){ 
            testTimer->interval = 2000000; 
            currentRPM = 0.0; 
            motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 100.0);
        }
        else { 
            testTimer->interval = 4000000;
            motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 0.0);
        }
        testTimer->start();
        while(!testTimer->poll()){
            if(timer->poll()){
                if(keypad->pollState(KeyState::KEY_DOWN) > 0){
                    if(keypad->buffer[0].key == BACK){
                        digitalWrite(spinner_power_enable_pin, LOW);
                        motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 0.0);
                        lcd.clear();
                        lcd.print("Stopping...");
                        delete timer;
                        delete testTimer;
                        delay(2000);
                        printlcdErrorMsg("Test stopped\nearly by user.");
                        digitalWrite(spinner_running_led_pin, LOW);
                        return nullptr;
                    }
                }
                lcd.setCursor(15, 0);
                lcd.print(animationGraphic[animationIndex]);
                if(animationIndex < 3){ animationIndex++; }
                else { animationIndex = 0; }
                currentAcceleration = (currentRPM - previousRPM) / timer->previousTrueElapsedInterval * 0.000001;
                if(abs(currentAcceleration) > abs(results[resultsIndex])){
                    results[resultsIndex] = currentAcceleration;
                }
                previousRPM = currentRPM;
            }
        }
        if(resultsIndex % 2 == 1){ indexTest++; }
        resultsIndex++;
    }
    digitalWrite(spinner_power_enable_pin, LOW);
    delete timer;
    motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 0.0);
    float positiveAcceleration = 0.2 * (results[0] + results[2] + results[4] + results[6] + results[8]);
    float negativeAcceleration = 0.2 * (results[1] + results[3] + results[5] + results[7] + results[9]);
    lcd.clear();
    lcd.print("Pos.acceleration");
    lcd.setCursor(0, 1);
    lcd.print(positiveAcceleration, 2);
    lcd.print("rpm/s");
    keypad->pollStateBlocking(KeyState::KEY_DOWN);
    lcd.clear();
    lcd.print("Neg.acceleration");
    lcd.setCursor(0, 1);
    lcd.print(negativeAcceleration, 2);
    lcd.print("rpm/s");
    keypad->pollStateBlocking(KeyState::KEY_DOWN);
    digitalWrite(spinner_running_led_pin, LOW);
    return nullptr;
}

void* speedTest(char* caller){
    const char animationGraphic[4] = {'|', '/', '-', '\\'};
    int animationIndex = 0;
    float minStartSpeed;
    float maxSpeed;
    float minSpeed;
    float startDutyCycle;
    float previousDutyCycle;
    float previousRPM = 0.0;
    int testIndex = 0;
    currentRPM = 0.0;
    SWtimer* timer = new SWtimer(200000, true);
    panicIfOOM(&timer, "@speedTest-0");
    lcd.clear();
    motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 0.0);
    printlcd("Running test.. |\nmay take a while");
    digitalWrite(spinner_power_enable_pin, HIGH);
    digitalWrite(spinner_running_led_pin, HIGH);
    timer->start();
    while(testIndex < 3){
        if(timer->poll()){
            if(keypad->pollState(KeyState::KEY_DOWN) > 0){
                if(keypad->buffer[0].key == BACK){
                    digitalWrite(spinner_power_enable_pin, LOW);
                    motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 0.0);
                    lcd.clear();
                    lcd.print("Stopping...");
                    delete timer;
                    delay(2000);
                    printlcdErrorMsg("Test stopped\nearly by user.");
                    digitalWrite(spinner_running_led_pin, LOW);
                    return nullptr;
                }
            }
            lcd.setCursor(15, 0);
            lcd.print(animationGraphic[animationIndex]);
            if(animationIndex < 3){ animationIndex++; }
            else { animationIndex = 0; }
            switch(testIndex){
                case 0: {
                    if(currentRPM == 0.0){
                        startDutyCycle += 0.1;
                        motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, startDutyCycle);
                    }
                    else{
                        lcd.setCursor(15, 0);
                        lcd.print('?');
                        delay(1000);
                        minStartSpeed = currentRPM;
                        previousRPM = currentRPM;
                        previousDutyCycle = startDutyCycle;
                        timer->interval = 500000;
                        testIndex++;
                    }
                    break;
                }
                case 1: {
                    if(currentRPM != previousRPM){
                        previousDutyCycle -= 0.1;
                        motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, previousDutyCycle);
                    }
                    else{
                        minSpeed = previousRPM;
                        motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 100.0);
                        testIndex++;
                    }
                    break;
                }
                case 2: {
                    if(abs(currentRPM - previousRPM) > 10.0){
                        previousRPM = currentRPM;

                    }
                    else{
                        maxSpeed = max(currentRPM, previousRPM);
                        testIndex++;
                    }
                    break;
                }
                default: {
                    digitalWrite(spinner_power_enable_pin, LOW);
                    motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 0.0);
                    delete timer;
                    printlcdErrorMsg("Firmware error!\n@speedTest-1");
                    delay(2000);
                    digitalWrite(spinner_running_led_pin, LOW);
                    return nullptr;
                }
            }
        }
    }
    digitalWrite(spinner_power_enable_pin, LOW);
    motorDriver->setPWM(motor_pwm_pin, motorPWMFrequency, 0.0);
    delete timer;
    lcd.clear();
    lcd.print("Starting speed:");
    lcd.setCursor(0, 1);
    lcd.print(minStartSpeed, 2);
    lcd.print(" rpm");
    keypad->pollStateBlocking(KeyState::KEY_DOWN);
    lcd.clear();
    lcd.print("Minimum speed:");
    lcd.setCursor(0, 1);
    lcd.print(minSpeed, 2);
    lcd.print(" rpm");
    keypad->pollStateBlocking(KeyState::KEY_DOWN);
    lcd.clear();
    lcd.print("Maximum speed:");
    lcd.setCursor(0, 1);
    lcd.print(maxSpeed, 2);
    lcd.print(" rpm");
    keypad->pollStateBlocking(KeyState::KEY_DOWN);
    digitalWrite(spinner_running_led_pin, LOW);
    return nullptr;
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
    const char animationGraphic[4] = {'|', '/', '-', '\\'};
    int animationIndex = 0;
    float totalError = 0.0;
    float greatestOvershoot = 0.0;
    lcd.clear();
    lcd.print("Running test.. ");
    lcd.setCursor(11, 0);
    lcd.print(animationGraphic[0]);
    pidEnabled = true;
    pidTestJob->start();
    updateTimer->start();
    displayTimer->start();
    while(true){
        if(updateTimer->poll()){
            if(!pidTestJob->update()){
                pidTestJob->stop();
                lcd.clear();
                lcd.print("Finishing...");
                delete updateTimer;
                delete displayTimer;
                delay(2000);
                pidTestJob->reset();
                break;
            }
            else{
                if(rpmTarget < currentRPM){
                    greatestOvershoot = greatestOvershoot < (currentRPM - rpmTarget) ? (currentRPM - rpmTarget) : greatestOvershoot;
                }
                totalError += abs(currentRPM - rpmTarget) / (float)updateTimer->previousTrueElapsedInterval;
            }
        }
        if(displayTimer->poll()){
            if(keypad->pollState(KeyState::KEY_DOWN) > 0){
                if(keypad->buffer[0].key == BACK){
                    pidTestJob->stop();
                    lcd.clear();
                    lcd.print("Stopping...");
                    delete updateTimer;
                    delete displayTimer;
                    pidTestJob->reset();
                    delay(2000);
                    printlcdErrorMsg("Test stopped\nearly by user.");
                    return nullptr;
                }
            }
            lcd.setCursor(15, 0);
            lcd.print(animationGraphic[animationIndex]);
            if(animationIndex < 3){ animationIndex++; }
            else { animationIndex = 0; }
        }
    }
    delete updateTimer;
    delete displayTimer;
    lcd.clear();
    lcd.print("Total error:");
    lcd.setCursor(0, 1);
    lcd.print(totalError, 2);
    lcd.print(" rpm");
    keypad->pollStateBlocking(KeyState::KEY_DOWN);
    lcd.clear();
    lcd.print("Max. overshoot:");
    lcd.setCursor(0, 1);
    lcd.print(greatestOvershoot, 2);
    lcd.print(" rpm");
    keypad->pollStateBlocking(KeyState::KEY_DOWN);
    return nullptr;
}