#include <SpinnerMenu.h>

void* jobsMenuConstructor(void* (*callFunc)()){
    
}

void* runProgrammed(){
    pidEnabled = true;
    return nullptr;
}

void* runAnalog(){
    lcd.clear();
    lcd.print("Analog control");
    SWtimer* timer = new SWtimer(500000, true);
    SWtimer* stopTimer = new SWtimer(2000000, false);
    pidEnabled = false;
    enableMotor();
    timer->start();
    while(true){
        if(timer->poll()){
            lcd.setCursor(0, 1);
            lcd.print("RPM: ");
            lcd.print(currentRPM);
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
            return nullptr;
        }
    }
}

void* automaticCalibration(){
    return nullptr;
}

void* setKp(){
    return nullptr;
}

void* setKi(){
    return nullptr;
}

void* setKd(){
    return nullptr;
}

void* setUserVariable(char* displayText, float* variable){
    TextBuffer* textBuffer = new TextBuffer(17);
    lcd.clear();
    lcd.print(displayText);
    while(true){
        lcd.setCursor(0, 1);
        if(keypad->pollStateBlocking(KeyState::KEY_DOWN) > 0){
            char key = keypad->buffer[0].key;
            if(key == ENTER){
                if(textBuffer->isEmpty()){ 
                    delete textBuffer;
                    return nullptr; 
                }
                *variable = (float)atof(textBuffer->buffer);
                delete textBuffer;
                return nullptr;
            }
            else if(key == BACK){
                if(textBuffer->isEmpty()){ 
                    delete textBuffer;
                    return nullptr; 
                }
                textBuffer->popBack();
            }
            else if(key == DOT){
                textBuffer->pushBack('.');
            }
            else{
                if(key < '0' || key > '9'){
                    textBuffer->pushBack(key);
                }
            }
        }
        lcd.print(textBuffer->buffer);
    }
}
void* setUserVariable(const char* displayText, float* variable){
    setUserVariable(const_cast<char*>(displayText), variable);
    return nullptr;
}

bool pollKeypadForSpecificCharPressed(char c){
    keypad->poll();
    if(keypad->getKeysWithState(KeyState::KEY_DOWN)){
        if(keypad->buffer[0].key == c){
            return true;
        }
    }
    return false;
}