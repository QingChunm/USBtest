#include "Misc\stdafx.h"
#include "AX32XXDevice.h"

#include <windows.h>

#include "Data\Elf\pof.h"


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
char readid_a[4][2] = {
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

//打印信息到LOG框
void printf_log(TCHAR *str, ...)
{
	va_list msg_list;
	va_start(msg_list, str);
	::SendMessage(hMainWnd, MYWM_PRINT_LOGR, (WPARAM)new TargetMsg(NULL, str, msg_list), pritf_index);
	va_end(msg_list);
}

//构造信息
TargetMsg::TargetMsg(void)
{
	_msgnum = NULL;
	_msg = NULL;
}

void TargetMsg::printfmsg(int msgnum, TCHAR *msg, va_list msg_list)
{
	_msgnum = msgnum;
	_msg = new TCHAR[_tcslen(msg) + 256];
	_vstprintf(_msg, msg, msg_list);
}

void TargetMsg::printfmsg(int msgnum, int msg, va_list msg_list)
{
	CString temp;
	LoadStringRes(temp, msg);
	_msgnum = msgnum;
	_msg = new TCHAR[temp.GetLength() + 256];
	_vstprintf(_msg, temp, msg_list);
}

TargetMsg::TargetMsg(int msgnum, TCHAR *msg, va_list msg_list)
{
	printfmsg(msgnum, msg, msg_list);
}

TargetMsg::TargetMsg(int msgnum, TCHAR *msg, ...)
{
	va_list msg_list;
	va_start(msg_list, msg);
	printfmsg(msgnum, msg, msg_list);
	va_end(msg_list);
}

TargetMsg::TargetMsg(int msgnum, int msg, va_list msg_list)
{
	printfmsg(msgnum, msg, msg_list);
}

TargetMsg::TargetMsg(int msgnum, int msg, ...)
{
	va_list msg_list;
	va_start(msg_list, msg);
	printfmsg(msgnum, msg, msg_list);
	va_end(msg_list);
}

//析构
TargetMsg::~TargetMsg(void)
{
	if (NULL != _msg)
		delete[] _msg;
}

//设置信息
void TargetMsg::SetMsg(int msgnum, TCHAR *msg, ...)
{
	va_list msg_list;
	va_start(msg_list, msg);
	if (NULL != _msg) {
		delete[] _msg;
		_msg = NULL;
	}
	_msgnum = msgnum;
	_msg = new TCHAR[_tcslen(msg) + 256];
	_vstprintf(_msg, msg, msg_list);
	va_end(msg_list);
}

//获取信息
TCHAR *TargetMsg::GetMsg(void)
{
	return _msg;
}

//获取状态条信息
TCHAR *TargetMsg::GetStaMsg(void)
{
	if (_msgnum == ERR_SPI_ID) {
		return _T("Flash ID ERROR");
	} else {
		return _msg;
	}
}
#endif

#if 1
AX32XXDevice::AX32XXDevice(const wchar_t* filename, const wchar_t* uvcName, const wchar_t* devLocation)
{
    m_devLocation = devLocation;
    m_uvcName = uvcName;
    m_fileHandle = ::CreateFile(filename,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);  //打开设备

	uf = new UsbFunction;
    uf->LoadFileHandle(m_fileHandle);

	_down_code = NULL;
	_down_code_len = 0;

#if 0
	_cbw_buf = new char[32];
	_rd_buf = new char[SPI_BLOCK_SIZE];
	_flash_name = new char[100];
	//_fat_buf = NULL;
	_removed = 0;
	_drive = drive;
	_index = index;
	_status = DT_READY;
	_down_code_len = 0;
	if_have_father = 0;
	//_precopy_len = 0;
	_down_code = NULL;
	_lasterr = NULL;
	iDeviceVesion = DeviceVesion;
	//_SpiVersion = SpiVersion;

	_flash_capacity = 0;

	spiDriver = new SpiDriver;

    spiDriver->LoadFileHandle(m_fileHandle);	
#endif	
}
#endif
AX32XXDevice::~AX32XXDevice(void)
{
#if 0
	delete[] _cbw_buf;
	delete[] _rd_buf;
	delete[] _flash_name;
#endif
	delete[] _down_code;
	//delete[] _fat_buf;

/*
	if(NULL != tCurPath){
		delete tCurPath;
		tCurPath = NULL;
	}
*/
    if (m_fileHandle != NULL)
	{
        ::CloseHandle(m_fileHandle);
        m_fileHandle = NULL;
	}
	delete uf;

}

void AX32XXDevice::AnalyzeElf(ELF_INFO *Dev)
{
	POF_HDL hdl = pof_open_file(Dev->Name);
	if( hdl != NULL ) 
	{

	}
	pof_read_symbol(hdl, Dev->Pubsym);
	CTX_HDL ctx = pof_context_init( hdl );
	if( ctx ) {
		int l;
		int cnt = 0;
		Dev->DriverFileLen = 0; 
		while(( l = pof_get_next_seg( hdl, ctx ))) {
			//memcpy((BYTE*)spiDriver->SpiDriverCtrl.SpiDriverBuf+spiDriver->SpiDriverCtrl.File_SpiDriver_Length,
				  //ctx->buf, 0x1000);
			memcpy((BYTE*)Dev->DriverBuf + Dev->DriverFileLen,ctx->buf, l);
			//spiDriver->SpiDriverCtrl.File_SpiDriver_Length += 0x1000;
			 Dev->DriverFileLen += l;
			 if(0 == cnt){
				 //record the first address
				 Dev->DriverLoadAddr = ctx->addr;
			 }
			 cnt++;
			//(*dev->raw_rw) (dev->hFile, 0, ctx->addr, l, ctx->buf);
			//spiDriver->SpiDriverParam.SpiWriteAdr  =  Code2xDataOffset+ctx->addr;
	}

	pof_context_free( ctx );
	} else printf( "No phdr\r\n" );
	pof_close( hdl );
}

BOOL AX32XXDevice::InstallDriver(void)
{
    return TRUE;
}

BOOL AX32XXDevice::CheckDriver(void)
{
	return TRUE;
}

int AX32XXDevice::ProbeDev(void)
{
	return 0;
}


int AX32XXDevice::LoadCodeIntoBuffer(int FlashCapacity, std::wstring pDownCodePath)
{		
	int _down_code_len = 0x100;
	char *_down_code = nullptr;

	//TCHAR *Temp;
	std::wstring strTemp;
	strTemp = pDownCodePath;

	FILE *fpin = nullptr;
	_tfopen_s(&fpin, pDownCodePath.c_str(), _T("rb"));
	if(fpin == NULL)
	{
		return 1;
	}
	
	fseek(fpin, 0, SEEK_END);
	_down_code_len = ftell(fpin);
	fseek(fpin, 0, SEEK_SET);

	//文件不能超过32M
	if (_down_code_len > 0x2000000)
	{
		return 2;
 	}
	
	if(_down_code != NULL)
		delete _down_code;
	if (_down_code_len < CODE_BANK_SIZE) {
		_down_code = new char[4*CODE_BANK_SIZE];
		memset(_down_code, 0xff, 4*CODE_BANK_SIZE);
	} else {
		_down_code = new char[_down_code_len];
	}
		
	memset(_down_code, 0, _down_code_len);
	fread(_down_code, 1, _down_code_len, fpin);

	fclose(fpin);

	return 0;
}
#if 0
//SPI 读
void AX32XXDevice::SpiRead(int Addr,UINT Length, char* ReadBuf, BOOL bEncrypt)
{
	
	BOOL bSuccess;
	bSuccess = spiDriver->SpiRead(Addr,Length,ReadBuf, bEncrypt);
	if (!bSuccess)
	{
		//throw new TargetMsg(ERR_USB_DEVICE, _T("USB ERROR[3]"));
	}
}

//SPI 写
void AX32XXDevice::SpiWrite(char *wrbuf, int Addr, UINT Length, BOOL bEncrypt)
{	
	
	BOOL bSuccess;
	bSuccess = spiDriver->SpiWrite(Addr,Length, wrbuf, _write_len, bEncrypt);
	if (!bSuccess)
	{
		//throw new TargetMsg(ERR_USB_DEVICE, _T("USB ERROR[2]"));
	}
}

//SPI 擦
void AX32XXDevice::SpiErase(int Addr, int Length)
{
	
	BOOL bSuccess;
	bSuccess = spiDriver->SpiWriteEnable();
	if (_cmd_erase != 0xd8)
	{
		for (int i=0; i<Length; i+=4*1024)
		{
			bSuccess = spiDriver->SpiErase(Addr+i);
			if (!bSuccess)
			{
				//throw new TargetMsg(ERR_USB_DEVICE, _T("USB ERROR[d8]"));
			}
		}
	}
	else
	{
		bSuccess = spiDriver->SpiErase(Addr);
		if (!bSuccess)
		{
			//throw new TargetMsg(ERR_USB_DEVICE, _T("USB ERROR[d8]"));
		}
	}
	/**/
}
#endif
//SPI 读ID

//从库中根所ID查找配置
int AX32XXDevice::SpiSearchID(int cmdid, int id)
{
#if 1
    int i, tmp_id, tmp_mask, capacity;
    int wrcmd;
    TCHAR secname[10], keyname_id[20], keyname_mask[20];
    _stprintf_s(keyname_id, _T("ID-%02X"), (BYTE)readid_a[cmdid][0]);
    _stprintf_s(keyname_mask, _T("ID-%02X-MASK"), (BYTE)readid_a[cmdid][0]);
    for (i = 1;; i++) {
        _stprintf_s(secname, _T("%d"), i);
        capacity = GetPrivateProfileInt(secname, _T("Capacity"), 0, m_FlashLibPath.c_str());
        if (capacity == 0) {  //读不到容量，则认为配置文件结束
            break;
        }
        tmp_id = GetPrivateProfileInt(secname, keyname_id, 0, m_FlashLibPath.c_str());
        tmp_mask = GetPrivateProfileInt(secname, keyname_mask, 0, m_FlashLibPath.c_str());
        _cmd_writestatus_en = GetPrivateProfileInt(secname, _T("Enable-Write-Status-Register"), 0, m_FlashLibPath.c_str());
        if (tmp_id != 0 && tmp_mask != 0 && (id & tmp_mask) == tmp_id) {
            //找到匹配的ID
            _flash_capacity = capacity;  //获取容量
            wrcmd = GetPrivateProfileInt(secname, _T("Page-Program"), 0, m_FlashLibPath.c_str());
            if (wrcmd == 0) {
                _cmd_write = GetPrivateProfileInt(secname, _T("Byte-Program"), 0x02, m_FlashLibPath.c_str());
                _write_len = 1;
            }
            else {
                _cmd_write = wrcmd;
                _write_len = GetPrivateProfileInt(secname, _T("Page-Size"), 256, m_FlashLibPath.c_str());
            }
            lib_id = i;
            return 1;
        }
    }
    lib_id = i;
#endif
    return 0;
}

void AX32XXDevice::Soft_crc16 (WORD poly, char val, WORD *crc) 
{
	int i;

	for (i=0; i<8; ++i) {
		if (((val>>7)^(*crc>>15))&1) *crc = (*crc<<1) ^ poly;
		else *crc <<= 1;
		val <<= 1;
	}
}

void AX32XXDevice::DownCode(std::wstring str){
}

void AX32XXDevice::InitDownloadParam(void){
}

void AX32XXDevice::DownloadPart(int index){
}

void AX32XXDevice::ExportSpiCodeToBin(std::wstring str){
}

int AX32XXDevice::DeviceReset(void){
	return 0;
}

#if 1
volatile int _status;
//设置状态
void AX32XXDevice::SetStatus(int status)
{
	_status = status;
}

//获取状态
int AX32XXDevice::GetStatus(void)
{
	/*if (_removed) {
		return DT_STOP;
	} else {
		return _status;
	}*/
	return _status;
}

const wchar_t* AX32XXDevice::GetDevLocation()
{
    return m_devLocation.c_str();
}

const wchar_t* AX32XXDevice::GetUvcInterface()
{
    return m_uvcName.c_str();
}
#endif

