#include "Misc\stdafx.h"
#include "ScsiAcc.h"

#include <cstddef>

#include <winioctl.h>
#include <Ntddscsi.h>
#include "..\3rd\include\spti\spti.h"


// 53f56307-b6bf-11d0-94f2-00a0c91efb8b
//GUID  GUID_CLASS_USBSTOR[2] = 
//{
//	{	// DISK
//		0x53f56307,0xb6bf,0x11d0,
//		(BYTE)0x94,(BYTE)0xf2,
//		(BYTE)0x00,(BYTE)0xa0,(BYTE)0xc9,(BYTE)0x1e,(BYTE)0xfb,(BYTE)0x8b
//	},
//	{	// CDROM
//		0x53f56308,0xb6bf,0x11d0,
//		(BYTE)0x94,(BYTE)0xf2,
//		(BYTE)0x00,(BYTE)0xa0,(BYTE)0xc9,(BYTE)0x1e,(BYTE)0xfb,(BYTE)0x8b
//	}
//};


//发命令从Scsi读出
BOOL ReadFromScsi(HANDLE fileHandle,int cdbLen,void *cdb,int dataLen,char *data)
{
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
	BOOL	status;
	ULONG	length = 0,
			returned = 0;
	int retry_times = 10;
	int lasterr;
	do {
		sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
		sptdwb.sptd.PathId = 0;
		sptdwb.sptd.TargetId = 1;
		sptdwb.sptd.Lun = 0;
		sptdwb.sptd.CdbLength = cdbLen;		//CDB命令的长度
		sptdwb.sptd.SenseInfoLength = 26;	//24;
		sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_IN;	//读数据
		sptdwb.sptd.DataTransferLength = dataLen;//sectorSize;	//读取数据的长度
		sptdwb.sptd.TimeOutValue = 200;		//响应超时时间
		sptdwb.sptd.DataBuffer = data;		//读取的数据的存放指针
		sptdwb.sptd.SenseInfoOffset =
		   offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, ucSenseBuf);

		memcpy(sptdwb.sptd.Cdb, cdb, cdbLen);
		length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
		status = DeviceIoControl(fileHandle,
								 IOCTL_SCSI_PASS_THROUGH_DIRECT,
								 &sptdwb,
								 length,
								 &sptdwb,
								 length,
								 &returned,
								 FALSE);
		if (status == 0) {
			lasterr = GetLastError();
			if (lasterr == 55) {
				return status;
			}
			Sleep(20);
			if (retry_times-- > 0) {
				continue;
			}
		}
		return status;
	} while (1);
}

//发命令向Scsi写入
BOOL WriteToScsi(HANDLE fileHandle,int cdbLen,void *cdb,int dataLen,char *data)
{
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
	BOOL	status;
	ULONG	length = 0,
			returned = 0;
	int retry_times = 10;
	int lasterr;
	do {
		sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
		sptdwb.sptd.PathId = 0;
		sptdwb.sptd.TargetId = 1;
		sptdwb.sptd.Lun = 0;
		sptdwb.sptd.CdbLength = cdbLen;
		sptdwb.sptd.SenseInfoLength = 26;	//24;
		sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_OUT;	//写数据
		sptdwb.sptd.DataTransferLength = dataLen;//sectorSize;
		sptdwb.sptd.TimeOutValue = 200;
		sptdwb.sptd.DataBuffer = data;
		sptdwb.sptd.SenseInfoOffset =
		   offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);

		memcpy(sptdwb.sptd.Cdb, cdb, cdbLen);
		length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
		status = DeviceIoControl(fileHandle,
								 IOCTL_SCSI_PASS_THROUGH_DIRECT,
								 &sptdwb,
								 length,
								 &sptdwb,
								 length,
								 &returned,
								 FALSE);
		if (status == 0) {
			lasterr = GetLastError();  //错误号31表示发送阻塞
			if (lasterr == 55) {  //错误号55表示设备已掉
				return status;
			}
			Sleep(20);
			if (retry_times-- > 0) {
				continue;
				//return status;
			}
		}
		return status;
	} while (1);
}



