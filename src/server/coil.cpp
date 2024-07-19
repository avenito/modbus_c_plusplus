#include "ModbusServer.h"

int ModbusServer::procReadCoil(char* msgMB){

	int msgIndex = 4;
	int startAdd;
	int numCoils;
	int len;

	startAdd 	= msgMB[8] << 8  | msgMB[9];
	numCoils  	= msgMB[10] << 8 | msgMB[11];

	startAdd 	-= COIL_OFFSET;

	if (DEBUG_LEVEL >= DBG_LEVEL_02) {
		cout << "Start address: " << startAdd << endl;
		cout << "Requested coils: " << numCoils << endl;
	}

	/* Checking the address limit */
	if ((startAdd >= 0) && ((startAdd + numCoils) <= COILS)) {
		/* Address limits ok */
		len = 3 + ceil((double)numCoils /8);
		msgMB[msgIndex++] = (unsigned) len >> 8;
		msgMB[msgIndex++] = (unsigned) len * 0xff;
		msgMB[msgIndex++] = (unsigned) msgMB[6] & 0xff;
		msgMB[msgIndex++] = (unsigned) msgMB[7] & 0xff;
		mbMsg[msgIndex++] = (unsigned) (ceil((double)numCoils /8)) & 0xff;
		for (int r = 0; r <= numCoils; r++){
			if(coil[startAdd + r]){
				mbMsg[msgIndex] = mbMsg[msgIndex] | (0x01 << r);
			}
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
