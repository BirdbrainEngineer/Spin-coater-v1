#pragma once

enum SpinnerTask{
    NONE,
    HOLD,
    RAMP,
    END,
};

struct SpinnerAction{
    SpinnerTask task = NONE;
    u32_t duration = 1000; // in milliseconds
    float rpm = 0; // if ramp task, then defines the rpm at end of ramp. Start rpm is the hold rpm/end rpm of previous task.
};

class SpinnerJob{
    public:
        SpinnerAction* sequence;
        u8_t sequenceLength;
        u8_t index;
        float currentTargetRpm;
        bool sequenceEdited;
        bool stopped;

        SpinnerJob();
        SpinnerJob(u8_t size);
        ~SpinnerJob();
        bool start();
        bool update();
        void stop();
        void reset();
        bool pushAction(SpinnerAction action);
        bool addAction(u8_t index, SpinnerAction action);

    private:
        unsigned long nextTransitionTime;
        unsigned long previousTransitionTime;
        float previousRpm;
        void init(u8_t size);
};