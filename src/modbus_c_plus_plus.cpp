//============================================================================
// Name        : modbus_c_plus_plus.cpp
// Author      : Alexandre Venito
// Version     : 1.0.0
// Copyright   : 
// Description : Example how to use Modbus Server C++
//============================================================================

#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

/* Instantiates the modbus server */
server/ModbusServer	Server();

/* Declare the function to be called when pressing CTR+C to finish the program */
void signal_callback_handler(int signum) {
	   cout << "\nExample finished! " << endl;
	   // Terminate program
	   exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {

	/* Declare the function to handler the CTR+C signal */
	signal(SIGINT, signal_callback_handler);

	int L_Port;

	return 0;
}
