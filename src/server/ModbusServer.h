/*
 * ModbusServer.h
 *
 *  Created on: Feb 21, 2024
 *      Author: root
 */

#ifndef MODBUSSERVER_H_
#define MODBUSSERVER_H_

/* Modbus function codes */
#define MB_FC_READ_COILS               0x01
#define MB_FC_READ_DISCRETE_INPUTS     0x02
#define MB_FC_READ_HOLDING_REGISTERS   0x03
#define MB_FC_READ_INPUT_REGISTERS     0x04
#define MB_FC_WRITE_SINGLE_COIL        0x05
#define MB_FC_WRITE_SINGLE_REGISTER    0x06
#define MB_FC_WRITE_MULTIPLE_REGISTERS 0x10

#define MB_BROADCAST_ADDRESS 0

#define MB_EXCEP_ILLEGAL_FUNCTION		0x01
#define MB_EXCEP_ILLEGAL_DATA_ADD		0x02
#define MB_EXCEP_ILLEGAL_DATA_VALUE		0x03

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <unistd.h>

//#include "components/Relay.h"

using namespace std;

class ModbusServer {
private:
	int port;
//	Relay *reles;

	const bool TRUE  = 1;
	const bool FALSE = 0;
	const int  totalConnections = 1;

    //buffer to send and receive messages with
    char 	msg[1500];
    int 	transID;
    int 	protocol;
    int 	len;
    int 	uID;
    int		func;
    int		startAdd;
    int		numRegs;
    int		bytesRead;
    int		msgIndex;
    int 	newSd;
    float	aux;

    sockaddr_in servAddr;
    int			serverSd;
    int 		bindStatus;

    int checkPDU(char* msg);	// Verifica se o endereco ta na faixa
    int procReadReg(char* msg);	// Processa o request e retorna o tamanho do envio
    void updateRelays(void);	// Atualiza registro dos reles

public:
	ModbusServer(void);
	virtual ~ModbusServer();

	bool init(int port);

	void runServer(void);

	int getPort(void);
};

#endif /* MODBUSSERVER_H_ */
