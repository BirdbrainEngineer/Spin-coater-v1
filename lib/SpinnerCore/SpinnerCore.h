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
    unsigned int duration;
    SpinnerTask task;
    float rpm;
};

class SpinnerJob{
    public:
        #define MAX_SEQUENCE_LENGTH 50
        #define BEGIN_DURATION 50
        unsigned long jobStartTime;
        float startRPM;
        float endRPM;
        int sequenceLength;
        int currentSequence;
        SpinnerJob();
        ~SpinnerJob();
        bool init(SpinnerAction* sequence); // Initializes the job sequence. Returns false if the job is not a valid job sequence.
        bool begin(); // begins the job sequence. Returns false if the job is not a valid job sequence.
        bool update(); // updates the job state. Returns false if the job has finished.

    private:
        unsigned long taskStartTime;
        float minRPM;
        SpinnerAction sequence[MAX_SEQUENCE_LENGTH];
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