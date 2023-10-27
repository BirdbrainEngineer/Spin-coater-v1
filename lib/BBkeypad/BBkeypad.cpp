#include <BBkeypad.h>


BBkeypad::BBkeypad(){}

BBkeypad::BBkeypad(char* keys, const u8_t numColumns, const u8_t numRows, const u8_t* columnPins, const u8_t* rowPins){
    this->numRows = numRows;
    this->numColumns = numColumns;
    this->debounceInterval = DEFAULT_DEBOUNCE_INTERVAL;
    this->holdTime = DEFAULT_HOLD_TIME;
    this->keypad = (Key*)malloc(sizeof(Key) * (numRows * numColumns));
    this->buffer = (Key*)malloc(sizeof(Key) * (numRows * numColumns));
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
            this->keypad[x * this->numRows + y] = Key{keys[x * this->numRows + y], 0, KeyState::IDLE};
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
                        case KeyState::HELD: break;

                        case KeyState::PRESSED:
                            if(currentTime - key->timestamp > this->holdTime){ 
                                key->state = KeyState::HELD;
                                key->timestamp = currentTime;
                                keyActivity = true;
                            }
                            break;

                        case KeyState::IDLE: 
                            key->state = KeyState::KEY_DOWN;
                            key->timestamp = currentTime;
                            keyActivity = true;
                            break;

                        case KeyState::KEY_DOWN:
                            key->state = KeyState::PRESSED;
                            keyActivity = true;
                            break;

                        case KeyState::KEY_UP:
                            if(currentTime - key->timestamp > this->debounceInterval){
                                key->state = KeyState::KEY_DOWN;
                                key->timestamp = currentTime;
                                keyActivity = true;
                            }
                            break;

                        default: break;
                    }
                    break;

                case HIGH:
                    switch(key->state){
                        case KeyState::IDLE: break;

                        case KeyState::PRESSED:
                            if(currentTime - key->timestamp < this->debounceInterval){ break; }
                            key->state = KeyState::KEY_UP;
                            key->timestamp = currentTime;
                            keyActivity = true;
                            break;

                        case KeyState::HELD:
                            key->state = KeyState::KEY_UP;
                            key->timestamp = currentTime;
                            keyActivity = true;
                            break;

                        case KeyState::KEY_DOWN:
                            if(currentTime - key->timestamp > this->debounceInterval){
                                key->state = KeyState::KEY_UP;
                                key->timestamp = currentTime;
                                keyActivity = true;
                            }
                            break;

                        case KeyState::KEY_UP: 
                            key->state = KeyState::IDLE; 
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

u16_t BBkeypad::pollState(KeyState state){
    if(this->poll()){
        return this->getKeysWithState(state);
    }
    return 0;
}

u16_t BBkeypad::pollStateBlocking(KeyState state){
    while(true){
        u16_t keys = this->pollState(state);
        if(keys > 0){ return keys; }
    }
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

u16_t BBkeypad::getKeysWithState(KeyState state){
    u16_t bufferIndex = 0;
    for(int i = 0; i < (this->numColumns * this->numRows); i++){
        if(this->keypad[i].state == state){
            this->buffer[bufferIndex] = this->keypad[i];
            bufferIndex++;
        }
    }
    return bufferIndex;
}

u16_t BBkeypad::getPressedKeys(){
    u16_t bufferIndex = 0;
    for(int i = 0; i < (this->numColumns * this->numRows); i++){
        if(this->keypad[i].state == KeyState::KEY_DOWN || 
            this->keypad[i].state == KeyState::PRESSED || 
            this->keypad[i].state == KeyState::HELD)
        {
            this->buffer[bufferIndex] = this->keypad[i];
            bufferIndex++;
        }
    }
    return bufferIndex;
}