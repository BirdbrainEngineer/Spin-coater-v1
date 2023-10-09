#include <SpinnerMenu.h>

void* setKp(){
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