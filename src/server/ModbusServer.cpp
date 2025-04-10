/*
 * ModbusServer.cpp
 *
 *  Created on: Feb 21, 2024
 *      Author: Alexandre Venito
 *     License: MiT
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
	/* Inicializa o objeto com zeros. */
    memset(&mbServer, 0, sizeof(mbServer));
    mbServer.sin_family = AF_INET;
    mbServer.sin_addr.s_addr = htonl(INADDR_ANY);
    mbServer.sin_port = htons(port);

    /* Create a socket. */
    /* Cria o socket */
    mbServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(mbServerSocket < 0)
    {
        cout << "Modbus Server Socket Error!!!" << endl;
        cout << "Erro no Socket Modbus Server!!!" << endl;
        return FALSE;
    }

    /* Socket established. Let's bind to the address. */
    /* Socket estabelecido. Associa ao edereço. */
    bindSocketStatus = bind(mbServerSocket, (struct sockaddr*) &mbServer,
        sizeof(mbServer));

    if(bindSocketStatus < 0)
    {
    	cout << endl << "Error binding the socket!!" << endl;
       	cout << "It is not possible to open the port " << port << "! Are you root?" << endl;
    	cout << endl << "Erro associando o socket!!" << endl;
       	cout << "Não é possível abrir a porta " << port << "! Você tem direitos de admin?" << endl;
        return FALSE;
    }

    /* All good. Let's listen to the connections. */
    /* Tudo ok. Escutando as conexões. */
    listen(mbServerSocket, totalConnections);

    return TRUE;
}

void ModbusServer::printFrame(char* mbMsg, int length) {
	cout << "Length: " << length << endl;
	for (int i = 0; i < length; i++) {
		cout << "char[" << i << "]: ";
		cout << (int) (mbMsg[i]) << endl;
	}
}

