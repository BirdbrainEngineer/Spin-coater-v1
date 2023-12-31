#pragma once
#include <Arduino.h>

class TextBuffer{
    public:
        char* buffer;
        u32_t bufferLen;
        u32_t pointer;
        TextBuffer(u32_t size);
        TextBuffer(char* buffer, u32_t size);
        ~TextBuffer();
        bool pushBack(char c);
        char popBack();
        bool pushFront(char c);
        char popFront();
        bool pushMid(char c, u32_t index);
        char popMid(u32_t index);
        bool pushString(const char* str);
        bool pushString(char* str);
        char readFront();
        char readMid(u32_t index);
        char readBack();
        bool isEmpty();
        bool isFull();
        void clear();
        void erase();
    private:
        bool ownsBuffer;
};