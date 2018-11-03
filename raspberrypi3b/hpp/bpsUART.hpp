
#pragma once

#include "bpsDefine.hpp"


class bpsUART
{
	private:
		int 	fdes;
	public:
				bpsUART			(const char *device, const int baud);
		int		send			(bpsUARTSendDataTypeDef* sendData, int len);
		int		recv			(bpsUARTSendDataTypeDef* recvData, int len);
		int 	dataAvailable	();
		void 	flush			();
		void 	close			();
}


