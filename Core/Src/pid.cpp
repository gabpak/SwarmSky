#include "pid.h"

uint32_t Compute_PID(PID &pid, uint32_t &measure){
	uint32_t error = pid.setpoint - measure;
	pid.integral += error;
    uint32_t derivative = error - pid.previous_error;
    pid.previous_error = error;

    return (pid.Kp * error) + (pid.Ki * pid.integral) + (pid.Kd * derivative);
}
