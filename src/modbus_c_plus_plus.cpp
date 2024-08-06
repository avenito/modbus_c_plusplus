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

/* Example */
void Example(){
	while(true){
		/* Registers */
		Server.registers[0]++;
		Server.registers[1]--;

		/* Inputs */
		Server.inputs[3]++;
		Server.inputs[5] += 2;

		/* Coils */
//		Server.coil[0] = !Server.coil[0];
//		Server.coil[6] = !Server.coil[0];

		/* Discrete inputs */
		Server.discrete_input[1] = !Server.discrete_input[1];
		Server.discrete_input[4] = !Server.discrete_input[1];

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
