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

#define	PORT		1501
#define VAZAO_CX01	5
#define VAZAO_VL01	5
#define VAZAO_VL02	5
#define VAZAO_VL03	5
#define VAZAO_VL04	5

using namespace std;

/* Instantiates the modbus server */
ModbusServer	Server = ModbusServer();
int	VzV01, VzV02;

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

int CheckLimit(int value, int max, int min){
	if (value > max){
		return max;
	}
	if (value < min){
		return min;
	}
	return value;
}

void Example(){
	while(true){

		/* Bomba 01 - Bombeia para Cx01 */
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

		/* Caixa 01 */
		if (Server.discrete_input[0] && (Server.inputs[3] > 0)){
			Server.inputs[0] += VAZAO_CX01;
			if (Server.inputs[0] >= 100){
				Server.discrete_input[1] = 1;
			} else {
				Server.discrete_input[1] = 0;
			}
		}
		/* Valvula 01 ajustavel - Liga Cx01 - Cx02 */
		Server.inputs[4] = Server.registers[1];
		if (Server.inputs[4] > 0 && Server.inputs[0] > 0){
			VzV01 = (VAZAO_VL01 * Server.inputs[4] / 100);
			Server.inputs[0] -= VzV01; // esvazia Cx01
			Server.inputs[1] += VzV01; // enche Cx02
		}
		/* Valvula 02 ajustavel - Liga Cx01 - Cx03 */
		Server.inputs[5] = Server.registers[2];
		if (Server.inputs[5] > 0 && Server.inputs[0] > 0){
			VzV02 = (VAZAO_VL02 * Server.inputs[5] / 100);
			Server.inputs[0] -= VzV02; // esvazia Cx01
			Server.inputs[2] += VzV02; // enche Cx03
		}

		/* Valvula 03 on/off - Saida Cx02 */
		Server.discrete_input[4] = Server.coil[1];
		if (Server.discrete_input[4] && Server.inputs[1] > 0){
			Server.inputs[1] -= VAZAO_VL03;
		}
		if (Server.inputs[1] >= 100){
			Server.discrete_input[2] = 1;
		} else {
			Server.discrete_input[2] = 0;
		}


		/* Valvula 04 on/off - Saida Cx03 */
		Server.discrete_input[5] = Server.coil[2];
		if (Server.discrete_input[5] && Server.inputs[2] > 0){
			Server.inputs[2] -= VAZAO_VL04;
		}
		if (Server.inputs[2] >= 100){
			Server.discrete_input[3] = 1;
		} else {
			Server.discrete_input[3] = 0;
		}

		/* Check limites */
		Server.inputs[0] = CheckLimit(Server.inputs[0], 100, 0);
		Server.inputs[1] = CheckLimit(Server.inputs[1], 100, 0);
		Server.inputs[2] = CheckLimit(Server.inputs[2], 100, 0);
		Server.inputs[3] = CheckLimit(Server.inputs[3], 100, 0);
		Server.inputs[4] = CheckLimit(Server.inputs[4], 100, 0);
		Server.inputs[5] = CheckLimit(Server.inputs[5], 100, 0);

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
