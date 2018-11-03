
#include "bpsTasks.h"

// remember to clean sendData and sharedData after done
bpsUARTSendDataTypeDef 	sendData;
bpsSharedDataTypeDef 	sharedData;
TaskHandle_t   			taskNumber[NUMBER_OF_TASK];
int16_t encoderValue;
//int16_t sizeofUARTData;
// void bpsTaskPWMGenerator(void* pointer)
// {
// 	bpsStartPWM();
// 	int16_t duty = 0, step = 100;
// 	while(1)
// 	{
// 		if (bpsControlMotor(BPS_X_AXIS, duty+=step) == HAL_OK)
// 			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,GPIO_PIN_RESET);
// 		if (bpsControlMotor(BPS_Y_AXIS, duty+=step) == HAL_OK)
// 			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_RESET);
// 		if (duty == 9600)
// 			duty = 0;
// 		vTaskDelay(50);
// 	}
// }


// void bpsTaskReadEncoder(void* pointer)
// {
	// bpsUARTSendDataTypeDef sendData;
	// bpsStartEncoder();
	// while(1)
	// {
	// 	bpsReadEncoderCnt(BPS_X_AXIS, &sendData.encoderCnt[BPS_X_AXIS]);
	// 	bpsReadEncoderCnt(BPS_Y_AXIS, &sendData.encoderCnt[BPS_Y_AXIS]);
	// 	bpsUARTSendData(&sendData);
	// }
//}

void bpsTaskUpdateUARTData(void* pointer)
{
	bpsSharedDataTypeDef* sd = (bpsSharedDataTypeDef*)pointer;
	bpsUARTReceiveDataTypeDef receiveData;
	bpsUARTReceiveData(&receiveData);	
	//sizeofUARTData = sizeof(bpsUARTSendDataTypeDef);
	ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
	xTaskNotifyGive(taskNumber[TASK_SEND_UART_DATA]);
	while(1)
	{
		//notify take from task control motor
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
		memcpy(&sd->UARTData, &receiveData, sizeof(bpsUARTReceiveDataTypeDef));		
		xTaskNotifyGive(taskNumber[TASK_UPDATE_SETPOINT]);
	}
}

