#include "Misc\stdafx.h"

#include <stdio.h>
#include <iostream>

#include "SpiDriver.h"

//extern TCHAR *curpath;
//unsigned int func_MemReadWrite; 



///using namespace std;
SpiDriver::SpiDriver()
{
	uf = new UsbFunction;
    enc_start = 0;
}

SpiDriver::~SpiDriver()
{
    delete uf;
}

void SpiDriver::LoadFileHandle(HANDLE handle)
{
	fileHandle = handle;
	uf->LoadFileHandle(fileHandle);
}

bool SpiDriver::SpiInstallDriver(void)
{

	bool bSuccess;
	memset((BYTE*)&SpiDriverParam,0,0x10);
	SpiDriverCtrl.SpiDriverStatus = SDS_UNOK;

	SpiDriverParam.W.L3.Func1 = ENDIAN_HALF(SpiDriverParam.Func_MemberRW);
	//SpiDriverParam.DataAddr = 
	SpiDriverParam.W.L3.Func2 = 0xffffffff;
	/**/
	for (int i=0; i < SpiDriverCtrl.File_SpiDriver_Length; i+=MAX_DRV_DATA_TRANS_SIZE)
	{
		SpiDriverParam.W.L3.DataAddr = ENDIAN_HALF(SpiDriverParam.SpiWriteAdr+i);
		bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam), MAX_DRV_DATA_TRANS_SIZE, &SpiDriverCtrl.SpiDriverBuf[i], USB_WRITE);
		if (!bSuccess)
		{	
			return FALSE;
		}
	}

	SpiDriverCtrl.SpiDriverStatus = SDS_OK;

	return TRUE;
#if 0
	bool bSuccess;
	memset((BYTE*)&SpiDriverParam,0,0x10);
	SpiDriverCtrl.SpiDriverStatus = SDS_UNOK;

	SpiDriverParam.W.L2.Func1 = SpiDriverParam.Func_MemberRW;
	//SpiDriverParam.DataAddr = 
	SpiDriverParam.W.L2.Func2 = 0xffffffff;
	//??
	for (int i=0; i < SpiDriverCtrl.File_SpiDriver_Length; i+=MAX_DRV_DATA_TRANS_SIZE)
	{
		SpiDriverParam.W.L2.DataAddr = SpiDriverParam.SpiWriteAdr+i;
		bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam), MAX_DRV_DATA_TRANS_SIZE, &SpiDriverCtrl.SpiDriverBuf[i], USB_WRITE);
		if (!bSuccess)
		{	
			return FALSE;
		}
	}

	SpiDriverCtrl.SpiDriverStatus = SDS_OK;

	return TRUE;
#endif
}

bool SpiDriver::SpiCheckDriver(void)
{
	bool bSuccess;
    memset((BYTE*)&SpiDriverParam,0,0x10);
	SpiDriverCtrl.SpiDriverStatus = SDS_INSTALLED;

	SpiDriverParam.W.L3.Func1 = ENDIAN_HALF(SpiDriverParam.Func_MemberRW);
	SpiDriverParam.W.L3.Func2 = 0xffffffff;

	//SpiReadBuf max size is 0x200
	for (int i=0; i < SpiDriverCtrl.File_SpiDriver_Length; i+=MAX_DATA_TRANS_SIZE)
	{
		SpiDriverParam.W.L3.DataAddr = ENDIAN_HALF(SpiDriverParam.SpiWriteAdr+i);
		bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam), MAX_DATA_TRANS_SIZE, SpiDriverCtrl.SpiReadBuf, USB_READ);
		if (!bSuccess)
		{	
			return FALSE;
		}
		if (memcmp(&SpiDriverCtrl.SpiDriverBuf[i],SpiDriverCtrl.SpiReadBuf,MAX_DATA_TRANS_SIZE))
		{
			SpiDriverCtrl.SpiDriverStatus = SDS_DESTROYED;
			return FALSE;
		}
	}

	SpiDriverCtrl.SpiDriverStatus = SDS_OK;
	return TRUE;
