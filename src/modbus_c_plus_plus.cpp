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
#define FLOWRATE_TK01	5
#define FLOWRATE_VL01	5
#define FLOWRATE_VL02	5
#define FLOWRATE_VL03	5
#define FLOWRATE_VL04	5

using namespace std;

/* Instantiates the modbus server */
/* Instancia o modbus server */

ModbusServer	Server = ModbusServer();
int				FRV01, FRV02;

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
 * 0 - Level Tank 01 / Nível Caixa 01
 * 1 - Level Tank 02 / Nível Caixa 02
 * 2 - Level Tank 03 / Nível Caixa 03
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
 * 1 - Tank 01 overflowing / Caixa 01 transbordando
 * 2 - Caixa 02 transbordando
 * 3 - Caixa 03 transbordando
 * 4 - Caixa 01 nível baixo
 * 5 - Caixa 02 nível baixo
 * 6 - Caixa 03 nível baixo
 */

#define Bomba_Ligada	Server.discrete_inputs[0]
#define Cx01_Transb		Server.discrete_inputs[1]
#define Cx02_Transb		Server.discrete_inputs[2]
#define Cx03_Transb		Server.discrete_inputs[3]
#define Cx01_Baixo		Server.discrete_inputs[4]
#define Cx02_Baixo		Server.discrete_inputs[5]
#define Cx03_Baixo		Server.discrete_inputs[6]

/* Coils:
 *
 * 0 - Ligar Bomba
 */

#define Liga_Bomba		Server.coils[0]


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
		Bomba_Ligada = Liga_Bomba;

		if (Bomba_Ligada) {
			if (RPM_FB >= 100){
				RPM_FB = 100;
			} else {
				if (RPM_FB == 0){RPM_FB = 5;}
				RPM_FB += RPM_FB * 0.5;
			}
		} else {
			if (RPM_FB <= 0){
				RPM_FB = 0;
			} else {
				RPM_FB -= 25;
			}
		}

		/* Caixa 01 */
		if (Bomba_Ligada && (RPM_FB > 0)){
			Level_Tk01 += FLOWRATE_TK01;
		}
		if (Level_Tk01 >= 100){
			Cx01_Transb = 1;
		} else {
			Cx01_Transb = 0;
		}

		/* Valvula 01 ajustavel - Liga Cx01 - Cx02 */
		Valv01_FB = Setpoint_Valv01;
		if (Valv01_FB > 0 && Level_Tk01 > 0){
			FRV01 = (FLOWRATE_VL01 * Valv01_FB / 100);
			Level_Tk01 -= FRV01; // esvazia Cx01
			Level_Tk02 += FRV01; // enche Cx02
		}
		/* Valvula 02 ajustavel - Liga Cx01 - Cx03 */
		Valv02_FB = Setpoint_Valv02;
		if (Valv02_FB > 0 && Level_Tk01 > 0){
			FRV02 = (FLOWRATE_VL02 * Valv02_FB / 100);
			Level_Tk01 -= FRV02; // esvazia Cx01
			Level_Tk03 += FRV02; // enche Cx03
		}

		/* Valvula 03 on/off - Saida Cx02 */
		Cx01_Baixo = Server.coils[1];
		if (Cx01_Baixo && Level_Tk02 > 0){
			Level_Tk02 -= FLOWRATE_VL03;
		}
		if (Level_Tk02 >= 100){
			Cx02_Transb = 1;
		} else {
			Cx02_Transb = 0;
		}


		/* Valvula 04 on/off - Saida Cx03 */
		Server.discrete_inputs[5] = Server.coils[2];
		if (Server.discrete_inputs[5] && Level_Tk03 > 0){
			Level_Tk03 -= FLOWRATE_VL04;
		}
		if (Level_Tk03 >= 100){
			Cx03_Transb = 1;
		} else {
			Cx03_Transb = 0;
		}

		/* Check limites */
		Level_Tk01 = CheckLimit(Level_Tk01, 100, 0);
		Level_Tk02 = CheckLimit(Level_Tk02, 100, 0);
		Level_Tk03 = CheckLimit(Level_Tk03, 100, 0);
		RPM_FB = CheckLimit(RPM_FB, 100, 0);
		Valv01_FB = CheckLimit(Valv01_FB, 100, 0);
		Valv02_FB = CheckLimit(Valv02_FB, 100, 0);

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
