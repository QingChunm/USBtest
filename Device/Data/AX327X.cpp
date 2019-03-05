#include "Misc\stdafx.h"

#include "AX327X.h"
#include "elf/pof.h"

#include <Shlwapi.h>

#include <codecvt>

#define IIC_PAGE_SIZE			8
#define SPI_BLOCK_SIZE			(64*1024)
#define CODE_BANK_SIZE			512//(2*1024)
#define SPI_PAGE_SIZE			(4*1024)

#define SPI_RESOURCE_INFO_ADDR	0x10

#define ISO_BUFFER_LEN isoBufferSize

//extern int SpiCsTotal[100];
extern int spi_cs_order;

extern int	ExportSpiCodeToFile;
extern HINSTANCE m_hInst;			//默认资源
extern HINSTANCE m_hInst_Lang;		//语言
//extern int CodVer[100];
//extern char readid_a[4][2];
//int _SpiVersion;
//读ID的四种方式
static char readid_a_[4][2] = {
	{(BYTE)0x9f, (BYTE)0x00},
	{(BYTE)0xab, (BYTE)0x03},
	{(BYTE)0x90, (BYTE)0x03},
	{(BYTE)0x15, (BYTE)0x00},
};/**/



#if 0
//extern	DWORD u32efuse_scr[4];
//extern	DWORD u32efuse_dst[4] ={0,0,0,0};
//extern	DWORD u8dev_sel = 0;

int first2k_en_mode;

#if SMART_MP_SAVE_TO_RAM
extern char *downCodeBackupBuf;
#endif

#define MYWM_SET_PERCENT ::RegisterWindowMessage(_T("SET_PERCENT"))
#define MYWM_PRINT_LOGR ::RegisterWindowMessage(_T("PRINT_LOG_RELEASE"))  //显示某个状态，并自动释放内存
#define SET_PER(x) ::SendMessage(hMainWnd, MYWM_SET_PERCENT, (WPARAM)(x), (LPARAM)_index);

//printf_log 中显示的log，需要保证全程的临界性
int pritf_index;
extern CRITICAL_SECTION g_MpSet;

#endif


#if 1
AX327X::AX327X(const wchar_t* filename, const wchar_t* uvcName, const wchar_t* devLocation) : AX32XXDevice(filename, uvcName, devLocation)
{
	/*_handle = handle;
	iDeviceVesion = DeviceVesion;
	index = _index;

	uf = new UsbFunction;
	uf->LoadFileHandle(_handle);	

	_down_code = NULL;
	_down_code_len = 0;
	*/
	tCurPath = new TCHAR[MAX_PATH];
	pDownCodePath = new TCHAR[MAX_PATH];

#if 0
	_cbw_buf = new char[32];
	_rd_buf = new char[SPI_BLOCK_SIZE];
	_flash_name = new char[100];
	//_fat_buf = NULL;
	_removed = 0;
	_handle = handle;
	_drive = drive;
	_index = index;
	
	_down_code_len = 0;
	if_have_father = 0;
	//_precopy_len = 0;
	_down_code = NULL;
	_lasterr = NULL;
	iDeviceVesion = DeviceVesion;
	//_SpiVersion = SpiVersion;

	_flash_capacity = 0;
	spiDriver = new SpiDriver;

	spiDriver->LoadFileHandle(_handle);	
#endif
}
#endif
AX327X::~AX327X (void)
{
#if 0
	delete[] _cbw_buf;
	delete[] _rd_buf;
	delete[] _flash_name;
#endif
	//delete[] _down_code;
	//delete[] _fat_buf;

/*
	if (_handle != NULL)
	{
		::CloseHandle(_handle);
		_handle = NULL;
	}
	delete uf;
*/
}

static uint32_t Code2xDataOffset;
static uint32_t func_MemReadWrite;
//static uint32_t func_probe;
//static uint32_t func_tgtrw;
static uint32_t func_Fini;
static uint32_t func_DriverCheck;
static uint32_t func_EepromWrite;
//static uint32_t func_Reset;

//static uint32_t func_MemReadWrite;
static uint32_t func_MemRWBuf;
static uint32_t func_Init;
static uint32_t func_SignalDrv;
static uint32_t func_PageProgram;
static uint32_t func_Reset;
static uint32_t func_GetHeader;
static uint32_t func_PreProcessRam;
static uint32_t func_eeprom_check;
static uint32_t func_CloseFunc;
static uint32_t func_DebugISP;
static uint32_t buf_UsbRx;

static POF_SYM pubsym[] = {
    {(uint32_t*)&func_MemReadWrite, SYM_TYPE_VMA,  "RBC_mem_rwex_DMA"},
	{(uint32_t*)&func_MemRWBuf,     SYM_TYPE_VMA,  "RBC_mem_rwex_buf"},
	{(uint32_t*)&func_Init,         SYM_TYPE_VMA,  "l1_func_spi_init"},
	{(uint32_t*)&func_SignalDrv,    SYM_TYPE_VMA,  "l1_func_signal_drive"},
    {(uint32_t*)&func_PageProgram,  SYM_TYPE_VMA,  "l2_func_spi_page_program"},
	{(uint32_t*)&func_Reset,        SYM_TYPE_VMA,  "l2_func_reset"},
	{(uint32_t*)&func_GetHeader,    SYM_TYPE_VMA,  "getheader2"},
	{(uint32_t*)&func_PreProcessRam,SYM_TYPE_VMA,  "l1_func_preprocess"},
    {NULL,0,NULL}
};

//下载固件程序
int AX327X::LoadDriver(void){
	//char *cpath;
	//cpath = new char[MAX_PATH];
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    std::string multiCharPath = conv.to_bytes(tSettingElfPath);


	//ELF_INFO ElfInfo;
	ElfInfo.Pubsym  = pubsym;
    ElfInfo.Name = const_cast<char*>(multiCharPath.c_str());
	AnalyzeElf(&ElfInfo);

    //delete [] cpath;

	if (!InstallDriver())
	{
		//throw new Msg(ERR_USB_DEVICE, _T("Install Driver Error"));
        return -1;
	}
	if (!CheckDriver())
	{
		//throw new Msg(ERR_USB_DEVICE, _T("Check Driver Error"));
        return -2;
	}
    return 0;
}

BOOL AX327X::InstallDriver(void)
{
	BOOL bSuccess;
	memset((BYTE*)&UsbCmd,0,0x10);

	UsbCmd.L2.Func1 = func_MemReadWrite;
	//UsbCmd.L3.DataAddr = 
	UsbCmd.L2.Func2 = 0xffffffff;

	for (DWORD i=0; i < ElfInfo.DriverFileLen; i+=MAX_DRV_DATA_TRANS_SIZE)
	{
		UsbCmd.L2.DataAddr = ElfInfo.DriverLoadAddr + i;
		bSuccess = uf->UFRunCode((BYTE *)&(UsbCmd), MAX_DRV_DATA_TRANS_SIZE, ElfInfo.DriverBuf+i, USB_WRITE);
		if (!bSuccess)
		{	
			return FALSE;
		}
	}
    return TRUE;
}

BOOL AX327X::CheckDriver(void)
{
	BOOL bSuccess;
	memset((BYTE*)&UsbCmd,0,0x10);

	UsbCmd.L2.Func1 = func_MemReadWrite;
	UsbCmd.L2.Func2 = 0xffffffff;

	//SpiReadBuf max size is 0x200
	for (DWORD i=0; i < ElfInfo.DriverFileLen; i+=MAX_DATA_TRANS_SIZE)
	{
		UsbCmd.L2.DataAddr = ElfInfo.DriverLoadAddr + i;
		bSuccess =  uf->UFRunCode((BYTE *)&(UsbCmd), MAX_DATA_TRANS_SIZE, ElfInfo.ReadBuf, USB_READ);
		if (!bSuccess)
		{	
			return FALSE;
		}
		if (memcmp(ElfInfo.DriverBuf + i,ElfInfo.ReadBuf,MAX_DATA_TRANS_SIZE))
		{
			return FALSE;
		}
	}
	return TRUE;
}

//初始化设备
int AX327X::InitDev(void)
{
	BOOL bSuccess;
	memset((BYTE*)&UsbCmd,0,0x10);

	UsbCmd.L2.Func1 = func_Init;
	bSuccess = uf->UFRunCode((BYTE *)&(UsbCmd),0,NULL,USB_WRITE);
	if (!bSuccess)
	{
        return -1;
		//throw new Msg(ERR_USB_DEVICE, _T("Spi Init Error"));
	}
    return 0;
}

int AX327X::DeviceReset(void)
{
	BOOL bSuccess;
	memset((BYTE*)&UsbCmd,0,0x10);
/*
	UsbCmd.L2.Func1 = 0;
	UsbCmd.L2.DataAddr = 0x4;
	UsbCmd.L2.Func2 = 0;
	UsbCmd.L2.Param =  0;
	bSuccess = uf->UFRunCode((BYTE *)&(UsbCmd),0,NULL,USB_WRITE);
	*/
	memset((BYTE*)&UsbCmd,0,0x10);
	UsbCmd.L2.Func1 = 0;
	UsbCmd.L2.Param = 0x4;
	UsbCmd.L2.Func2 =func_Reset;
	bSuccess = uf->UFRunCode((BYTE *)&(UsbCmd),0,NULL,USB_WRITE);

	return 0;
}

