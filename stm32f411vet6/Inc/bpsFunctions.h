
#ifndef USER_FUNCTION
#define USER_FUNCTION

#include "bpsDefine.h"

HAL_StatusTypeDef		bpsStartEncoder				(void);
HAL_StatusTypeDef		bpsReadEncoderCnt			(bpsAxisTypeDef axis, int16_t* encoderCnt_out);
HAL_StatusTypeDef 		bpsSetEncoderCnt			(bpsAxisTypeDef axis, int16_t value);
HAL_StatusTypeDef		bpsStartPWM					(void);
HAL_StatusTypeDef		bpsSetPWMDuty				(bpsAxisTypeDef axis, uint16_t duty, bpsDirectionTypeDef direction);
HAL_StatusTypeDef		bpsUARTReceiveData			(bpsUARTReceiveDataTypeDef* receiveData);
HAL_StatusTypeDef		bpsUARTSendData				(bpsUARTSendDataTypeDef* sendData);
HAL_StatusTypeDef 		bpsDiscretePID				(int16_t setpoint, int16_t currentPoint, float Kp, float Ki, float Kd,
													int16_t* errorSamples_out, float* PIDSamples_out, float time, BOOL clamping);
HAL_StatusTypeDef 		bpsAppendInts				(int16_t* errorSamples, int16_t newSample);
HAL_StatusTypeDef		bpsAppendFloats				(float* PIDSamples, float newSample);
HAL_StatusTypeDef		bpsControlMotor				(bpsAxisTypeDef axis, float PID);
HAL_StatusTypeDef		bpsCalSetpoint4CircleMode 	(int16_t centerOrdinate, uint16_t radius, float* currentAngle_out, uint16_t speed, 
													int16_t* setpoint_out, bpsAxisTypeDef axis);
HAL_StatusTypeDef 		bpsCalSetpoint4RectMode 	(bpsRectangleTypeDef *rect, int *timeElapse, int16_t *setpoint);												
float 					encoderSaturation			(float PID);
float 					PWMSaturation				(float PID);
#endif