#if 0
	bool bSuccess;
    memset((BYTE*)&SpiDriverParam,0,0x10);
	SpiDriverCtrl.SpiDriverStatus = SDS_INSTALLED;

	SpiDriverParam.W.L2.Func1 = SpiDriverParam.Func_MemberRW;
	SpiDriverParam.W.L2.Func2 = 0xffffffff;

	//SpiReadBuf max size is 0x200
	for (int i=0; i < SpiDriverCtrl.File_SpiDriver_Length; i+=MAX_DATA_TRANS_SIZE)
	{
		SpiDriverParam.W.L2.DataAddr = SpiDriverParam.SpiWriteAdr+i;
		bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam), MAX_DATA_TRANS_SIZE, SpiDriverCtrl.SpiReadBuf, USB_READ);
		if (!bSuccess)
		{	
			return FALSE;
		}
		if (memcmp(&SpiDriverCtrl.SpiDriverBuf[i],SpiDriverCtrl.SpiReadBuf,MAX_DATA_TRANS_SIZE))
		{
			SpiDriverCtrl.SpiDriverStatus = SDS_DESTROYED;
			return FALSE;
		}
	}

	SpiDriverCtrl.SpiDriverStatus = SDS_OK;
	return TRUE;
#endif
}

bool SpiDriver::SpiPortInit(void)
{
	if (SpiDriverCtrl.SpiDriverStatus != SDS_OK)
	{
		return FALSE;
	}
    
	bool bSuccess;
	memset((BYTE*)&SpiDriverParam,0,0x10);

	SpiDriverParam.W.L3.Func1 =  ENDIAN_HALF(SpiDriverParam.Func_SpiPortInit);

	bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam),0,NULL,USB_WRITE);

	if (!bSuccess)
	{	
		return FALSE;
	}

	return TRUE;
#if 0
	if (SpiDriverCtrl.SpiDriverStatus != SDS_OK)
	{
		return FALSE;
	}
    
	bool bSuccess;
	memset((BYTE*)&SpiDriverParam,0,0x10);

	SpiDriverParam.W.L1.Func1 = SpiDriverParam.Func_SpiPortInit;

	bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam),0,NULL,USB_WRITE);

	if (!bSuccess)
	{	
		return FALSE;
	}

	return TRUE;
#endif
}

bool SpiDriver::SpiReadID(BYTE cmdid)
{
	if (SpiDriverCtrl.SpiDriverStatus != SDS_OK)
	{
		return FALSE;
	}

	bool bSuccess;
	memset((BYTE*)&SpiDriverParam,0,0x10);

	if(0 == FirmStyle)
	{	
		SpiDriverParam.W.L1.Func1 = SpiDriverParam.Func_SpiSignalDrv;
		SpiDriverParam.W.L1.Ctrl = 0x3;
		SpiDriverParam.W.L1.SiLen = 1;
		SpiDriverParam.W.L1.Si[0] = cmdid;
	}
	else{
		SpiDriverParam.W.L2.Func1 = SpiDriverParam.Func_MemberRW;
		SpiDriverParam.W.L2.DataAddr = 0xffffffff;
		SpiDriverParam.W.L2.Func2 = SpiDriverParam.Func_DriverCheck; 
	}
	bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam),4,SpiDriverCtrl.SpiReadBuf,USB_READ);
	
	if (!bSuccess)
	{	
		return FALSE;
	}

	return TRUE;
}

bool SpiDriver::GetEfuse(void)
{
	bool bSuccess;
	memset((BYTE*)&SpiDriverParam,0,0x10);

	SpiDriverParam.W.L2.Func1 = SpiDriverParam.Func_MemberRW;
	SpiDriverParam.W.L2.DataAddr = 0xffffffff;
	SpiDriverParam.W.L2.Func2 = SpiDriverParam.Func_EfuseRead; 
		
	bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam),16,SpiDriverCtrl.SpiReadBuf,USB_READ);
	
	if (!bSuccess)
	{	
		return FALSE;
	}

	return TRUE;
}

bool SpiDriver::GetTarget(void)
{
	bool bSuccess;
	memset((BYTE*)&SpiDriverParam,0,0x10);

	SpiDriverParam.W.L3.Func1 = ENDIAN_HALF(SpiDriverParam.Func_ReadTargetSta);
	bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam),16,SpiDriverCtrl.SpiReadBuf,USB_READ);

	if (!bSuccess)
	{	
		return FALSE;
	}

	return TRUE;
}

