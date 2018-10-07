
#ifndef USER_TASKS
#define USER_TASKS

#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "bpsFunctions.h"

#define NUMBER_OF_TASK                  10
#define TASK_UPDATE_UART_DATA           0
#define TASK_UPDATE_SETPOINT            1
#define TASK_CAL_REF_ENCODER_VALUE      2
#define TASK_CONTROL_MOTOR              3


typedef struct
{
    bpsUARTReceiveDataTypeDef				UARTData;
    bpsCommandTypeDef                       previousCommand;
    int16_t                                 ballCoordinate[BPS_NUMBER_OF_AXIS];
    int16_t              				 	setpoint[BPS_NUMBER_OF_AXIS];
    int16_t                                 refEncoderValue[BPS_NUMBER_OF_AXIS];
    bpsDataTypeDef                          content;
    bpsPIDTypeDef                           PIDs;
    int16_t                                 errorSamples[BPS_NUMBER_OF_PID_LOOP][BPS_NUMBER_OF_AXIS][NUMBER_OF_SAMPLE];
    float                                   PIDSamples[BPS_NUMBER_OF_PID_LOOP][BPS_NUMBER_OF_AXIS][NUMBER_OF_SAMPLE];
}
bpsSharedDataTypeDef; //212

extern bpsSharedDataTypeDef sharedData;
extern TaskHandle_t        taskNumber[NUMBER_OF_TASK];
extern bpsUARTSendDataTypeDef 	sendData;
extern int16_t encoderValue;

//void bpsTaskPWMGenerator	                (void* pointer);
//void bpsTaskReadEncoder		            (void* pointer);
void bpsTaskUpdateUARTData	                (void* pointer);
void bpsTaskUpdateSetpoint                  (void* pointer);
void bpsTaskCalRefEncoderValue              (void *pointer);
void bpsTaskControlMotor                    (void* pointer);
void bpsTaskUARTSendData                    (void* pointer);
void bpsBallAndPlateSystemStart             (void);

#endif 
