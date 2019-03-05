#ifndef _SPI_DRIVER_
#define _SPI_DRIVER_

#include "UsbFunction.h"

#define USB_WRITE    true
#define USB_READ     false

#define SPI_RUN_CODE_ADDR					0x2200
#define	MAX_DRIVER_SIZE						0xc000//0x1400		//	(0x2200+0xE00 = 0x3000)
#define MAX_DATA_TRANS_SIZE					0x200
#define	MAX_DRV_DATA_TRANS_SIZE             0x400

#define ENDIAN_HALF(x) (WORD)(((x)>>8)|((x)<<8))

struct SPI_PARAM
{
	union{
		  struct{
			WORD Func1;
			WORD DataAddr;
			WORD Param1;
			WORD Param2;
			DWORD Func2;
			DWORD Param;
		  }L3;
          struct{
			DWORD Func1;
			DWORD DataAddr;
			DWORD Func2;
			DWORD Param;
		  }L2;
		  struct{
		    DWORD Func1;
			BYTE  Param;
			BYTE  Ctrl;
			BYTE  SiLen;
			BYTE  Si[5];
		  }L1;
	}W;

	WORD ChipInfo;
	WORD ParamLoadNum;
	DWORD SpiWriteAdr;
	DWORD Func_MemberRW;
	DWORD Func_MemberRWBuf;
	DWORD Func_SpiPortInit;
	DWORD Func_SpiSignalDrv;
	DWORD Func_SpiPageProgram;
	DWORD Func_SpiReset;
	DWORD Func_SpiGetHeader;
	DWORD Func_PreProcessRam;
    //DWORD Func_SpiReadID;
    //DWORD Func_Erase;
	//DWORD Func_SpiCheckStatus;
	//DWORD Func_SpiRead;

	DWORD Func_DriverCheck;
	DWORD Func_EfuseRead;
	DWORD Func_EfuseWrite;
	DWORD Func_EepromWrite;
	DWORD Func_ReadTargetSta;

};


#define BIT(n) (1<<(n))

/*#define SPI_VCSR_BUSY_WAIT			BIT(0)
#define SPI_VCSR_ENCRYPT			BIT(1)
#define SPI_VCSR_WRITE_ONEBYONE		BIT(2)
#define SPI_VCSR_WRITE_CONTINUOUS	BIT(3)
#define SPI_VCSR_READ			(BIT(2) | BIT(3))
#define SPI_VCSR_MINIENCRYPT		BIT(4)
#define SPI_VCSR_FIXKEY				BIT(5)
*/
/**************************************************************************
SPI CMD 虚拟控制状态寄存器 CmdVCSR:（通用接口使用）
bit0 : BUSY WAIT (执行该命令时是否等待Busy)
bit1 ：ENCRYPTION （是否加密）
bit3、2 ：00 -> 无Data传输
          01 -> Data Out （SPI Write Data ---- Write Data One by One）
          10 -> Data Out （SPI Write Data ---- Data Continuous Operating）
          11 -> Data In  （SPI Read Data  ---- Continuous）
bit4 : MINI_ENCRYPT  （拆分成256byte进行独立加密）
bit5 :
bit6 :
bit7 :
**************************************************************************/


typedef enum
{
	SDS_UNOK,
	SDS_INSTALLED,
	SDS_OK,
	SDS_DESTROYED,
	SDS_UNINSTALL,	
} SDS;

struct SPI_CTRL
{
	SDS  SpiDriverStatus;
	int  File_SpiDriver_Length;

	char SpiDriverBuf[MAX_DRIVER_SIZE];
	char SpiReadBuf[512];
	char SpiWriteBuf[512];
};

class SpiDriver 
{
public:
	SpiDriver();
	~SpiDriver(void);

	void LoadFileHandle(HANDLE handle);

	bool SpiInstallDriver(void);
	bool SpiCheckDriver(void);
	bool SpiPortInit( void);
	bool SpiReadID(BYTE cmdid);
	bool GetEfuse(void);
	bool GetTarget(void);
	bool PreProcessRam(void);
	bool SpiReadStatusReg( void);
	bool SpiWriteEnable(void);
	bool SpiErase(UINT SpiAddr);
	bool SpiRead(UINT ReadAddr, UINT ReadLength, char *ReadBuf, BYTE bEncrypt);
	void Soft_crc16 (WORD poly, char val, WORD *crc);
	void SetCRC(char *srcData,int srcDataLen);
	void SetChecksum(char *srcData);
	bool SetEncryptAddr(char *srcData,int srcDataLen);
	bool SpiWrite(UINT WriteAddr, UINT WriteLength, char *WriteBuf, BYTE SpiWriteLengthPreOnce, BYTE bEncrypt);
	bool SpiDriver::EfuseWrite(char *WriteBuf);
	bool SpiDriver::EepromWrite(char *WriteBuf,UINT WriteLength,UINT WriteAddr);
	bool SpiDriver::BerryWrite(char *WriteBuf,UINT WriteLength,UINT WriteAddr);
	bool SpiReset(void);
	void SpiGetParam(BYTE *pBuffer);

	//BOOL ChipVersion(void);
	//BOOL SpiSetParam( void);
	//void SpiDriver::BE2LE(DWORD *pBEBuffer,DWORD *pLEBuffer);
	//BOOL SpiGenericInterfaceHandle(  BYTE cmd, BYTE ParamLen,BYTE param1,BYTE param2,BYTE param3,BYTE param4,BYTE spiVCSR);

	SPI_CTRL  SpiDriverCtrl;
	SPI_PARAM SpiDriverParam;
	UsbFunction *uf;
	unsigned long enc_start;

	DWORD FirmStyle;
private:

	HANDLE fileHandle;
};



#endif