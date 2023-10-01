#include <BBkeypad.h>
#include <Arduino.h>


BBkeypad::BBkeypad(){}

BBkeypad::BBkeypad(char* keys, const u8_t numColumns, const u8_t numRows, const u8_t* columnPins, const u8_t* rowPins){
    this->numRows = numRows;
    this->numColumns = numColumns;
    this->debounceInterval = DEFAULT_DEBOUNCE_INTERVAL;
    this->holdTime = DEFAULT_HOLD_TIME;
    this->keypad = new Key[numRows * numColumns];
    this->buffer = new Key[numRows * numColumns];
    this->columnPins = columnPins;
    this->rowPins = rowPins;
    for(int i = 0; i < this->numColumns; i++){
        pinMode(this->columnPins[i], OUTPUT);
        digitalWrite(this->columnPins[i], HIGH);
    }
    for(int i = 0; i < this->numRows; i++){
        pinMode(this->rowPins[i], INPUT_PULLUP);
    }
    for(int x = 0; x < this->numRows; x++){
        for(int y = 0; y < this->numColumns; y++){
            this->keypad[x * this->numRows + y] = Key{keys[x * this->numRows + y], 0, IDLE};
        }
    }
    for(int i = 0; i < (this->numColumns * this->numRows); i++){
        this->buffer[i] = this->keypad[0];
    }
}

BBkeypad::~BBkeypad(){
    free(this->keypad);
    free(this->buffer);
}

bool BBkeypad::poll(){
    bool keyActivity = false;
    unsigned long currentTime = millis();
    for(int x = 0; x < this->numColumns; x++){
        digitalWrite(this->columnPins[x], LOW);
        for(int y = 0; y < this->numRows; y++){
            Key* key = &this->keypad[y * this->numRows + x];
            switch(digitalRead(this->rowPins[y])){
                case LOW:
                    switch(key->state){
                        case HELD: break;

                        case PRESSED:
                            if(currentTime - key->timestamp > this->holdTime){ 
                                key->state = HELD;
                                key->timestamp = currentTime;
                                keyActivity = true;
                            }
                            break;

                        case IDLE: 
                            key->state = KEY_DOWN;
                            key->timestamp = currentTime;
                            keyActivity = true;
                            break;

                        case KEY_DOWN:
                            key->state = PRESSED;
                            keyActivity = true;
                            break;

                        case KEY_UP:
                            if(currentTime - key->timestamp > this->debounceInterval){
                                key->state = KEY_DOWN;
                                key->timestamp = currentTime;
                                keyActivity = true;
                            }
                            break;

                        default: break;
                    }
                    break;

                case HIGH:
                    switch(key->state){
                        case IDLE: break;

                        case PRESSED:
                            if(currentTime - key->timestamp > this->debounceInterval){ break; }
                            key->state = KEY_UP;
                            key->timestamp = currentTime;
                            keyActivity = true;
                            break;

                        case HELD:
                            key->state = KEY_UP;
                            key->timestamp = currentTime;
                            keyActivity = true;
                            break;

                        case KEY_DOWN:
                            if(currentTime - key->timestamp > this->debounceInterval){
                                key->state = KEY_UP;
                                key->timestamp = currentTime;
                                keyActivity = true;
                            }
                            break;

                        case KEY_UP: 
                            key->state = IDLE; 
                            keyActivity = true;
                            break;

                        default: break;
                    }
                    break;
            }
        }
        digitalWrite(this->columnPins[x], HIGH);
    }
    return keyActivity;
}

void BBkeypad::pollBlocking(){
    while(!this->poll());
}

unsigned long BBkeypad::getSize(){
    return ((sizeof(Key) * (this->numRows * this->numColumns * 2)) + 
    (sizeof(unsigned long) * 2) + 
    (sizeof(u8_t) * 2)) +
    (sizeof(u8_t) * (this->numRows + this->numColumns));
}

void BBkeypad::setHoldTime(unsigned long holdTime){
    this->holdTime = holdTime;
}

void BBkeypad::setDebounceInterval(unsigned long debounceInterval){
    this->debounceInterval = debounceInterval;
}

unsigned int BBkeypad::getKeysWithState(KeyState state){
    int bufferIndex = 0;
    for(int i = 0; i < (this->numColumns * this->numRows); i++){
        if(this->keypad[i].state == state){
            this->buffer[bufferIndex] = this->keypad[i];
            bufferIndex++;
        }
    }
    return bufferIndex;
}

unsigned int BBkeypad::getPressedKeys(){
    int bufferIndex = 0;
    for(int i = 0; i < (this->numColumns * this->numRows); i++){
        if(this->keypad[i].state == KEY_DOWN || this->keypad[i].state == PRESSED || this->keypad[i].state == HELD){
            this->buffer[bufferIndex] = this->keypad[i];
            bufferIndex++;
        }
    }
    return bufferIndex;
}