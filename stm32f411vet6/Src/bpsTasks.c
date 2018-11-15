
#include "bpsTasks.h"

// remember to clean sendData and sharedData after done
bpsUARTSendDataTypeDef 	sendData;
bpsSharedDataTypeDef 	sharedData;
TaskHandle_t   			taskNumber[NUMBER_OF_TASK];
int16_t encoderValue;
int16_t encX, encXMin, encXMax, encY, encYMin, encYMax;
BOOL delay1ms = false;
void bpsTaskUpdateUARTData(void* pointer)
{
	bpsSharedDataTypeDef* sd = (bpsSharedDataTypeDef*)pointer;
	bpsUARTReceiveDataTypeDef receiveData;
	bpsUARTReceiveData(&receiveData);	
	//sizeofUARTData = sizeof(bpsUARTSendDataTypeDef);
	ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
	while(1)
	{
		//notify take from task control motor
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
		memcpy(&sd->UARTData, &receiveData, sizeof(bpsUARTReceiveDataTypeDef));		
		delay1ms = false;
		xTaskNotifyGive(taskNumber[TASK_UPDATE_SETPOINT]);
		xTaskNotifyGive(taskNumber[TASK_SEND_UART_DATA]);
	}
}

void bpsTaskUpdateSetpoint(void* pointer)
{
	bpsSharedDataTypeDef* sd = (bpsSharedDataTypeDef*)pointer;
	while(1)
	{
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
		switch(sd->UARTData.command)
		{
			case BPS_UPDATE_PID:
				// led for debug
				//HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
				memcpy(&sd->PIDParams, &sd->UARTData.content, sizeof(bpsPIDTypeDef));

				// clear errorSamples
				memset(&sd->errorSamples, 0, sizeof(sd->errorSamples));
				// clear PIDSamples
				memset(&sd->PIDSamples, 0, sizeof(sd->PIDSamples));
				xTaskNotifyGive(taskNumber[TASK_CAL_REF_ENCODER_VALUE]);
				break;
			case BPS_MODE_SETPOINT:
				memcpy(&sd->setpoint, &sd->UARTData.content, sizeof(bpsCircleTypeDef));
				xTaskNotifyGive(taskNumber[TASK_CAL_REF_ENCODER_VALUE]);
				break;
			case BPS_MODE_CIRCLE:
				bpsCalSetpoint4CircleMode(	sd->UARTData.content.circleProperties.centerCoordinate[BPS_X_AXIS], 
					sd->UARTData.content.circleProperties.radius, &sd->UARTData.content.circleProperties.currentAngle,
					sd->UARTData.content.circleProperties.speed, &sd->setpoint[BPS_X_AXIS], BPS_X_AXIS);
				bpsCalSetpoint4CircleMode(	sd->UARTData.content.circleProperties.centerCoordinate[BPS_Y_AXIS], 
					sd->UARTData.content.circleProperties.radius, &sd->UARTData.content.circleProperties.currentAngle,
					sd->UARTData.content.circleProperties.speed, &sd->setpoint[BPS_Y_AXIS], BPS_Y_AXIS);		
				xTaskNotifyGive(taskNumber[TASK_CAL_REF_ENCODER_VALUE]);
				break;
			case BPS_MODE_RECTANGLE:
				xTaskNotifyGive(taskNumber[TASK_CAL_REF_ENCODER_VALUE]);
				break;
			default:
				break;
		}
	}
}

void bpsTaskCalRefEncoderValue(void *pointer)
{
	bpsSharedDataTypeDef* sd = (bpsSharedDataTypeDef*)pointer;
	while(1)
	{
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
		bpsCalculatePID(sd->setpoint[BPS_X_AXIS], sd->UARTData.ballCoordinate[BPS_X_AXIS], sd->PIDParams.Kp[BPS_OUTER_PID][BPS_X_AXIS], 
						sd->PIDParams.Ki[BPS_OUTER_PID][BPS_X_AXIS], sd->PIDParams.Kd[BPS_OUTER_PID][BPS_X_AXIS], 
						&sd->errorSamples[BPS_OUTER_PID][BPS_X_AXIS][0], &sd->PIDSamples[BPS_OUTER_PID][BPS_X_AXIS][0],
						DT_OUTER_LOOP);
		bpsCalculatePID(sd->setpoint[BPS_Y_AXIS], sd->UARTData.ballCoordinate[BPS_Y_AXIS], sd->PIDParams.Kp[BPS_OUTER_PID][BPS_Y_AXIS], 
						sd->PIDParams.Ki[BPS_OUTER_PID][BPS_Y_AXIS], sd->PIDParams.Kd[BPS_OUTER_PID][BPS_Y_AXIS], 
						&sd->errorSamples[BPS_OUTER_PID][BPS_Y_AXIS][0], &sd->PIDSamples[BPS_OUTER_PID][BPS_Y_AXIS][0],
						DT_OUTER_LOOP);
		sd->refEncoderValue[BPS_X_AXIS] = sd->PIDSamples[BPS_OUTER_PID][BPS_X_AXIS][0] + encX;
		sd->refEncoderValue[BPS_Y_AXIS] = sd->PIDSamples[BPS_OUTER_PID][BPS_Y_AXIS][0] + encY;
		xTaskNotifyGive(taskNumber[TASK_CONTROL_MOTOR]);
	}
}

