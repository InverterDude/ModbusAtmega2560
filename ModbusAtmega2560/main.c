/*
 * ModbusAtmega2560.c
 *
 * Created: 04.02.2016 14:31:39
 * Author : m.dereschkewitz
 *
 * FreeModbus Libary: AVR Demo Application
 *
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 */ 

#define F_OSC 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- Defines ------------------------------------------*/
#define REG_INPUT_START 1000
#define REG_INPUT_NREGS 4

#define REG_HOLDING_START 1000
#define REG_HOLDING_NREGS 10

/* ----------------------- Static variables ---------------------------------*/
static USHORT   usRegInputStart = REG_INPUT_START;
static USHORT   usRegInputBuf[REG_INPUT_NREGS];
static USHORT   usRegHoldingStart = REG_HOLDING_START;
static USHORT   usRegHoldingBuf[REG_HOLDING_NREGS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};


/* ----------------------- Start implementation -----------------------------*/
int main(void)
{
	const UCHAR     ucSlaveID[] = { 0xAA, 0xBB, 0xCC };
	eMBErrorCode    eStatus;

	DDRB |= (1 << PINB7);
	PORTB |= (1 << PINB7);

	eStatus = eMBInit( MB_RTU, 0x0A, 0, 9600, MB_PAR_NONE );

	// eStatus = eMBSetSlaveID( 0x34, TRUE, ucSlaveID, 3 );
	sei(  );

	/* Enable the Modbus Protocol Stack. */
	eStatus = eMBEnable(  );

	for( ;; )
	{
		( void )eMBPoll(  );

		/* Here we simply count the number of poll cycles. */
		usRegInputBuf[0]++;
		usRegHoldingBuf[0]++;
	}
}

eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
	return MB_ENOREG;
}

eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
	eMBErrorCode    eStatus = MB_ENOERR;
	int             iRegIndex = 0;

	switch(eMode)
	{
		case MB_REG_READ:
			
			if(usAddress >= REG_HOLDING_START && usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS)
			{
				iRegIndex = ( int )( usAddress - usRegInputStart );
				while( usNRegs > 0 )
				{
					*pucRegBuffer++ = ( unsigned char )( usRegHoldingBuf[iRegIndex] >> 8 );
					*pucRegBuffer++ = ( unsigned char )( usRegHoldingBuf[iRegIndex] & 0xFF );
					iRegIndex++;
					usNRegs--;
				}
			}
			else eStatus = MB_ENOREG;
		break;

		case MB_REG_WRITE:
			if(usAddress >= REG_HOLDING_START && usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS)
			{

				if(PORTB && PINB7) PORTB &= ~(1 << PINB7);
				else PORTB |= (1 << PINB7);
				
				while( usNRegs > 0 )
				{
					iRegIndex = ( int )( usAddress - usRegInputStart );
					usRegHoldingBuf[iRegIndex] = ( (USHORT) (*pucRegBuffer++ & 0xFF) << 8) + ( (USHORT) *pucRegBuffer++ & 0xFF);;
					iRegIndex++;
					usNRegs--;
				}

			}
			else eStatus = MB_ENOREG;
		break;
	}

	return eStatus;

}



eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,
eMBRegisterMode eMode )
{
	eMBErrorCode eStatus = MB_ENOERR;

	if(usAddress == 1 && usNCoils == 1 && eMode == MB_REG_READ)
	{
		if(PORTB && PINB7) PORTB &= ~(1 << PINB7);
		else PORTB |= (1 << PINB7);
		
		*pucRegBuffer = 1;
	}

	return eStatus;
}

eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
	return MB_ENOREG;
}