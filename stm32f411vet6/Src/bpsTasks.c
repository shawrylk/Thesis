
#include "bpsTasks.h"

// remember to clean sendData and sharedData after done
bpsUARTSendDataTypeDef 	sendData;
bpsSharedDataTypeDef 	sharedData;
TaskHandle_t   			taskNumber[NUMBER_OF_TASK];
int16_t encoderValue;
int16_t encX = 0, encY = 0;
BOOL delay1ms = false;
int timeElapse = TIME_FOR_A_RECT;

void bpsTaskUpdateUARTData(void* pointer)
{
	bpsSharedDataTypeDef* sd = (bpsSharedDataTypeDef*)pointer;
	bpsUARTReceiveDataTypeDef receiveData;
	bpsUARTReceiveData(&receiveData);	
	//sizeofUARTData = sizeof(bpsUARTSendDataTypeDef);
	//ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
	while(1)
	{
		//notify take from receive UART data completely callback
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
		// copy data directly, no need to use mutex to protect shared data 
		//because tasks run in turn and all function here are blocking
		// so everything is synchronous
		memcpy(&sd->UARTData, &receiveData, sizeof(bpsUARTReceiveDataTypeDef));		
		delay1ms = false;
		xTaskNotifyGive(taskNumber[TASK_UPDATE_SETPOINT]);
		//xTaskNotifyGive(taskNumber[TASK_SEND_UART_DATA]);
	}
}

