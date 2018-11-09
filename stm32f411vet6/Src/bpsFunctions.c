
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

HAL_StatusTypeDef bpsStartPWM()
{
	HAL_StatusTypeDef	ret;
	ret = HAL_TIM_Base_Start(&PWM_HANDLE); 
	ret |= HAL_TIM_PWM_Start(&PWM_HANDLE,TIM_CHANNEL_1);
	ret |= HAL_TIM_PWM_Start(&PWM_HANDLE,TIM_CHANNEL_2);
	ret |= HAL_TIM_PWM_Start(&PWM_HANDLE,TIM_CHANNEL_3);
	ret |= HAL_TIM_PWM_Start(&PWM_HANDLE,TIM_CHANNEL_4);
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
			__HAL_TIM_SET_COMPARE(&PWM_HANDLE,PWM_PIN_1_X,duty);
			__HAL_TIM_SET_COMPARE(&PWM_HANDLE,PWM_PIN_2_X,0);
		}
		else if (axis == BPS_Y_AXIS)
		{
			__HAL_TIM_SET_COMPARE(&PWM_HANDLE,PWM_PIN_1_Y,duty);
			__HAL_TIM_SET_COMPARE(&PWM_HANDLE,PWM_PIN_2_Y,0);
		}
		else return HAL_ERROR;
	}
	else if (direction == BPS_BACKWORD)
	{
		if (axis == BPS_X_AXIS)
		{
			__HAL_TIM_SET_COMPARE(&PWM_HANDLE,PWM_PIN_1_X,0);
			__HAL_TIM_SET_COMPARE(&PWM_HANDLE,PWM_PIN_2_X,duty);
		}
		else if (axis == BPS_Y_AXIS)
		{
			__HAL_TIM_SET_COMPARE(&PWM_HANDLE,PWM_PIN_1_Y,0);
			__HAL_TIM_SET_COMPARE(&PWM_HANDLE,PWM_PIN_2_Y,duty);
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

HAL_StatusTypeDef bpsCalculatePID(int16_t setpoint, int16_t currentPoint, float Kp, 
									float Ki, float Kd, int16_t* errorSamples_out, float* PIDSamples_out, float time)
{
	HAL_StatusTypeDef ret;
	if (PIDSamples_out == NULL || errorSamples_out == NULL)
		return HAL_ERROR;
	int16_t e = setpoint - currentPoint;
	float PID = (Kp + Ki * time / 2 + Kd / time) * e
			+	(-Kp + Ki * time / 2 - Kd / time * 2) * *errorSamples_out
			+	(Kd / time) * *(errorSamples_out + 1)
			+	*PIDSamples_out;
	ret = bpsAppendErrorSamples(errorSamples_out, e);
	ret |= bpsAppendPIDSamples(PIDSamples_out, PID);
	return ret;
}

HAL_StatusTypeDef bpsAppendErrorSamples(int16_t* errorSamples, int16_t newSample)
{

	if (errorSamples == NULL)
		return HAL_ERROR;
	for (int i = NUMBER_OF_SAMPLE - 1; i > 0; i--)
		*(errorSamples + i) = *(errorSamples + i - 1);
	*errorSamples= newSample;
	return HAL_OK;
	
}

HAL_StatusTypeDef bpsAppendPIDSamples(float* PIDSamples, float newSample)
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
	uint16_t absPID = PID >= 0 ? (uint16_t)PID : -(uint16_t)PID;
	if (absPID > MAX_PWM_DUTY)
		absPID = MAX_PWM_DUTY;
	if (PID >= 0)
	{
		return bpsSetPWMDuty(axis, absPID, BPS_FORWARD);
	}
	else
	{
		return bpsSetPWMDuty(axis, absPID, BPS_BACKWORD);
	}
}

HAL_StatusTypeDef bpsCalSetpoint4CircleMode (int16_t centerOrdinate, uint16_t radius, uint16_t* currentAngle_out, uint16_t speed, 
											int16_t* setpoint_out, bpsAxisTypeDef axis)
{
	if (radius == 0 || speed == 0)
		return HAL_ERROR;
	if (currentAngle_out == NULL || setpoint_out == NULL)
		return HAL_ERROR;

	*setpoint_out = (int16_t)(centerOrdinate + radius * (axis ? cos(*currentAngle_out * speed) : sin(*currentAngle_out * speed)));
	if (*currentAngle_out == 360 * (uint16_t)(1 / speed)) *currentAngle_out = 0; else *currentAngle_out++;
	return HAL_OK;
}
