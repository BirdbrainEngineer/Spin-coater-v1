#include <SpinnerCore.h>
#include <main.h>

SpinnerJob::SpinnerJob(){
    this->init(0);
}

SpinnerJob::SpinnerJob(u8_t size){
    this->init(size);
}

SpinnerJob::~SpinnerJob(){
    free(this->sequence);
}

void SpinnerJob::start(){
    this->previousTransitionTime = millis();
    this->nextTransitionTime = this->previousTransitionTime + this->sequence[0].duration;
}

bool SpinnerJob::update(){ //returns false if the job has finished
    unsigned long currentTime = millis();
    if(currentTime > nextTransitionTime){
        this->previousRpm =this->sequence[this->index].rpm;
        this->index++;
        this->previousTransitionTime = this->nextTransitionTime;
        this->nextTransitionTime = this->previousTransitionTime + this->sequence[this->index].duration;
        return this->update();
    }
    switch(this->sequence[this->index].task){
        case HOLD:  this->currentTargetRpm = this->sequence[this->index].rpm;
                    return true;

        case RAMP:  {
                        float ramp = (currentTime - this->previousTransitionTime) / this->sequence[this->index].duration;
                        this->currentTargetRpm = this->previousRpm + (this->sequence[this->index].rpm - this->previousRpm) * ramp;
                        return true;
                    }

        case END:   stopMotor();
                    this->currentTargetRpm = 0.0;
                    return false;

        case NONE:  stopMotor();
                    printlcdErrorMsg("Illegal\nSpinnerAction!");
                    return false;

        default:    stopMotor();
                    printlcdErrorMsg("Unknown\nSpinnerAction!");
                    return false;
    }
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
    this->sequenceEdited = true;
    return true;
}

bool SpinnerJob::addAction(u8_t index, SpinnerAction action){
    if(index == UINT8_MAX){ return false; }
    if(index >= this->sequenceLength){ this->sequenceLength = index + 1; }
    this->sequence[index].duration = action.duration;
    this->sequence[index].rpm = action.rpm;
    this->sequence[index].task = action.task;
    this->sequenceEdited = true;
    return true;
}

void SpinnerJob::init(u8_t size){
    this->sequence = (SpinnerAction*)malloc(sizeof(SpinnerAction) * UINT8_MAX);
    if(this->sequence == nullptr){
        printlcdErrorMsg("Out of memory!\nReboot imminent!");
        reboot(100);
        while(true);
    }
    this->sequenceLength = size;
    this->index = 0;
    this->currentTargetRpm = 0.0;
    this->nextTransitionTime = 0;
    this->previousRpm = 0.0;
    this->sequenceEdited = false;
}