void bpsTaskUpdateSetpoint(void* pointer)
{
	bpsSharedDataTypeDef* sd = (bpsSharedDataTypeDef*)pointer;
	float currentAngle = 0;
	//int timeElapse = TIME_FOR_A_RECT;
	while(1)
	{
		// wait notification from update UART data task
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
				//memset(&sd->integralSum, 0, sizeof(sd->integralSum));
				xTaskNotifyGive(taskNumber[TASK_CAL_REF_ENCODER_VALUE]);
				break;
			case BPS_MODE_SETPOINT:
				if (delay1ms)
					xTaskNotifyGive(taskNumber[TASK_CONTROL_MOTOR]);
				else
				{
					memcpy(&sd->setpoint, &sd->UARTData.content, sizeof(bpsPointTypeDef));
					sd->setpoint[BPS_X_AXIS] = 480 - sd->setpoint[BPS_X_AXIS];
					xTaskNotifyGive(taskNumber[TASK_CAL_REF_ENCODER_VALUE]);
				}
				break;
			case BPS_MODE_CIRCLE:
				if (delay1ms)
					xTaskNotifyGive(taskNumber[TASK_CONTROL_MOTOR]);
				else
				{	
					bpsCalSetpoint4CircleMode(480 -	sd->UARTData.content.circleProperties.centerCoordinate[BPS_X_AXIS], 
					sd->UARTData.content.circleProperties.radius, &currentAngle,
					sd->UARTData.content.circleProperties.speed, &sd->setpoint[BPS_X_AXIS], BPS_X_AXIS);
					bpsCalSetpoint4CircleMode(sd->UARTData.content.circleProperties.centerCoordinate[BPS_Y_AXIS], 
					sd->UARTData.content.circleProperties.radius, &currentAngle,
					sd->UARTData.content.circleProperties.speed, &sd->setpoint[BPS_Y_AXIS], BPS_Y_AXIS);	
					xTaskNotifyGive(taskNumber[TASK_CAL_REF_ENCODER_VALUE]);
				}
				break;
			case BPS_MODE_RECTANGLE:
				if (delay1ms)
					xTaskNotifyGive(taskNumber[TASK_CONTROL_MOTOR]);
				else
				{	
					bpsCalSetpoint4RectMode(&sd->UARTData.content.rectangleProperties, &timeElapse, &sd->setpoint[BPS_X_AXIS]);
					xTaskNotifyGive(taskNumber[TASK_CAL_REF_ENCODER_VALUE]);
				}
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
		// wait notification from update setpoint task
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
		if (sd->UARTData.detectedBall)
		{

			bpsDiscretePID(sd->setpoint[BPS_X_AXIS], 480 - sd->UARTData.ballCoordinate[BPS_X_AXIS], 
							sd->PIDParams.Kp[BPS_OUTER_PID][BPS_X_AXIS], 
							sd->PIDParams.Ki[BPS_OUTER_PID][BPS_X_AXIS], sd->PIDParams.Kd[BPS_OUTER_PID][BPS_X_AXIS], 
							&sd->errorSamples[BPS_OUTER_PID][BPS_X_AXIS][0], &sd->PIDSamples[BPS_OUTER_PID][BPS_X_AXIS][0],
							DT_OUTER_LOOP, false);
			bpsDiscretePID(sd->setpoint[BPS_Y_AXIS], sd->UARTData.ballCoordinate[BPS_Y_AXIS],
							sd->PIDParams.Kp[BPS_OUTER_PID][BPS_Y_AXIS], 
							sd->PIDParams.Ki[BPS_OUTER_PID][BPS_Y_AXIS], sd->PIDParams.Kd[BPS_OUTER_PID][BPS_Y_AXIS], 
							&sd->errorSamples[BPS_OUTER_PID][BPS_Y_AXIS][0], &sd->PIDSamples[BPS_OUTER_PID][BPS_Y_AXIS][0],
							DT_OUTER_LOOP, false);			
			sd->encoderCntRef[BPS_X_AXIS] = encoderSaturation(sd->PIDSamples[BPS_OUTER_PID][BPS_X_AXIS][0]);
			sd->encoderCntRef[BPS_Y_AXIS] = encoderSaturation(sd->PIDSamples[BPS_OUTER_PID][BPS_Y_AXIS][0]);
		}
		else
		{
			sd->encoderCntRef[BPS_X_AXIS] = 0;
			sd->encoderCntRef[BPS_Y_AXIS] = 0;
			memset(&sd->errorSamples, 0, sizeof(sd->errorSamples));
			memset(&sd->PIDSamples, 0, sizeof(sd->PIDSamples));
		}
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
		// wait notification from calculate reference encoder value task
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
		
		bpsReadEncoderCnt(BPS_X_AXIS, &encX);
		bpsDiscretePID(sd->encoderCntRef[BPS_X_AXIS], encX, 
						sd->PIDParams.Kp[BPS_INNER_PID][BPS_X_AXIS], 
						sd->PIDParams.Ki[BPS_INNER_PID][BPS_X_AXIS], sd->PIDParams.Kd[BPS_INNER_PID][BPS_X_AXIS], 
						&sd->errorSamples[BPS_INNER_PID][BPS_X_AXIS][0], &sd->PIDSamples[BPS_INNER_PID][BPS_X_AXIS][0],
						DT_INNER_LOOP, true);
		if (sd->PIDSamples[BPS_INNER_PID][BPS_X_AXIS][0] > 40 || sd->PIDSamples[BPS_INNER_PID][BPS_X_AXIS][0] < -40)
			bpsControlMotor(BPS_X_AXIS, sd->PIDSamples[BPS_INNER_PID][BPS_X_AXIS][0]);
		else
			bpsControlMotor(BPS_X_AXIS, 0);

		bpsReadEncoderCnt(BPS_Y_AXIS, &encY);
		bpsDiscretePID(sd->encoderCntRef[BPS_Y_AXIS], encY, 
						sd->PIDParams.Kp[BPS_INNER_PID][BPS_Y_AXIS], 
						sd->PIDParams.Ki[BPS_INNER_PID][BPS_Y_AXIS], sd->PIDParams.Kd[BPS_INNER_PID][BPS_Y_AXIS], 
						&sd->errorSamples[BPS_INNER_PID][BPS_Y_AXIS][0], &sd->PIDSamples[BPS_INNER_PID][BPS_Y_AXIS][0],
						DT_INNER_LOOP, true);	
		if (sd->PIDSamples[BPS_INNER_PID][BPS_Y_AXIS][0] > 40 || sd->PIDSamples[BPS_INNER_PID][BPS_Y_AXIS][0] < -40)
			bpsControlMotor(BPS_Y_AXIS, sd->PIDSamples[BPS_INNER_PID][BPS_Y_AXIS][0]);	
		else
			bpsControlMotor(BPS_Y_AXIS, 0);	
		if (delay1ms)
			vTaskDelay(pdMS_TO_TICKS(1));
		delay1ms = true;
		xTaskNotifyGive(taskNumber[TASK_UPDATE_SETPOINT]);
	}
}

