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
		Server.registers[0]++;
		this_thread::sleep_for(std::chrono::milliseconds(500));
		Server.registers[1]--;
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
		cout << "Modbus server initialized ... listening port " << Server.getPort() << "." << endl;
	} else {
		cout << "Error!!!" << endl;
		exit(0);
	}

	/* Start example thread. */
	thread t1 (Example);
	t1.detach();
	/* Start modbus server.  */
	thread t2 (MBServer);
	t2.detach();

	while(true){
		this_thread::sleep_for(std::chrono::milliseconds(1500));
	}

	return 0;
}
