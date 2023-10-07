#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

enum SpinnerTask{
    HOLD,
    RAMP,
    END
};

struct SpinnerAction{
    SpinnerTask task;
    u32_t duration; // in milliseconds
    float rpm; // if ramp task, then defines the rpm at end of ramp. Start rpm is the hold rpm/end rpm of previous task.
};
