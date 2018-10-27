
#ifndef USER_FUNCTION
#define USER_FUNCTION

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <unistd.h>
#include <iostream>

#define MIN_PWM_DUTY				0
#define	MAX_PWM_DUTY				9599
#define NUMBER_OF_SAMPLE			2
#define	DT_OUTER_LOOP				0.016f  // 16 ms
#define	DT_INNER_LOOP				0.001f	//  1 ms
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
	float						Kp[BPS_NUMBER_OF_PID_LOOP][BPS_NUMBER_OF_AXIS];
    float						Ki[BPS_NUMBER_OF_PID_LOOP][BPS_NUMBER_OF_AXIS];
    float						Kd[BPS_NUMBER_OF_PID_LOOP][BPS_NUMBER_OF_AXIS];
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
	bpsCommandTypeDef			command;
	bpsDataTypeDef				content;
}	
bpsSocketReceiveDataTypeDef; //52

typedef	struct	
{
	int16_t						ballCoordinate[BPS_NUMBER_OF_AXIS];
	bpsCommandTypeDef			command;
	bpsDataTypeDef				content;
 	char                        nullTerminated;
}	
bpsUARTSendDataTypeDef; //56

typedef enum
{
	BPS_ERROR,
	BPS_OK
}
bpsStatusTypeDef;

bpsStatusTypeDef		bpsUARTSendData				(bpsUARTSendDataTypeDef* sendData, int len);
bpsStatusTypeDef 		bpsUARTInit					(void);
bpsStatusTypeDef		bpsUARTReceiveData			(bpsUARTSendDataTypeDef* recvData, int len);
#endif
