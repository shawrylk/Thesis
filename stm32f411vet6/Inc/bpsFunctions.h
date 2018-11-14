
#ifndef USER_FUNCTION
#define USER_FUNCTION

#include "bpsDefine.h"

HAL_StatusTypeDef		bpsStartEncoder				(void);
HAL_StatusTypeDef		bpsReadEncoderCnt			(bpsAxisTypeDef axis, int16_t* encoderCnt_out);
HAL_StatusTypeDef		bpsStartPWM					(void);
HAL_StatusTypeDef		bpsSetPWMDuty				(bpsAxisTypeDef axis, uint16_t duty, bpsDirectionTypeDef direction);
HAL_StatusTypeDef		bpsUARTReceiveData			(bpsUARTReceiveDataTypeDef* receiveData);
HAL_StatusTypeDef		bpsUARTSendData				(bpsUARTSendDataTypeDef* sendData);
HAL_StatusTypeDef 		bpsCalculatePID				(int16_t setpoint, int16_t currentPoint, float Kp, 
													float Ki, float Kd, int16_t* errorSamples_out, float* PIDSamples_out, float time);
HAL_StatusTypeDef 		bpsAppendErrorSamples		(int16_t* errorSamples, int16_t newSample);
HAL_StatusTypeDef		bpsAppendPIDSamples			(float* PIDSamples, float newSample);
HAL_StatusTypeDef		bpsControlMotor				(bpsAxisTypeDef axis, float PID);
HAL_StatusTypeDef		bpsCalSetpoint4CircleMode 	(int16_t centerOrdinate, uint16_t radius, uint16_t* currentAngle_out, uint16_t speed, 
													int16_t* setpoint_out, bpsAxisTypeDef axis);
HAL_StatusTypeDef 		bpsFindThresholds			(bpsAxisTypeDef axis, int16_t *min, int16_t *max);
#endif
