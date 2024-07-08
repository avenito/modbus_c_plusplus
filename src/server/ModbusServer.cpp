/*
 * ModbusServer.cpp
 *
 *  Created on: Feb 21, 2024
 *      Author: Alexandre Venito
 */

#include "ModbusServer.h"

ModbusServer::ModbusServer(void) {
//	this->reles = reles;
}

bool ModbusServer::init(int port) {
	this->port = port;
    //setup a socket and connection tools
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);

    //open stream oriented socket with internet address
    //also keep track of the socket descriptor
    serverSd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSd < 0)
    {
        cout << "Error establishing the server socket" << endl;
        return FALSE;
    }
    //bind the socket to its local address
    bindStatus = bind(serverSd, (struct sockaddr*) &servAddr,
        sizeof(servAddr));

    if(bindStatus < 0)
    {
    	cout << endl << "Error binding socket to local address" << endl;
       	cout << "It is not possible to open port " << port << "! Are you root?" << endl;
        return FALSE;
    }
    //listen for up to totalConnections requests at a time
    listen(serverSd, totalConnections);
    return TRUE;
}

void ModbusServer::runServer(void){
    //receive a request from client using accept
    //we need a new address to connect with the client
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    while(1){
    	bytesRead = 1;
		//accept, create a new socket descriptor to
		//handle the new connection with client
    	cout << endl << "Waiting for a client to connect on port " << port << " ..." << endl;
    	newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
		if(newSd < 0)
		{
			cerr << "Error accepting request from client!" << endl;
			exit(1);
		}

		cout << "Connected with client!" << endl;

		while(bytesRead > 0) {
			//receive a message from the client (listen)
			cout << "Waiting modbus request..." << endl;
			memset(&msg, 0, sizeof(msg));//clear the buffer
			bytesRead = recv(newSd, (char*)&msg, sizeof(msg), 0);

			cout << "Bytes read: " << bytesRead << endl;

			transID 	= msg[0] << 8 | msg[1];
			protocol 	= msg[2] << 8 | msg[3];
			len			= msg[4] << 8 | msg[5];
			uID			= (int) msg[6];
			func		= (int) msg[7];

//	       	for (int i = 0; i < bytesRead; i++){
//	       		cout << "char[" << i << "]: ";
//	       		cout << (int)msg[i] << endl;
//	       	}

			startAdd = msg[8] << 8  | msg[9];
			numRegs  = msg[10] << 8 | msg[11];

			/* Selecting the function */
			switch (func){
				case 0:
						shutdown (newSd, SHUT_RDWR);
						break;
				case MB_FC_READ_COILS:
						cout << "MB_FC_READ_COILS" << endl;
						cout << "Start Address: " << startAdd << endl;
						cout << "Num. Coils: " << numRegs << endl;
						msgIndex = 4;
						len = 3 + ceil((double)numRegs /8);
						cout << "Len answer: " << len << endl;
						msg[msgIndex++] = (unsigned) len >> 8;
						msg[msgIndex++] = (unsigned) len & 0xff;
						msg[msgIndex++] = (unsigned) uID & 0xff;
						msg[msgIndex++] = (unsigned) func & 0xff;
						msg[msgIndex++] = (unsigned) (ceil((double)numRegs /8)) & 0xff;
						for (int r = 0; r < numRegs; r++){
							if(reles[uID - 1].coil[startAdd + r]){
								msg[msgIndex] = msg[msgIndex] | (0x01 << r);
							}
						}

						send(newSd, (char*)&msg, (len + 6), 0);
						break;

				case MB_FC_READ_DISCRETE_INPUTS:
						cout << "MB_FC_READ_DISCRETE_INPUTS" << endl;
						cout << "Start Address: " << startAdd << endl;
						cout << "Num. Disc. Inputs: " << numRegs << endl;
						msgIndex = 4;
						len = 3 + ceil((double)numRegs /8);
						cout << "Len answer: " << len << endl;
						msg[msgIndex++] = (unsigned) len >> 8;
						msg[msgIndex++] = (unsigned) len & 0xff;
						msg[msgIndex++] = (unsigned) uID & 0xff;
						msg[msgIndex++] = (unsigned) func & 0xff;
						msg[msgIndex++] = (unsigned) (ceil((double)numRegs /8)) & 0xff;
						for (int r = 0; r < numRegs; r++){
							if(reles[uID - 1].coil[startAdd + r]){
								msg[msgIndex] = msg[msgIndex] | (0x01 << r);
							}
						}

						send(newSd, (char*)&msg, (len + 6), 0);
						break;

				case MB_FC_READ_HOLDING_REGISTERS:
						cout << "MB_FC_READ_HOLDING_REGISTERS" << endl;
						len = procReadReg((char*)&msg);
						send(newSd, (char*)&msg, len, 0);
						break;

				case MB_FC_READ_INPUT_REGISTERS:
						cout << "MB_FC_READ_INPUT_REGISTERS" << endl;
						len = procReadReg((char*)&msg);
						send(newSd, (char*)&msg, len, 0);
						break;

				case MB_FC_WRITE_SINGLE_COIL:
						cout << "MB_FC_WRITE_SINGLE_COIL" << endl;
						cout << "Output Address: " << startAdd << endl;
						cout << "Value: " << numRegs << endl;
						break;

				case MB_FC_WRITE_SINGLE_REGISTER:
						cout << "MB_FC_WRITE_SINGLE_REGISTER" << endl;
						cout << "Register Address: " << startAdd << endl;
						cout << "Value: " << numRegs << endl;

						msgIndex = 10;
						reles[uID - 1].regs[startAdd] = (unsigned) msg[msgIndex++] << 8;
						reles[uID - 1].regs[startAdd] |= (unsigned) msg[msgIndex++] & 0xff;

						send(newSd, (char*)&msg, (12), 0);
						break;

				case MB_FC_WRITE_MULTIPLE_REGISTERS:
						cout << "MB_FC_WRITE_MULTIPLE_REGISTERS" << endl;
						cout << "Start Address: " << startAdd << endl;
						cout << "Num. Registers: " << numRegs << endl;
						msgIndex = 13;
						for (int r = 0; r <= numRegs; r++){
							reles[uID - 1].regs[startAdd + r] = (unsigned) msg[msgIndex++] << 8;
							reles[uID - 1].regs[startAdd + r] |= (unsigned) msg[msgIndex++] & 0xff;
						}

						msgIndex = 4;
						len = 6;
						msg[msgIndex++] = (unsigned) len >> 8;
						msg[msgIndex++] = (unsigned) len & 0xff;

						send(newSd, (char*)&msg, (len + 6), 0);
						break;
				default:
						cout << "MB_EXCEP_ILLEGAL_FUNCTION" << endl;
						msg[7] += 0x80;
						msg[8] = MB_EXCEP_ILLEGAL_FUNCTION;
						send(newSd, (char*)&msg, 9, 0);
			}
		}
	}

    //we need to close the socket descriptors after we're all done
    close(newSd);
    close(serverSd);
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

		for (int r = 0; r <= numRegs; r++){
			msg[msgIndex++] = (unsigned) reles[msg[6] - 1].regs[startAdd + r] >> 8;
			msg[msgIndex++] = (unsigned) reles[msg[6] - 1].regs[startAdd + r] & 0xff;
		}

	} else {
		msg[7] += 0x80;
		msg[8] = MB_EXCEP_ILLEGAL_DATA_ADD;
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
	totalRegs	= sizeof(reles[uID].regs)/sizeof(int);

	/* Selecting the function */
	switch (func){
		case MB_FC_READ_COILS:
		case MB_FC_READ_DISCRETE_INPUTS:
		case MB_FC_WRITE_SINGLE_COIL:

			startAdd -= reles[uID].coilOffSet;

			break;

		case MB_FC_READ_HOLDING_REGISTERS:
		case MB_FC_READ_INPUT_REGISTERS:
		case MB_FC_WRITE_SINGLE_REGISTER:
		case MB_FC_WRITE_MULTIPLE_REGISTERS:

			startAdd -= reles[uID].regOffSet;

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
