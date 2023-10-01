class PID_controller {

  private:
    float error1;
    float error2;
    float integral;

  public:
    float Kp;
    float Ki;
    float Kd;
    float min;
    float max;
    PID_controller();
    PID_controller(float Kp, float Ki, float Kd);
    PID_controller(float Kp, float Ki, float Kd, float min, float max);
    ~PID_controller();
    float compute(float target, float current, unsigned long microsElapsed);
    void setPID(float Kp, float Ki, float Kd);
    void setOutputRange(float min, float max);
};