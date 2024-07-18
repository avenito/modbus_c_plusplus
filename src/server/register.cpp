#include "ModbusServer.h"

int ModbusServer::procReadReg(char* msgMB){

	int msgIndex = 4;
	int startAdd;
	int numRegs;

	startAdd 	= msgMB[8] << 8  | msgMB[9];
	numRegs  	= msgMB[10] << 8 | msgMB[11];

	startAdd 	-= REGISTER_OFFSET;

	if (DEBUG_LEVEL >= DBG_LEVEL_02) {
		cout << "Start address: " << startAdd << endl;
		cout << "Requested registers: " << numRegs << endl;
	}

	/* Checking the address limit */
	if ((startAdd >= 0) && ((startAdd + numRegs) <= REGISTERS)) {
		/* Address limits ok */
		len = 3 + numRegs * 2;
		msgMB[msgIndex++] = (unsigned) len >> 8;
		msgMB[msgIndex++] = (unsigned) len * 0xff;
		msgMB[msgIndex++] = (unsigned) msgMB[6] & 0xff;
		msgMB[msgIndex++] = (unsigned) msgMB[7] & 0xff;
		msgMB[msgIndex++] = (unsigned) (numRegs * 2) & 0xff;
		for (int r = 0; r <= numRegs; r++){
			msgMB[msgIndex++] = (unsigned) registers[startAdd + r] >> 8;
			msgMB[msgIndex++] = (unsigned) registers[startAdd + r] & 0xff;
		}
	} else {
		/*Out of the address limits */
		msgMB[7] += 0x80;
		msgMB[8] = EXCEP_ILLEGAL_DATA_ADD;
		len = 3;
		if (DEBUG_LEVEL >= DBG_LEVEL_02) {
			cout << "Illegal address!" << endl << endl;
		}
	}

	return (len + 6);
}
