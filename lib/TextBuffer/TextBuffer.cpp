#include <TextBuffer.h>
#include <Arduino.h>

TextBuffer::TextBuffer(char *buffer, u32_t size){
    this->buffer = buffer;
    this->bufferLen = size;
    this->textLen = bufferLen - 1;
    this->pointer = 0;
}

TextBuffer::~TextBuffer(){
    this->erase();
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
    pointer == 0 ? true : false;
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
