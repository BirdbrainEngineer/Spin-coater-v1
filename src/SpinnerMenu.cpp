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

SpinnerJob* jobCreation(){ //DELICIOUS spaghetti!
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
            getUserInput(input, true);
            jobConfig.Kp = atof(input->buffer);
            printlcd("Enter Ki:");
            input->clear();
            getUserInput(input, true);
            jobConfig.Ki = atof(input->buffer);
            printlcd("Enter Kd:");
            input->clear();
            getUserInput(input, true);
            jobConfig.Kd = atof(input->buffer);
            job->addConfig(jobConfig);
      }
      delete input;
      free(sequence);
      return job;
}