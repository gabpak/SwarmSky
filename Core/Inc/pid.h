#ifndef INC_PID_H_
#define INC_PID_H_

#include "main.h"

/*
 * 	Definition of the parameters of a PID
 */
typedef struct{
	uint32_t Kp;
	uint32_t Ki;
	uint32_t Kd;
	uint32_t setpoint;
	uint32_t integral;
	uint32_t previous_error;
}PID;

uint32_t Compute_PID(PID &pid, uint32_t &measure);

#endif /* INC_PID_H_ */