bool SpiDriver::PreProcessRam(void)
{
	if (SpiDriverCtrl.SpiDriverStatus != SDS_OK)
	{
		return FALSE;
	}
    if(0x6832 == SpiDriverParam.ChipInfo) //tiga 
	{
		bool bSuccess;
		memset((BYTE*)&SpiDriverParam,0,0x10);

		SpiDriverParam.W.L1.Func1 = SpiDriverParam.Func_PreProcessRam;

		bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam),0,NULL,USB_WRITE);

		if (!bSuccess)
		{	
			return FALSE;
		}
	}
	return TRUE;
}

bool SpiDriver::SpiReadStatusReg(void)
{
#if 1
	if (SpiDriverCtrl.SpiDriverStatus != SDS_OK)
	{
		return FALSE;
	}
	
	bool bSuccess;
	memset((BYTE*)&SpiDriverParam,0,0x10);

	SpiDriverParam.W.L1.Func1 = SpiDriverParam.Func_SpiSignalDrv;
	SpiDriverParam.W.L1.Ctrl = 0x7;
	SpiDriverParam.W.L1.SiLen = 1;
	SpiDriverParam.W.L1.Si[0] = 0x5;
		
	bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam),4,SpiDriverCtrl.SpiReadBuf,USB_READ);
	if(SpiDriverCtrl.SpiReadBuf[0] & 0x01){
		return FALSE;
	}
	if (!bSuccess)
	{	
		return FALSE;
	}

#endif	
	return TRUE;
}

bool SpiDriver::SpiWriteEnable(void)
{
	if (SpiDriverCtrl.SpiDriverStatus != SDS_OK)
	{
		return FALSE;
	}
	
	bool bSuccess;
	memset((BYTE*)&SpiDriverParam,0,0x10);


	SpiDriverParam.W.L1.Func1 = SpiDriverParam.Func_SpiSignalDrv;
	SpiDriverParam.W.L1.Ctrl = 0x3;
	SpiDriverParam.W.L1.SiLen = 1;
	SpiDriverParam.W.L1.Si[0] = 0x06;

	bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam),0,NULL,USB_WRITE);
	//SpiReadStatusReg(); 
	if (!bSuccess)
	{	
		return FALSE;
	}
	return TRUE;
}


bool SpiDriver::SpiErase(UINT SpiAddr)
{
	if (SpiDriverCtrl.SpiDriverStatus != SDS_OK)
	{
		return FALSE;
	}
	
	bool bSuccess;
	memset((BYTE*)&SpiDriverParam,0,0x10);

   /* SpiWriteEnable();

	SpiDriverParam.W.L1.Func1 = SpiDriverParam.Func_Erase;
	SpiDriverParam.DataAddr = (SpiAddr<<8)|0xd8;*/
	
	SpiDriverParam.W.L1.Func1 = SpiDriverParam.Func_SpiSignalDrv;
	SpiDriverParam.W.L1.Ctrl = 0x7;
	SpiDriverParam.W.L1.SiLen = 4;
	SpiDriverParam.W.L1.Si[0] = 0xD8;
	SpiDriverParam.W.L1.Si[1] = (BYTE)(SpiAddr >> 16);
	SpiDriverParam.W.L1.Si[2] = (BYTE)(SpiAddr >> 8);
	SpiDriverParam.W.L1.Si[3] = (BYTE)(SpiAddr >> 0);

	bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam),0,NULL,USB_WRITE);
	if (!bSuccess)
	{	
		return FALSE;
	}
	if (!SpiReadStatusReg())
	{	
		return FALSE;
	}

	return TRUE;
}

