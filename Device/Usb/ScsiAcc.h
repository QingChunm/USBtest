
#ifndef		_SCSI_ACC_
#define		_SCSI_ACC_

//�������Scsi����
BOOL ReadFromScsi(HANDLE fileHandle,int cdbLen,void *cdb,int dataLen,char *data);

//��������Scsiд��
BOOL WriteToScsi(HANDLE fileHandle,int cdbLen,void *cdb,int dataLen,char *data);

// Fucntion define.....

int SearchAutoMp3Device(char **ppUsbName);
BOOL OpenAutoMp3Dev(char *pDeviceName);
void CloseAutoMp3Dev();

extern unsigned int gwSectorSize;

//extern HANDLE fileHandle;

#include "ntddscsi.h"
#include <setupapi.h>
#include <basetyps.h>

#endif

