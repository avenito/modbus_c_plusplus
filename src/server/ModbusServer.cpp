/*
 * ModbusServer.cpp
 *
 *  Created on: Feb 21, 2024
 *      Author: Alexandre Venito
 */

#include "ModbusServer.h"

ModbusServer::ModbusServer(void) {

}

/*
 * We must connect our server to a port
 */
bool ModbusServer::init(int port) {

	this->port = port;

	/* Initialize the connection object with zeros. */
    memset(&mbServer, 0, sizeof(mbServer));
    mbServer.sin_family = AF_INET;
    mbServer.sin_addr.s_addr = htonl(INADDR_ANY);
    mbServer.sin_port = htons(port);

    /* Create a socket. */
    mbServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(mbServerSocket < 0)
    {
        cout << "Modbus Server Socket Error!!!" << endl;
        return FALSE;
    }

    /* Socket established. Let's bind to the address. */
    bindSocketStatus = bind(mbServerSocket, (struct sockaddr*) &mbServer,
        sizeof(mbServer));

    if(bindSocketStatus < 0)
    {
    	cout << endl << "Error binding the socket!!" << endl;
       	cout << "It is not possible to open the port " << port << "! Are you root?" << endl;
        return FALSE;
    }

    /* All good. Let's listen to the connections. */
    listen(mbServerSocket, totalConnections);

    return TRUE;
}