//获取设备信息
int AX327X::ProbeDev(void)
{
#if 1
	int i;
	int flash_ids[4];  //保存4个ID值

	for (i=0; i<4; i++) {
		flash_ids[i] = SpiReadID(readid_a_[i][0]);
	}

	for (i=0; i<4; i++) {
		if (SpiSearchID(i, flash_ids[i])) {
			return 1;
		}
	}

    //TODO:这里写个日志吧
	//throw new Msg(ERR_SPI_ID,_T("FlashID ERROR! ID-9F=%08X;"), flash_ids[0]);/*ID-AB=%08X; ID-90=%08X; ID-15=%08X;*/
	return 0;

#endif

}

//SPI 读ID
int AX327X::SpiReadID(int cmdid)
{
	BOOL bSuccess;
	memset((BYTE*)&UsbCmd,0,0x10);

	UsbCmd.L1.Func1 = func_SignalDrv;
	UsbCmd.L1.Ctrl = 0x3;
	UsbCmd.L1.SiLen = 1;
	UsbCmd.L1.Si[0] = cmdid;

	bSuccess = uf->UFRunCode((BYTE *)&(UsbCmd),4,ElfInfo.ReadBuf,USB_READ);
	
	if (!bSuccess)
	{	
        return -1;
		//throw new Msg(ERR_USB_DEVICE, _T("USB ERROR SPI[9f]"));
	}

	return ((int)(
		((unsigned char)ElfInfo.ReadBuf[0] << 24)
		+ ((unsigned char)ElfInfo.ReadBuf[1] << 16)
		+ ((unsigned char)ElfInfo.ReadBuf[2] << 8)
		+ ((unsigned char)ElfInfo.ReadBuf[3])
		));
}

//检查Flash容量是否有效
void AX327X::CheckCapacity(void)
{
#if 0
    int flashsize, codelength;
	if (_flash_capacity < SPI_BLOCK_SIZE) {
		return;								//非Flash，就不校验了
	}
    flashsize = _flash_capacity;
	flashsize = flashsize >> 1;
	codelength = _down_code_len;
	if (flashsize > codelength)				//剩余空间大于一半才进行检测。防止写坏资源
	{
		CHAR data1[512], data2[512], data3[512];

		memset(data1, 0x5a, 512);
		memset(data2, 0xa5, 512);

		SpiErase(0,1);
		SpiWrite(data1, 0, 512, FALSE);

		SpiErase(flashsize, 1);
		SpiWrite(data2, flashsize, 512, FALSE);

		SpiRead(0, 512, data3, FALSE);
		if (memcmp(data1, data3, 512))
		{
			//throw new TargetMsg(ERR_SPI_SIZE, _T("FlashLib ERROR:ID-9F=%08X; ID-AB=%08X; ID-90=%08X; ID-15=%08X; LIBNUM=%d;"), 
		  flash_ids[0], flash_ids[1], flash_ids[2], flash_ids[3], lib_id);

		}
		SpiRead(flashsize, 512, data3, FALSE);
		if (memcmp(data2, data3, 512))
		{
		//	//throw new TargetMsg(ERR_SPI_SIZE, "FlashLib容量错误");
			//throw new TargetMsg(ERR_SPI_SIZE, _T("FlashLib ERROR:ID-9F=%08X; ID-AB=%08X; ID-90=%08X; ID-15=%08X; LIBNUM=%d;"), 
		  flash_ids[0], flash_ids[1], flash_ids[2], flash_ids[3], lib_id);
		}
	}	
#endif
}

BOOL AX327X::SpiWriteEnable(void)
{	
	memset((BYTE*)&UsbCmd,0,0x10);

	UsbCmd.L1.Func1 = func_SignalDrv;
	UsbCmd.L1.Ctrl = 0x3;
	UsbCmd.L1.SiLen = 1;
	UsbCmd.L1.Si[0] = 0x06;

    return (uf->UFRunCode((BYTE *)&(UsbCmd), 0, NULL, USB_WRITE));
	//if (!bSuccess)
	//{	
	//	throw new Msg(ERR_USB_DEVICE, _T("USB ERROR SPI[06]"));
	//}
	//return TRUE;
}

BOOL AX327X::SpiReadStatusReg(void)
{
	BOOL bSuccess;
	memset((BYTE*)&UsbCmd,0,0x10);

	UsbCmd.L1.Func1 = func_SignalDrv;
	UsbCmd.L1.Ctrl = 0x7;
	UsbCmd.L1.SiLen = 1;
	UsbCmd.L1.Si[0] = 0x5;
		
	bSuccess = uf->UFRunCode((BYTE *)&(UsbCmd),4,ElfInfo.ReadBuf,USB_READ);
	if (!bSuccess)
	{	
		return FALSE;
	}
	if(ElfInfo.ReadBuf[0] & 0x01){//flash busy
		return FALSE;
	}
	return TRUE;
}
#if 1
//SPI 写
bool AX327X::SpiWrite(char *wrbuf, int Addr, UINT Length, BOOL bEncrypt)
{	
	if (Length > 65536)
	{
        return false;
	}
	
	memset((BYTE*)&UsbCmd,0,0x10);

	ULONG enc;

	//ULONG enc_start;
	UINT i,PageSize,WritePageAddr;
	
	for (i=0; i<Length; i+=PageSize)
	{
		PageSize = 0x100;

		if (PageSize > Length-i) PageSize = Length-i;
		
		if (!enc_start || enc_start > Addr + i)
			enc = 0;
		else {
			if (enc_start == Addr + i) enc = 0x20000;
			else                          enc = 0x10000;
			if (i+PageSize>=Length) enc |= 0x80000;
			else             enc |= 0x40000;
		}

		WritePageAddr = ((Addr + i)>>8)|enc; 

		UsbCmd.L2.Func1 = func_MemReadWrite;
		UsbCmd.L2.DataAddr = func_MemRWBuf;
		UsbCmd.L2.Func2 = func_PageProgram;
		UsbCmd.L2.Param = WritePageAddr;

        if (uf->UFRunCode((BYTE *)&(UsbCmd), 0x100, wrbuf + i, USB_WRITE) != TRUE)
		{	
            return false;
			//throw new Msg(ERR_USB_DEVICE, _T("USB ERROR SPI[02]"));
		}
	}

    return true;
#if 0
	BOOL bSuccess;
	bSuccess = spiDriver->SpiWrite(Addr,Length, wrbuf, _write_len, bEncrypt);
	if (!bSuccess)
	{
		throw new Msg(ERR_USB_DEVICE, _T("USB ERROR[2]"));
	}
#endif
}

//SPI 擦
int AX327X::SpiErase(int Addr, int Length)
{
	BOOL bSuccess;
	SpiWriteEnable();
	if(1)//if (_cmd_erase != 0xd8)
	{
		for (int i=0; i<Length; i+=4*1024)
		{
			memset((BYTE*)&UsbCmd,0,0x10);
			
			UsbCmd.L1.Func1 = func_SignalDrv;
			UsbCmd.L1.Ctrl = 0x7;
			UsbCmd.L1.SiLen = 4;
			UsbCmd.L1.Si[0] = 0x20;
			UsbCmd.L1.Si[1] = (BYTE)((Addr+i) >> 16);
			UsbCmd.L1.Si[2] = (BYTE)((Addr+i) >> 8);
			UsbCmd.L1.Si[3] = (BYTE)((Addr+i) >> 0);


			bSuccess = uf->UFRunCode((BYTE *)&(UsbCmd),0,NULL,USB_WRITE);
			if (!bSuccess)
			{	
                return -1;
				//throw new Msg(ERR_USB_DEVICE, _T("USB ERROR SPI[d8]"));
			}
			if (!SpiReadStatusReg())
			{	
                return -2;
				//throw new Msg(ERR_USB_DEVICE, _T("USB ERROR SPI[05]"));
			}

		}
	}
	else
	{
		memset((BYTE*)&UsbCmd,0,0x10);
		
		UsbCmd.L1.Func1 = func_SignalDrv;
		UsbCmd.L1.Ctrl = 0x7;
		UsbCmd.L1.SiLen = 4;
		UsbCmd.L1.Si[0] = 0xD8;
		UsbCmd.L1.Si[1] = (BYTE)(Addr >> 16);
		UsbCmd.L1.Si[2] = (BYTE)(Addr >> 8);
		UsbCmd.L1.Si[3] = (BYTE)(Addr >> 0);

		bSuccess = uf->UFRunCode((BYTE *)&(UsbCmd),0,NULL,USB_WRITE);
		if (!bSuccess)
		{	
            return -1;
			//throw new Msg(ERR_USB_DEVICE, _T("USB ERROR SPI[d8]"));
		}
		if (!SpiReadStatusReg())
		{	
            return -2;
			//throw new Msg(ERR_USB_DEVICE, _T("USB ERROR SPI[05]"));
		}
	}

    return 0;
#if 0
	BOOL bSuccess;
	bSuccess = spiDriver->SpiWriteEnable();
	if (_cmd_erase != 0xd8)
	{
		for (int i=0; i<Length; i+=4*1024)
		{
			bSuccess = spiDriver->SpiErase(Addr+i);
			if (!bSuccess)
			{
				throw new Msg(ERR_USB_DEVICE, _T("USB ERROR[d8]"));
			}
		}
	}
	else
	{
		bSuccess = spiDriver->SpiErase(Addr);
		if (!bSuccess)
		{
			throw new Msg(ERR_USB_DEVICE, _T("USB ERROR[d8]"));
		}
	}
#endif
}