//
//
//// New Function
//BOOL ReadDevCapacity()
//{
//	unsigned char pBuffer[32];
//	unsigned char cdb[32];
//
//	memset(&cdb, 0, 16);
//	memset(pBuffer,0, 8);
//
//	gwSectorSize = 512;
//
//	cdb[0]  = 0x25;
//	if(ReadFromScsi(fileHandle, 0x0A, (void*)&cdb, 8, (char *)pBuffer))	//读取的buffer大小必须为64的整数倍
//	{
//		if( (pBuffer[4]|pBuffer[5]|pBuffer[6]|pBuffer[7]) == 0)
//			return FALSE;
//		else
//		{
//			gwSectorSize = (pBuffer[6] << 8) | pBuffer[7];
//			return TRUE;
//		}
//	}
//
//	return FALSE;
//
//}
///*
//BOOL GetDisksProperty(HANDLE hDevice, PSTORAGE_DEVICE_DESCRIPTOR pDevDesc)
//{
//	STORAGE_PROPERTY_QUERY	Query;	
//	DWORD dwOutBytes;				
//	BOOL bResult;					
//
//	Query.PropertyId = StorageDeviceProperty;
//	Query.QueryType = PropertyStandardQuery;
//
//	bResult = ::DeviceIoControl(hDevice,			
//			IOCTL_STORAGE_QUERY_PROPERTY,			
//			&Query, sizeof(STORAGE_PROPERTY_QUERY),	
//			pDevDesc, pDevDesc->Size,				
//			&dwOutBytes,							
//			(LPOVERLAPPED)NULL);					
//
//	TRACE(_T("LastError:%i\n"),GetLastError());
//	return bResult;
//}
//*/
//
////--------------------------------------------------------------
////查找CDROM设备
////--------------------------------------------------------------
//int SearchAutoMp3Device(char **ppUsbName)
//{
//	int NumberValidDev;
//	int iGuid;
//
//	ULONG NumberDevices;
//	HDEVINFO                 hardwareDeviceInfo;
//	SP_INTERFACE_DEVICE_DATA deviceInfoData;
//	ULONG                    i;
//	BOOLEAN                  done;
//	PUSB_DEVICE_DESCRIPTOR   usbDeviceInst;
//	PUSB_DEVICE_DESCRIPTOR	*UsbDevices = &usbDeviceInst;
//	
//	*UsbDevices = NULL;
//	
//	NumberValidDev = 0;	// Valid number of disk devices..
//
//	for(iGuid = 0; iGuid < 2; iGuid++)
//	{
//
//		NumberDevices = 0;	// Total number of disk devices
//
//		// Open a handle to the plug and play dev node.
//		// SetupDiGetClassDevs() returns a device information set that contains info on all
//		// installed devices of a specified class.
//		hardwareDeviceInfo = SetupDiGetClassDevs (
//			&GUID_CLASS_USBSTOR[iGuid],
//			NULL, // Define no enumerator (global)
//			NULL, // Define no
//			(DIGCF_PRESENT | // Only Devices present
//			DIGCF_INTERFACEDEVICE)); // Function class devices.
//		
//		// Take a wild guess at the number of devices we have;
//		// Be prepared to realloc and retry if there are more than we guessed
//		NumberDevices = 4;
//		done = FALSE;
//		deviceInfoData.cbSize = sizeof (SP_INTERFACE_DEVICE_DATA);
//		
//		i=0;
//		while (!done) {
//			NumberDevices *= 2;
//			
//			if (*UsbDevices) {
//				*UsbDevices = 
//					(PUSB_DEVICE_DESCRIPTOR)realloc (*UsbDevices, (NumberDevices * sizeof (USB_DEVICE_DESCRIPTOR)));
//			} else {
//				*UsbDevices = (PUSB_DEVICE_DESCRIPTOR)calloc (NumberDevices, sizeof (USB_DEVICE_DESCRIPTOR));
//			}
//			
//			if (NULL == *UsbDevices) {
//				// SetupDiDestroyDeviceInfoList destroys a device information set
//				// and frees all associated memory.
//				SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
//				break;	// 
//			}
//			
//			usbDeviceInst = *UsbDevices + i;
//			for (; i < NumberDevices; i++) 
//			{
//				// SetupDiEnumDeviceInterfaces() returns information about device interfaces
//				// exposed by one or more devices. Each call returns information about one interface;
//				// the routine can be called repeatedly to get information about several interfaces
//				// exposed by one or more devices.
//				
//				if (SetupDiEnumDeviceInterfaces (hardwareDeviceInfo,
//					0, // We don't care about specific PDOs
//					&GUID_CLASS_USBSTOR[iGuid],
//					i,
//					&deviceInfoData)) 
//				{
//					PSP_INTERFACE_DEVICE_DETAIL_DATA     functionClassDeviceData = NULL;
//					ULONG                                predictedLength = 0;
//					ULONG                                requiredLength = 0;
//					// allocate a function class device data structure to receive the
//					// goods about this particular device.
//					SetupDiGetInterfaceDeviceDetail (
//						hardwareDeviceInfo,
//						&deviceInfoData,
//						NULL, // probing so no output buffer yet
//						0, // probing so output buffer length of zero
//						&requiredLength,
//						NULL); // not interested in the specific dev-node
//					
//					
//					predictedLength = requiredLength;	// sizeof (SP_FNCLASS_DEVICE_DATA) + 512;
//					functionClassDeviceData = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc (predictedLength);
//					functionClassDeviceData->cbSize = sizeof (SP_INTERFACE_DEVICE_DETAIL_DATA);
//					
//					// Retrieve the information from Plug and Play.
//					if ( SetupDiGetInterfaceDeviceDetail (
//						hardwareDeviceInfo,
//						&deviceInfoData,
//						functionClassDeviceData,
//						predictedLength,
//						&requiredLength,
//						NULL))
//					{
//						if(0 == strnicmp(functionClassDeviceData->DevicePath, "\\\\?\\USBSTOR", 11))
//						{
//							ppUsbName[NumberValidDev] = (char*)malloc(strlen(functionClassDeviceData->DevicePath) + 16);
//							strcpy( ppUsbName[NumberValidDev], functionClassDeviceData->DevicePath);
//							NumberValidDev++;
//						}
//					}
//					free( functionClassDeviceData );
//				} // SetupDiEnumDeviceInterfaces
//				else 
//				{
//					if (ERROR_NO_MORE_ITEMS == GetLastError()) 
//					{
//						done = TRUE;
//						break;
//					}
//				}
//			}	// for (; i < NumberDevices; i++) 
//		}	// while (!done) 
//
//	} //	for(iGuid = 0; iGuid < 2; iGuid++)
//	
//	// SetupDiDestroyDeviceInfoList() destroys a device information set
//	// and frees all associated memory.
//	SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
//	free ( *UsbDevices );
//	return NumberValidDev;
//}
//
////关闭设备的读写
//void CloseAutoMp3Dev()
//{
//	if (fileHandle != INVALID_HANDLE_VALUE)
//	{
//		CloseHandle(fileHandle);
//		fileHandle = INVALID_HANDLE_VALUE;
//	}
//
//}
//
//BOOL OpenAutoMp3Dev(char *pDeviceName)
//{
//	fileHandle = CreateFile(pDeviceName,
//		GENERIC_WRITE | GENERIC_READ,
//		FILE_SHARE_WRITE | FILE_SHARE_READ,
//		NULL,
//		OPEN_EXISTING,
//		0,
//		NULL);
//
//	if (fileHandle != INVALID_HANDLE_VALUE)
//	{
//		ReadDevCapacity();
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//}
//
//



