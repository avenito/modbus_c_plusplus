#include "ModbusServer.h"

/* Read */
int ModbusServer::procReadReg(char* msgMB){

	int msgIndex = 4;
	int startAdd;
	int numRegs;
	int len;

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

/* Write Single */
int ModbusServer::procWriteSingReg(char* msgMB){

	int msgIndex = 10;
	int addr;
	int val;
	int len;

	addr = msgMB[8] << 8  | msgMB[9];
	val  = msgMB[10] << 8 | msgMB[11];

	addr -= REGISTER_OFFSET;

	if (DEBUG_LEVEL >= DBG_LEVEL_02) {
		cout << "Register address: " << addr << endl;
		cout << "Value: " << val << endl;
	}

	/* Checking the address limit */
	if ((addr >= 0) && (addr <= REGISTERS)) {
		/* Address limits ok */
		len = 12;
		registers[addr] = (unsigned) mbMsg[msgIndex++] << 8;
		registers[addr] |= (unsigned) mbMsg[msgIndex] & 0xff;
	} else {
		/*Out of the address limits */
		msgMB[7] += 0x80;
		msgMB[8] = EXCEP_ILLEGAL_DATA_ADD;
		len = 3;
		if (DEBUG_LEVEL >= DBG_LEVEL_02) {
			cout << "Illegal address!" << endl << endl;
		}
	}

	return (len);
}

/* Write Multiple */
int ModbusServer::procWriteMultReg(char* msgMB){

	int msgIndex = 13;
	int startAddr;
	int numRegs;
	int len;

	startAddr = msgMB[8] << 8  | msgMB[9];
	numRegs  = msgMB[10] << 8 | msgMB[11];

	startAddr -= REGISTER_OFFSET;

	if (DEBUG_LEVEL >= DBG_LEVEL_02) {
		cout << "Register address: " << startAddr << endl;
		cout << "Value: " << numRegs << endl;
	}

	/* Checking the address limit */
	if ((startAddr >= 0) && ((startAddr + numRegs) <= REGISTERS)) {
		/* Address limits ok */
		for (int r = 0; r <= numRegs; r++){
			registers[startAdd + r] = (unsigned) mbMsg[msgIndex++] << 8;
			registers[startAdd + r] |= (unsigned) mbMsg[msgIndex++] & 0xff;
		}
		msgIndex = 4;
		len = 6;
		mbMsg[msgIndex++] = (unsigned) len >> 8;
		mbMsg[msgIndex++] = (unsigned) len & 0xff;
	} else {
		/*Out of the address limits */
		msgMB[7] += 0x80;
		msgMB[8] = EXCEP_ILLEGAL_DATA_ADD;
		len = 3;
		if (DEBUG_LEVEL >= DBG_LEVEL_02) {
			cout << "Illegal address!" << endl << endl;
		}
	}

	return (len);
}