void bpsTaskControlMotor(void* pointer)
{
	bpsSharedDataTypeDef* sd = (bpsSharedDataTypeDef*)pointer;
	bpsStartPWM();
	bpsStartEncoder();
	//int16_t encoderValue; comment for debug
	while(1)
	{
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
		bpsReadEncoderCnt(BPS_X_AXIS, &encoderValue);
		bpsCalculatePID(sd->refEncoderValue[BPS_X_AXIS], encoderValue, sd->PIDParams.Kp[BPS_INNER_PID][BPS_X_AXIS], 
						sd->PIDParams.Ki[BPS_INNER_PID][BPS_X_AXIS], sd->PIDParams.Kd[BPS_INNER_PID][BPS_X_AXIS], 
						&sd->errorSamples[BPS_INNER_PID][BPS_X_AXIS][0], &sd->PIDSamples[BPS_INNER_PID][BPS_X_AXIS][0],
						DT_INNER_LOOP);
		bpsReadEncoderCnt(BPS_Y_AXIS, &encoderValue);
		bpsCalculatePID(sd->refEncoderValue[BPS_Y_AXIS], encoderValue, sd->PIDParams.Kp[BPS_INNER_PID][BPS_Y_AXIS], 
						sd->PIDParams.Ki[BPS_INNER_PID][BPS_Y_AXIS], sd->PIDParams.Kd[BPS_INNER_PID][BPS_Y_AXIS], 
						&sd->errorSamples[BPS_INNER_PID][BPS_Y_AXIS][0], &sd->PIDSamples[BPS_INNER_PID][BPS_Y_AXIS][0],
						DT_INNER_LOOP);				
		bpsControlMotor(BPS_X_AXIS, sd->PIDSamples[BPS_INNER_PID][BPS_X_AXIS][0]);
		bpsControlMotor(BPS_Y_AXIS, sd->PIDSamples[BPS_INNER_PID][BPS_Y_AXIS][0]);
		if (delay1ms)
			vTaskDelay(pdMS_TO_TICKS(1));
		delay1ms = true;
		xTaskNotifyGive(taskNumber[TASK_UPDATE_SETPOINT]);
	}
}

void bpsTaskUARTSendData(void* pointer)
{
	memset((void*)&sendData, 0 , sizeof(bpsUARTSendDataTypeDef));
	int16_t temp;
	while(1)
	{
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
		bpsReadEncoderCnt(BPS_X_AXIS, &temp);
		sendData.encoderCnt[BPS_X_AXIS] = temp;
		bpsReadEncoderCnt(BPS_Y_AXIS, &temp);
		sendData.encoderCnt[BPS_Y_AXIS] = temp;
		bpsUARTSendData(&sendData);
	}
}