void ModbusServer::runMbServer(void){

	/* Wait for the client connection. */

    sockaddr_in newSocketAdd;
    socklen_t newSocketAddSize = sizeof(newSocketAdd);

    /* This is an infinite loop. It finishes with CTR+C signal. */
    while(1){
    	bytesRead = 1;

		if (DEBUG_LEVEL >= DBG_LEVEL_01) {
			cout << endl << "Waiting for a client to connect on port " << port << " ..." << endl;
		}

    	/* Blocking function waiting the client connection.
    	 * That's why the modbus server must run as a thread. */
    	newSocket = accept(mbServerSocket, (sockaddr *)&newSocketAdd, &newSocketAddSize);

    	if(newSocket < 0)
		{
    		if (DEBUG_LEVEL >= DBG_LEVEL_01) {
    			cerr << "Error accepting request from client!" << endl;
    		}
    		exit(1);
		}

		if (DEBUG_LEVEL >= DBG_LEVEL_01) {
			cout << "Client connected ... ";
		}

		while(bytesRead > 0) {

			/* Blank the buffer. */
			memset(&mbMsg, 0, sizeof(mbMsg));
			/* Wait for the connection */
			if (DEBUG_LEVEL >= DBG_LEVEL_02) {
				cout << "waiting modbus request ..." << endl;
			}

			bytesRead = recv(newSocket, (char*)&mbMsg, sizeof(mbMsg), 0);

			if (DEBUG_LEVEL == DBG_LEVEL_03) {
				cout << "Bytes read: " << bytesRead << endl;
				for (int i = 0; i < bytesRead; i++){
					cout << "char[" << i << "]: ";
					cout << (int)mbMsg[i] << endl;
				}
			}

			transID 	= mbMsg[0] << 8 | mbMsg[1];
			protocol 	= mbMsg[2] << 8 | mbMsg[3];
			len			= mbMsg[4] << 8 | mbMsg[5];
			uID			= (int) mbMsg[6];
			func		= (int) mbMsg[7];
			startAdd 	= mbMsg[8] << 8  | mbMsg[9];
			numRegs  	= mbMsg[10] << 8 | mbMsg[11];

			/* Selecting the function */
			switch (func){
				case 0:
						shutdown (newSocket, SHUT_RDWR);
						break;

				case READ_COILS:
						if (DEBUG_LEVEL >= DBG_LEVEL_02) {
							cout << "READ_COILS" << endl;
							cout << "Start Address: " << startAdd << endl;
							cout << "Num. Coils: " << numRegs << endl;
						}
						msgIndex = 4;
						len = 3 + ceil((double)numRegs /8);
						if (DEBUG_LEVEL == DBG_LEVEL_03) {
							cout << "Len answer: " << len << endl;
						}
						mbMsg[msgIndex++] = (unsigned) len >> 8;
						mbMsg[msgIndex++] = (unsigned) len & 0xff;
						mbMsg[msgIndex++] = (unsigned) uID & 0xff;
						mbMsg[msgIndex++] = (unsigned) func & 0xff;
						mbMsg[msgIndex++] = (unsigned) (ceil((double)numRegs /8)) & 0xff;
//						for (int r = 0; r < numRegs; r++){
//							if(reles[uID - 1].coil[startAdd + r]){
//								mbMsg[msgIndex] = mbMsg[msgIndex] | (0x01 << r);
//							}
//						}

						send(newSocket, (char*)&mbMsg, (len + 6), 0);
						break;

				case READ_DISCRETE_INPUTS:
						if (DEBUG_LEVEL >= DBG_LEVEL_02) {
							cout << "READ_DISCRETE_INPUTS" << endl;
							cout << "Start Address: " << startAdd << endl;
							cout << "Num. Disc. Inputs: " << numRegs << endl;
						}
						msgIndex = 4;
						len = 3 + ceil((double)numRegs /8);
						if (DEBUG_LEVEL == DBG_LEVEL_03) {
							cout << "Len answer: " << len << endl;
						}
						mbMsg[msgIndex++] = (unsigned) len >> 8;
						mbMsg[msgIndex++] = (unsigned) len & 0xff;
						mbMsg[msgIndex++] = (unsigned) uID & 0xff;
						mbMsg[msgIndex++] = (unsigned) func & 0xff;
						mbMsg[msgIndex++] = (unsigned) (ceil((double)numRegs /8)) & 0xff;
//						for (int r = 0; r < numRegs; r++){
//							if(reles[uID - 1].coil[startAdd + r]){
//								mbMsg[msgIndex] = mbMsg[msgIndex] | (0x01 << r);
//							}
//						}

						send(newSocket, (char*)&mbMsg, (len + 6), 0);
						break;

				case READ_HOLDING_REGISTERS:
						if (DEBUG_LEVEL >= DBG_LEVEL_02) {
							cout << "READ_HOLDING_REGISTERS" << endl;
						}
						len = procReadReg((char*)&mbMsg);
						send(newSocket, (char*)&mbMsg, len, 0);
						break;

				case READ_INPUT_REGISTERS:
						if (DEBUG_LEVEL >= DBG_LEVEL_02) {
							cout << "READ_INPUT_REGISTERS" << endl;
						}
						len = procReadReg((char*)&mbMsg);
						send(newSocket, (char*)&mbMsg, len, 0);
						break;

				case WRITE_SINGLE_COIL:
						if (DEBUG_LEVEL >= DBG_LEVEL_02) {
							cout << "WRITE_SINGLE_COIL" << endl;
							cout << "Output Address: " << startAdd << endl;
							cout << "Value: " << numRegs << endl;
						}
						break;

				case WRITE_SINGLE_REGISTER:
						if (DEBUG_LEVEL >= DBG_LEVEL_02) {
							cout << "WRITE_SINGLE_REGISTER" << endl;
							cout << "Register Address: " << startAdd << endl;
							cout << "Value: " << numRegs << endl;
						}
						msgIndex = 10;
//						reles[uID - 1].regs[startAdd] = (unsigned) mbMsg[msgIndex++] << 8;
//						reles[uID - 1].regs[startAdd] |= (unsigned) mbMsg[msgIndex++] & 0xff;

						send(newSocket, (char*)&mbMsg, (12), 0);
						break;

				case WRITE_MULTIPLE_REGISTERS:
						if (DEBUG_LEVEL >= DBG_LEVEL_02) {
							cout << "WRITE_MULTIPLE_REGISTERS" << endl;
							cout << "Start Address: " << startAdd << endl;
							cout << "Num. Registers: " << numRegs << endl;
						}
						msgIndex = 13;
//						for (int r = 0; r <= numRegs; r++){
//							reles[uID - 1].regs[startAdd + r] = (unsigned) mbMsg[msgIndex++] << 8;
//							reles[uID - 1].regs[startAdd + r] |= (unsigned) mbMsg[msgIndex++] & 0xff;
//						}

						msgIndex = 4;
						len = 6;
						mbMsg[msgIndex++] = (unsigned) len >> 8;
						mbMsg[msgIndex++] = (unsigned) len & 0xff;

						send(newSocket, (char*)&mbMsg, (len + 6), 0);
						break;
				default:
						if (DEBUG_LEVEL >= DBG_LEVEL_02) {
							cout << "ILLEGAL_FUNCTION" << endl;
						}
						mbMsg[7] += 0x80;
						mbMsg[8] = EXCEP_ILLEGAL_FUNCTION;
						send(newSocket, (char*)&mbMsg, 9, 0);
			}
		}
	}

    //we need to close the socket descriptors after we're all done
    close(newSocket);
    close(mbServerSocket);
	if (DEBUG_LEVEL >= DBG_LEVEL_02) {
		cout << "Connection closed..." << endl;
	}
}

