#ifndef AX32XXDEVICE_H
#define AX32XXDEVICE_H


#include "Usb\UsbCmd\UsbFunction.h"
#include "Usb\UsbCmd\SpiDriver.h"

//#include "ufc/SpiDriver.h"
//#include "xmlDataInfo.h"
//
//#include "struct.h"

#include <tchar.h>
#include <string>

//enum {
//	ERR_DEFAULT,
//	ERR_SPI_ID,
//	ERR_SPI_VERIFY,
//	ERR_USB_DEVICE,
//	ERR_ISO_PATH,
//	ERR_ISO_SIZE,
//	ERR_SPI_SIZE,
//	ERR_PR_FORMAT,
//	ERR_VIDPID_FORMAT,
//	ERR_CHIP_VER,
//	ERR_CFG_OTHER,
//	ERR_CFG,
//	ERR_FORMAT,
//	ERR_DENIED,
//
//	ERR_EXPORT,
//};

union USB_CMD{
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
};

#define	MAX_DRV_DATA_TRANS_SIZE             0x400
struct ELF_INFO
{
	//SDS  SpiDriverStatus;
	DWORD  DriverFileLen;
	DWORD  DriverLoadAddr;

	void*   Unique;
	char    *Name;
	char    *Firmware;
	struct POF_SYM  *Pubsym;
	char DriverBuf[MAX_DRIVER_SIZE];
	char ReadBuf[512];
	char WriteBuf[512];     
	//char DriverBuf[10];
	//char ReadBuf[10];
	//char WriteBuf[10];  
};



class AX32XXDevice
{
public:
    AX32XXDevice(const wchar_t* filename, const wchar_t* uvcName, const wchar_t* devLocation);
    ~AX32XXDevice(void);

	int _flash_id;  //Flash ID
	int _flash_capacity;  //Flash 容量
	char *_flash_name;  //Flash 名称
	int if_have_father;
	int if_have_child;

	char *_down_code;  //实际下载的程序
	int _down_code_len;  //实际下载程序的长度
	char *_rd_buf;  //Read Buffer

	int lib_id;

	USB_CMD  UsbCmd;
	UsbFunction *uf;
	//ELF_INFO ElfInfo;


	BYTE _cmd_read;  //u8 data RdCmd _at_ 0x68;  //读 0x03
	BYTE _cmd_write;  //u8 data WrCmd _at_ 0x69;  //写 0x02
	BYTE _cmd_erase;  //u8 data ErCmd _at_ 0x6a;  //擦 0xd8
	BYTE _write_len;  //u8 data WrPerLen _at_ 0x6b;  //每次写的长度。0表示256Byte
	BYTE _cmd_readstatus;  //u8 data RdStaRegCmd _at_ 0x6c;  //读状态寄存器 0x05
	BYTE _cmd_writestatus;  //u8 data WrStaRegCmd _at_ 0x6d;  //写寄存器 0x01
	BYTE _cmd_writestatus_en;  //u8 data EnWrStaRegCmd _at_ 0x6e;  //写寄存器使能开 0x50
	BYTE _cmd_write_en;  //u8 data EnWrCmd _at_ 0x6f;  //写使能开 0x06

public:
	void AX32XXDevice::AnalyzeElf(ELF_INFO *Dev);

protected:
	virtual int LoadDriver() = 0;
	virtual BOOL InstallDriver(void);
	virtual BOOL CheckDriver(void);
	virtual int InitDev(void) = 0;
	//virtual int DeviceReset(void);
	virtual int ProbeDev(void);

public:
    const wchar_t* GetDevLocation();
    const wchar_t* GetUvcInterface();
	int GetTarget(void);

	int SpiReadID(int cmdid);
	void CheckCapacity(void);

	//virtual BOOL SpiWriteEnable(void);
	//virtual BOOL SpiReadStatusReg(void);
    virtual bool SpiWrite(char *wrbuf, int Addr, UINT Length, BOOL bEncrypt) = 0;
    virtual int SpiErase(int Addr, int Length) = 0;
	virtual bool SpiRead(int Addr,UINT Length, char* ReadBuf, BOOL bEncrypt) = 0;
	void Soft_crc16 (WORD poly, char val, WORD *crc); 
	virtual void DownCode(std::wstring str);
	virtual void InitDownloadParam(void);
	virtual bool InitDebugParam(void) = 0;
	virtual void DownloadPart(int index);
	virtual void ExportSpiCodeToBin(std::wstring str);
	virtual int DeviceReset(void);
	virtual void DownBinCode(int addr) {};
	//virtual int PreProbe(void);

	SpiDriver *spiDriver;

	int LoadCodeIntoBuffer(int FlashCapacity, std::wstring pDownCodePath);
    int SpiSearchID(int cmdid, int id);

	void SetStatus(int status);
	int GetStatus(void);

protected:
    std::wstring m_devLocation = L"";
    std::wstring m_uvcName = L"";
    HANDLE m_fileHandle = NULL;

    std::wstring m_FlashLibPath = L"";
	std::wstring m_ExportPath = L"";
};


#define SPI_SIZE_ADDR					17
#define SPI_CODE_TOTAL_LENGTH_ADDR		24
#define LOADER1_CRC16_ADDR				28	



#endif/*AX32XXDEVICE_H*/