void ModbusServer::runMbServer(void){

	/* Wait for the client connection. */
	/* Espera pela conexão */

    sockaddr_in newSocketAdd;
    socklen_t newSocketAddSize = sizeof(newSocketAdd);
    int	l_connections = 0;

    /* This is an infinite loop. It finishes with CTR+C signal. */
    /* O servidor fica em loop infinito. Deve ser terminado com CTRC+C. */
    while(1){
    	bytesRead = 1;

		if (DEBUG_LEVEL >= DBG_LEVEL_01) {
			cout << endl << "Waiting for a client to connect ...";
		}

    	/* Blocking function waiting the client connection.
    	 * That's why the modbus server must run as a thread. */
		/* Esta é uma função bloqueante, por isso devemos usá-la como thread. */

		newSocket = accept(mbServerSocket, (sockaddr *)&newSocketAdd, &newSocketAddSize);

		if(newSocket < 0)
		{
			if (DEBUG_LEVEL >= DBG_LEVEL_01) {
				cerr << "Error accepting request from client! Errno: " << errno << endl;
			}
			exit(0);
		}

		if (DEBUG_LEVEL >= DBG_LEVEL_01) {
			cout << endl << "Client IP: " << inet_ntoa(newSocketAdd.sin_addr) <<  " connected. Connection "<< ++l_connections << ". ";
			cout << "Waiting modbus request ..." << endl;
		}

		while(bytesRead > 0) {

			/* Blank the buffer. */
			/* Zera o buffer. */
			memset(&mbMsg, 0, sizeof(mbMsg));

			/* Wait for the connection */
			/* Espera pela conexão. */
			bytesRead = recv(newSocket, (char*)&mbMsg, sizeof(mbMsg), 0);

			if (DEBUG_LEVEL == DBG_LEVEL_03) {
				cout << "Request:" << endl;
				printFrame((char*)&mbMsg, bytesRead);
			}

			uID			= (int) mbMsg[6];
			func		= (int) mbMsg[7];

			/* Selecting the function */
			/* Seleciona a função. */
			switch (func){
				case 0:
						shutdown (newSocket, SHUT_RDWR);
						break;

				case READ_COILS:
						if (DEBUG_LEVEL >= DBG_LEVEL_02) {
							cout << "READ_COILS" << endl;
						}
						len = procReadCoil((char*)&mbMsg);
						if (DEBUG_LEVEL == DBG_LEVEL_03) {
							cout << "Response:" << endl;
							printFrame((char*)&mbMsg, len);
						}
						send(newSocket, (char*)&mbMsg, (len), 0);
						break;

				case READ_DISCRETE_INPUTS:
						if (DEBUG_LEVEL >= DBG_LEVEL_02) {
							cout << "READ_DISCRETE_INPUTS" << endl;
						}
						len = procReadDiscInput((char*)&mbMsg);
						if (DEBUG_LEVEL == DBG_LEVEL_03) {
							cout << "Response:" << endl;
							printFrame((char*)&mbMsg, len);
						}
						send(newSocket, (char*)&mbMsg, (len), 0);
						break;

				case READ_HOLDING_REGISTERS:
						if (DEBUG_LEVEL >= DBG_LEVEL_02) {
							cout << "READ_HOLDING_REGISTERS" << endl;
						}
						len = procReadReg((char*)&mbMsg);
						if (DEBUG_LEVEL == DBG_LEVEL_03) {
							cout << "Response:" << endl;
							printFrame((char*)&mbMsg, len);
						}
						send(newSocket, (char*)&mbMsg, len, 0);
						break;

				case READ_INPUT_REGISTERS:
						if (DEBUG_LEVEL >= DBG_LEVEL_02) {
							cout << "READ_INPUT_REGISTERS" << endl;
						}
						len = procReadInput((char*)&mbMsg);
						if (DEBUG_LEVEL == DBG_LEVEL_03) {
							cout << "Response:" << endl;
							printFrame((char*)&mbMsg, len);
						}
						send(newSocket, (char*)&mbMsg, len, 0);
						break;

				case WRITE_SINGLE_COIL:
						if (DEBUG_LEVEL >= DBG_LEVEL_02) {
							cout << "WRITE_SINGLE_COIL" << endl;
						}
						len = procWriteSingCoil((char*)&mbMsg);
						if (DEBUG_LEVEL == DBG_LEVEL_03) {
							cout << "Response:" << endl;
							printFrame((char*)&mbMsg, len);
						}
						send(newSocket, (char*)&mbMsg, (len), 0);
						break;

				case WRITE_SINGLE_REGISTER:
						if (DEBUG_LEVEL >= DBG_LEVEL_02) {
							cout << "WRITE_SINGLE_REGISTER" << endl;
						}
						len = procWriteSingReg((char*)&mbMsg);
						if (DEBUG_LEVEL == DBG_LEVEL_03) {
							cout << "Response:" << endl;
							printFrame((char*)&mbMsg, len);
						}
						send(newSocket, (char*)&mbMsg, (len), 0);
						break;

				case WRITE_MULTIPLE_REGISTERS:
						if (DEBUG_LEVEL >= DBG_LEVEL_02) {
							cout << "WRITE_MULTIPLE_REGISTERS" << endl;
							cout << "Start Address: " << startAdd << endl;
							cout << "Num. Registers: " << numRegs << endl;
						}
						len = procWriteMultReg((char*)&mbMsg);
						if (DEBUG_LEVEL == DBG_LEVEL_03) {
							cout << "Response:" << endl;
							printFrame((char*)&mbMsg, len);
						}
						send(newSocket, (char*)&mbMsg, (len + 6), 0);
						break;
				default:
						if (DEBUG_LEVEL >= DBG_LEVEL_02) {
							cout << "ILLEGAL_FUNCTION" << endl;
						}
						mbMsg[7] += 0x80;
						mbMsg[8] = EXCEP_ILLEGAL_FUNCTION;
						if (DEBUG_LEVEL == DBG_LEVEL_03) {
							cout << "Response:" << endl;
							printFrame((char*)&mbMsg, bytesRead);
						}
						send(newSocket, (char*)&mbMsg, 9, 0);
			}
		}

		close(newSocket);
	}

	if (DEBUG_LEVEL >= DBG_LEVEL_02) {
		cout << "Connection closed newSocket: " << close(newSocket) << " -- mbServerSocket: " << close(mbServerSocket) << endl;
	}
}

int ModbusServer::getPort(void) {
	return this->port;
}

ModbusServer::~ModbusServer() {}