bool SpiDriver::SpiRead(UINT ReadAddr, UINT ReadLength, char *ReadBuf, BYTE bEncrypt)
{
	if (SpiDriverCtrl.SpiDriverStatus != SDS_OK)
	{
		return FALSE;
	}
	if (ReadLength > 65536)
	{
		return FALSE;
	}
	
	bool bSuccess;
	memset((BYTE*)&SpiDriverParam,0,0x10);

	int i,PageSize;
	
	for (i=0; i<ReadLength; i+=PageSize)
	{
		PageSize = 0x200;

		if (PageSize > ReadLength-i) PageSize = ReadLength-i;

		SpiDriverParam.W.L1.Func1 = SpiDriverParam.Func_SpiSignalDrv;
		SpiDriverParam.W.L1.Ctrl = 0x7;
		SpiDriverParam.W.L1.SiLen = 4;
		SpiDriverParam.W.L1.Si[0] = 0x3;
		SpiDriverParam.W.L1.Si[1] = (BYTE)((ReadAddr + i) >> 16);
		SpiDriverParam.W.L1.Si[2] = (BYTE)((ReadAddr + i) >> 8);
		SpiDriverParam.W.L1.Si[3] = (BYTE)((ReadAddr + i) >> 0);
		//??
		bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam),PageSize,ReadBuf+i,USB_READ);
		//bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam),ReadLength,ReadBuf,FALSE);

		if (!bSuccess)
		{	
			return FALSE;
		}
	}
	
	return TRUE;
}

void SpiDriver::Soft_crc16 (WORD poly, char val, WORD *crc) 
{
	int i;

	for (i=0; i<8; ++i) {
		if (((val>>7)^(*crc>>15))&1) *crc = (*crc<<1) ^ poly;
		else *crc <<= 1;
		val <<= 1;
	}
}

void SpiDriver::SetCRC(char *srcData,int srcDataLen)
{
	WORD crc;
	BYTE *sec_num;
	crc = 0xffff;
	//crc = 0;
	ULONG i,crc_start,crc_len,crc_len_s,crc_len_f;
	sec_num = (BYTE*)(srcData + srcData[9] * 16 + 0x14); // 加载扇区号
	crc_start = ((sec_num[3]<<24)|(sec_num[2]<<16)|(sec_num[1]<<8)|sec_num[0])<<9;
	sec_num = (BYTE*)(srcData + srcData[9] * 16 + 0x18); // 加载扇区个数
	crc_len = ((sec_num[3]<<24)|(sec_num[2]<<16)|(sec_num[1]<<8)|sec_num[0])<<9;

	char *crcData= (char *)malloc(crc_len);
	memset(crcData,0xff,crc_len);
	if((srcDataLen-crc_start)>crc_len)
	   memcpy(crcData,srcData+crc_start,crc_len);
	else
	   memcpy(crcData,srcData+crc_start,srcDataLen-crc_start);
	for(i=0;i<crc_len;i++)
	{
		Soft_crc16 (0x1021, *(crcData+i), &crc);
	}
	free(crcData);
	*(DWORD*)(srcData + srcData[9] * 16 + 0x20) = crc;
}

void SpiDriver::SetChecksum(char *srcData)
{
	//bootsect校验byte
	BYTE CheckSum = 0;
	ULONG i;
	for(i=0;i<0x200;i++)
	{
		CheckSum += srcData[i]; 
	}
	srcData[8] = ~CheckSum + 1;
}

bool SpiDriver::SetEncryptAddr(char *srcData,int srcDataLen)
{
	//berry mpw version 
	if(0xffffffff == SpiDriverParam.Func_SpiGetHeader)
	{
		return true;
	}

	BYTE *sec_num;
	//WORD crc;
	if (  srcData[0] == 0 && 
		  srcData[1] == 0 && 
		  srcData[4] == 'B' && 
		  srcData[5] == 'L' && 
		  srcData[6] == 'D' && 
		  srcData[7] == 'R' && 
		  srcData[0x1fe] == 0x55 && 
		  srcData[0x1ff] == (char)0xAA         //bootsect标志位判断
		)  
	{
       //可以将checksum和crc放到上一层
		SetCRC(srcData,srcDataLen);

		if(srcData[10] & (1<<4)){                //加密标志位判断
			srcData[10] |= 0x80; // 修改加密标志
			sec_num = (BYTE*)(srcData + srcData[9] * 16 + 0x14); // 加载扇区号
			enc_start = ((sec_num[3]<<24)|(sec_num[2]<<16)|(sec_num[1]<<8)|sec_num[0])<<9;

			bool bSuccess;
			memset((BYTE*)&SpiDriverParam,0,0x10);

			SpiDriverParam.W.L2.Func1 = SpiDriverParam.Func_MemberRW;
			SpiDriverParam.W.L2.DataAddr = SpiDriverParam.Func_MemberRWBuf;
			SpiDriverParam.W.L2.Func2 = SpiDriverParam.Func_SpiGetHeader;
			SpiDriverParam.W.L2.Param =  0;

			bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam), 0x200, srcData, USB_WRITE);

			if (!bSuccess)
			{	
				return FALSE;
			}
		}

		SetChecksum(srcData);

	}else{
        enc_start = 0; 
	}

	return TRUE;
}

