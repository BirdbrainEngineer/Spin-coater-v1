#include <SWtimer.h>

SWtimer::SWtimer(unsigned long interval, bool continuous){
    this->interval = interval;
    this->continuous = continuous;
    this->running = false;
    this->paused = false;
    this->previousTrueElapsedInterval = 0;
}
SWtimer::~SWtimer(){}

bool SWtimer::poll(){
    if(!this->running || this->paused){ return false; }
    unsigned long currentTime = micros();
    if(currentTime - this->intervalStartTime >= this->interval){
        this->previousTrueElapsedInterval = currentTime - this->intervalStartTime;
        if(!this->continuous){
            this->running = false;
            return true;
        }
        this->intervalStartTime = this->intervalStartTime + this->interval;
        return true;
    }
    return false;
}
void SWtimer::start(){
    if(this->running || this->paused){ return; }
    this->previousTrueElapsedInterval = 0;
    this->intervalStartTime = micros();
    this->running = true;
}
void SWtimer::stop(){
    this->running = false;
    this->paused = false;
}
void SWtimer::pause(){
    if(!this->running || this->paused){ return; }
    this->pauseTime = micros() - this->intervalStartTime;
    this->running = false;
    this->paused = true;
}
void SWtimer::resume(){
    if(!this->paused){ return; }
    this->intervalStartTime = micros() - this->pauseTime;
    this->running = true;
    this->paused = false;
}
bool SWtimer::isRunning(){
    return this->running;
}
bool SWtimer::isPaused(){
    return this->paused;
}