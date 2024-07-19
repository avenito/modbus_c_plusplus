#include "ModbusServer.h"

int ModbusServer::procReadInput(char* msgMB){

	int msgIndex = 4;
	int startAdd;
	int numInputs;
	int len;

	startAdd 	= msgMB[8] << 8  | msgMB[9];
	numInputs  	= msgMB[10] << 8 | msgMB[11];

	startAdd 	-= INPUT_OFFSET;

	if (DEBUG_LEVEL >= DBG_LEVEL_02) {
		cout << "Start address: " << startAdd << endl;
		cout << "Requested inputs: " << numInputs << endl;
	}

	/* Checking the address limit */
	if ((startAdd >= 0) && ((startAdd + numInputs) <= INPUTS)) {
		/* Address limits ok */
		len = 3 + numInputs * 2;
		msgMB[msgIndex++] = (unsigned) len >> 8;
		msgMB[msgIndex++] = (unsigned) len * 0xff;
		msgMB[msgIndex++] = (unsigned) msgMB[6] & 0xff;
		msgMB[msgIndex++] = (unsigned) msgMB[7] & 0xff;
		msgMB[msgIndex++] = (unsigned) (numInputs * 2) & 0xff;
		for (int r = 0; r <= numInputs; r++){
			msgMB[msgIndex++] = (unsigned) inputs[startAdd + r] >> 8;
			msgMB[msgIndex++] = (unsigned) inputs[startAdd + r] & 0xff;
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