bool SpiDriver::SpiWrite(UINT WriteAddr, UINT WriteLength, char *WriteBuf, BYTE SpiWriteLengthPreOnce, BYTE bEncrypt)
{
	if (SpiDriverCtrl.SpiDriverStatus != SDS_OK)
	{
		return FALSE;
	}
	if (WriteLength > 65536)
	{
		return FALSE;
	}
	
	bool bSuccess;
	memset((BYTE*)&SpiDriverParam,0,0x10);

	ULONG enc;

	UINT i,PageSize,WritePageAddr;
	
	for (i=0; i<WriteLength; i+=PageSize)
	{
		PageSize = 0x100;

		if (PageSize > WriteLength-i) PageSize = WriteLength-i;
		
		if (!enc_start || enc_start > WriteAddr + i)
			enc = 0;
		else {
			if (enc_start == WriteAddr + i) enc = 0x20000;
			else                          enc = 0x10000;
			if (i+PageSize>=WriteLength) enc |= 0x80000;
			else             enc |= 0x40000;
		}
		/*
		if(!m_ThreadParameter[DevID].pProcessFlash->Flash_WR_Page(0xb600,((Addr+i)>>8)|enc,srcData+i))
		{
			return false;
			if(GetLastError() == 0x000037)
			{
				return false;
			}
		}*/
		WritePageAddr = ((WriteAddr + i)>>8)|enc; 

		SpiDriverParam.W.L2.Func1 = SpiDriverParam.Func_MemberRW;
		SpiDriverParam.W.L2.DataAddr = SpiDriverParam.Func_MemberRWBuf;
		SpiDriverParam.W.L2.Func2 = SpiDriverParam.Func_SpiPageProgram;
		SpiDriverParam.W.L2.Param =  WritePageAddr;

		bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam), 0x100, WriteBuf+i, USB_WRITE);
		
		if (!bSuccess)
		{	
			return FALSE;
		}

	}
	return TRUE;
}


bool SpiDriver::EfuseWrite(char *WriteBuf)
{
	if (SpiDriverCtrl.SpiDriverStatus != SDS_OK)
	{
		return FALSE;
	}
	
	bool bSuccess;

	SpiDriverParam.W.L2.Func1= SpiDriverParam.Func_MemberRW;
	SpiDriverParam.W.L2.DataAddr = 0xffffffff;
	SpiDriverParam.W.L2.Func2 = SpiDriverParam.Func_EfuseWrite; 
		
	bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam),16,WriteBuf,USB_WRITE);
	
	if (!bSuccess)
	{	
		return FALSE;
	}

	return TRUE;
}

bool SpiDriver::EepromWrite(char *WriteBuf,UINT WriteLength,UINT WriteAddr)
{
	if (SpiDriverCtrl.SpiDriverStatus != SDS_OK)
	{
		return FALSE;
	}
	
	bool bSuccess;
	memset((BYTE*)&SpiDriverParam,0,0x10);

	SpiDriverParam.W.L2.Func1 = SpiDriverParam.Func_MemberRW;
	SpiDriverParam.W.L2.DataAddr = 0xffffffff;
	SpiDriverParam.W.L2.Func2 = SpiDriverParam.Func_EepromWrite; 
	SpiDriverParam.W.L2.Param = WriteAddr; 

	bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam),256,WriteBuf,USB_WRITE);
	
	if (!bSuccess)
	{	
		return FALSE;
	}

	return TRUE;
}