//SPI 读
bool AX327X::SpiRead(int Addr,UINT Length, char* ReadBuf, BOOL bEncrypt)
{
	DWORD i,PageSize;

	if (Length > 65536)
	{
		return false;
	}
	memset((BYTE*)&UsbCmd,0,0x10);

	for (i=0; i<Length; i+=PageSize)
	{
		PageSize = 0x200;

		if (PageSize > Length-i) PageSize = Length-i;

		UsbCmd.L1.Func1 = func_SignalDrv;
		UsbCmd.L1.Ctrl = 0x7;
		UsbCmd.L1.SiLen = 4;
		UsbCmd.L1.Si[0] = 0x3;
		UsbCmd.L1.Si[1] = (BYTE)((Addr + i) >> 16);
		UsbCmd.L1.Si[2] = (BYTE)((Addr + i) >> 8);
		UsbCmd.L1.Si[3] = (BYTE)((Addr + i) >> 0);

        if (uf->UFRunCode((BYTE *)&(UsbCmd), PageSize, ReadBuf + i, USB_READ) != TRUE)
        {
            return false;
        }

		//bSuccess = uf->UFRunCode((BYTE *)&(UsbCmd),PageSize,ReadBuf+i,USB_READ);
		//if (!bSuccess)
		//{	
		//	throw new Msg(ERR_USB_DEVICE, _T("USB ERROR SPI[03]"));
		//}
	}

    return true;
#if 0
	BOOL bSuccess;
	bSuccess = spiDriver->SpiRead(Addr,Length,ReadBuf, bEncrypt);
	if (!bSuccess)
	{
		throw new Msg(ERR_USB_DEVICE, _T("USB ERROR[3]"));
	}
#endif
}
#endif

bool AX327X::PreProcess(void)
{
	//tiga项目专属
    if (0 == func_PreProcessRam)
        return false;

	memset((BYTE*)&UsbCmd,0,0x10);

	UsbCmd.L1.Func1 = func_PreProcessRam;
    return uf->UFRunCode((BYTE *)&(UsbCmd), 0, NULL, USB_WRITE);

	//if (!bSuccess)
	//{	
	//	throw new Msg(ERR_USB_DEVICE, _T("USB ERROR[0]"));
	//}
}

void AX327X::SetCRC(char *srcData,int srcDataLen)
{
#if 1
	WORD crc;
	BYTE *sec_num;
	crc = 0xffff;
	//crc = 0;
	ULONG i,crc_start,crc_len; 
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
#endif
}

void AX327X::SetChecksum(char *srcData)
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

