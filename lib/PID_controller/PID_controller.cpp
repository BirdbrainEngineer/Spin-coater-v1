#include "PID_controller.h"

PID_controller::PID_controller(){
  this->error1 = 0.0;
  this->error2 = 0.0;
  this->integral = 0.0;
  this->setPID(0.0, 0.0, 0.0);
  this->setOutputRange(0.0, 1.0);
}
PID_controller::PID_controller(float Kp, float Ki, float Kd){
  this->error1 = 0.0;
  this->error2 = 0.0;
  this->integral = 0.0;
  this->setPID(Kp, Ki, Kd);
  this->setOutputRange(0.0, 1.0);
}
PID_controller::PID_controller(float Kp, float Ki, float Kd, float min, float max){
  this->error1 = 0.0;
  this->error2 = 0.0;
  this->integral = 0.0;
  this->setPID(Kp, Ki, Kd);
  this->setOutputRange(min, max);
}
PID_controller::~PID_controller(){}

float PID_controller::compute(float target, float current, unsigned long microsElapsed){
  float dt = (float)((double)microsElapsed * 0.000001);
  float error = target - current; //same as proportional
  this->integral += error * dt;
  float derivative = (error - this->error1) / dt;
  this->error1 = error;
  float output = this->Kp * error + this->Ki * integral + this->Kd * derivative;
  output = output > this->max ? this->max : (output < this->min ? this->min : output);
  return output;
}
void PID_controller::setPID(float Kp, float Ki, float Kd){
  this->Kp = Kp;
  this->Ki = Ki;
  this->Kd = Kd;
}
void PID_controller::setOutputRange(float min, float max){
  this->min = min;
  this->max = max;
}