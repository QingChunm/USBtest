#ifndef USBFUNCTION_H
#define USBFUNCTION_H

class UsbFunction
{
public:
	UsbFunction();
	~UsbFunction(void);

	void LoadFileHandle(HANDLE handle);

	BOOL UFRunCode(BYTE *pParamBuf, UINT dataLength, char *pDataBuf , BOOL bDataOut2Device);
    BOOL UFISPCode(BYTE *pParamBuf, UINT dataLength, char *pDataBuf , BOOL bDataOut2Device);
    BOOL UFUpdate(void);

private:
	BYTE	ScsiCBW_Buf[16];
	HANDLE	fileHandle;
};

#endif