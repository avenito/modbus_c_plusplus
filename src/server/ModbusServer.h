/*
 * ModbusServer.h
 *
 *  Created on: Feb 21, 2024
 *      Author: Alexandre Venito
 */

#ifndef MODBUSSERVER_H_
#define MODBUSSERVER_H_

/* Coils, registers and inputs */
#define DISCRETE_INPUT_OFFSET		0
#define DISCRETE_INPUTS				8
#define COIL_OFFSET					0
#define COILS						8
#define INPUT_OFFSET				0
#define INPUTS						16
#define REGISTER_OFFSET				0
#define REGISTERS					16

/* Modbus function codes */
#define READ_COILS               	0x01
#define READ_DISCRETE_INPUTS     	0x02
#define READ_HOLDING_REGISTERS   	0x03
#define READ_INPUT_REGISTERS     	0x04
#define WRITE_SINGLE_COIL       	0x05
#define WRITE_SINGLE_REGISTER    	0x06
#define WRITE_MULTIPLE_REGISTERS 	0x10

#define BROADCAST_ADDRESS 			0x00

/* Illegal function codes */
#define EXCEP_ILLEGAL_FUNCTION		0x01
#define EXCEP_ILLEGAL_DATA_ADD		0x02
#define EXCEP_ILLEGAL_DATA_VALUE	0x03

/* Debug levels */
#define DBG_LEVEL_01				0x01
#define DBG_LEVEL_02				0x02
#define DBG_LEVEL_03				0x03

#define DEBUG_LEVEL					DBG_LEVEL_03

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <unistd.h>

using namespace std;

class ModbusServer {

private:

	const bool TRUE  = 1;
	const bool FALSE = 0;
	const int  totalConnections = 1;

    sockaddr_in mbServer;
	int 		port;
    int			mbServerSocket;
    int 		bindSocketStatus;

    /* Buffer to send and receive messages */
    char 	mbMsg[1500];
    int 	transID;
    int 	protocol;
    int 	len;
    int 	uID;
    int		func;
    int		startAdd;
    int		numRegs;
    int		bytesRead;
    int		msgIndex;
    int 	newSocket;
    float	aux;

	/* Handle the request and return the size */
    int procReadReg(char* msgMB);
    int procReadInput(char* msgMB);
    int procReadDiscInput(char* msgMB);
    int procReadCoil(char* msgMB);
    int procWriteSingCoil(char* msgMB);
    int procWriteSingReg(char* msgMB);
    int procWriteMultReg(char* msgMB);

    void printFrame(char* msgMB, int bytesRead);

    void updateRelays(void);	// Atualiza registro dos reles

public:

    /* Discrete inputs, coils, registers and inputs */
	bool	coil[COILS];
	bool	discrete_input[DISCRETE_INPUTS];
	int		inputs[INPUTS];
	int		registers[REGISTERS];

    ModbusServer(void);
	virtual ~ModbusServer();

	bool init(int port);

	void runMbServer(void);

	int getPort(void);
};

#endif /* MODBUSSERVER_H_ */
