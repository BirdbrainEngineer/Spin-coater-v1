#include <Arduino.h>

class TextBuffer{
    public:
        char* buffer;
        TextBuffer(char* buffer, u32_t size);
        ~TextBuffer();
        bool pushBack(char c);
        char popBack();
        bool pushFront(char c);
        char popFront();
        bool pushMid(char c, u32_t index);
        char popMid(u32_t index);
        char readFront();
        char readMid(u32_t index);
        char readBack();
        bool isEmpty();
        void clear();
        void erase();
    private:
        u32_t bufferLen;
        u32_t textLen;
        u32_t pointer;
};