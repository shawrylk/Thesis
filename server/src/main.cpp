#include "../hpp/bpsServer.hpp"
#include "../hpp/bpsUARTData.hpp"
int sendFunc (char *sendData, int sendLen);
int recvFunc (char *recvData, int recvLen);

main (int argc, char *argv[])
{
  Server server((char *)"pi",(char *)"raspberry",4,60);
  std::cout << server.Start(sendFunc, recvFunc) << "\n";
}


int sendFunc (char *sendData, int sendLen)
{
  bpsPointTypeDef *data = (bpsPointTypeDef*)sendData;
  data->setpointCoordinate[BPS_X_AXIS] = 120;
  data->setpointCoordinate[BPS_Y_AXIS] = 130;
}

int recvFunc (char *recvData, int recvLen)
{
  bpsUARTReceiveDataTypeDef *data = (bpsUARTReceiveDataTypeDef*)recvData ;
  switch (data->command)
  {
              case  BPS_MODE_CIRCLE:
                std::cout << "circle mode \n";
                std::cout << "x: " << data->content.circleProperties.centerCoordinate[BPS_X_AXIS] << std::endl;
                std::cout << "y: " << data->content.circleProperties.centerCoordinate[BPS_Y_AXIS] << std::endl;
                std::cout << "r: " << data->content.circleProperties.radius << std::endl;
                std::cout << "s: " << data->content.circleProperties.speed << std::endl;
                break;
              default:
                std::cout << "other mode \n";
                break;
  }
  return 0;
}
