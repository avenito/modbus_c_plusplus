//============================================================================
// Name        : modbus_c_plus_plus.cpp
// Author      : Alexandre Venito
// Version     : 1.0.0
// Copyright   : 
// Description : Example how to use Modbus Server C++
//============================================================================

#include <iostream>
#include <thread>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include "server/ModbusServer.h"

#define	PORT	1501

using namespace std;

/* Instantiates the modbus server */
ModbusServer	Server = ModbusServer();

/* Declare the function to be called when pressing CTR+C to finish the program */
void signal_callback_handler(int signum) {
	   cout << "\nExample finished! " << endl;
	   // Terminate program
	   exit(EXIT_SUCCESS);
}

/* Example
 * -------
 *
 * Inputs:
 *
 * 0 - Nivel Caixa 01
 * 1 - Nivel Caixa 02
 * 2 - Nivel Caixa 03
 * 3 - RPM Bomba 01
 * 4 - Valvula 01
 * 5 - Valvula 02
 *
 * Registers:
 *
 * 0 - Ajuste RPM Bomba 01
 * 1 - Ajuste valvula 01
 * 2 - Ajuste valvula 02
 *
 * Dicrete-Inputs:
 *
 * 0 - Bomba ligada
 * 1 - Caixa 01 transbordando
 * 2 - Caixa 02 transbordando
 * 3 - Caixa 03 transbordando
 * 4 - Valvula 03
 * 5 - Valvula 04
 *
 * Coils:
 *
 * 0 - Ligar Bomba
 * 1 - Abre vavula 03
 * 2 - Abre vavula 04
 *
 */

void Example(){
	while(true){

		Server.discrete_input[0] = Server.coil[0];

		if (Server.discrete_input[0]) {
			if (Server.inputs[3] >= 100){
				Server.inputs[3] = 100;
			} else {
				if (Server.inputs[3] == 0){Server.inputs[3] = 5;}
				Server.inputs[3] += Server.inputs[3] * 0.5;
			}
		} else {
			if (Server.inputs[3] <= 0){
				Server.inputs[3] = 0;
			} else {
				Server.inputs[3] -= 25;
			}
		}



		this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

/* Modbus server */
void MBServer(){
	Server.runMbServer();
}

int main(int argc, char *argv[]) {

	int port = PORT;

	/* Declare the function to handler the CTR+C signal */
	signal(SIGINT, signal_callback_handler);

	/* Initialize the modbus server. */
	if (Server.init(port)){
		cout << endl << "Modbus server initialized and listening to port " << Server.getPort() << "." << endl;
	} else {
		cout << "Error!!!" << endl;
		exit(0);
	}

	/* Start modbus server.  */
	thread t2 (MBServer);
	t2.detach();

	/* Start example thread. */
	thread t1 (Example);
	t1.detach();

	while(true){
		this_thread::sleep_for(std::chrono::milliseconds(1500));
	}

	return 0;
}
