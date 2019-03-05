#include "Misc\stdafx.h"

#include <basetyps.h>
#include <setupapi.h>

#include "Usb\UsbCmd\UsbFunction.h"
#include "Usb\ScsiAcc.h"



#define UF_MODE_CODE		0xCB

UsbFunction::UsbFunction()
{

}

UsbFunction::~UsbFunction(void)
{

}

void UsbFunction::LoadFileHandle(HANDLE handle)
{
	fileHandle = handle;
}

//‘À––UF≥Ã–Ú
BOOL UsbFunction::UFRunCode(BYTE *pParamBuf, UINT dataLength, char *pDataBuf , BOOL bDataOut2Device)
{

	BOOL bCmdOptSuccess;

	memset(ScsiCBW_Buf,0,sizeof(ScsiCBW_Buf));
	ScsiCBW_Buf[0] = UF_MODE_CODE;

	memcpy(&ScsiCBW_Buf[1],pParamBuf,(sizeof(ScsiCBW_Buf) - 1));

	if (bDataOut2Device)
	{
		bCmdOptSuccess = WriteToScsi(fileHandle,sizeof(ScsiCBW_Buf),ScsiCBW_Buf, dataLength, pDataBuf);
	}
	else
	{
		bCmdOptSuccess = ReadFromScsi(fileHandle,sizeof(ScsiCBW_Buf),ScsiCBW_Buf, dataLength, pDataBuf);
	}

	return bCmdOptSuccess;
}

BOOL UsbFunction::UFISPCode(BYTE *pParamBuf, UINT dataLength, char *pDataBuf , BOOL bDataOut2Device)
{

	BOOL bCmdOptSuccess;

	memset(ScsiCBW_Buf,0,sizeof(ScsiCBW_Buf));
	ScsiCBW_Buf[0] = 0xCD;

	memcpy(&ScsiCBW_Buf[1],pParamBuf,(sizeof(ScsiCBW_Buf) - 1));

	if (bDataOut2Device)
	{
		bCmdOptSuccess = WriteToScsi(fileHandle,sizeof(ScsiCBW_Buf),ScsiCBW_Buf, dataLength, pDataBuf);
	}
	else
	{
		bCmdOptSuccess = ReadFromScsi(fileHandle,sizeof(ScsiCBW_Buf),ScsiCBW_Buf, dataLength, pDataBuf);
	}

	return bCmdOptSuccess;
}

BOOL UsbFunction::UFUpdate(void){

	memset(ScsiCBW_Buf, 0, sizeof(ScsiCBW_Buf));

	ScsiCBW_Buf[0] = 0xDA;	 

	ReadFromScsi(fileHandle, sizeof(ScsiCBW_Buf), ScsiCBW_Buf, 0, NULL);

    return TRUE;
}

