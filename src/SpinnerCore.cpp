#include <main.h>

SpinnerJob::SpinnerJob(char* jobName){
    this->name = (char*)malloc(sizeof(char) * maxJobNameLength);
    panicIfOOM(this->name, "@SpinnerJob-0");
    int i = 0;
    while(jobName[i] != '\0' && i < maxJobNameLength - 2){
        this->name[i] = jobName[i];
        i++;
    }
    this->name[i] = '\0';
    this->sequence = nullptr;
    this->useEmbeddedPIDConstants = false;
}

SpinnerJob::~SpinnerJob(){
    free(this->name);
    if(this->sequence != nullptr){free(this->sequence);}
}

void SpinnerJob::init(u8_t size){
    this->sequence = (SpinnerAction*)malloc(sizeof(SpinnerAction) * size);
    panicIfOOM(this->sequence, "@SpinnerJob-1");
    this->sequenceLength = size;
    this->index = 0;
    this->currentTargetRpm = 0.0;
    this->nextTransitionTime = 0;
    this->previousRpm = 0.0;
    this->stopped = true;
}

bool SpinnerJob::start(){ //returns false if the job could not be started
    if(this->sequence[this->sequenceLength - 1].task != END){ return false; }
    pidEnabled = true;
    this->stopped = false;
    enableMotor();
    this->previousTransitionTime = millis();
    this->nextTransitionTime = this->previousTransitionTime + this->sequence[0].duration;
    return true;
}

bool SpinnerJob::update(){ //returns false if the job has finished
    if(this->stopped){ return false; }
    unsigned long currentTime = millis();
    if(currentTime > nextTransitionTime){
        this->previousRpm =this->sequence[this->index].rpm;
        this->index++;
        this->previousTransitionTime = currentTime;
        this->nextTransitionTime = this->previousTransitionTime + this->sequence[this->index].duration;
        if(this->sequence[this->index].task == END){ 
            stopTest("Job successfully\nfinished!");
            disableMotor();
            this->currentTargetRpm = 0.0;
            this->stopped = true;
            return false;
        }
        return this->update();
    }
    switch(this->sequence[this->index].task){
        case HOLD:{
            this->currentTargetRpm = this->sequence[this->index].rpm;
            return true;
        }
        case RAMP:{
            float ramp = (float)(currentTime - this->previousTransitionTime) / (float)this->sequence[this->index].duration;
            this->currentTargetRpm = this->previousRpm + ((this->sequence[this->index].rpm - this->previousRpm) * ramp);
            return true;
        }
        case END:{
            pidEnabled = false;
            disableMotor();
            this->currentTargetRpm = 0.0;
            this->stopped = true;
            return false;
        }
        case NONE:{
            this->nextTransitionTime = this->previousTransitionTime;
            return this->update();
        }
        default:{
            pidEnabled = false;
            disableMotor();
            this->stopped = true;
            printlcdErrorMsg("    Illegal\nSpinnerAction!");
            return false;
        }
    }
}

void SpinnerJob::stop(){
    pidEnabled = false;
    disableMotor();
    this->stopped = true;
}

void SpinnerJob::reset(){
    this->index = 0;
    this->currentTargetRpm = 0.0;
    this->previousRpm = 0.0;
    this->previousTransitionTime = 0;
}

bool SpinnerJob::pushAction(SpinnerAction action){
    if(this->sequenceLength == UINT8_MAX){ return false; }
    this->sequence[this->sequenceLength].duration = action.duration;
    this->sequence[this->sequenceLength].rpm = action.rpm;
    this->sequence[this->sequenceLength].task = action.task;
    this->sequenceLength++;
    return true;
}

bool SpinnerJob::addAction(u8_t index, SpinnerAction action){
    if(index == UINT8_MAX){ return false; }
    if(index >= this->sequenceLength){ this->sequenceLength = index + 1; }
    this->sequence[index].duration = action.duration;
    this->sequence[index].rpm = action.rpm;
    this->sequence[index].task = action.task;
    return true;
}

void SpinnerJob::addConfig(Config config){
    this->config.Kd = config.Kd;
    this->config.Ki = config.Ki;
    this->config.Kp = config.Kp;
    this->config.analogAlpha = config.analogAlpha;
    this->config.rpmAlpha = config.rpmAlpha;
}

Config* SpinnerJob::getConfig(){
    return &this->config;
}

void SpinnerJob::changeName(char* newName){
    free(this->name);
    this->name = (char*)malloc(sizeof(char) * maxJobNameLength);
    panicIfOOM(this->name, "@SpinnerJob-name");
    int i = 0;
    while(newName[i] != '\0' && i < maxJobNameLength - 2){
        this->name[i] = newName[i];
        i++;
    }
    this->name[i] = '\0';
}



//@@@@@@@@@@@@@@@@@@@@@ JobTable @@@@@@@@@@@@@@@@@@@@@@@@@

JobTable::JobTable(const char* repositoryPath){
    this->repositoryPath = const_cast<char*>(repositoryPath);
    this->jobs = (SpinnerJob**)malloc(sizeof(SpinnerJob*) * UINT8_MAX);
    panicIfOOM(this->jobs, "@:JobTable-jobs");
    this->numJobs = 0;
}
JobTable::~JobTable(){
    for(int i = 0; i < this->numJobs; i++){
        free(this->jobs[i]);
    }
    free(this->jobs);
}
void JobTable::setRepositoryPath(const char* path){
    this->repositoryPath = const_cast<char*>(path);
}
SpinnerJob* JobTable::getJob(int index){
    if(index < 0 || index >= this->numJobs){ return nullptr; }
    return this->jobs[index];
}
int JobTable::exists(char* name){
    for(int i = 0; i < this->numJobs; i++){
        if(strcmp(this->jobs[i]->name, name) == 0){
            return i;
        }
    }
    return -1;
}
bool JobTable::addJob(SpinnerJob* job){
    if(this->numJobs == UINT8_MAX){ return false; }
    this->jobs[this->numJobs] = job;
    this->numJobs++;
    return true;
}
bool JobTable::removeJob(int index){
    if(index < 0 || index >= this->numJobs){ return false; }
    free(this->jobs[index]);
    while(index < this->numJobs){
        this->jobs[index] = this->jobs[index + 1];
        index++;
    }
    this->numJobs--;
    return true;
}

bool JobTable::isEmpty(){
    return this->numJobs == 0 ? true : false;
}

char* JobTable::getRepositoryPath(){
    return this->repositoryPath;
}