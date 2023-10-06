#include <SpinnerCore.h>
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

SpinnerJob::SpinnerJob(){}
SpinnerJob::~SpinnerJob(){}

bool SpinnerJob::begin(){
    if(sequence[0].task != BEGIN || sequence[sequenceLength - 1].task != END){
        return 0;
    }
    jobStartTime = millis();
    startRPM = sequence[0].rpm;
    endRPM = sequence[0].rpm;
    currentSequence = 1;
}