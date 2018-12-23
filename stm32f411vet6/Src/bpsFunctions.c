
#include "bpsFunctions.h"

HAL_StatusTypeDef bpsStartEncoder()
{
	HAL_StatusTypeDef ret;
	ret = HAL_TIM_Encoder_Start(&ENCODER_X_HANDLE,TIM_CHANNEL_ALL);
	ret |=HAL_TIM_Encoder_Start(&ENCODER_Y_HANDLE,TIM_CHANNEL_ALL);
	return ret;
}

HAL_StatusTypeDef bpsReadEncoderCnt(bpsAxisTypeDef axis, int16_t* encoderCnt_out)
{
	if (axis == BPS_X_AXIS)
		*encoderCnt_out = ENCODER_X_REG->CNT;
	else if (axis == BPS_Y_AXIS)
		*encoderCnt_out = ENCODER_Y_REG->CNT;
	else return HAL_ERROR;
	return HAL_OK;
}

HAL_StatusTypeDef bpsSetEncoderCnt(bpsAxisTypeDef axis, int16_t value)
{
	if (axis == BPS_X_AXIS)
		ENCODER_X_REG->CNT = value;
	else if (axis == BPS_Y_AXIS)
		ENCODER_Y_REG->CNT = value;
	else return HAL_ERROR;
	return HAL_OK;
}

HAL_StatusTypeDef bpsStartPWM()
{
	HAL_StatusTypeDef	ret;
	ret = HAL_TIM_Base_Start(&PWM_X_HANDLE); 
	ret |= HAL_TIM_PWM_Start(&PWM_X_HANDLE,TIM_CHANNEL_1);
	ret |= HAL_TIM_PWM_Start(&PWM_X_HANDLE,TIM_CHANNEL_2);
	ret |= HAL_TIM_Base_Start(&PWM_Y_HANDLE); 
	ret |= HAL_TIM_PWM_Start(&PWM_Y_HANDLE,TIM_CHANNEL_1);
	ret |= HAL_TIM_PWM_Start(&PWM_Y_HANDLE,TIM_CHANNEL_2);
	return ret;
}

HAL_StatusTypeDef bpsSetPWMDuty(bpsAxisTypeDef axis, uint16_t duty, bpsDirectionTypeDef direction)
{
	if (duty > MAX_PWM_DUTY)
		return HAL_ERROR;
	if (direction == BPS_FORWARD)
	{
		if (axis == BPS_X_AXIS)
		{
			__HAL_TIM_SET_COMPARE(&PWM_X_HANDLE,PWM_PIN_1_X,duty);
			__HAL_TIM_SET_COMPARE(&PWM_X_HANDLE,PWM_PIN_2_X,0);
		}
		else if (axis == BPS_Y_AXIS)
		{
			__HAL_TIM_SET_COMPARE(&PWM_Y_HANDLE,PWM_PIN_1_Y,duty);
			__HAL_TIM_SET_COMPARE(&PWM_Y_HANDLE,PWM_PIN_2_Y,0);
		}
		else return HAL_ERROR;
	}
	else if (direction == BPS_BACKWORD)
	{
		if (axis == BPS_X_AXIS)
		{
			__HAL_TIM_SET_COMPARE(&PWM_X_HANDLE,PWM_PIN_1_X,0);
			__HAL_TIM_SET_COMPARE(&PWM_X_HANDLE,PWM_PIN_2_X,duty);
		}
		else if (axis == BPS_Y_AXIS)
		{
			__HAL_TIM_SET_COMPARE(&PWM_Y_HANDLE,PWM_PIN_1_Y,0);
			__HAL_TIM_SET_COMPARE(&PWM_Y_HANDLE,PWM_PIN_2_Y,duty);
		}
		else return HAL_ERROR;
	}
	else return HAL_ERROR;
	
 	return HAL_OK;
}


HAL_StatusTypeDef bpsUARTReceiveData(bpsUARTReceiveDataTypeDef* receiveData)
{
	return HAL_UART_Receive_DMA(&RECEIVE_DATA_HANDLE, (uint8_t*)receiveData, sizeof(bpsUARTReceiveDataTypeDef));
}


HAL_StatusTypeDef bpsUARTSendData(bpsUARTSendDataTypeDef* sendData)
{
	return HAL_UART_Transmit_IT(&SEND_DATA_HANDLE, (uint8_t*)sendData, sizeof(bpsUARTSendDataTypeDef));
}