bool SpiDriver::BerryWrite(char *WriteBuf,UINT WriteLength,UINT WriteAddr)
{
	if (SpiDriverCtrl.SpiDriverStatus != SDS_OK)
	{
		return FALSE;
	}
	
	bool bSuccess;
	memset((BYTE*)&SpiDriverParam,0,0x10);

	SpiDriverParam.W.L3.Func1 = ENDIAN_HALF(SpiDriverParam.Func_EepromWrite);
	//SpiDriverParam.W.L2.DataAddr = 0xffffffff;
	//SpiDriverParam.W.L2.Func2 = SpiDriverParam.Func_EepromWrite; 
	//SpiDriverParam.W.L2.Param = WriteAddr; 

	bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam),256,WriteBuf,USB_WRITE);
	
	if (!bSuccess)
	{	
		return FALSE;
	}

	return TRUE;
}

bool SpiDriver::SpiReset(void)
{
#if 1
	if (0xffffffff == SpiDriverParam.Func_SpiReset)
	{
		return TRUE;
	}
	
	bool bSuccess;
	memset((BYTE*)&SpiDriverParam,0,0x10);

	if(0x6832 == SpiDriverParam.ChipInfo)//tiga
	{	
		SpiDriverParam.W.L2.Func1 = 0;
		SpiDriverParam.W.L2.DataAddr = 0x4;
		SpiDriverParam.W.L2.Func2 = 0;
		SpiDriverParam.W.L2.Param =  0;
		bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam),0,NULL,USB_WRITE);

		memset((BYTE*)&SpiDriverParam,0,0x10);
		SpiDriverParam.W.L2.Func1 = 0;
		SpiDriverParam.W.L2.Param = 0x4;
		SpiDriverParam.W.L2.Func2 = SpiDriverParam.Func_SpiReset;
		bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam),0,NULL,USB_WRITE);
	}/**/
	if(0xff32 == SpiDriverParam.ChipInfo) //"mpwx"
	{
		SpiDriverParam.W.L2.Func1 = SpiDriverParam.Func_SpiReset;
		bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam),0,NULL,USB_WRITE);
	}
	if(0x0032 == SpiDriverParam.ChipInfo) //"berry"
	{
		//SpiDriverParam.W.L2.Func1 = SpiDriverParam.Func_SpiReset;
		//bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam),0,NULL,USB_WRITE);
	}
	if(0x7032 == SpiDriverParam.ChipInfo) //visual"
	{
		memset((BYTE*)&SpiDriverParam,0,0x10);
		SpiDriverParam.W.L2.Func1 = 0;
		SpiDriverParam.W.L2.Param = 0x4;
		SpiDriverParam.W.L2.Func2 = SpiDriverParam.Func_SpiReset;
		bSuccess = uf->UFRunCode((BYTE *)&(SpiDriverParam),0,NULL,USB_WRITE);
	}

	if (!bSuccess)
	{	
		return FALSE;
	}

#endif	
	return TRUE;
}

/*
void SpiDriver::BE2LE(DWORD *pBEBuffer,DWORD *pLEBuffer){
    DWORD BEData = *pBEBuffer;   
	DWORD LEData;
    LEData = (BEData&0xff000000)>>24;
    LEData |= ((BEData&0xff0000)>>16)<<8;
	LEData |= ((BEData&0xff00)>>8)<<16;
	LEData |= (BEData&0xff)<<24;
	*pLEBuffer = LEData;

}*/

void SpiDriver::SpiGetParam(BYTE *pBuffer)
{
	//计算出参数个数,并置为0xffffffff
	BYTE  ParamNum = (BYTE)(&SpiDriverParam.Func_DriverCheck - (DWORD *)&SpiDriverParam.ChipInfo);
	memset(&SpiDriverParam.ChipInfo, 0xffffffff, ParamNum*4);
	//固件程序第一个word是低两byte是ID，第二byte是加载参数个数，接下来是函数入口，
	//BYTE  ParamLoadNum = SpiDriverParam.ChipInfo & 0xff00 >> 8;
	SpiDriverParam.ParamLoadNum = pBuffer[2];
	memcpy(&SpiDriverParam.ChipInfo, (BYTE*)&pBuffer[0], SpiDriverParam.ParamLoadNum*4);
	BYTE BE2LE_Flag = *pBuffer;

	DWORD *pBuf = &SpiDriverParam.SpiWriteAdr;
}