void bpsTaskUpdateSetpoint(void* pointer)
{
	bpsSharedDataTypeDef* sd = (bpsSharedDataTypeDef*)pointer;
	while(1)
	{
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
		memcpy(&sd->ballCoordinate, &sd->UARTData.ballCoordinate, sizeof(sd->UARTData.ballCoordinate));
		if (sd->UARTData.command != BPS_MODE_DEFAULT)
			sd->latchedCommand = sd->UARTData.command;
		switch(sd->latchedCommand)
		{
			case BPS_UPDATE_PID:
				// led for debug
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
				if (sd->UARTData.command == BPS_UPDATE_PID)
					memcpy(&sd->PIDParams, &sd->UARTData.content, sizeof(bpsPIDTypeDef));
				// clear errorSamples
				memset(&sd->errorSamples, 0, sizeof(sd->errorSamples));
				// clear PIDSamples
				memset(&sd->PIDSamples, 0, sizeof(sd->PIDSamples));
				//xTaskNotifyGive(taskNumber[TASK_CAL_REF_ENCODER_VALUE]);
				break;
			case BPS_MODE_SETPOINT:
				if (sd->UARTData.command == BPS_MODE_SETPOINT)
					memcpy(&sd->setpoint, &sd->UARTData.content, sizeof(bpsPointTypeDef));
				xTaskNotifyGive(taskNumber[TASK_CAL_REF_ENCODER_VALUE]);
				break;
			case BPS_MODE_CIRCLE:
				if (sd->UARTData.command == BPS_MODE_CIRCLE)
					memcpy(&sd->content, &sd->UARTData.content, sizeof(bpsCircleTypeDef));
				bpsCalSetpoint4CircleMode(	sd->content.circleProperties.centerCoordinate[BPS_X_AXIS], 
													sd->content.circleProperties.radius, &sd->content.circleProperties.currentAngle,
													sd->content.circleProperties.speed, &sd->setpoint[BPS_X_AXIS], BPS_X_AXIS);
				bpsCalSetpoint4CircleMode(	sd->content.circleProperties.centerCoordinate[BPS_Y_AXIS], 
													sd->content.circleProperties.radius, &sd->content.circleProperties.currentAngle,
													sd->content.circleProperties.speed, &sd->setpoint[BPS_Y_AXIS], BPS_Y_AXIS);		
				xTaskNotifyGive(taskNumber[TASK_CAL_REF_ENCODER_VALUE]);
				break;
			case BPS_MODE_RECTANGLE:
				if (sd->UARTData.command == BPS_MODE_RECTANGLE)
					memcpy(&sd->content, &sd->UARTData.content, sizeof(bpsRectangleTypeDef));
				xTaskNotifyGive(taskNumber[TASK_CAL_REF_ENCODER_VALUE]);
				break;
			case BPS_MODE_DEFAULT:
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
		bpsCalculatePID(sd->setpoint[BPS_X_AXIS], sd->ballCoordinate[BPS_X_AXIS], sd->PIDParams.Kp[BPS_OUTER_PID][BPS_X_AXIS], 
						sd->PIDParams.Ki[BPS_OUTER_PID][BPS_X_AXIS], sd->PIDParams.Kd[BPS_OUTER_PID][BPS_X_AXIS], 
						&sd->errorSamples[BPS_OUTER_PID][BPS_X_AXIS][0], &sd->PIDSamples[BPS_OUTER_PID][BPS_X_AXIS][0],
						DT_OUTER_LOOP);
		bpsCalculatePID(sd->setpoint[BPS_Y_AXIS], sd->ballCoordinate[BPS_Y_AXIS], sd->PIDParams.Kp[BPS_OUTER_PID][BPS_Y_AXIS], 
						sd->PIDParams.Ki[BPS_OUTER_PID][BPS_Y_AXIS], sd->PIDParams.Kd[BPS_OUTER_PID][BPS_Y_AXIS], 
						&sd->errorSamples[BPS_OUTER_PID][BPS_Y_AXIS][0], &sd->PIDSamples[BPS_OUTER_PID][BPS_Y_AXIS][0],
						DT_OUTER_LOOP);
		sd->refEncoderValue[BPS_X_AXIS] = sd->PIDSamples[BPS_OUTER_PID][BPS_X_AXIS][0];
		sd->refEncoderValue[BPS_Y_AXIS] = sd->PIDSamples[BPS_OUTER_PID][BPS_Y_AXIS][0];
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
		//vTaskDelay(pdMS_TO_TICKS(1));
		xTaskNotifyGive(taskNumber[TASK_UPDATE_SETPOINT]);
	}
}

void bpsTaskUARTSendData(void* pointer)
{
	//using Ziegler–Nichols method with Ku = 70, Tu = 120ms, Kp = 0.6Ku, Ki = 1.2Ku/Tu, Kp = 0.075*Ku*Tu
	//bpsUARTSendDataTypeDef 	sendData;
	//bpsPIDTypeDef			PID = {{{10,0},{30,0}},{{0,0},{0,0}},{{0,0},{0,0}}};
	//uint8_t cnt = 0;
	//	bpsPIDTypeDef			PID = {	.Kp[BPS_OUTER_PID][BPS_X_AXIS] = 0.05,
	//									.Ki[BPS_OUTER_PID][BPS_X_AXIS] = 0,
	//									.Kd[BPS_OUTER_PID][BPS_X_AXIS] = 0.03,

	//									.Kp[BPS_OUTER_PID][BPS_Y_AXIS] = 20,
	//									.Ki[BPS_OUTER_PID][BPS_Y_AXIS] = 0,
	//									.Kd[BPS_OUTER_PID][BPS_Y_AXIS] = 0,

	//									.Kp[BPS_INNER_PID][BPS_X_AXIS] = 1,
	//									.Ki[BPS_INNER_PID][BPS_X_AXIS] = 230,
	//									.Kd[BPS_INNER_PID][BPS_X_AXIS] = 0.56,

	//									.Kp[BPS_INNER_PID][BPS_Y_AXIS] = 25,
	//									.Ki[BPS_INNER_PID][BPS_Y_AXIS] = 0,
	//									.Kd[BPS_INNER_PID][BPS_Y_AXIS] = 0};

	//	sendData.ballCoordinate[BPS_X_AXIS] = 0;
	//	sendData.ballCoordinate[BPS_Y_AXIS] = 0;
	//	memcpy(&sendData.content, &PID, sizeof(bpsPIDTypeDef)); 
	//	sendData.command = BPS_UPDATE_PID;
	//	bpsUARTSendData(&sendData);

	//	sendData.ballCoordinate[BPS_X_AXIS] = 0x22;
	//	sendData.ballCoordinate[BPS_Y_AXIS] = 0x22;
	//	sendData.content.pointProperties.setpointCoordinate[BPS_X_AXIS] = 0x55;
	//	sendData.content.pointProperties.setpointCoordinate[BPS_Y_AXIS] = 0x55;
	//	sendData.command = BPS_MODE_SETPOINT;
	//	bpsUARTSendData(&sendData);
	//	vTaskDelay(pdMS_TO_TICKS(16));
	ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
	memset((void*)&sendData, 0 , sizeof(bpsUARTSendDataTypeDef));
	while(1)
	{
			//vTaskDelay(pdMS_TO_TICKS(16));
	//		sendData.ballCoordinate[BPS_X_AXIS] = 0xFF;
	//		sendData.ballCoordinate[BPS_Y_AXIS] = 0xFF;
	//		sendData.command = BPS_MODE_DEFAULT;
	//		bpsUARTSendData(&sendData);
//		sendData.ballCoordinate[BPS_X_AXIS] = 0x11;
//		sendData.ballCoordinate[BPS_Y_AXIS] = 0x22;
//		sendData.content.pointProperties.setpointCoordinate[BPS_X_AXIS] = 0x33;
//		sendData.content.pointProperties.setpointCoordinate[BPS_Y_AXIS] = 0x44;
//		sendData.command = BPS_MODE_SETPOINT;
		sendData.encoderCnt[BPS_X_AXIS] = 0x12;
		sendData.encoderCnt[BPS_Y_AXIS] = 0x34;
		bpsUARTSendData(&sendData);
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

void bpsTaskSetup(void *pointer)
{
	bpsSharedDataTypeDef* sd = (bpsSharedDataTypeDef*)pointer;
	sd->UARTData.command = BPS_UPDATE_PID;
	sd->UARTData.content.PIDProperties.Kp[BPS_OUTER_PID][BPS_X_AXIS] = 10;
	sd->UARTData.content.PIDProperties.Ki[BPS_OUTER_PID][BPS_X_AXIS] = 0;
	sd->UARTData.content.PIDProperties.Kd[BPS_OUTER_PID][BPS_X_AXIS] = 0;

	sd->UARTData.content.PIDProperties.Kp[BPS_OUTER_PID][BPS_Y_AXIS] = 10;
	sd->UARTData.content.PIDProperties.Ki[BPS_OUTER_PID][BPS_Y_AXIS] = 0;
	sd->UARTData.content.PIDProperties.Kd[BPS_OUTER_PID][BPS_Y_AXIS] = 0;

	sd->UARTData.content.PIDProperties.Kp[BPS_INNER_PID][BPS_X_AXIS] = 10;
	sd->UARTData.content.PIDProperties.Ki[BPS_INNER_PID][BPS_X_AXIS] = 0;
	sd->UARTData.content.PIDProperties.Kd[BPS_INNER_PID][BPS_X_AXIS] = 0;

	sd->UARTData.content.PIDProperties.Kp[BPS_INNER_PID][BPS_Y_AXIS] = 10;
	sd->UARTData.content.PIDProperties.Ki[BPS_INNER_PID][BPS_Y_AXIS] = 0;
	sd->UARTData.content.PIDProperties.Kd[BPS_INNER_PID][BPS_Y_AXIS] = 0;
	xTaskNotifyGive(taskNumber[TASK_UPDATE_SETPOINT]);
	vTaskDelay(pdMS_TO_TICKS(DT_OUTER_LOOP));
	sd->UARTData.ballCoordinate[BPS_X_AXIS] = 240;
	sd->UARTData.ballCoordinate[BPS_Y_AXIS] = 240;
	sd->UARTData.command = BPS_MODE_SETPOINT;
	sd->UARTData.content.pointProperties.setpointCoordinate[BPS_X_AXIS] = 240;
	sd->UARTData.content.pointProperties.setpointCoordinate[BPS_X_AXIS] = 240;
	xTaskNotifyGive(taskNumber[TASK_UPDATE_SETPOINT]);
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
    xTaskCreate(bpsTaskUpdateUARTData, "Update UART Data", 500, (void*)&sharedData, 5, &taskNumber[TASK_UPDATE_UART_DATA]);
    xTaskCreate(bpsTaskUpdateSetpoint, "Update Setpoint", 500, (void*)&sharedData, 4, &taskNumber[TASK_UPDATE_SETPOINT]);
	xTaskCreate(bpsTaskCalRefEncoderValue, "Calculate reference Encoder Value", 500, (void*)&sharedData, 3, 
				&taskNumber[TASK_CAL_REF_ENCODER_VALUE]);
    xTaskCreate(bpsTaskControlMotor, "Control Motor", 500, (void*)&sharedData, 2, &taskNumber[TASK_CONTROL_MOTOR]);
  	//xTaskCreate(bpsTaskPWMGenerator, "PWM Generator", 300, (void*)&sharedData, 1, NULL);
	  //xTaskCreate(bpsTaskReadEncoder, "Read Encoder", 300, (void*)&sharedData, 1, NULL);
    xTaskCreate(bpsTaskUARTSendData, "Example send data", 500, NULL,1, &taskNumber[TASK_SEND_UART_DATA]);
	xTaskCreate(bpsTaskSetup, "Setup Task", 100, NULL, 0, NULL);

}
