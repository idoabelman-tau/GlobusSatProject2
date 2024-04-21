#include "TRXVU.h"

#define RECIEVER_I2C_ADDRESS 0x60
#define TRANSMITTER_I2C_ADDRESS 0x61

int InitTrxvu(){
	//definitions

	ISIStrxvuI2CAddress TRX_I2C_ADDR_STRUCT;
	ISIStrxvuFrameLengths TRX_FRAME_LENGTH_STRUCT;
	ISIStrxvuBitrate TRX_BIT_RATE_STRUCT;
	int TRX_ERR_FLAG;

	//set TRX default values
	TRX_I2C_ADDR_STRUCT.addressVu_rc = RECIEVER_I2C_ADDRESS;
	TRX_I2C_ADDR_STRUCT.addressVu_tc = TRANSMITTER_I2C_ADDRESS;

	TRX_FRAME_LENGTH_STRUCT.maxAX25frameLengthTX = SIZE_TXFRAME;
	TRX_FRAME_LENGTH_STRUCT.maxAX25frameLengthRX = SIZE_RXFRAME;

	TRX_BIT_RATE_STRUCT = trxvu_bitrate_1200;  //TODO: set correct bit

	// initialize
	TRX_ERR_FLAG = IsisTrxvu_initialize(&TRX_I2C_ADDR_STRUCT,&TRX_FRAME_LENGTH_STRUCT,&TRX_BIT_RATE_STRUCT,1);

	if(TRX_ERR_FLAG != E_NO_SS_ERR && TRX_ERR_FLAG != E_IS_INITIALIZED){
		//TODO: log error
		return  TRX_ERR_FLAG;
	}

	return E_NO_SS_ERR;
}

