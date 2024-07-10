/*
 * ModbusServer.h
 *
 *  Created on: Feb 21, 2024
 *      Author: root
 */

#ifndef MODBUSSERVER_H_
#define MODBUSSERVER_H_

/* Modbus function codes */
#define READ_COILS               	0x01
#define READ_DISCRETE_INPUTS     	0x02
#define READ_HOLDING_REGISTERS   	0x03
#define READ_INPUT_REGISTERS     	0x04
#define WRITE_SINGLE_COIL       	0x05
#define WRITE_SINGLE_REGISTER    	0x06
#define WRITE_MULTIPLE_REGISTERS 	0x10

#define BROADCAST_ADDRESS 0

#define EXCEP_ILLEGAL_FUNCTION		0x01
#define EXCEP_ILLEGAL_DATA_ADD		0x02
#define EXCEP_ILLEGAL_DATA_VALUE	0x03

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

	int port;

	const bool TRUE  = 1;
	const bool FALSE = 0;
	const int  totalConnections = 1;

    //buffer to send and receive messages with
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

    sockaddr_in mbServer;
    int			mbServerSocket;
    int 		bindSocketStatus;

    int checkPDU(char* msg);	// Verifica se o endereco ta na faixa
    int procReadReg(char* msg);	// Processa o request e retorna o tamanho do envio
    void updateRelays(void);	// Atualiza registro dos reles

public:
	ModbusServer(void);
	virtual ~ModbusServer();

	bool init(int port);

	void runMbServer(void);

	int getPort(void);
};

#endif /* MODBUSSERVER_H_ */
