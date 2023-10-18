#pragma once
#include <main.h>

extern const u8_t maxJobNameLength;

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
        char* name;
        SpinnerAction* sequence;
        u8_t sequenceLength;
        u8_t index;
        float currentTargetRpm;
        bool stopped;
        Config config; 

        SpinnerJob(char* name);
        ~SpinnerJob();
        bool start();
        bool update();
        void stop();
        void reset();
        void addConfig(Config config);
        Config* getConfig();
        bool pushAction(SpinnerAction action);
        bool addAction(u8_t index, SpinnerAction action);
        void init(u8_t size);
        void changeName(char* newName);

    private:
        unsigned long nextTransitionTime;
        unsigned long previousTransitionTime;
        float previousRpm;
};

class JobTable{
    public:
        u8_t numJobs;
        JobTable(const char* repositoryPath);
        ~JobTable();
        void loadJobs();
        void setRepositoryPath(const char* path);
        SpinnerJob* getJob(int index);
        int exists(char* name);
        bool addJob(SpinnerJob* job);
        bool removeJob(int index);
        bool isEmpty();
    private:
        char* repositoryPath;
        SpinnerJob** jobs;
};