void bpsTaskSetup(void *pointer)
{
	//int16_t encX, encX1, encX2, encY, encY1, encY2;
	bpsSharedDataTypeDef* sd = (bpsSharedDataTypeDef*)pointer;
	sd->UARTData.command = BPS_UPDATE_PID;
	sd->UARTData.content.PIDProperties.Kp[BPS_OUTER_PID][BPS_X_AXIS] = 10;
	sd->UARTData.content.PIDProperties.Ki[BPS_OUTER_PID][BPS_X_AXIS] = 10;
	sd->UARTData.content.PIDProperties.Kd[BPS_OUTER_PID][BPS_X_AXIS] = 0.1;

	sd->UARTData.content.PIDProperties.Kp[BPS_OUTER_PID][BPS_Y_AXIS] = 10;
	sd->UARTData.content.PIDProperties.Ki[BPS_OUTER_PID][BPS_Y_AXIS] = 10;
	sd->UARTData.content.PIDProperties.Kd[BPS_OUTER_PID][BPS_Y_AXIS] = 0.1;

	sd->UARTData.content.PIDProperties.Kp[BPS_INNER_PID][BPS_X_AXIS] = 48;
	sd->UARTData.content.PIDProperties.Ki[BPS_INNER_PID][BPS_X_AXIS] = 312;
	sd->UARTData.content.PIDProperties.Kd[BPS_INNER_PID][BPS_X_AXIS] = 1;

	sd->UARTData.content.PIDProperties.Kp[BPS_INNER_PID][BPS_Y_AXIS] = 48;
	sd->UARTData.content.PIDProperties.Ki[BPS_INNER_PID][BPS_Y_AXIS] = 312;
	sd->UARTData.content.PIDProperties.Kd[BPS_INNER_PID][BPS_Y_AXIS] = 1;
	xTaskNotifyGive(taskNumber[TASK_UPDATE_SETPOINT]);
	vTaskDelay(pdMS_TO_TICKS(11));
	sd->UARTData.ballCoordinate[BPS_X_AXIS] = 240;
	sd->UARTData.ballCoordinate[BPS_Y_AXIS] = 240;
	sd->UARTData.command = BPS_MODE_SETPOINT;
	sd->UARTData.content.pointProperties.setpointCoordinate[BPS_X_AXIS] = 240;
	sd->UARTData.content.pointProperties.setpointCoordinate[BPS_Y_AXIS] = 240;
	xTaskNotifyGive(taskNumber[TASK_UPDATE_SETPOINT]);
	vTaskDelay(pdMS_TO_TICKS(11));

	bpsFindThresholds(BPS_X_AXIS, &sd->thresholds[BPS_X_AXIS][BPS_MIN], &sd->thresholds[BPS_X_AXIS][BPS_MAX]);
	encX = (sd->thresholds[BPS_X_AXIS][BPS_MIN] + sd->thresholds[BPS_X_AXIS][BPS_MAX])/2;
	HAL_Delay(100);

	bpsFindThresholds(BPS_Y_AXIS, &sd->thresholds[BPS_Y_AXIS][BPS_MIN], &sd->thresholds[BPS_Y_AXIS][BPS_MAX]);
	encY = (sd->thresholds[BPS_Y_AXIS][BPS_MIN] + sd->thresholds[BPS_Y_AXIS][BPS_MAX])/2;
	
	bpsControlMotor(BPS_X_AXIS, 0);
	bpsControlMotor(BPS_Y_AXIS, 0);
	do {
	//do {
		bpsReadEncoderCnt(BPS_X_AXIS, &encoderValue);
		bpsCalculatePID(encX, encoderValue, sd->PIDParams.Kp[BPS_INNER_PID][BPS_X_AXIS], 
						sd->PIDParams.Ki[BPS_INNER_PID][BPS_X_AXIS], sd->PIDParams.Kd[BPS_INNER_PID][BPS_X_AXIS], 
						&sd->errorSamples[BPS_INNER_PID][BPS_X_AXIS][0], &sd->PIDSamples[BPS_INNER_PID][BPS_X_AXIS][0],
						DT_INNER_LOOP);
		bpsControlMotor(BPS_X_AXIS, sd->PIDSamples[BPS_INNER_PID][BPS_X_AXIS][0]);
	//	HAL_Delay(1);
	//} while( sd->errorSamples[BPS_INNER_PID][BPS_X_AXIS][0] != 0);

	//do {
		bpsReadEncoderCnt(BPS_Y_AXIS, &encoderValue);
		bpsCalculatePID(encY, encoderValue, sd->PIDParams.Kp[BPS_INNER_PID][BPS_Y_AXIS], 
						sd->PIDParams.Ki[BPS_INNER_PID][BPS_Y_AXIS], sd->PIDParams.Kd[BPS_INNER_PID][BPS_Y_AXIS], 
						&sd->errorSamples[BPS_INNER_PID][BPS_Y_AXIS][0], &sd->PIDSamples[BPS_INNER_PID][BPS_Y_AXIS][0],
						DT_INNER_LOOP);		
		bpsControlMotor(BPS_Y_AXIS, sd->PIDSamples[BPS_INNER_PID][BPS_Y_AXIS][0]);	
		HAL_Delay(1);
	//} while( sd->errorSamples[BPS_INNER_PID][BPS_Y_AXIS][0] != 0);
	} while (sd->errorSamples[BPS_INNER_PID][BPS_X_AXIS][0] != 0 && sd->errorSamples[BPS_INNER_PID][BPS_Y_AXIS][0] != 0);
	bpsControlMotor(BPS_X_AXIS, 0);
	bpsControlMotor(BPS_Y_AXIS, 0);
	//vTaskPrioritySet(taskNumber[TASK_SETUP],0);
	vTaskDelay(portMAX_DELAY);
	while(1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == RECEIVE_DATA_HANDLE.Instance)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		vTaskNotifyGiveFromISR(taskNumber[TASK_UPDATE_UART_DATA], &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		// Give notify for another task to update new receive data
	}
}


void bpsBallAndPlateSystemStart()
{
	xTaskCreate(bpsTaskSetup, "Setup Task", 500, (void*)&sharedData, 6, &taskNumber[TASK_SETUP]);
    xTaskCreate(bpsTaskUpdateUARTData, "Update UART Data", 500, (void*)&sharedData, 5, &taskNumber[TASK_UPDATE_UART_DATA]);
    xTaskCreate(bpsTaskUpdateSetpoint, "Update Setpoint", 500, (void*)&sharedData, 4, &taskNumber[TASK_UPDATE_SETPOINT]);
	xTaskCreate(bpsTaskCalRefEncoderValue, "Calculate reference Encoder Value", 500, (void*)&sharedData, 3, 
				&taskNumber[TASK_CAL_REF_ENCODER_VALUE]);
    xTaskCreate(bpsTaskControlMotor, "Control Motor", 500, (void*)&sharedData, 2, &taskNumber[TASK_CONTROL_MOTOR]);
    xTaskCreate(bpsTaskUARTSendData, "Example send data", 500, NULL,1, &taskNumber[TASK_SEND_UART_DATA]);
}
