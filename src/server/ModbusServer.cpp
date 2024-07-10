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

    	cout << endl << "Waiting for a client to connect on port " << port << " ..." << endl;

    	/* Blocking function waiting the client connection.
    	 * That's why the modbus server must run as a thread. */
    	newSocket = accept(mbServerSocket, (sockaddr *)&newSocketAdd, &newSocketAddSize);

    	if(newSocket < 0)
		{
			cerr << "Error accepting request from client!" << endl;
			exit(1);
		}

		cout << "Client connected ";

		while(bytesRead > 0) {

			/* Blank the buffer. */
			memset(&mbMsg, 0, sizeof(mbMsg));
			cout << " and waiting modbus request ..." << endl;
			bytesRead = recv(newSocket, (char*)&mbMsg, sizeof(mbMsg), 0);

			if (DEBUG_LEVEL == DBG_LEVEL_03) {
				cout << "Bytes read: " << bytesRead << endl;
			}

			transID 	= mbMsg[0] << 8 | mbMsg[1];
			protocol 	= mbMsg[2] << 8 | mbMsg[3];
			len			= mbMsg[4] << 8 | mbMsg[5];
			uID			= (int) mbMsg[6];
			func		= (int) mbMsg[7];

//	       	for (int i = 0; i < bytesRead; i++){
//	       		cout << "char[" << i << "]: ";
//	       		cout << (int)msg[i] << endl;
//	       	}

			startAdd = mbMsg[8] << 8  | mbMsg[9];
			numRegs  = mbMsg[10] << 8 | mbMsg[11];

			/* Selecting the function */
			switch (func){
				case 0:
						shutdown (newSocket, SHUT_RDWR);
						break;
				case READ_COILS:
						cout << "MB_FC_READ_COILS" << endl;
						cout << "Start Address: " << startAdd << endl;
						cout << "Num. Coils: " << numRegs << endl;
						msgIndex = 4;
						len = 3 + ceil((double)numRegs /8);
						cout << "Len answer: " << len << endl;
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
						cout << "MB_FC_READ_DISCRETE_INPUTS" << endl;
						cout << "Start Address: " << startAdd << endl;
						cout << "Num. Disc. Inputs: " << numRegs << endl;
						msgIndex = 4;
						len = 3 + ceil((double)numRegs /8);
						cout << "Len answer: " << len << endl;
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
						cout << "MB_FC_READ_HOLDING_REGISTERS" << endl;
						len = procReadReg((char*)&mbMsg);
						send(newSocket, (char*)&mbMsg, len, 0);
						break;

				case READ_INPUT_REGISTERS:
						cout << "MB_FC_READ_INPUT_REGISTERS" << endl;
						len = procReadReg((char*)&mbMsg);
						send(newSocket, (char*)&mbMsg, len, 0);
						break;

				case WRITE_SINGLE_COIL:
						cout << "MB_FC_WRITE_SINGLE_COIL" << endl;
						cout << "Output Address: " << startAdd << endl;
						cout << "Value: " << numRegs << endl;
						break;

				case WRITE_SINGLE_REGISTER:
						cout << "MB_FC_WRITE_SINGLE_REGISTER" << endl;
						cout << "Register Address: " << startAdd << endl;
						cout << "Value: " << numRegs << endl;

						msgIndex = 10;
//						reles[uID - 1].regs[startAdd] = (unsigned) mbMsg[msgIndex++] << 8;
//						reles[uID - 1].regs[startAdd] |= (unsigned) mbMsg[msgIndex++] & 0xff;

						send(newSocket, (char*)&mbMsg, (12), 0);
						break;

				case WRITE_MULTIPLE_REGISTERS:
						cout << "MB_FC_WRITE_MULTIPLE_REGISTERS" << endl;
						cout << "Start Address: " << startAdd << endl;
						cout << "Num. Registers: " << numRegs << endl;
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
						cout << "MB_EXCEP_ILLEGAL_FUNCTION" << endl;
						mbMsg[7] += 0x80;
						mbMsg[8] = EXCEP_ILLEGAL_FUNCTION;
						send(newSocket, (char*)&mbMsg, 9, 0);
			}
		}
	}

    //we need to close the socket descriptors after we're all done
    close(newSocket);
    close(mbServerSocket);
    cout << "Connection closed..." << endl;
}

int ModbusServer::procReadReg(char* msg){

	int msgIndex = 4;
	int len, aux;
	int startAdd;
	int numRegs;

	startAdd = checkPDU(msg);

	if (startAdd >= 0){

		numRegs  = msg[10] << 8 | msg[11];

		len = 3 + numRegs * 2;

		msg[msgIndex++] = (unsigned) len >> 8;
		msg[msgIndex++] = (unsigned) len * 0xff;
		msg[msgIndex++] = (unsigned) msg[6] & 0xff;
		msg[msgIndex++] = (unsigned) msg[7] & 0xff;
		msg[msgIndex++] = (unsigned) (numRegs * 2) & 0xff;

//		for (int r = 0; r <= numRegs; r++){
//			msg[msgIndex++] = (unsigned) reles[msg[6] - 1].regs[startAdd + r] >> 8;
//			msg[msgIndex++] = (unsigned) reles[msg[6] - 1].regs[startAdd + r] & 0xff;
//		}

	} else {
		msg[7] += 0x80;
		msg[8] = EXCEP_ILLEGAL_DATA_ADD;
		len = 3;
	}

	return (len + 6);
}

int ModbusServer::checkPDU(char* msg){

	int	uID;
	int startAdd;
	int func;
	int numRegs, totalRegs;

	uID			= msg[6] - 1;
	func		= msg[7];
	startAdd 	= msg[8] << 8  | msg[9];
	numRegs  	= msg[10] << 8 | msg[11];
//	totalRegs	= sizeof(reles[uID].regs)/sizeof(int);

	/* Selecting the function */
	switch (func){
		case READ_COILS:
		case READ_DISCRETE_INPUTS:
		case WRITE_SINGLE_COIL:

//			startAdd -= reles[uID].coilOffSet;

			break;

		case READ_HOLDING_REGISTERS:
		case READ_INPUT_REGISTERS:
		case WRITE_SINGLE_REGISTER:
		case WRITE_MULTIPLE_REGISTERS:

//			startAdd -= reles[uID].regOffSet;

			break;

		default:
			;
	}

	cout << "rele[" << uID << "] - " << totalRegs << " regs" << endl;

	if ((startAdd < 0) || ((startAdd + numRegs) > totalRegs)) {
		return -1;
	} else {
		return startAdd;
	}
}

int ModbusServer::getPort(void) {
	return this->port;
}

ModbusServer::~ModbusServer() {}
