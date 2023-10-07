#include <Arduino.h>

enum class KeyState {
    IDLE,       // Key is not pressed
    KEY_DOWN,   // Key was just pressed
    KEY_UP,     // Key was just released
    PRESSED,    // Key is pressed and is waiting to transition into 'HELD' state after the specified interval has elapsed
    HELD,       // Key is currently being held down
};

struct Key {
    char key;                   // Character of the key
    unsigned long timestamp;    // Stores the timestamp of when the key was last pressed down or released
    KeyState state;             // Current state of the key
};
class BBkeypad {
    private:
        u8_t numRows;
        u8_t numColumns;
        unsigned long holdTime; // milliseconds
        unsigned long debounceInterval; // milliseconds
        const u8_t* columnPins;
        const u8_t* rowPins;
        Key* keypad;

    public:
        const unsigned long DEFAULT_DEBOUNCE_INTERVAL = 50UL; // Default debounce interval in milliseconds
        const unsigned long DEFAULT_HOLD_TIME = 1000UL; // Default hold time in milliseconds

        Key* buffer; // Buffer used by some functions. Garbage in indexes past the index returned by the relevant function.

        BBkeypad();
        BBkeypad(char* keys, const u8_t numColumns, const u8_t numRows, const u8_t* columnPins, const u8_t* rowPins);
        ~BBkeypad();
        unsigned long getSize(); // Returns the number of bytes used by the whole object both on the stack and heap. 
        void setHoldTime(unsigned long holdTime); // Sets the time interval it takes for a key to transition from 'PRESSED' to 'HELD'.
        void setDebounceInterval(unsigned long debounceInterval); // Sets the debounce interval in milliseconds, minimum 1.
        bool poll(); // Polls the keypad for key press changes. Returns 'true' if any key's state has changed.
        void pollBlocking(); // Polls the keypad as long as it takes for any key's state to change. This function blocks all other code until a key's state changes!
        unsigned int getKeysWithState(KeyState state); // Places all the keys with the specified state into the buffer and returns the number of keys in the buffer.
        unsigned int getPressedKeys(); // Places all the keys that are currently pressed down ('KEY_DOWN', 'PRESSED', 'HELD' states) into the buffer and returns the number of keys in the buffer.
        
};