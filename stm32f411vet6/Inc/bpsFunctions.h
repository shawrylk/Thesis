
#ifndef USER_FUNCTION
#define USER_FUNCTION

#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

extern 	TIM_HandleTypeDef 			htim2;
extern 	TIM_HandleTypeDef 			htim3;
extern 	TIM_HandleTypeDef			htim4;
extern	UART_HandleTypeDef 			huart2;
extern	DMA_HandleTypeDef			hdma_usart2_rx;

#define ENCODER_X_REG 				TIM3
#define ENCODER_Y_REG 				TIM4
#define PWM_HANDLE					htim2
#define ENCODER_X_HANDLE			htim3
#define ENCODER_Y_HANDLE			htim4
#define	SEND_DATA_HANDLE			huart2
#define	RECEIVE_DATA_HANDLE			huart2
#define	DMA_HANDLE					hdma_usart2_rx
#define MIN_PWM_DUTY				0
#define	MAX_PWM_DUTY				9599
#define NUMBER_OF_SAMPLE			2
#define	DT_OUTER_LOOP				0.016f  // 16 ms
#define	DT_INNER_LOOP				0.001f	//  1 ms
#define	PWM_PIN_1_X					TIM_CHANNEL_1
#define	PWM_PIN_2_X					TIM_CHANNEL_2
#define	PWM_PIN_1_Y					TIM_CHANNEL_3
#define	PWM_PIN_2_Y					TIM_CHANNEL_4
#define PULSE_PER_REVOLUTION        3072

typedef enum
{
    BPS_OUTER_PID,
    BPS_INNER_PID,
    BPS_NUMBER_OF_PID_LOOP
}
bpsPIDPositionTypeDef;

typedef enum 
{
	BPS_X_AXIS,
	BPS_Y_AXIS,
	BPS_NUMBER_OF_AXIS
} 
bpsAxisTypeDef;

typedef enum
{
	BPS_FORWARD,
	BPS_BACKWORD
}
bpsDirectionTypeDef;

typedef enum 
{
	BPS_TOP_LEFT,
	BPS_TOP_RIGHT,
	BPS_BOT_LEFT,
	BPS_BOT_RIGHT,
	BPS_NUMBER_OF_VERTEX
}	
bpsVertexTypeDef;

typedef enum 
{
	BPS_UPDATE_PID,
	BPS_MODE_SETPOINT,
	BPS_MODE_CIRCLE,
	BPS_MODE_RECTANGLE,	
	BPS_MODE_DEFAULT
}	
bpsCommandTypeDef;

typedef struct 
{
	int16_t 		setpointCoordinate[BPS_NUMBER_OF_AXIS];
} 
bpsPointTypeDef; //4

typedef struct 
{
	int16_t			centerCoordinate[BPS_NUMBER_OF_AXIS];
	uint16_t		radius;
	uint16_t		currentAngle;
	uint16_t		speed;
}	
bpsCircleTypeDef; //10

typedef struct 
{
	int16_t	vertexCoordinate[BPS_NUMBER_OF_VERTEX][BPS_NUMBER_OF_AXIS];
}	
bpsRectangleTypeDef; //16

typedef struct
{
	float                                   Kp[BPS_NUMBER_OF_PID_LOOP][BPS_NUMBER_OF_AXIS];
    float                                   Ki[BPS_NUMBER_OF_PID_LOOP][BPS_NUMBER_OF_AXIS];
    float                                   Kd[BPS_NUMBER_OF_PID_LOOP][BPS_NUMBER_OF_AXIS];
}
bpsPIDTypeDef; //48

typedef union 
{
	bpsPointTypeDef 			pointProperties;
	bpsCircleTypeDef			circleProperties;
	bpsRectangleTypeDef			rectangleProperties;
	bpsPIDTypeDef				PIDProperties;
}	
bpsDataTypeDef; //48

typedef struct 
{
	int16_t						ballCoordinate[BPS_NUMBER_OF_AXIS];
	bpsCommandTypeDef			command;
	bpsDataTypeDef				content;
}	
bpsUARTReceiveDataTypeDef; //56

typedef	struct	
{
	//int16_t		encoderCnt[2];
	int16_t						ballCoordinate[BPS_NUMBER_OF_AXIS];
	bpsCommandTypeDef			command;
	bpsDataTypeDef				content;
}	
bpsUARTSendDataTypeDef; //56

HAL_StatusTypeDef		bpsStartEncoder				(void);
HAL_StatusTypeDef		bpsReadEncoderCnt			(bpsAxisTypeDef axis, int16_t* ret);
HAL_StatusTypeDef		bpsStartPWM					(void);
HAL_StatusTypeDef		bpsSetPWMDuty				(bpsAxisTypeDef axis, uint16_t duty, bpsDirectionTypeDef direction);
HAL_StatusTypeDef		bpsUARTReceiveData			(bpsUARTReceiveDataTypeDef* receiveData);
HAL_StatusTypeDef		bpsUARTSendData				(bpsUARTSendDataTypeDef* sendData);
HAL_StatusTypeDef 		bpsCalculatePID				(int16_t setpoint, int16_t currentPoint, float Kp, 
													float Ki, float Kd, int16_t* eSamples, float* PIDs, float time);
HAL_StatusTypeDef 		bpsAppendErrorSamples		(int16_t* errorSamples, int16_t newSample);
HAL_StatusTypeDef		bpsAppendPIDSamples			(float* PIDSamples, float newSample);
HAL_StatusTypeDef		bpsControlMotor				(bpsAxisTypeDef axis, float PID);
HAL_StatusTypeDef		bpsCalSetpoint4CircleMode 	(int16_t centerOrdinate, uint16_t radius, uint16_t* currentAngle, uint16_t speed, 
													int16_t* setpoint);
#endif
