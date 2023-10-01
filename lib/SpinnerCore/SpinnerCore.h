#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

enum SpinnerTask{
    BEGIN,
    HOLD,
    RAMP,
    END
};

struct SpinnerAction{
    unsigned long duration;
    SpinnerTask task;
    float rpm;
};

class SpinnerJob{
    public:
        unsigned long jobStartTime;
        float startRPM;
        float endRPM;
        int sequenceLength;
        int currentSequence;
        SpinnerJob();
        ~SpinnerJob();
        int begin(); // begins the current job sequence. Returns the total number of tasks in the sequence.
        void next();

    private:
        unsigned long taskStartTime;
        float minRPM;
        SpinnerAction sequence[UINT8_MAX];
};

class SpinnerCore{
    public:
        SpinnerCore();
        ~SpinnerCore();
        void loadJobTable();

    private:
        SpinnerJob jobTable[UINT8_MAX];
        File* jobRepository;
};