// void bpsTaskUARTSendData(void* pointer)
// {
// 	memset((void*)&sendData, 0 , sizeof(bpsUARTSendDataTypeDef));
// 	int16_t temp;
// 	while(1)
// 	{
// 		// wait for notification from update UART data task
// 		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
// 		// it can get here because of control motor task is blocked
// 		bpsReadEncoderCnt(BPS_X_AXIS, &temp);
// 		sendData.encoderCnt[BPS_X_AXIS] = temp;
// 		bpsReadEncoderCnt(BPS_Y_AXIS, &temp);
// 		sendData.encoderCnt[BPS_Y_AXIS] = temp;
// 		bpsUARTSendData(&sendData);
// 	}
// }

void bpsTaskSetup(void *pointer)
{
	
	//this task run first, then blocked forever
	bpsSharedDataTypeDef* sd = (bpsSharedDataTypeDef*)pointer;
	sd->UARTData.command = BPS_UPDATE_PID;
	sd->UARTData.content.PIDProperties.Kp[BPS_OUTER_PID][BPS_X_AXIS] = 1.9;
	sd->UARTData.content.PIDProperties.Ki[BPS_OUTER_PID][BPS_X_AXIS] = 0.6;
	sd->UARTData.content.PIDProperties.Kd[BPS_OUTER_PID][BPS_X_AXIS] = 0.7;

	sd->UARTData.content.PIDProperties.Kp[BPS_OUTER_PID][BPS_Y_AXIS] = 2;
	sd->UARTData.content.PIDProperties.Ki[BPS_OUTER_PID][BPS_Y_AXIS] = 0.6;
	sd->UARTData.content.PIDProperties.Kd[BPS_OUTER_PID][BPS_Y_AXIS] = 0.65;

	sd->UARTData.content.PIDProperties.Kp[BPS_INNER_PID][BPS_X_AXIS] = 0.75;
	sd->UARTData.content.PIDProperties.Ki[BPS_INNER_PID][BPS_X_AXIS] = 0;
	sd->UARTData.content.PIDProperties.Kd[BPS_INNER_PID][BPS_X_AXIS] = 0;

	sd->UARTData.content.PIDProperties.Kp[BPS_INNER_PID][BPS_Y_AXIS] = 0.7;
	sd->UARTData.content.PIDProperties.Ki[BPS_INNER_PID][BPS_Y_AXIS] = 0;
	sd->UARTData.content.PIDProperties.Kd[BPS_INNER_PID][BPS_Y_AXIS] = 0;

	xTaskNotifyGive(taskNumber[TASK_UPDATE_SETPOINT]);
	vTaskDelay(pdMS_TO_TICKS(11));
	sd->UARTData.detectedBall = 0;
	sd->UARTData.ballCoordinate[BPS_X_AXIS] = 240;
	sd->UARTData.ballCoordinate[BPS_Y_AXIS] = 240;
	// sd->UARTData.command = BPS_MODE_SETPOINT;
	// sd->UARTData.content.pointProperties.setpointCoordinate[BPS_X_AXIS] = 240;
	// sd->UARTData.content.pointProperties.setpointCoordinate[BPS_Y_AXIS] = 240;
	// sd->UARTData.command = BPS_MODE_CIRCLE;
	// sd->UARTData.content.circleProperties.centerCoordinate[BPS_X_AXIS] = 200;
	// sd->UARTData.content.circleProperties.centerCoordinate[BPS_Y_AXIS] = 200;
	// sd->UARTData.content.circleProperties.radius	= 40;
	// sd->UARTData.content.circleProperties.speed		= 1;
	xTaskNotifyGive(taskNumber[TASK_UPDATE_SETPOINT]);
	vTaskDelay(pdMS_TO_TICKS(11));
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
		// Give notify for update UART data task to update new receive data
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
    //xTaskCreate(bpsTaskUARTSendData, "Example send data", 500, NULL,1, &taskNumber[TASK_SEND_UART_DATA]);
}
