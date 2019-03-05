
#pragma once
//#include "ufc/SpiDriver.h"
#include "AX32XXDevice.h"
#include "Usb/UsbCmd/UsbFunction.h"
#include "Usb/UsbCmd/SpiDriver.h"

#include "IspData.h"


#define CHART_LEN   256 

typedef struct
{
    DWORD lcd_order;
    DWORD lcd_struct_adr;
    DWORD lcd_struct_len;
    DWORD lcd_init_tab_adr;
    DWORD lcd_init_tab_len;
    DWORD lcd_gamma_tab_adr;
    DWORD lcd_gamma_tab_len;
    DWORD lcd_gamma_tab_unit;
    DWORD lcd_contra_tab_adr;
    DWORD lcd_contra_tab_len;
    DWORD lcd_contra_tab_unit;
    DWORD align[5];
}Lcd_Header_;

class AX327X : public AX32XXDevice
{
public:
    AX327X(const wchar_t* filename, const wchar_t* uvcName, const wchar_t * devLocation);
    ~AX327X(void);

public:
    virtual int LoadDriver() override;
    BOOL InstallDriver(void);
    BOOL CheckDriver(void);
    virtual int InitDev(void) override;
    int DeviceReset(void);
    virtual int ProbeDev(void);
    int GetTarget(void);

    int SpiReadID(int cmdid);
    void CheckCapacity(void);

    BOOL SpiWriteEnable(void);
    BOOL SpiReadStatusReg(void);
    virtual bool SpiWrite(char *wrbuf, int Addr, UINT Length, BOOL bEncrypt) override;
    virtual int SpiErase(int Addr, int Length) override;
    virtual bool SpiRead(int Addr,UINT Length, char* ReadBuf, BOOL bEncrypt) override;
    bool PreProcess(void);
    void SetCRC(char *srcData,int srcDataLen);
    void SetChecksum(char *srcData);
    BOOL SetEncryptAddr(char *srcData,int srcDataLen);
    int DownBinCode();
    void DownCode(const std::wstring& str);
    //virtual int DeviceReset(void);
    virtual int PreProbe(void);

private:

    //UINT *pISP_Param;
    //UINT *pLCD_Param;
    //_ISP_UI ISP_UI;
    //_LCD_UI LCD_UI;

    std::wstring tCurPath;
    std::wstring tTempDebugPath;
    std::wstring tTempISPInfoPath;
    std::wstring tTempISP1BinPath;
    std::wstring tTempISP2BinPath;
    std::wstring tTempLCDBinPath;
    std::wstring tTempLCDInitBinPath;
    std::wstring tTempGamachartBinPath;

    std::wstring tTempSensorRegDebugPath;
    std::wstring tTempSensorStructDebugPath;

    std::wstring tSettingOrderPath;
    std::wstring tSettingElfPath;
    std::wstring pDownCodePath;
    std::wstring tTempDebugBinPath;

    int func_Sensor_DebugRegister;
    int func_sdk_MemReadWrite;
    int func_sdk_MemRead;
    int func_sdk_MemWrite;
    int func_Get_Sensor_Data;
    int func_Set_Sensor_Data;
    int func_IspDebugWrite;
    int func_IspDebugRead;
    int func_UsbCutRaw;
    int func_LcdDebugWrite;
    int func_LcdDebugRead;
    int func_LcdRegDebugToggle;
	int func_LcdRegDebugWrite;
	int func_IspReadFlash;

    int isp_tab_len;
    int isp_tab_addr;
    int isp_tab_vma;
    int sensor_cmd_addr;

    int isp_struct_addr;
    int isp_struct_len;
    int sensor_struct_addr;
    int sensor_struct_len;
    int sensor_struct_len2;
    int lcd_struct_addr;
    int lcd_struct_len;
    int lcd_init_addr;
    int lcd_init_len;

public:
    void  InitDownloadParam(void);
    virtual bool  InitDebugParam(void) override;
    void  DebugSensorReg(void);
    bool  DebugIspWrite(int param, char* data, int dataSize);
	bool  DebugLcdWrite(int param, char* data, int dataSize);
    bool  DebugGamachartWirte(int id);
    void  DebugLcdRegWirte(void);
	bool  DebugLcdRead(int reserved1, char* data, int reserved2);
    bool  DebugIspRead(int param, char* data, int dataSize);
    void  VisionConvertParamToUI(void);
    void  VisionConvertParamToUI_lcd(void);
    void  DebugStruct(void);
    void  DebugSensorStructWrite(void);
    void  DebugSensorStructRead(void);
    void  DebugToDownload(void);
    void  ThunderLoadTab(void);
    void  VisionLoadCodeIntoBuffer(std::wstring str);
    //void  DownloadPart(int index);
    void  LoadCodeIntoBuffer(int FlashCapacity);
    void  DownPartBinCode(int addr);
    //void  ExportSpiCodeToBin(std::wstring str);
    bool CutRaw(char* rawFilePath);
    bool SetSensorData(int param, const char* data, int dataSize);
    bool GetSensorData(char* data, int dataSize);
	bool IspReadFlash(int param, char* data, int dataSize);
    void  ThunderConvertXmlToBin();
    Lcd_Header_  lcdheader;



private:
    //USB_CMD  UsbCmd;
    //UsbFunction *uf;
    ELF_INFO ElfInfo;
    ULONG enc_start;

};




