#include <Arduino.h>

class SWtimer{
    public:
        bool continuous; // if true, then starts the next interval immediately after the previous interval has elapsed.
        unsigned long interval; //interval length in microseconds
        SWtimer(unsigned long interval = 1000000, bool continuous = true);
        ~SWtimer();
        void start(); // starts the timer from this time instant
        void stop(); // stops the timer
        void pause(); // pauses the timer
        void resume(); // resumes the timer
        bool poll(); //returns true if the interval has elapsed
        bool isRunning(); // if true, then the timer is currently running. When timer is paused it is always also not running
        bool isPaused(); // if true, then the timer is currently paused. When timer is paused it is always also not running
    private:
        unsigned long intervalStartTime;
        unsigned long intervalEndTime;
        unsigned long pauseTime;
        bool running;
        bool paused;
};