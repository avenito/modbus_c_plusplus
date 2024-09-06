//============================================================================
// Name        : modbus_c_plus_plus.cpp
// Author      : Alexandre Venito
// Version     : 1.0.1
// Copyright   : MiT
// Description : Example how to use Modbus Server C++
// 				 Exemplo de como usar o Modbus Server C++
//============================================================================

#include <iostream>
#include <thread>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include "server/ModbusServer.h"

#define	PORT			1501
#define FLOWRATE_PM01	4
#define FLOWRATE_VL01	5
#define FLOWRATE_VL02	5
#define FLOWRATE_VL03	5
#define	MIN_RPM			30

using namespace std;

/* Instantiates the modbus server */
/* Instancia o modbus server */

ModbusServer	Server = ModbusServer();
int				FRV01, FRV02, FRV03;

/* Declare the function to be called when pressing CTR+C to finish the program */
/* Declara a função para ser chamda quando pressionarmos CTR+C para terminar o programa */

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
 * 0 - Level Tank 01 / Nível Tanque 01
 * 1 - Level Tank 02 / Nível Tanque 02
 * 2 - Level Tank 03 / Nível Tanque 03
 * 3 - RPM Pump 01 / RPM Bomba 01
 * 4 - Valve 01 / Válvula 01
 * 5 - Valve 02 / Válvula 02
 * 6 - Valve 03 / Válvula 03
 */

#define Level_Tk01		Server.inputs[0]
#define Level_Tk02		Server.inputs[1]
#define Level_Tk03		Server.inputs[2]
#define RPM_FB			Server.inputs[3]
#define Valv01_FB		Server.inputs[4]
#define Valv02_FB		Server.inputs[5]
#define Valv03_FB		Server.inputs[6]

/* Registers:
 *
 * 0 - Setpoint RPM Pump 01 / Ajuste RPM Bomba 01
 * 1 - Setpoint valve 01 / Ajuste valvula 01
 * 2 - Setpoint valve 02 / Ajuste valvula 02
 * 3 - Setpoint valve 03 / Ajuste valvula 03
 */

#define Setpoint_RPM		Server.registers[0]
#define Setpoint_Valv01		Server.registers[1]
#define Setpoint_Valv02		Server.registers[2]
#define Setpoint_Valv03		Server.registers[3]

/* Dicrete-Inputs:
 *
 * 0 - Pump 01 ON / Bomba 01 ON
 * 1 - Tank 01 overflowing / Tanque 01 transbordando
 * 2 - Tank 02 overflowing / Tanque 02 transbordando
 * 3 - Tank 03 overflowing / Tanque 03 transbordando
 * 4 - Tank 01 low level / Tanque 01 nível baixo
 * 5 - Tank 01 low level / Tanque 02 nível baixo
 * 6 - Tank 01 low level / Tanque 03 nível baixo
 */

#define Pump01_ON		Server.discrete_inputs[0]
#define Tk01_Overflow	Server.discrete_inputs[1]
#define Tk02_Overflow	Server.discrete_inputs[2]
#define Tk03_Overflow	Server.discrete_inputs[3]
#define Tk01_Lowlevel	Server.discrete_inputs[4]
#define Tk02_Lowlevel	Server.discrete_inputs[5]
#define Tk03_Lowlevel	Server.discrete_inputs[6]

/* Coils:
 *
 * 0 - Turn Pump 01 ON / Ligar Bomba
 */

#define Set_Pump01_ON		Server.coils[0]


int CheckLimit(int min, int value, int max){
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

		/* Pump 01 / Bomba 01 */
		Pump01_ON = Set_Pump01_ON;

		if (Pump01_ON) {
			if (RPM_FB == 0){RPM_FB = 5;}
			RPM_FB += RPM_FB * 0.5;
			if (RPM_FB >= MIN_RPM){
				RPM_FB = CheckLimit(MIN_RPM, Setpoint_RPM, 100);
			}
		} else {
			RPM_FB -= 25;
			RPM_FB = CheckLimit(0, RPM_FB, 100);
		}

		/* Tank 01 / Tanque 01 */
		if (Pump01_ON && (RPM_FB > 0)){
			Level_Tk01 += RPM_FB * FLOWRATE_PM01 * 0.1;
		}

		if (Level_Tk01 >= 1000){
			Tk01_Overflow = 1;
		} else {
			Tk01_Overflow = 0;
			if (Level_Tk01 <= 50){
				Tk01_Lowlevel = 1;
			} else {
				Tk01_Lowlevel = 0;
			}
		}

		Level_Tk01 = CheckLimit(0, Level_Tk01, 1000);


		/* Valve 01 connects Tank 01 to Tank 02 / Válvula 01 interliga Tanque 01 com Tanque 02 */
		Valv01_FB = Setpoint_Valv01;

		if (Valv01_FB > 0 && Level_Tk01 > 0){
			FRV01 = FLOWRATE_VL01 * Valv01_FB * 0.1;
			Level_Tk01 -= FRV01;
			Level_Tk02 += FRV01;
		}

		Valv01_FB = CheckLimit(0, Valv01_FB, 100);

		if (Level_Tk02 >= 1000){
			Tk02_Overflow = 1;
		} else {
			Tk02_Overflow = 0;
			if (Level_Tk02 <= 5){
				Tk02_Lowlevel = 1;
			} else {
				Tk02_Lowlevel = 0;
			}
		}

		Level_Tk02 = CheckLimit(0, Level_Tk02, 1000);


		/* Valve 02 connects Tank 02 to Tank 03 / Válvula 02 interliga Tanque 02 com Tanque 03 */
		Valv02_FB = Setpoint_Valv02;
		if (Valv02_FB > 0 && Level_Tk02 > 0){
			FRV02 = FLOWRATE_VL02 * Valv02_FB * 0.1;
			Level_Tk02 -= FRV02;
			Level_Tk03 += FRV02;
		}

		Valv02_FB = CheckLimit(0, Valv02_FB, 100);


		/* Valve 03 is the Tank 03 output / Válvula 03 é a saída do Tanque 03 */
		Valv03_FB = Setpoint_Valv03;
		if (Valv03_FB > 0 && Level_Tk03 > 0){
			FRV03 = FLOWRATE_VL03 * Valv03_FB * 0.1;
			Level_Tk03 -= FRV03;
		}

		Valv03_FB = CheckLimit(0, Valv02_FB, 100);

		if (Level_Tk03 >= 1000){
			Tk03_Overflow = 1;
		} else {
			Tk03_Overflow = 0;
			if (Level_Tk03 <= 5){
				Tk03_Lowlevel = 1;
			} else {
				Tk03_Lowlevel = 0;
			}
		}

		Level_Tk03 = CheckLimit(0, Level_Tk03, 1000);

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
