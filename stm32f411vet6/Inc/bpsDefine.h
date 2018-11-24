#ifndef BPS_DEFINE
#define BPS_DEFINE

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
#define	MAX_PWM_DUTY				9599 
#define MIN_PWM_DUTY				-9599
#define MAX_ENCODER_CNT				80		// 8 pulses are 1 degree, so 80 pulses are 10 degrees
#define MIN_ENCODER_CNT				-80
#define NUMBER_OF_SAMPLE			2
#define	DT_OUTER_LOOP				0.0112f  // 11.2 ms
#define	DT_INNER_LOOP				0.0010f	//  1 ms
#define	PWM_PIN_2_X					TIM_CHANNEL_1
#define	PWM_PIN_1_X					TIM_CHANNEL_2
#define	PWM_PIN_2_Y					TIM_CHANNEL_3
#define	PWM_PIN_1_Y					TIM_CHANNEL_4
#define PULSE_PER_REVOLUTION        3072
#define BOOL						int8_t
#define true						1
#define false						0
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
	int8_t						detectedBall;
	int16_t						ballCoordinate[BPS_NUMBER_OF_AXIS];
	bpsCommandTypeDef			command;
	bpsDataTypeDef				content;
}	
bpsUARTReceiveDataTypeDef; //56

typedef	struct	
{
	int16_t						encoderCnt[BPS_NUMBER_OF_AXIS];
}	
bpsUARTSendDataTypeDef; //4

#endif