int ModbusServer::procReadReg(char* msgMB){

	int msgIndex = 4;
	int startAdd;
	int numRegs;

	startAdd 	= msgMB[8] << 8  | msgMB[9];
	numRegs  	= msgMB[10] << 8 | msgMB[11];

	startAdd 	-= REGISTER_OFFSET;

	if (DEBUG_LEVEL == DBG_LEVEL_03) {
		cout << "Total registers: " << REGISTERS << endl;
	}

	if ((startAdd >= 0) || ((startAdd + numRegs) <= REGISTERS)) {
		len = 3 + numRegs * 2;
		msgMB[msgIndex++] = (unsigned) len >> 8;
		msgMB[msgIndex++] = (unsigned) len * 0xff;
		msgMB[msgIndex++] = (unsigned) msgMB[6] & 0xff;
		msgMB[msgIndex++] = (unsigned) msgMB[7] & 0xff;
		msgMB[msgIndex++] = (unsigned) (numRegs * 2) & 0xff;
		for (int r = 0; r <= numRegs; r++){
			msgMB[msgIndex++] = (unsigned) registers[startAdd + r] >> 8;
			msgMB[msgIndex++] = (unsigned) registers[startAdd + r] & 0xff;
		}
	} else {
		msgMB[7] += 0x80;
		msgMB[8] = EXCEP_ILLEGAL_DATA_ADD;
		len = 3;
	}

	return (len + 6);
}

int ModbusServer::checkPDU(char* msgMB){

	int	uID;
	int startAdd;
	int func;
	int numRegs;

	uID			= msgMB[6] - 1;
	func		= msgMB[7];
	startAdd 	= msgMB[8] << 8  | msgMB[9];
	numRegs  	= msgMB[10] << 8 | msgMB[11];

	/* Selecting the function */
	switch (func){
		case READ_COILS:
		case WRITE_SINGLE_COIL:
			startAdd -= COIL_OFFSET;
			break;

		case READ_DISCRETE_INPUTS:
			startAdd -= DISCRETE_INPUT_OFFSET;
			break;

		case READ_INPUT_REGISTERS:
			startAdd -= INPUT_OFFSET;
			break;

		case READ_HOLDING_REGISTERS:
		case WRITE_SINGLE_REGISTER:
		case WRITE_MULTIPLE_REGISTERS:
			startAdd -= REGISTER_OFFSET;

			break;

		default:
			;
	}

//	if (DEBUG_LEVEL == DBG_LEVEL_03) {
//		cout << "rele[" << uID << "] - " << blockSize << " regs" << endl;
//	}
//
//	if ((startAdd < 0) || ((startAdd + numRegs) > blockSize)) {
//		return -1;
//	} else {
//		return startAdd;
//	}
}

int ModbusServer::getPort(void) {
	return this->port;
}

ModbusServer::~ModbusServer() {}
