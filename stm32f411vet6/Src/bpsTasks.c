
#include "bpsTasks.h"

// remember to clean sendData and sharedData after done
bpsUARTSendDataTypeDef 	sendData;
bpsSharedDataTypeDef 	sharedData;
TaskHandle_t   			taskNumber[NUMBER_OF_TASK];
int16_t encoderValue;
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
		switch(sd->UARTData.command)
		{
			case BPS_UPDATE_PID:
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
				memcpy(&sd->PIDs, &sd->UARTData.content, sizeof(bpsPIDTypeDef));
				// clear errorSamples
				memset(&sd->errorSamples, 0, sizeof(sd->errorSamples));
				// clear PIDSamples
				memset(&sd->PIDSamples, 0, sizeof(sd->PIDSamples));
				sd->previousCommand = sd->UARTData.command;
				//xTaskNotifyGive(taskNumber[TASK_CAL_REF_ENCODER_VALUE]);
				break;
			case BPS_MODE_SETPOINT:
				memcpy(&sd->setpoint, &sd->UARTData.content, sizeof(bpsPointTypeDef));
				sd->previousCommand = sd->UARTData.command;
				xTaskNotifyGive(taskNumber[TASK_CAL_REF_ENCODER_VALUE]);
				break;
			case BPS_MODE_CIRCLE:
				memcpy(&sd->content, &sd->UARTData.content, sizeof(bpsCircleTypeDef));
				sd->previousCommand = sd->UARTData.command;
				bpsCalSetpoint4CircleMode(	sd->content.circleProperties.centerCoordinate[BPS_X_AXIS], 
													sd->content.circleProperties.radius, &sd->content.circleProperties.currentAngle,
													sd->content.circleProperties.speed, &sd->setpoint[BPS_X_AXIS]);
				bpsCalSetpoint4CircleMode(	sd->content.circleProperties.centerCoordinate[BPS_Y_AXIS], 
													sd->content.circleProperties.radius, &sd->content.circleProperties.currentAngle,
													sd->content.circleProperties.speed, &sd->setpoint[BPS_Y_AXIS]);		
				xTaskNotifyGive(taskNumber[TASK_CAL_REF_ENCODER_VALUE]);
				break;
			case BPS_MODE_RECTANGLE:
				memcpy(&sd->content, &sd->UARTData.content, sizeof(bpsRectangleTypeDef));
				sd->previousCommand = sd->UARTData.command;
				xTaskNotifyGive(taskNumber[TASK_CAL_REF_ENCODER_VALUE]);
				break;
			case BPS_MODE_DEFAULT:
			default:
				switch (sd->previousCommand)
				{
					case BPS_MODE_SETPOINT:
						xTaskNotifyGive(taskNumber[TASK_CAL_REF_ENCODER_VALUE]);
						break;
					case BPS_MODE_CIRCLE:
						bpsCalSetpoint4CircleMode(	sd->content.circleProperties.centerCoordinate[BPS_X_AXIS], 
													sd->content.circleProperties.radius, &sd->content.circleProperties.currentAngle,
													sd->content.circleProperties.speed, &sd->setpoint[BPS_X_AXIS]);
						bpsCalSetpoint4CircleMode(	sd->content.circleProperties.centerCoordinate[BPS_Y_AXIS], 
													sd->content.circleProperties.radius, &sd->content.circleProperties.currentAngle,
													sd->content.circleProperties.speed, &sd->setpoint[BPS_Y_AXIS]);					
						xTaskNotifyGive(taskNumber[TASK_CAL_REF_ENCODER_VALUE]);
						break;
					case BPS_MODE_RECTANGLE:
						break;
				}
				
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
		bpsCalculatePID(sd->setpoint[BPS_X_AXIS], sd->ballCoordinate[BPS_X_AXIS], sd->PIDs.Kp[BPS_OUTER_PID][BPS_X_AXIS], 
						sd->PIDs.Ki[BPS_OUTER_PID][BPS_X_AXIS], sd->PIDs.Kd[BPS_OUTER_PID][BPS_X_AXIS], 
						&sd->errorSamples[BPS_OUTER_PID][BPS_X_AXIS][0], &sd->PIDSamples[BPS_OUTER_PID][BPS_X_AXIS][0],
						DT_OUTER_LOOP);
		int16_t	refEncoderValueTemp = sd->PIDSamples[BPS_OUTER_PID][BPS_X_AXIS][0];
		memcpy(&sd->refEncoderValue, &refEncoderValueTemp, sizeof(int16_t));
		xTaskNotifyGive(taskNumber[TASK_CONTROL_MOTOR]);
	}
}

