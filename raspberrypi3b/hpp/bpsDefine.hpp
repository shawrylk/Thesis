#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <termios.h>
#include <functional>

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

typedef struct
{
	int16_t						ballCoordinate[BPS_NUMBER_OF_AXIS];
    int16_t                     encoderCnt[BPS_NUMBER_OF_AXIS];
}
bpsSocketSendDataTypeDef;

typedef	struct	
{
	int8_t						detectedBall; //1
	int16_t						ballCoordinate[BPS_NUMBER_OF_AXIS]; //4
	bpsCommandTypeDef			command; //4
	bpsDataTypeDef				content; //48
}	
bpsUARTSendDataTypeDef; //60

typedef	struct	
{
	int8_t						detectedBall; //1
	bpsCommandTypeDef			command; //4
}	
fuckyou; //56

typedef struct
{
    int16_t                     encoderCnt[BPS_NUMBER_OF_AXIS];
}
bpsUARTReceiveDataTypeDef;

typedef enum
{
	BPS_ERROR,
	BPS_OK
}
bpsStatusTypeDef;
