#include <TextBuffer.h>

TextBuffer::TextBuffer(u32_t size){
    this->buffer = (char*)malloc(size * sizeof(char));
    this->bufferLen = size;
    this->pointer = 0;
    this->ownsBuffer = true;
}

TextBuffer::TextBuffer(char *buffer, u32_t size){
    this->buffer = buffer;
    this->bufferLen = size;
    this->pointer = 0;
    this->ownsBuffer = false;
}

TextBuffer::~TextBuffer(){
    if(ownsBuffer){
        free(buffer);
    }
    else{
        this->erase();
    }
}

bool TextBuffer::pushBack(char c){
    if(pointer == bufferLen){ return false; }
    else {
        buffer[pointer] = c;
        pointer++;
        buffer[pointer] = '\0';
        return true;
    }
}

char TextBuffer::popBack(){
    if(pointer == 0){ return '\0'; }
    else {
        pointer--;
        char c = buffer[pointer];
        buffer[pointer] = '\0';
        return c;
    }
}

bool TextBuffer::pushFront(char c){
    if(pointer == bufferLen){ return false; }
    else {
        for(int i = pointer; i > 0; i--){
            buffer[i] = buffer[i - 1];
        }
        buffer[0] = c;
        pointer++;
        buffer[pointer] = '\0';
        return true;
    }
}

char TextBuffer::popFront(){
    if(pointer == 0){ return '\0'; }
    else {
        char c = buffer[0];
        for(int i = 0; i < pointer; i++){
            buffer[i] = buffer[i + 1];
        }
        pointer--;
        buffer[pointer] = '\0';
        return c;
    }
}

bool TextBuffer::pushMid(char c, u32_t index){
    if(pointer == bufferLen){ return false; }
    else {
        for(int i = pointer; i > index; i--){
            buffer[i] = buffer[i - 1];
        }
        buffer[index] = c;
        pointer++;
        buffer[pointer] = '\0';
        return true;
    }
}

char TextBuffer::popMid(u32_t index){
    if(pointer == 0){ return '\0'; }
    else {
        char c = buffer[index];
        for(int i = index; i < pointer; i++){
            buffer[i] = buffer[i + 1];
        }
        pointer--;
        buffer[pointer] = '\0';
        return c;
    }
}

bool TextBuffer::pushString(const char* str){
    return this->pushString(const_cast<char*>(str));
}

bool TextBuffer::pushString(char* str){
    u32_t inputPointer = 0;
    while(str[inputPointer] != '\0'){
        if(this->pointer != bufferLen - 1){
            this->pushBack(str[inputPointer]);
            inputPointer++;
        }
        else { return false; }
    }
    return true;
}

char TextBuffer::readFront(){
    return buffer[0];
}

char TextBuffer::readMid(u32_t index){
    if(pointer == 0){ return '\0'; }
    else {
        return buffer[index];
    }
}

char TextBuffer::readBack(){
    if (pointer == 0){ return '\0'; }
    return buffer[pointer - 1];
}

bool TextBuffer::isEmpty(){
    return pointer == 0 ? true : false;
}

void TextBuffer::clear(){
    pointer = 0;
    buffer[0] = '\0';
}

void TextBuffer::erase(){
    for(int i = 0; i < bufferLen; i++){
        this->buffer[i] = '\0';
    }
}