BOOL AX327X::SetEncryptAddr(char *srcData,int srcDataLen)
{
	//berry mpw version 
	if(0xffffffff == func_GetHeader)
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

			BOOL bSuccess;
			memset((BYTE*)&UsbCmd,0,0x10);

			UsbCmd.L2.Func1 = func_MemReadWrite;
			UsbCmd.L2.DataAddr = func_MemRWBuf;
			UsbCmd.L2.Func2 = func_GetHeader;
			UsbCmd.L2.Param =  0;

			bSuccess = uf->UFRunCode((BYTE *)&(UsbCmd), 0x200, srcData, USB_WRITE);

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

#if 1
int AX327X::DownBinCode()
{
	if (_down_code_len > _flash_capacity)
	{
        return -1;
		//throw new Msg(ERR_CFG, _T("目标文件过大"));
 	}

	PreProcess();

	int i;
	//预先擦除SPI Flash
	if (0/*erase_all*/) {
		for (i=0; i<_flash_capacity; i+=SPI_BLOCK_SIZE)
		{
			SpiErase(i, SPI_BLOCK_SIZE);
			//SET_PER(_dcode_from + (step1 * i) / _flash_capacity);
		}
	} else {
		for (i=0; i<_down_code_len; i+=SPI_BLOCK_SIZE)
		{
			SpiErase(i, SPI_BLOCK_SIZE);
			//SET_PER(_dcode_from + (step1 * i) / _down_code_len);
		}
	}

	if(!SetEncryptAddr(_down_code,_down_code_len)){
		//throw new Msg(ERR_CFG, IDS_LOG_TARGET_OPENERR);//打开目标文件出错
	}

	if (_down_code_len < SPI_BLOCK_SIZE)
	{
		if (_down_code_len > CODE_BANK_SIZE) {
			memset(_rd_buf,0xff,SPI_BLOCK_SIZE);
			memcpy(_rd_buf, _down_code+CODE_BANK_SIZE, (_down_code_len - CODE_BANK_SIZE));
			SpiWrite(_rd_buf, CODE_BANK_SIZE, SPI_BLOCK_SIZE-CODE_BANK_SIZE, FALSE);
		}
	}
	else
	{
		SpiWrite(_down_code+CODE_BANK_SIZE, CODE_BANK_SIZE, SPI_BLOCK_SIZE-CODE_BANK_SIZE, FALSE);
	}


	//写SPI flash
	for (i=SPI_BLOCK_SIZE; i<_down_code_len; i+=SPI_BLOCK_SIZE) 
	{
		if ((_down_code_len - i) >= SPI_BLOCK_SIZE)
		{
			SpiWrite(_down_code+i, i, SPI_BLOCK_SIZE, FALSE);
		}
		else
		{
			char *buf;
			buf = new char[SPI_BLOCK_SIZE];
			memset(buf,0xff,SPI_BLOCK_SIZE);
			memcpy(buf, _down_code+i, (_down_code_len - i ));
			SpiWrite(buf, i, SPI_BLOCK_SIZE, FALSE);
		}
	}

	if (_down_code_len) {
		SpiWrite(_down_code, 0, CODE_BANK_SIZE, FALSE);
		//SpiRead(0,CODE_BANK_SIZE,_rd_buf, FALSE);
		/*if (memcmp(_rd_buf,_down_code,CODE_BANK_SIZE))
		{
			SpiErase(0, SPI_BLOCK_SIZE);	//校验失败则擦除0 Block，毁掉代码
			//LoadStringRes(temp, IDS_LOG_CODE_VERIFY_ERR);
			//throw new TargetMsg(ERR_SPI_VERIFY, _T("%s!"), temp);
		}*/
	}


#if 0
	
//if(!(spiDriver->enc_start)){//??
if(1){	//读SPI Flash校验
	_dcode_from += step2;
	UINT Residence = 0;
	Residence = (_down_code_len-512) % SPI_BLOCK_SIZE;
	
	/*SpiRead(CODE_BANK_SIZE,SPI_BLOCK_SIZE-CODE_BANK_SIZE,_rd_buf, FALSE);
	if (_down_code_len < SPI_BLOCK_SIZE)
	{
		if (memcmp(_rd_buf,_down_code + CODE_BANK_SIZE, (Residence>CODE_BANK_SIZE) ? (Residence-CODE_BANK_SIZE) : 0))
		{
			//LoadStringRes(temp, IDS_LOG_CODE_VERIFY_ERR);
			////throw new TargetMsg(ERR_SPI_VERIFY, _T("%s!(SR=%02X)"), temp, ReadFlashStatus());//??
		}
	}
	else
	{
		if (memcmp(_rd_buf,_down_code + CODE_BANK_SIZE, SPI_BLOCK_SIZE-CODE_BANK_SIZE))
		{
			//LoadStringRes(temp, IDS_LOG_CODE_VERIFY_ERR);
			////throw new TargetMsg(ERR_SPI_VERIFY, _T("%s!(SR=%02X)"), temp, ReadFlashStatus());
		}
	}*/

	for (i=512; i<_down_code_len-512; i+=SPI_BLOCK_SIZE) //全部读出 
	{
		SpiRead(i,SPI_BLOCK_SIZE,_rd_buf, FALSE);

		if (memcmp(_rd_buf,_down_code + i, (_down_code_len - i-512) >= SPI_BLOCK_SIZE?SPI_BLOCK_SIZE:Residence))
		{
			LoadStringRes(temp, IDS_LOG_CODE_VERIFY_ERR);
			//throw new TargetMsg(ERR_SPI_VERIFY, _T("%s!"), temp);
		}
		SET_PER(_dcode_from + (step3 * i) / _down_code_len);
	}
}
	//校验通过则置上代码有效标志，并写入SPI Flash校验
	SET_PER(99);

	if (_down_code_len) {
		SpiWrite(_down_code, 0, CODE_BANK_SIZE, FALSE);
		SpiRead(0,CODE_BANK_SIZE,_rd_buf, FALSE);
		if (memcmp(_rd_buf,_down_code,CODE_BANK_SIZE))
		{
			SpiErase(0, SPI_BLOCK_SIZE);	//校验失败则擦除0 Block，毁掉代码
			LoadStringRes(temp, IDS_LOG_CODE_VERIFY_ERR);
			//throw new TargetMsg(ERR_SPI_VERIFY, _T("%s!"), temp);
		}
	}
#endif
}
#endif

//下载AutoRun程序
void AX327X::DownCode(const std::wstring& str)
{
	LoadDriver();
	InitDev();
	if(1 == ProbeDev()) //flash on
	{
		//VisionLoadCodeIntoBuffer(str);
		DownBinCode();
	}

}
#if 0
int AX327X::DeviceReset(void)
{
	BOOL bSuccess;
	memset((BYTE*)&UsbCmd,0,0x10);
/*
	UsbCmd.L2.Func1 = 0;
	UsbCmd.L2.DataAddr = 0x4;
	UsbCmd.L2.Func2 = 0;
	UsbCmd.L2.Param =  0;
	bSuccess = uf->UFRunCode((BYTE *)&(UsbCmd),0,NULL,USB_WRITE);
	*/
	memset((BYTE*)&UsbCmd,0,0x10);
	UsbCmd.L2.Func1 = 0;
	UsbCmd.L2.Param = 0x4;
	UsbCmd.L2.Func2 =func_Reset;
	bSuccess = uf->UFRunCode((BYTE *)&(UsbCmd),0,NULL,USB_WRITE);

	return 0;
}
#endif
int AX327X::PreProbe()
{
	//还要判断是不是约定的512,判断有没有将checksum打开
	SpiRead(0, 512, _rd_buf, FALSE);
	if (  _rd_buf[0] == 0 && 
		  _rd_buf[1] == 0 && 
		  _rd_buf[4] == 'B' && 
		  _rd_buf[5] == 'L' && 
		  _rd_buf[6] == 'D' && 
		  _rd_buf[7] == 'R' && 
		  _rd_buf[0x1fe] == 0x55 && 
		  _rd_buf[0x1ff] == (char)0xAA         //bootsect标志位判断
		  ) {
		SetCRC(_down_code,_down_code_len);
		SetChecksum(_rd_buf);
	}
	if(memcmp(_rd_buf,_down_code,0x200))
	{
		return 0;
		////throw new TargetMsg (ERR_CFG,_T("not"));
	}else{
		return 1;
		////throw new TargetMsg (ERR_CFG,_T("same"));
		//AfxMessageBox(_T("same"));
	}
}

void AX327X::InitDownloadParam(void)
{
    TCHAR tCurPath[MAX_PATH];
	GetModuleFileName(NULL, tCurPath, MAX_PATH);
    PathAddBackslash(tCurPath);

    tSettingElfPath = tCurPath;
    tSettingElfPath += _T("%s\\setting\\AX327X\\firmware.elf");

	tTempISP1BinPath = new TCHAR[MAX_PATH];
    tTempISP1BinPath += _T("%s\\temp\\ISPSetting.bin");

	tTempISP2BinPath = new TCHAR[MAX_PATH];
    tTempISP2BinPath += _T("%s\\temp\\ISPSetting.bin");

	tTempLCDBinPath = new TCHAR[MAX_PATH];
    tTempLCDBinPath += _T("%s\\temp\\LCDSetting.bin"), tCurPath;
	
	tTempLCDInitBinPath = new TCHAR[MAX_PATH];
    tTempLCDInitBinPath += _T("%s\\temp\\LCDInitSetting.bin");

	tTempGamachartBinPath = new TCHAR[MAX_PATH];
    tTempGamachartBinPath += _T("%s\\temp\\Gamachart.bin");

	tTempDebugBinPath = new TCHAR[MAX_PATH];
    tTempDebugBinPath += _T("%s\\temp\\sensor.bin");

    //_stprintf(m_FlashLibPath.c_str(), _T("%s\\setting\\FlashLib.ini"), tCurPath);

    m_FlashLibPath += tCurPath;
    m_FlashLibPath += _T("%s\\setting\\FlashLib.ini");
}

bool AX327X::InitDebugParam(void)
{
	tTempDebugPath = new TCHAR[MAX_PATH]; 
	tTempISPInfoPath = new TCHAR[MAX_PATH];	
	tSettingOrderPath = new TCHAR[MAX_PATH]; 
	tTempSensorRegDebugPath = new TCHAR[MAX_PATH]; 
	tTempSensorStructDebugPath = new TCHAR[MAX_PATH];

	//GetModuleFileName(NULL, tCurPath, MAX_PATH);
	//*(_tcsrchr(tCurPath,'\\')) = 0;

	////
	//_stprintf(tTempDebugPath, _T("%s\\temp\\ProgramModeDebug.xml"), tCurPath);
	//_stprintf(tTempISPInfoPath, _T("%s\\temp\\ISPInfo.xml"), tCurPath);
	//_stprintf(tTempSensorRegDebugPath, _T("%s\\temp\\SensorRegWR.xml"), tCurPath);
	//_stprintf(tTempSensorStructDebugPath, _T("%s\\temp\\SensorStructDebug.xml"), tCurPath);

	////
	//_stprintf(tSettingOrderPath, _T("%s\\setting\\AX327X\\order.ini"), tCurPath);

//debug
	/*func_Sensor_DebugRegister = GetPrivateProfileInt(_T("ORDER"), _T("Sensor_DebugRegister"), 1, tSettingOrderPath); 
	func_Sensor_GetID = GetPrivateProfileInt(_T("ORDER"), _T("Sensor_GetID"), 1, tSettingOrderPath); 
	func_sdk_MemReadWrite = GetPrivateProfileInt(_T("ORDER"), _T("cb_mem_rwex"), 1, tSettingOrderPath);
	func_IspDebugWrite = GetPrivateProfileInt(_T("ORDER"), _T("isp_debug_write"), 1, tSettingOrderPath); 
	func_IspDebugRead = GetPrivateProfileInt(_T("ORDER"), _T("isp_debug_read"), 1, tSettingOrderPath); 
	func_LcdDebugWrite=GetPrivateProfileInt(_T("ORDER"), _T("lcd_debug_write"), 1, tSettingOrderPath);
	func_LcdDebugRead=GetPrivateProfileInt(_T("ORDER"), _T("lcd_debug_read"), 1, tSettingOrderPath);
	*/

    func_sdk_MemWrite = 0x0000110;
    func_sdk_MemRead = 0x0000118;


	int addr[11][2] = {
		{0x0000140,0},
		{0x0000150,0},
		{0x0000160,0},
		{0x0000170,0},
		{0x0000180,0},
		{0x0000190,0},
		{0x00001a0,0},
		{0x00001b0,0},
		{0x00001c0,0},
		{0x00001d0,0},
		{0x00001e0,0},
	};


	BOOL bSuccess;
	char buf[4];
	memset((BYTE*)&UsbCmd,0,0x10);

	UsbCmd.L2.Func1 = func_sdk_MemRead;  
	UsbCmd.L2.Func2 = 0xffffffff;
	UsbCmd.L2.Param = 0xffffffff;

    BYTE temp[0x200] = { 0 };
	UsbCmd.L2.DataAddr = 0x100;  //Ax328X sram :0x100
	bSuccess = uf->UFISPCode((BYTE *)&(UsbCmd),0x200,(char *)(temp),USB_READ);
	if (!bSuccess)
	{
        return false;
	}
/*	
	for(int i=0;i<11;i++){
		UsbCmd.L2.DataAddr = addr[i][0];
		bSuccess = uf->UFISPCode((BYTE *)&(UsbCmd),4,(char *)(&addr[i][1]),USB_READ);
		if (!bSuccess)
		{
			throw new Msg(ERR_USB_DEVICE, _T("Spi Init Error"));
		}
	}
*/
/*
	isp_tab_addr = addr[0][1];
	isp_tab_len = addr[1][1];
	isp_tab_vma = addr[2][1];
	func_Sensor_DebugRegister = addr[3][1];
	func_Sensor_GetID = addr[4][1];
	func_IspDebugWrite = addr[5][1]; 
	func_IspDebugRead =  addr[6][1];
	func_LcdDebugWrite=  addr[7][1];
	func_LcdDebugRead= addr[8][1];
	func_LcdRegDebugToggle= addr[9][1];
	func_LcdRegDebugWrite= addr[10][1];
*/
	memcpy(&isp_tab_addr,temp+0x30,4);
	memcpy(&isp_tab_len,temp+0x34,4);
	memcpy(&isp_tab_vma,temp+0x38,4);
	memcpy(&sensor_cmd_addr,temp+0x3c,4);

	memcpy(&func_Sensor_DebugRegister,temp+0x40,4);
	memcpy(&func_Get_Sensor_Data,temp+0x44,4);
    memcpy(&func_Set_Sensor_Data, temp + 0x48, 4);
	memcpy(&func_IspDebugWrite,temp+0x4c,4);
	memcpy(&func_IspDebugRead,temp+0x50,4);
    memcpy(&func_UsbCutRaw, temp + 0x54, 4);

	memcpy(&func_LcdDebugWrite,temp+0x60,4);
	memcpy(&func_LcdDebugRead,temp+0x64,4);
	memcpy(&func_LcdRegDebugToggle,temp+0x68,4);
	memcpy(&func_LcdRegDebugWrite,temp+0x6c,4);

	memcpy(&func_IspReadFlash, temp + 0x70, 4);

	//isp_tab_len = GetPrivateProfileInt(_T("RAM"), _T("_res_inf_tab_len"), 1, tSettingOrderPath); 
	//isp_tab_addr = GetPrivateProfileInt(_T("RAM"), _T("_res_inf_tab_addr"), 1, tSettingOrderPath); 
	//isp_tab_vma = GetPrivateProfileInt(_T("RAM"), _T("_res_inf_tab_vma"), 1, tSettingOrderPath); 
	//sensor_cmd_addr = GetPrivateProfileInt(_T("RAM"), _T("sensor_cmd"), 1, tSettingOrderPath); 

    return true;
}

// NOTE:据说这部分功能还用不到，先不管
//void AX327X::DebugSensorReg(void)
//{
//    CXmlDataInfo SensorRegXml;
//    SensorRegXml.LoadSensorRegFile(tTempSensorRegDebugPath);
//    int cnt = SensorRegXml.m_arrXmlDataInfo.GetSize();
//	char buf[8];
//	int temp[1];
//	memset(buf,0xff,8);
//	//buf[0]= 0x14;
//	//buf[1]= 0x11;
//	BOOL bSuccess;
//	memset((BYTE*)&UsbCmd,0,0x10);
//
//	int addrbit = SensorRegXml.AddrBit/8;
//	int databit = SensorRegXml.DataBit/8;
//	UsbCmd.L2.Func1 = func_sdk_MemWrite;
//	UsbCmd.L2.DataAddr = 0xffffffff;
//	UsbCmd.L2.Func2 = func_Sensor_DebugRegister;
//	UsbCmd.L2.Param = (databit<<8)|(addrbit);
//	
//	for(int i=0;i<cnt;i++){
//		temp[0] = _tcstoul(SensorRegXml.m_arrXmlDataInfo[i].sAddr,NULL,16);
//		memcpy(buf,temp,addrbit*sizeof(char));
//		temp[0] = _tcstoul(SensorRegXml.m_arrXmlDataInfo[i].sData,NULL,16);
//		memcpy(buf+addrbit,temp,databit*sizeof(char));
//
//		bSuccess = uf->UFISPCode((BYTE *)&(UsbCmd),8,(char *)buf,USB_WRITE);
//		if (!bSuccess)
//		{
//			throw new Msg(ERR_USB_DEVICE, _T("Spi Init Error"));
//		}
//	}
//}

bool AX327X::DebugIspWrite(int param, char* data, int dataSize)
{
    USB_CMD  UsbCmd = { 0 };

    ::memset((void*)&UsbCmd, 0, 0x10);

    UsbCmd.L2.Func1 = func_sdk_MemWrite;
    UsbCmd.L2.DataAddr = 0xffffffff;
    UsbCmd.L2.Func2 = func_IspDebugWrite;
    UsbCmd.L2.Param = param;
    return uf->UFISPCode((BYTE *)&(UsbCmd), dataSize, data, USB_WRITE);
}

bool AX327X::DebugLcdWrite(int param, char* data, int dataSize)
{
	USB_CMD  UsbCmd = { 0 };

	::memset((void*)&UsbCmd, 0, 0x10);

	UsbCmd.L2.Func1 = func_sdk_MemWrite;
	UsbCmd.L2.DataAddr = 0xffffffff;
	UsbCmd.L2.Func2 = func_LcdDebugWrite;
	UsbCmd.L2.Param = param;
	return uf->UFISPCode((BYTE *)&(UsbCmd), dataSize, data, USB_WRITE);
}

bool AX327X::SetSensorData(int param, const char* data, int dataSize)
{
    USB_CMD  UsbCmd = { 0 };

    ::memset((void*)&UsbCmd, 0, 0x10);

    UsbCmd.L2.Func1 = func_sdk_MemWrite;
    UsbCmd.L2.DataAddr = 0xffffffff;
    UsbCmd.L2.Func2 = func_Set_Sensor_Data;
    UsbCmd.L2.Param = param;
    return uf->UFISPCode((BYTE *)&(UsbCmd), dataSize, const_cast<char*>(data), USB_WRITE);
}

bool AX327X::IspReadFlash(int param, char* data, int dataSize)
{
	USB_CMD  UsbCmd = { 0 };

	::memset((void*)&UsbCmd, 0, 0x10);

	UsbCmd.L2.Func1 = func_sdk_MemRead;
	UsbCmd.L2.DataAddr = 0xffffffff;
	UsbCmd.L2.Func2 = func_IspReadFlash;
	UsbCmd.L2.Param = param;
	return uf->UFISPCode((BYTE *)&(UsbCmd), dataSize, data, USB_READ);
}

//NOTE:暂时不管
//void AX327X::DebugLcdWirte(int sel)
//{
//	if(1 == sel){
//		CXmlDataInfo LCDXml;
//
//		pLCD_Param = new UINT[LCDXml.GetValidModule(_T("LCD"))];//参数总个数
//		memset(pLCD_Param,0,LCDXml.GetValidModule(_T("LCD"))*sizeof(UINT));
//
//		LCDXml.LoadLCDParamFile(pLCD_Param,tTempISPInfoPath);
//		int status = LCDXml.DebugLCDStatus;
//		if(status){
//			int cnt = LCDXml.GetValidModule(_T("LCD"));
//			BOOL bSuccess;
//			memset((BYTE*)&UsbCmd,0,0x10);
//
//			UsbCmd.L2.Func1 = func_sdk_MemWrite;
//			UsbCmd.L2.DataAddr = 0xffffffff;
//			UsbCmd.L2.Func2 = func_LcdDebugWrite;
//			UsbCmd.L2.Param = 0;//i*2;
//			bSuccess = uf->UFISPCode((BYTE *)&(UsbCmd),cnt*4,(char *)(&pLCD_Param[0]),USB_WRITE);
//			if (!bSuccess)
//			{
//				throw new Msg(ERR_USB_DEVICE, _T("Spi Init Error"));
//			}
//		}
//		LCDXml.FixLCDStatus(0,tTempISPInfoPath);
//	}
//	if(0 == sel){
//		TCHAR* tTempLcdregBinPath;
//		tTempLcdregBinPath = new TCHAR[MAX_PATH];
//		_stprintf(tTempLcdregBinPath,_T("%s\\temp\\temp_LCD\\LCDREG.bin"),tCurPath);
//		FILE *fpin;
//		fpin = _tfopen(tTempLcdregBinPath, _T("rb"));
//		if(fpin == NULL)
//		{
//			return;
//		}
//		fseek(fpin, 0, SEEK_END);
//		int len = ftell(fpin);
//		if(0 == len){ return; }
//
//		BYTE *buf;
//		buf = new BYTE[len];
//		fseek(fpin, 0, SEEK_SET);
//		fread(buf, 1, len, fpin);
//		fclose(fpin);
//
//		delete []tTempLcdregBinPath;
//		//_tremove(tTempGamachartBinPath);
//
//		BOOL bSuccess;
//		memset((BYTE*)&UsbCmd,0,0x10);
//
//		UsbCmd.L2.Func1 = func_sdk_MemWrite;
//		UsbCmd.L2.DataAddr = 0xffffffff;
//		UsbCmd.L2.Func2 = func_LcdDebugWrite;
//		UsbCmd.L2.Param = 0xff;
//		bSuccess = uf->UFISPCode((BYTE *)&(UsbCmd),len,(char *)buf,USB_WRITE);
//		if (!bSuccess)
//		{
//			throw new Msg(ERR_USB_DEVICE, _T("Spi Init Error"));
//		}
//
//	}
//
//}

bool AX327X::DebugGamachartWirte(int id)
{
	BYTE buf[CHART_LEN];

	FILE *fpin;
	int len;
    _tfopen_s(&fpin, tTempGamachartBinPath.c_str(), _T("rb+"));
	if(fpin == NULL)
	{
		return false;
	}
	fseek(fpin, 0, SEEK_SET);
	fread(buf, 1, CHART_LEN, fpin);
	fclose(fpin);
	_tremove(tTempGamachartBinPath.c_str());

    BOOL bSuccess;
    memset((BYTE*)&UsbCmd, 0, 0x10);

    UsbCmd.L2.Func1 = func_sdk_MemWrite;
    UsbCmd.L2.DataAddr = 0xffffffff;
    UsbCmd.L2.Func2 = func_IspDebugWrite;
    UsbCmd.L2.Param = /*0x100;*/(id << 8);//i*2;
    return uf->UFISPCode((BYTE *)&(UsbCmd), CHART_LEN, (char *)buf, USB_WRITE);
}

//void AX327X::DebugLcdRegWirte(void)
//{
//	TCHAR* tTempLcdregBinPath;
//	tTempLcdregBinPath = new TCHAR[MAX_PATH];
//	_stprintf(tTempLcdregBinPath,_T("%s\\temp\\temp_LCD\\LCDREG.bin"),tCurPath);
//	FILE *fpin;
//	fpin = _tfopen(tTempLcdregBinPath, _T("rb"));
//	if(fpin == NULL)
//	{
//		return;
//	}
//	fseek(fpin, 0, SEEK_END);
//	int len = ftell(fpin);
//	if(0 == len){ return; }
//
//	BYTE *buf;
//	buf = new BYTE[len];
//	fseek(fpin, 0, SEEK_SET);
//	fread(buf, 1, len, fpin);
//	fclose(fpin);
//
//	delete []tTempLcdregBinPath;
//	//_tremove(tTempGamachartBinPath);
//
////step1:disable lcd kick
//	BOOL bSuccess;
//	memset((BYTE*)&UsbCmd,0,0x10);
//
//	UsbCmd.L2.Func1 = func_sdk_MemWrite;
//	UsbCmd.L2.DataAddr = 0xffffffff;
//	UsbCmd.L2.Func2 = func_LcdRegDebugToggle;
//	UsbCmd.L2.Param = 1;
//
//	bSuccess = uf->UFISPCode((BYTE *)&(UsbCmd),0,NULL,USB_WRITE);
//	if (!bSuccess)
//	{
//		throw new Msg(ERR_USB_DEVICE, _T("Disable lcd kick Error"));
//	}
//
////step2: wait until a frame end;
//	Sleep(10);
//
////step3: send data;
//
//	int i,cnt;;
//	for(i=0; i<len; i+=cnt)
//	{	
//		if((len -i)<256)
//			cnt = len-i;
//		else 
//			cnt = 256;
//
//		BOOL bSuccess;
//		memset((BYTE*)&UsbCmd,0,0x10);
//
//		UsbCmd.L2.Func1 = func_sdk_MemWrite;
//		UsbCmd.L2.DataAddr = 0xffffffff;
//		UsbCmd.L2.Func2 = func_LcdRegDebugWrite;
//		UsbCmd.L2.Param = buf[len-1];  //thunder: _lcd_desc_s.bus.cmd
//
//		bSuccess = uf->UFISPCode((BYTE *)&(UsbCmd),cnt,(char *)buf+i,USB_WRITE);
//		if (!bSuccess)
//		{
//			throw new Msg(ERR_USB_DEVICE, _T("LcdRegDebugWrite Error"));
//		}
//	}
//	delete []buf;
//
////step4: enable lcd kick
//	//BOOL bSuccess;
//	memset((BYTE*)&UsbCmd,0,0x10);
//
//	UsbCmd.L2.Func1 = func_sdk_MemWrite;
//	UsbCmd.L2.DataAddr = 0xffffffff;
//	UsbCmd.L2.Func2 = func_LcdRegDebugToggle;
//	UsbCmd.L2.Param = 0;
//
//	bSuccess = uf->UFISPCode((BYTE *)&(UsbCmd),0,NULL,USB_WRITE);
//	if (!bSuccess)
//	{
//		throw new Msg(ERR_USB_DEVICE, _T("Enable lcd kick Error"));
//	}
//}

bool AX327X::DebugIspRead(int ispModule, char* data, int dataSize)
{
    memset((BYTE*)&UsbCmd, 0, 0x10);

    UsbCmd.L2.Func1 = func_sdk_MemRead;
    UsbCmd.L2.DataAddr = 0xffffffff;
    UsbCmd.L2.Func2 = func_IspDebugRead;
    UsbCmd.L2.Param = (int)ispModule;
    return uf->UFISPCode((BYTE *)&(UsbCmd), dataSize, data, USB_READ);
}

bool AX327X::DebugLcdRead(int reserved1, char* data, int reserved2)
{
	memset((BYTE*)&UsbCmd, 0, 0x10);

	UsbCmd.L2.Func1 = func_sdk_MemRead;
	UsbCmd.L2.DataAddr = 0xffffffff;
	UsbCmd.L2.Func2 = func_LcdDebugRead;
	UsbCmd.L2.Param = 0;
	return uf->UFISPCode((BYTE *)&(UsbCmd), 512, data, USB_READ);
}

//void AX327X::VisionConvertParamToUI_lcd(void){
//	CXmlDataInfo LCDXml;
//	int i = 0;
//	LCD_UI.GAMA.Brightness = pLCD_Param[i];
//	LCD_UI.GAMA.Saturation = pLCD_Param[i+1];
//	LCD_UI.GAMA.contra_index = pLCD_Param[i+2];
//	LCD_UI.GAMA.R = pLCD_Param[i+3];
//	LCD_UI.GAMA.G = pLCD_Param[i+4];
//	LCD_UI.GAMA.B = pLCD_Param[i+5];
//
//}

//void AX327X::VisionConvertParamToUI(void){
//	CXmlDataInfo ISPXml;
//	ISPXml.AllMod = ISPXml.GetValidModule(_T("ISP"));//默认打开的模块
//	if(ISPXml.AllMod & (1<<INDEX_BLC))
//	{
//		int i = ISPXml.GetStructStartAddr(INDEX_BLC);
//		ISP_UI.BLC.R  = pISP_Param[i];
//		ISP_UI.BLC.Gr = pISP_Param[(i+1)];
//		ISP_UI.BLC.Gb = pISP_Param[(i+2)];
//		ISP_UI.BLC.B  = pISP_Param[(i+3)];
//	}
//
//	if(ISPXml.AllMod & (1<<INDEX_DDC))
//	{
//		int i = ISPXml.GetStructStartAddr(INDEX_DDC);
//		ISP_UI.DDC.denoise_class = pISP_Param[i+29];
//		ISP_UI.DDC.dpc_class = pISP_Param[(i+11)];
//	}
//	if(ISPXml.AllMod & (1<<INDEX_CCM))                                                    
//	{   
//		int i = ISPXml.GetStructStartAddr(INDEX_CCM);
//		ISP_UI.CCM.Enhance_G = 64-(pISP_Param[i+1]+4)*2;                    
//		ISP_UI.CCM.Enhance_B = 64-(pISP_Param[i+2]-4)*2;                    
//		ISP_UI.CCM.Enhance_R = 64-(pISP_Param[i+3]-4)*2;  		              
//		ISP_UI.CCM.R = pISP_Param[i+0]-56 - (ISP_UI.CCM.Enhance_R-64) + 64; 
//		ISP_UI.CCM.G = pISP_Param[i+4]-72 - (ISP_UI.CCM.Enhance_G-64) + 64; 
//		ISP_UI.CCM.B = pISP_Param[i+8]-64 - (ISP_UI.CCM.Enhance_B-64) + 64; 
//                                                                                     
//	}                                                                                        
//	if(ISPXml.AllMod & (1<<INDEX_AWB))                                                    
//	{   
//		int i = ISPXml.GetStructStartAddr(INDEX_AWB);
//		ISP_UI.AWB.AWB_class = pISP_Param[i+14];
//		ISP_UI.AWB.Rgain1 = pISP_Param[i+15]&0x3ff;
//		ISP_UI.AWB.Ggain1 = (pISP_Param[i+15]>>10)&0x3ff;
//		ISP_UI.AWB.Bgain1 = (pISP_Param[i+15]>>20)&0x3ff;
//		ISP_UI.AWB.Rgain2 = pISP_Param[i+16]&0x3ff;
//		ISP_UI.AWB.Ggain2 = (pISP_Param[i+16]>>10)&0x3ff;
//		ISP_UI.AWB.Bgain2 = (pISP_Param[i+16]>>20)&0x3ff;
//		ISP_UI.AWB.Rgain3 = pISP_Param[i+17]&0x3ff;
//		ISP_UI.AWB.Ggain3 = (pISP_Param[i+17]>>10)&0x3ff;
//		ISP_UI.AWB.Bgain3 = (pISP_Param[i+17]>>20)&0x3ff;
//		ISP_UI.AWB.Rgain4 = pISP_Param[i+18]&0x3ff;
//		ISP_UI.AWB.Ggain4 = (pISP_Param[i+18]>>10)&0x3ff;
//		ISP_UI.AWB.Bgain4 = (pISP_Param[i+18]>>20)&0x3ff;                                                                                  
//	}
//#if 0
//	if((ISPXml.AllMod & (1<<INDEX_YGAMA))||(ISPXml.AllMod & (1<<INDEX_CBHS)))          
//	{   
//		int i = ISPXml.GetStructStartAddr(INDEX_YGAMA);
//		ISP_UI.YGAMA_class = 14 - pISP_Param[i];                                             
//		ISP_UI.CBHS.c = pISP_Param[i+1];                                                       
//                                                         
//	}                                                                                        
//                                                                                           
//	if(ISPXml.AllMod & (1<<INDEX_RGBGAMA))                                                
//	{   
//		int i = ISPXml.GetStructStartAddr(INDEX_RGBGAMA);
//		ISP_UI.RGBGAMA_class = pISP_Param[i];                                           
//                                                              
//	}
//#endif
//	if(ISPXml.AllMod & (1<<INDEX_EE))                                                     
//	{   
//		int i = ISPXml.GetStructStartAddr(INDEX_EE);
//		ISP_UI.EE.sharp_lv = 31 - pISP_Param[i+1];
//		ISP_UI.EE.sharp_th = pISP_Param[i+9];
//	}                                                                                        
//	if(ISPXml.AllMod & (1<<INDEX_CCF))                                                    
//	{   
//		int i = ISPXml.GetStructStartAddr(INDEX_CCF);
//                                                         
//		 ISP_UI.CCF.rate  = pISP_Param[i+1];                                                     
//		 ISP_UI.CCF.ymax  = pISP_Param[i+2];                                                                                                              
//		 ISP_UI.CCF.wclass = pISP_Param[i+5];                                                   
//		 ISP_UI.CCF.wymin  = pISP_Param[i+6];                                                    
//	} 
//	if(ISPXml.AllMod & (1<<INDEX_LSC))                                                   
//	{   
//		int i = ISPXml.GetStructStartAddr(INDEX_LSC);  
//		if(839 == pISP_Param[i])
//			ISP_UI.LSC.resolution = 0;
//		else
//			ISP_UI.LSC.resolution = 1;
//		ISP_UI.LSC.b2 = pISP_Param[i+3];
//		ISP_UI.LSC.b4 = pISP_Param[i+6];                                                 
//	} 
//	if(ISPXml.AllMod & (1<<INDEX_CBHS))                                                   
//	{   
//		int i = ISPXml.GetStructStartAddr(INDEX_CBHS);  
//		ISP_UI.CBHS.c = pISP_Param[i];
//		ISP_UI.CBHS.b = pISP_Param[i+1];
//		ISP_UI.CBHS.h = pISP_Param[i+3];
//		ISP_UI.CBHS.s = pISP_Param[i+11];                                                 
//	} 
//	if(ISPXml.AllMod & (1<<INDEX_MD))                                                     
//	{   
//		int i = ISPXml.GetStructStartAddr(INDEX_MD);
//		ISP_UI.MD_class = (70 - pISP_Param[i])/10;                                                
//	}
//	if(ISPXml.AllMod & (1<<INDEX_EXP))                                                     
//	{   
//		int i = ISPXml.GetStructStartAddr(INDEX_EXP);
//		ISP_UI.EXP_class = pISP_Param[i+9];                                                
//	}
//}

//NOTE:用不到，先不管
//void AX327X::DebugStruct(void){
//
//	CXmlDataInfo StructXml;
//	StructXml.LoadStructFile(tTempSensorStructDebugPath);
//
//	int cnt = StructXml.StructData.GetSize();
//	if((1 == StructXml.DebugStatus)&&(0 != cnt))
//	{
//		int temp[256];
//		BOOL bSuccess;
//		memset((BYTE*)&UsbCmd,0,0x10);
//
//		UsbCmd.L2.Func1 = func_sdk_MemWrite;
//		UsbCmd.L2.DataAddr = StructXml.StructAddr;
//		UsbCmd.L2.Func2 = 0xffffffff;
//		UsbCmd.L2.Param = 0;
//		for(int i=0;i<cnt;i++){
//			temp[i] = StructXml.StructData[i];
//		}
//		//cnt = (cnt+0x3)&~ 0x3; //4 bytes algin
//		//memcpy(temp,&SensorStructXml.SensorStructData,cnt);
//		bSuccess = uf->UFISPCode((BYTE *)&(UsbCmd),cnt*4,(char *)temp,USB_WRITE);
//		if (!bSuccess)
//		{
//			throw new Msg(ERR_USB_DEVICE, _T("Spi Init Error"));
//		}
//	}
//}

//void AX327X::DebugSensorStructWrite(void){
//
//	CXmlDataInfo ISPXml;
//	ISPXml.LoadSensorFile(tTempISPInfoPath);
//
//	int cnt = ISPXml.SensorData.GetSize();
//	if((1 == ISPXml.DebugSensorStatus)&&(0 != cnt))
//	{
//		int temp[256];
//		BOOL bSuccess;
//		memset((BYTE*)&UsbCmd,0,0x10);
//
//		UsbCmd.L2.Func1 = func_sdk_MemWrite;
//		UsbCmd.L2.DataAddr = sensor_cmd_addr;
//		UsbCmd.L2.Func2 = 0xffffffff;
//		UsbCmd.L2.Param = 0;
//		for(int i=0;i<cnt;i++){
//			temp[i] = ISPXml.SensorData[i];
//		}
//		bSuccess = uf->UFISPCode((BYTE *)&(UsbCmd),cnt*4,(char *)temp,USB_WRITE);
//		if (!bSuccess)
//		{
//			throw new Msg(ERR_USB_DEVICE, _T("Spi Init Error"));
//		}
//	}
//}
//
//void AX327X::DebugSensorStructRead(void){
//
//	CXmlDataInfo ISPXml;
//	ISPXml.LoadSensorFile(tTempISPInfoPath);
//
//	int cnt = ISPXml.SensorData.GetSize();
//	if((1 == ISPXml.DebugSensorStatus)&&(0 != cnt))
//	{
//		int temp[256];
//		BOOL bSuccess;
//		memset((BYTE*)&UsbCmd,0,0x10);
//
//		UsbCmd.L2.Func1 = func_sdk_MemRead;
//		UsbCmd.L2.DataAddr = sensor_cmd_addr;
//		UsbCmd.L2.Func2 = 0xffffffff;
//		UsbCmd.L2.Param = 0;
//		bSuccess = uf->UFISPCode((BYTE *)&(UsbCmd),cnt*4,(char *)temp,USB_READ);
//		if (!bSuccess)
//		{
//			throw new Msg(ERR_USB_DEVICE, _T("Spi Init Error"));
//		}
//		for(int i=0;i<cnt;i++){
//			ISPXml.SensorData[i] = temp[i];
//		}
//		ISPXml.SaveSensorFile(tTempISPInfoPath);
//	}
//		//gReadOnlineDone = 1;
//
//}

void AX327X::DebugToDownload(void){

	uf->UFUpdate();
    //return 0x10; 
}

typedef enum
{
	__ISP_STRUCT__ = 0,
	__ISP_CONTRA__,
	__ISP_RGB_GMA__,
	__ISP_Y_GAMA__,
	__EXP_TAB__,
	__EXP_EVSTEP__,
	__SENSOR_STRUCT__,
	__SENSOR_INIT_TAB__,
	__SENSOR_YGAMA_TAB__,
	__SENSOR_RGBGAMA_TAB__,
	__LCD_STRUCT__,
	__LCD_INIT_TAB__,
	__LCD_GAMA__,
	__LCD_CONTRA__,
	__NULL__ = 0x1fffffff
}RES_ID;

typedef struct
{
	RES_ID res_id;
	int len;
	int  attribute;
	int  p_vma;
	//void* p_vma;
}RES_INF;

void AX327X ::ThunderLoadTab(void){

	char temp[0x200];
	SpiRead(0x300,0x200,temp, FALSE);

	memcpy(&isp_tab_addr,temp+0x30,4);
	memcpy(&isp_tab_len,temp+0x34,4);
	memcpy(&isp_tab_vma,temp+0x38,4);
	memcpy(&sensor_cmd_addr,temp+0x3c,4);

    memcpy(&func_Sensor_DebugRegister, temp + 0x40, 4);
    memcpy(&func_Get_Sensor_Data, temp + 0x44, 4);
    memcpy(&func_Set_Sensor_Data, temp + 0x48, 4);
    memcpy(&func_IspDebugWrite, temp + 0x4c, 4);
    memcpy(&func_IspDebugRead, temp + 0x50, 4);

	memcpy(&func_LcdDebugWrite,temp+0x60,4);
	memcpy(&func_LcdDebugRead,temp+0x64,4);
	memcpy(&func_LcdRegDebugToggle,temp+0x68,4);
	memcpy(&func_LcdRegDebugWrite,temp+0x6c,4);

	char *buf_src;

	buf_src = new char[isp_tab_len];
	
	RES_INF buf;
	SpiRead(isp_tab_addr,isp_tab_len,buf_src, FALSE);

    //CXmlDataInfo Xml;
    //BYTE id;
    //Xml.SaveSensorID(2, &id, tTempISPInfoPath);

	int i;
	//int cnt=0;
	//int res_start_addr;
	for(i=0; i<isp_tab_len; i+=sizeof(RES_INF)){
		memcpy(&buf,buf_src+i,sizeof(RES_INF));

		if(__ISP_STRUCT__ == buf.res_id){
			isp_struct_addr = isp_tab_addr + isp_tab_len+4;
			isp_struct_len = buf.len;
			//res_start_addr = buf.p_vma;
		}
		if(__SENSOR_STRUCT__ == buf.res_id){
			//if( id==buf.attribute){
			//	//sensor_struct_addr = isp_tab_addr + isp_tab_len + buf.p_vma - res_start_addr;
			//	sensor_struct_addr = isp_tab_addr + buf.p_vma - isp_tab_vma;
			//	sensor_struct_len = buf.len;
			//	int temp = 0;
			//	temp = Xml.GetStructStartAddr(MODULE_NUM);
			//	sensor_struct_len2 = buf.len - 3*4 - temp*4; 
			//}
		}
		if(__LCD_STRUCT__ == buf.res_id){
			//lcd_struct_addr = isp_tab_addr + isp_tab_len + buf.p_vma - res_start_addr + buf.len - 6*4;
			lcd_struct_addr = isp_tab_addr + buf.p_vma - isp_tab_vma + buf.len - 6*4;
			lcd_init_addr = isp_tab_addr + buf.p_vma - isp_tab_vma;
			//TRACE("%d",lcd_struct_addr);
		//	lcd_struct_len = buf.len;
		}
		//cnt += buf.len;
	}

	delete []buf_src;

}

//void AX327X ::VisionLoadCodeIntoBuffer(std::wstring str) 
//{	
//	FILE *fpin;
//	fpin = _tfopen(str, _T("rb"));
//	if(fpin == NULL)
//	{
//		//throw new Msg(ERR_CFG, IDS_LOG_TARGET_OPENERR);		//打开目标文件出错
//		throw new Msg(ERR_CFG, _T("打开文件出错"));		//打开目标文件出错
//		return;
//	}
//	
//	fseek(fpin, 0, SEEK_END);
//	_down_code_len = ftell(fpin);
//	fseek(fpin, 0, SEEK_SET);
//
//	//文件不能超过32M
//	if (_down_code_len > 0x2000000)
//	{
//		//throw new Msg(ERR_CFG, IDS_LOG_TARGET_LARGE);			//目标文件过大
// 	}
//	
//	if(_down_code != NULL)
//		delete _down_code;
//
//	_down_code = new char[_down_code_len];
//
//	memset(_down_code, 0, _down_code_len);
//	fread(_down_code, 1, _down_code_len, fpin);
//
//	fclose(fpin);
//
//}

// NOTE: LCD的逻辑，先不管
//void AX327X::DownloadPart(int index){
//
//	LoadDriver();
//	InitDev();
//	ThunderLoadTab();
//
//	if(1 == index){
//		VisionLoadCodeIntoBuffer(tTempISP1BinPath);
//		DownPartBinCode(isp_struct_addr);
//	}
//	if(2 == index){
//		VisionLoadCodeIntoBuffer(tTempISP2BinPath);
//		CXmlDataInfo ISPXml;
//	    ISPXml.LoadSensorFile(tTempISPInfoPath);
//		if((1 == ISPXml.DebugSensorStatus)&&(0 != ISPXml.SensorData.GetSize()))
//		{
//			DownPartBinCode(sensor_struct_addr);
//		}else{
//			DownPartBinCode(sensor_struct_addr + sensor_struct_len2);
//		}
//	}
//	if(3 == index){
//		VisionLoadCodeIntoBuffer(tTempLCDBinPath);
//		DownPartBinCode(lcd_struct_addr);
//	}
//	if(4 == index){
//		VisionLoadCodeIntoBuffer(tTempLCDInitBinPath);
//		DownPartBinCode(lcd_init_addr);
//	}
//}

//void AX327X::LoadCodeIntoBuffer(int FlashCapacity) 
//{		
//	int _down_code_len = 0x100;
//	char *_down_code;
//
//	//TCHAR *Temp;
//	CString strTemp;
//	memcpy(strTemp.GetBuffer(MAX_PATH), pDownCodePath, MAX_PATH * sizeof(TCHAR));
//	strTemp.ReleaseBuffer();
//
//	if (strTemp.GetLength() > MAX_PATH)
//	{
//		//throw new TargetMsg(ERR_CFG, IDS_LOG_TARGET_LONG);		//目标文件文件名过长或存储深度太深
//	}
//
//    if (!memcmp(strTemp.Right(4),_T(".bin"), 4 * sizeof(TCHAR)))
//	{
//		//DownFileTpye = FILE_BIN;
//		strTemp = strTemp.Left(strTemp.Find(_T(".bin")));
//		strTemp += _T("BackUp.bin");
//	}
//	else
//	{
//		//DownFileTpye = FILE_INVALID;
//		//throw new TargetMsg(ERR_CFG, IDS_LOG_TARGET_MUST_BINCOD);	//目标文件必须是bin或者cod
//	    AfxMessageBox(_T("no bin file"));
//		return;
//	}
//
//	FILE *fpin;
//	fpin = _tfopen(pDownCodePath, _T("rb"));
//	if(fpin == NULL)
//	{
//		AfxMessageBox(_T("open file fail"));
//		return;
//		////throw new TargetMsg(ERR_CFG, IDS_LOG_TARGET_OPENERR);		//打开目标文件出错
//	}
//	
//	fseek(fpin, 0, SEEK_END);
//	_down_code_len = ftell(fpin);
//	fseek(fpin, 0, SEEK_SET);
//
//	//文件不能超过32M
//	if (_down_code_len > 0x2000000)
//	{
//		////throw new TargetMsg(ERR_CFG, IDS_LOG_TARGET_LARGE);			//目标文件过大
// 	}
//	
//	if(_down_code != NULL)
//		delete _down_code;
//	if (_down_code_len < CODE_BANK_SIZE) {
//		_down_code = new char[4*CODE_BANK_SIZE];
//		memset(_down_code, 0xff, 4*CODE_BANK_SIZE);
//	} else {
//		_down_code = new char[_down_code_len];
//	}
//		
//	memset(_down_code, 0, _down_code_len);
//	fread(_down_code, 1, _down_code_len, fpin);
//
//	fclose(fpin);
//
//}

void AX327X ::DownPartBinCode(int addr)
{
	//align read 4k
	//int flash_start_addr = isp_struct_addr;
	int flash_start_addr = addr;
	//int _code_len;

	int flash_align_addr = flash_start_addr & ~(SPI_PAGE_SIZE-1); //(4KB align)
	int floor_add_len = flash_start_addr - flash_align_addr;
	int ceil_align_addr = (flash_align_addr + floor_add_len + _down_code_len + (SPI_PAGE_SIZE -1)) & ~(SPI_PAGE_SIZE-1);//(4KB align)
	int final_len = ceil_align_addr - flash_align_addr;
	int ceil_add_len = final_len - floor_add_len - _down_code_len;

	char *buf_src;
	char *buf_dst;

	buf_src = new char[final_len];
	buf_dst = new char[final_len];


	SpiRead(flash_align_addr,final_len,buf_src, FALSE);

	for (int i=0; i<final_len; i+=SPI_PAGE_SIZE)
	{
		SpiErase(i+flash_align_addr, SPI_BLOCK_SIZE);
	}
	memcpy(buf_dst,buf_src,floor_add_len);
	memcpy(buf_dst+floor_add_len,_down_code,_down_code_len);
	memcpy(buf_dst+floor_add_len+_down_code_len,buf_src+floor_add_len+_down_code_len,ceil_add_len);
	SpiWrite(buf_dst, flash_align_addr,final_len,  FALSE);

	delete []buf_src;
	delete []buf_dst;

}

//void AX327X::ExportSpiCodeToBin(CString str)
//{
//	LoadDriver();
//	InitDev();
//	if(1 != ProbeDev()) 
//		return;
//
//	char *buf_src;
//
//	if(str.IsEmpty()){
//		throw new Msg(ERR_CFG, _T("invalid path"));	
//		//throw new Msg(ERR_CFG, IDS_LOG_TARGET_OPENERR);		//打开目标文件出错
//		//return;
//	}
//
//	buf_src = new char[SPI_BLOCK_SIZE];
//
//	int i;
//	FILE *fpin;
//	fpin = _tfopen(str, _T("wb"));
//	if(fpin == NULL)
//	{
//		throw new Msg(ERR_CFG, _T("open file err"));	
//		////throw new TargetMsg(ERR_CFG, IDS_LOG_TARGET_OPENERR);		//打开目标文件出错
//	}
//	
//	fseek(fpin, 0, SEEK_SET);
//
//	for(i=0;i<_flash_capacity;i+=SPI_BLOCK_SIZE){
//		SpiRead(i,SPI_BLOCK_SIZE,buf_src, FALSE);
//	
//		fwrite(buf_src, 1, SPI_BLOCK_SIZE, fpin);
//	}
//
//	fclose(fpin);
//	delete []buf_src;
//
//}

bool AX327X::GetSensorData(char* data, int dataSize)
{
	memset((BYTE*)&UsbCmd,0,0x10);

	UsbCmd.L2.Func1 = func_sdk_MemRead;
	UsbCmd.L2.DataAddr = 0xffffffff;
	UsbCmd.L2.Func2 = func_Get_Sensor_Data;
    UsbCmd.L2.Param = 0;

    return uf->UFISPCode((BYTE *)&(UsbCmd), dataSize, data, USB_READ);
}

bool AX327X::CutRaw(char* rawFilePath)
{
    memset((BYTE*)&UsbCmd, 0, 0x10);

    UsbCmd.L2.Func1 = func_sdk_MemRead;
    UsbCmd.L2.DataAddr = 0xffffffff;
    UsbCmd.L2.Func2 = func_UsbCutRaw;
    UsbCmd.L2.Param = 0;

    char tmpFilePath[512] = { 0 };
    bool ret = uf->UFISPCode((BYTE *)&(UsbCmd), sizeof(tmpFilePath), tmpFilePath, USB_READ);
    if (ret)
    {
        strcpy_s(rawFilePath, 512, tmpFilePath);
    }

    return ret;
}

//void AX327X::ThunderConvertXmlToBin(void)
//{
//	TCHAR tTempDebugBinPath2[MAX_PATH];
//	_stprintf(tTempDebugBinPath2, _T("%s\\user\\sensor.bin"), tCurPath);
//
//	CXmlDataInfo ISPXml;
//	BYTE id;
//	ISPXml.SaveSensorID(2,&id,tTempISPInfoPath);
//	TCHAR *ptr;
//	WORD pos;
//	ptr = wcsstr(tTempDebugBinPath2,L".bin");
//	pos = ptr - tTempDebugBinPath2;
//
//	CString str;
//	str.Format(L"_0x%02x",id);
//	tTempDebugBinPath2[pos] = 0;
//	wcscat(tTempDebugBinPath2+pos,str);
//	wcscat(tTempDebugBinPath2+pos,L".bin");
//
//	FILE *fpin;
//	fpin = _tfopen(tTempDebugBinPath2, _T("wb+"));
//	if(fpin == NULL)
//	{
//		AfxMessageBox(_T("ERR! open file fail"));
//		return;		//打开目标文件出错
//	}
//	fseek(fpin, 0, SEEK_SET);
//	fwrite(pISP_Param, 1, ISPXml.GetStructStartAddr(MODULE_NUM)*sizeof(int), fpin);
//	fclose(fpin);
//
//
//	//FILE *fpin;
//	fpin = _tfopen(tTempDebugBinPath, _T("wb+"));
//	if(fpin == NULL)
//	{
//		AfxMessageBox(_T("ERR! open file fail"));
//		return;		//打开目标文件出错
//	}
//	fseek(fpin, 0, SEEK_SET);
//
//	/*ISPXml.LoadSensorFile(tTempInfoPath);
//	int cnt = ISPXml.SensorData.GetSize();
//	if((1 == ISPXml.DebugSensorStatus)&&(0 != cnt))
//	{
//		for(int i=0;i<cnt;i++){
//			fwrite(&(ISPXml.SensorData[i]), 1, 4, fpin);
//		}
//	}
//	fwrite(pISP_Param, 1, ISPXml.GetStructStartAddr(MODULE_NUM)*sizeof(int), fpin);
//	*/
//
//	//ISPXml.LoadParamFile(pISP_Param,tTempInfoPath);
//	fwrite(pISP_Param, 1, ISPXml.GetStructStartAddr(MODULE_NUM)*sizeof(int), fpin);
//
//	fclose(fpin);
//}
