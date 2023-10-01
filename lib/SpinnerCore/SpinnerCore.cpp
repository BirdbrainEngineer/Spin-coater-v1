#include <SpinnerCore.h>
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

SpinnerJob::SpinnerJob(){
    
}

SpinnerJob::~SpinnerJob(){

}

int SpinnerJob::begin(){
    jobStartTime = millis();
    startRPM = sequence[0].rpm;
    endRPM = sequence[0].rpm;
    currentSequence = 1;
    while()
}

void SpinnerJob::next(){
    if()
}