void bpsTaskControlMotor(void* pointer)
{
	bpsSharedDataTypeDef* sd = (bpsSharedDataTypeDef*)pointer;
	bpsStartPWM();
	bpsStartEncoder();
	
	while(1)
	{
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
		bpsReadEncoderCnt(BPS_X_AXIS, &encoderValue);
		bpsCalculatePID(sd->refEncoderValue[BPS_X_AXIS], encoderValue, sd->PIDs.Kp[BPS_INNER_PID][BPS_X_AXIS], 
						sd->PIDs.Ki[BPS_INNER_PID][BPS_X_AXIS], sd->PIDs.Kd[BPS_INNER_PID][BPS_X_AXIS], 
						&sd->errorSamples[BPS_INNER_PID][BPS_X_AXIS][0], &sd->PIDSamples[BPS_INNER_PID][BPS_X_AXIS][0],
						DT_INNER_LOOP);
		bpsControlMotor(BPS_X_AXIS, sd->PIDSamples[BPS_INNER_PID][BPS_X_AXIS][0]);
		//bpsControlMotor(BPS_Y_AXIS, sd->PIDSamples[BPS_INNER_PID][BPS_Y_AXIS][0]);
		vTaskDelay(pdMS_TO_TICKS(1));
		xTaskNotifyGive(taskNumber[TASK_UPDATE_SETPOINT]);
	}
}

void bpsTaskUARTSendData(void* pointer)
{
	//using Ziegler–Nichols method with Ku = 70, Tu = 120ms, Kp = 0.6Ku, Ki = 1.2Ku/Tu, Kp = 0.075*Ku*Tu
	//bpsUARTSendDataTypeDef 	sendData;
	//bpsPIDTypeDef			PID = {{{10,0},{30,0}},{{0,0},{0,0}},{{0,0},{0,0}}};
	//uint8_t cnt = 0;
	bpsPIDTypeDef			PID = {	.Kp[BPS_OUTER_PID][BPS_X_AXIS] = 0.05,
									.Ki[BPS_OUTER_PID][BPS_X_AXIS] = 0,
									.Kd[BPS_OUTER_PID][BPS_X_AXIS] = 0.03,

									.Kp[BPS_OUTER_PID][BPS_Y_AXIS] = 20,
									.Ki[BPS_OUTER_PID][BPS_Y_AXIS] = 0,
									.Kd[BPS_OUTER_PID][BPS_Y_AXIS] = 0,

									.Kp[BPS_INNER_PID][BPS_X_AXIS] = 1,
									.Ki[BPS_INNER_PID][BPS_X_AXIS] = 230,
									.Kd[BPS_INNER_PID][BPS_X_AXIS] = 0.56,

									.Kp[BPS_INNER_PID][BPS_Y_AXIS] = 25,
									.Ki[BPS_INNER_PID][BPS_Y_AXIS] = 0,
									.Kd[BPS_INNER_PID][BPS_Y_AXIS] = 0};

	sendData.ballCoordinate[BPS_X_AXIS] = 0;
	sendData.ballCoordinate[BPS_Y_AXIS] = 0;
	memcpy(&sendData.content, &PID, sizeof(bpsPIDTypeDef)); 
	sendData.command = BPS_UPDATE_PID;
	bpsUARTSendData(&sendData);
	vTaskDelay(pdMS_TO_TICKS(16));

	sendData.ballCoordinate[BPS_X_AXIS] = 5;
	sendData.ballCoordinate[BPS_Y_AXIS] = 5;
	sendData.content.pointProperties.setpointCoordinate[BPS_X_AXIS] = 384;
	sendData.content.pointProperties.setpointCoordinate[BPS_Y_AXIS] = 384;
	sendData.command = BPS_MODE_SETPOINT;
	bpsUARTSendData(&sendData);
	vTaskDelay(pdMS_TO_TICKS(16));
	int n =10;
	while(1)
	{
		vTaskDelay(pdMS_TO_TICKS(16));
		sendData.ballCoordinate[BPS_X_AXIS] = n++;
		sendData.ballCoordinate[BPS_Y_AXIS] = n++;
		sendData.command = BPS_MODE_DEFAULT;
		bpsUARTSendData(&sendData);
	}
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
    xTaskCreate(bpsTaskUARTSendData, "Example send data",500,NULL,1,NULL);
}