HAL_StatusTypeDef bpsDiscretePID(int16_t setpoint, int16_t currentPoint, float Kp, 
									float Ki, float Kd, int16_t* errorSamples_out, float* PIDSamples_out, float time, BOOL clamping, int max, int min)
{
	HAL_StatusTypeDef ret;
	float PID, lastPID = 0;
	if (PIDSamples_out == NULL || errorSamples_out == NULL)
		return HAL_ERROR;
	int16_t e = setpoint - currentPoint;
	if (clamping)
	{
		if (lastPID > max ? 1 : lastPID < min ? 1 : 0  && e*Ki > 0 ? 1 : e*Ki < 0 ? 1 : 0) 
			Ki = 0;
	}
	
	PID = (Kp + Ki * time / 2 + Kd / time) * e
			+	(-Kp + Ki * time / 2 - Kd / time * 2) * *errorSamples_out
			+	(Kd / time) * *(errorSamples_out + 1)
			+ 	*PIDSamples_out;
	lastPID = PID;
	if (clamping)
		PID = saturation(PID, max, min);
	ret = bpsAppendInts(errorSamples_out, e);
	ret |= bpsAppendFloats(PIDSamples_out, PID);
	return ret;
}



HAL_StatusTypeDef bpsAppendInts(int16_t* errorSamples, int16_t newSample)
{

	if (errorSamples == NULL)
		return HAL_ERROR;
	for (int i = NUMBER_OF_SAMPLE - 1; i > 0; i--)
		*(errorSamples + i) = *(errorSamples + i - 1);
	*errorSamples = newSample;
	return HAL_OK;
	
}

HAL_StatusTypeDef bpsAppendFloats(float* PIDSamples, float newSample)
{

	if (PIDSamples == NULL)
		return HAL_ERROR;
	for (int i = NUMBER_OF_SAMPLE - 1; i > 0; i--)
		*(PIDSamples + i) = *(PIDSamples + i - 1);
	*PIDSamples= newSample;
	return HAL_OK;
	
}

HAL_StatusTypeDef bpsControlMotor(bpsAxisTypeDef axis ,float PID)
{
	if (PID >= 0)
	{
		return bpsSetPWMDuty(axis, PID, BPS_FORWARD);
	}
	else
	{
		return bpsSetPWMDuty(axis, -1 * PID, BPS_BACKWORD);
	}
}

HAL_StatusTypeDef bpsCalSetpoint4CircleMode (int16_t centerOrdinate, uint16_t radius, float* currentAngle_out, uint16_t speed, 
											int16_t* setpoint_out, bpsAxisTypeDef axis)
{
	if (radius == 0 || speed == 0)
		return HAL_ERROR;
	if (currentAngle_out == NULL || setpoint_out == NULL)
		return HAL_ERROR;
	*setpoint_out = centerOrdinate + radius * (axis == BPS_Y_AXIS ?  sin(*currentAngle_out / 180 * 3.1415f) 
																	: cos(*currentAngle_out / 180 * 3.1415f));
	if (axis == BPS_X_AXIS)
		*setpoint_out = 480 - *setpoint_out;
	if (*currentAngle_out >= 360) 
		*currentAngle_out = 0; 
	else 
		*currentAngle_out += (float)speed / 1.2f;
	return HAL_OK;
}

HAL_StatusTypeDef bpsCalSetpoint4RectMode (bpsRectangleTypeDef *rect, int *timeElapse, int16_t *setpoint)
{
	if (rect == NULL || timeElapse == NULL || setpoint == NULL)
		return HAL_ERROR;
	if (*timeElapse > TIME_FOR_A_RECT * 0.75)
	{
		*setpoint = 480 - rect->vertexCoordinate[BPS_TOP_LEFT][BPS_X_AXIS];
		*(setpoint + 1) =  rect->vertexCoordinate[BPS_TOP_LEFT][BPS_Y_AXIS];
		*timeElapse-=1;
		return HAL_OK;
	}
	if (*timeElapse > TIME_FOR_A_RECT * 0.5)
	{
		*setpoint = 480 - rect->vertexCoordinate[BPS_TOP_RIGHT][BPS_X_AXIS];
		*(setpoint + 1) =  rect->vertexCoordinate[BPS_TOP_RIGHT][BPS_Y_AXIS];
		*timeElapse-=1;
		return HAL_OK;
	}
	if (*timeElapse > TIME_FOR_A_RECT * 0.25)
	{
		*setpoint = 480 - rect->vertexCoordinate[BPS_BOT_RIGHT][BPS_X_AXIS];
		*(setpoint + 1) =  rect->vertexCoordinate[BPS_BOT_RIGHT][BPS_Y_AXIS];
		*timeElapse-=1;
		return HAL_OK;
	}
	if (*timeElapse > 0)
	{
		*setpoint = 480 - rect->vertexCoordinate[BPS_BOT_LEFT][BPS_X_AXIS];
		*(setpoint + 1) =  rect->vertexCoordinate[BPS_BOT_LEFT][BPS_Y_AXIS];
		*timeElapse-=1;
		return HAL_OK;
	}
	*timeElapse = TIME_FOR_A_RECT;
	return HAL_OK;
	
}

float saturation(float PID, int max, int min)
{
	if (PID > max)
		return max;
	else if (PID < min)
		return min;
	else
		return PID;
}


