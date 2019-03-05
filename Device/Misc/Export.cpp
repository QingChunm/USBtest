// Device.cpp : 定义 DLL 应用程序的导出函数。
//

#include "Misc\stdafx.h"
#include "Export.h"

#include "..\DeviceManager\DeviceManager.h"

#include "..\Data\IspData.h"
#include "..\Data\AX327X.h"


// 这是导出变量的一个示例
DEVICE_API int nDevice = 0;

#ifdef _DEBUG
Blc blcData;
short lscData[572];
Awb awbData;
Gamma gammaData;

COMMON commonData;
#endif // DEBUG

void CopyToMember(void* ispModuleData, int offsetOfMember, int sizeOfMember, void* receivedBuffer, int startPos, int dataSize)
{
    int endPos = startPos + dataSize;

    if (startPos <= offsetOfMember)
    {
        if (endPos > offsetOfMember)
        {
            memcpy((char*)ispModuleData + offsetOfMember,
                (char*)receivedBuffer + (offsetOfMember - startPos),
                endPos > offsetOfMember + sizeOfMember ? sizeOfMember : endPos - offsetOfMember);
        }
    }

    else if (startPos > offsetOfMember && startPos <= offsetOfMember + sizeOfMember)
    {
        memcpy((char*)ispModuleData + startPos,
            receivedBuffer,
            endPos > offsetOfMember + sizeOfMember ?
        sizeOfMember : endPos - startPos);
    }
}

DEVICE_API bool WriteAx327XIspProperty(const wchar_t* location, int parameter, void* receivedBuffer, int dataSize)
{
    AX327X* devicePtr = dynamic_cast<AX327X*>(DeviceManager::GetInstance().GetDevice(location));
    if (devicePtr != nullptr)
    {
#ifdef _DEBUG
        int module = parameter & 0xff;
        int startPos = parameter >> 8;
        switch (module)
        {
        case _BLC_POS_:
        {
            CopyToMember(&blcData, offsetof(Blc, blkl_r), sizeof(blcData.blkl_r), receivedBuffer, startPos, dataSize);
            CopyToMember(&blcData, offsetof(Blc, blkl_gr), sizeof(blcData.blkl_gr), receivedBuffer, startPos, dataSize);
            CopyToMember(&blcData, offsetof(Blc, blkl_gb), sizeof(blcData.blkl_gb), receivedBuffer, startPos, dataSize);
            CopyToMember(&blcData, offsetof(Blc, blkl_b), sizeof(blcData.blkl_b), receivedBuffer, startPos, dataSize);
            break;
        }

        case _LSC_POS_:
        {
            CopyToMember(&lscData, 0, sizeof(lscData), receivedBuffer, startPos, dataSize);
            break;
        }

        case _AWB_POS_:
        {
            CopyToMember(&awbData, offsetof(Awb, seg_mode), sizeof(awbData.seg_mode), receivedBuffer, startPos, dataSize);
            CopyToMember(&awbData, offsetof(Awb, rg_start), sizeof(awbData.rg_start), receivedBuffer, startPos, dataSize);
            CopyToMember(&awbData, offsetof(Awb, rgmin), sizeof(awbData.rgmin), receivedBuffer, startPos, dataSize);
            CopyToMember(&awbData, offsetof(Awb, rgmax), sizeof(awbData.rgmax), receivedBuffer, startPos, dataSize);
            CopyToMember(&awbData, offsetof(Awb, weight_in), sizeof(awbData.weight_in), receivedBuffer, startPos, dataSize);
            CopyToMember(&awbData, offsetof(Awb, weight_mid), sizeof(awbData.weight_mid), receivedBuffer, startPos, dataSize);
            CopyToMember(&awbData, offsetof(Awb, ymin), sizeof(awbData.ymin), receivedBuffer, startPos, dataSize);
            CopyToMember(&awbData, offsetof(Awb, ymax), sizeof(awbData.ymax), receivedBuffer, startPos, dataSize);
            CopyToMember(&awbData, offsetof(Awb, hb_rate), sizeof(awbData.hb_rate), receivedBuffer, startPos, dataSize);
            CopyToMember(&awbData, offsetof(Awb, hb_class), sizeof(awbData.hb_class), receivedBuffer, startPos, dataSize);
            CopyToMember(&awbData, offsetof(Awb, hr_rate), sizeof(awbData.hr_rate), receivedBuffer, startPos, dataSize);
            CopyToMember(&awbData, offsetof(Awb, hr_class), sizeof(awbData.hr_class), receivedBuffer, startPos, dataSize);
            CopyToMember(&awbData, offsetof(Awb, awb_scene_mod), sizeof(awbData.awb_scene_mod), receivedBuffer, startPos, dataSize);
            CopyToMember(&awbData, offsetof(Awb, manu_awb_gain), sizeof(awbData.manu_awb_gain), receivedBuffer, startPos, dataSize);
            CopyToMember(&awbData, offsetof(Awb, yuv_mod_en), sizeof(awbData.yuv_mod_en), receivedBuffer, startPos, dataSize);
            CopyToMember(&awbData, offsetof(Awb, cb_th), sizeof(awbData.cb_th), receivedBuffer, startPos, dataSize);
            CopyToMember(&awbData, offsetof(Awb, cr_th), sizeof(awbData.cr_th), receivedBuffer, startPos, dataSize);
            CopyToMember(&awbData, offsetof(Awb, cbcr_th), sizeof(awbData.cbcr_th), receivedBuffer, startPos, dataSize);
            CopyToMember(&awbData, offsetof(Awb, ycbcr_th), sizeof(awbData.ycbcr_th), receivedBuffer, startPos, dataSize);
            CopyToMember(&awbData, offsetof(Awb, seg_gain), sizeof(short) * 8 * 3, receivedBuffer, startPos, dataSize);
            CopyToMember(&awbData, offsetof(Awb, awb_tab), sizeof(awbData.awb_tab), receivedBuffer, startPos, dataSize);
            break;
        }

        case _YGAMA_POS_:
        {
            CopyToMember(&gammaData, offsetof(Gamma, pad_num), sizeof(gammaData.pad_num), receivedBuffer, startPos, dataSize);
            CopyToMember(&gammaData, offsetof(Gamma, using_ygama), sizeof(gammaData.using_ygama), receivedBuffer, startPos, dataSize);
            break;
        }

        default:
            break;
        }
#endif  //_DEBUG

        return devicePtr->DebugIspWrite(parameter, (char*)receivedBuffer, dataSize);
    }

    return false;
}


DEVICE_API bool WriteAx327XLcdProperty(const wchar_t* location, int parameter, void* receivedBuffer, int dataSize)
{
	AX327X* devicePtr = dynamic_cast<AX327X*>(DeviceManager::GetInstance().GetDevice(location));
	if (devicePtr != nullptr)
	{
		return devicePtr->DebugLcdWrite(parameter, (char*)receivedBuffer, dataSize);
	}

	return false;
}

DEVICE_API bool ReadAx327XLcdProperty(const wchar_t* location, void* dataBuffer)
{
	AX327X* devicePtr = dynamic_cast<AX327X*>(DeviceManager::GetInstance().GetDevice(location));
	if (devicePtr != nullptr)
	{
		return devicePtr->DebugLcdRead(0, (char*)dataBuffer, 0);
	}

	return false;
}


DEVICE_API bool ReadAx327XIspProperty(const wchar_t* location, int parameter, void* dataBuffer, int dataSize)
{
    AX327X* devicePtr = dynamic_cast<AX327X*>(DeviceManager::GetInstance().GetDevice(location));
    if (devicePtr != nullptr)
    {
//#ifdef _DEBUG
//        int module = parameter & 0xff;
//        int startPos = parameter >> 8;
//        switch (module)
//        {
//        case _BLC_POS_:
//        {
//            memcpy(dataBuffer, (char*)&blcData + startPos, dataSize);
//            break;
//        }
//        case _LSC_POS_:
//        {
//            memcpy(dataBuffer, (char*)&lscData + startPos, dataSize);
//            break;
//        }
//        case _AWB_POS_:
//        {
//            memcpy(dataBuffer, (char*)&awbData + startPos, dataSize);
//            break;
//        }
//        case _YGAMA_POS_:
//        {
//            memcpy(dataBuffer, (char*)&gammaData + startPos, dataSize);
//            break;
//        }
//        default:
//            break;
//        }
//#endif //_DEBUG

        return devicePtr->DebugIspRead(parameter, (char*)dataBuffer, dataSize);
    }

    return false;
}

DEVICE_API bool ReadAx327XSensorProperty(const wchar_t* location, int parameter, void* dataBuffer, int dataSize)
{
    AX327X* devicePtr = dynamic_cast<AX327X*>(DeviceManager::GetInstance().GetDevice(location));
    if (devicePtr != nullptr)
    {
#ifdef _DEBUG
		devicePtr->GetSensorData((char*)&commonData, dataSize);
#endif
		return devicePtr->GetSensorData((char*)dataBuffer, dataSize);
    }
    return false;
}

DEVICE_API bool WriteAx327XSensorProperty(const wchar_t* location, int parameter, void* dataBuffer, int dataSize)
{
    AX327X* devicePtr = dynamic_cast<AX327X*>(DeviceManager::GetInstance().GetDevice(location));
    if (devicePtr != nullptr)
    {
        return devicePtr->SetSensorData(parameter, (char*)dataBuffer, dataSize);
    }
    return false;
}

DEVICE_API bool GetFlashBinData(const wchar_t* location, int parameter, void* dataBuffer, int dataSize)
{
	AX327X* devicePtr = dynamic_cast<AX327X*>(DeviceManager::GetInstance().GetDevice(location));
	if (devicePtr != nullptr)
	{
		return devicePtr->IspReadFlash(parameter, (char*)dataBuffer, dataSize);
	}
	return false;
}

DEVICE_API bool Ax327XCutRaw(const wchar_t* location, char* rawFilePath)
{
    AX327X* devicePtr = dynamic_cast<AX327X*>(DeviceManager::GetInstance().GetDevice(location));
    if (devicePtr != nullptr)
    {
        return devicePtr->CutRaw(rawFilePath);
    }
    return false;
}

DEVICE_API void Initialize()
{
    DeviceManager::GetInstance().Initialize();
}

DEVICE_API void UnInitialize()
{
    DeviceManager::GetInstance().Uninitialize();
}

DEVICE_API void RegDeviceChangeCallback(fnDeviceChangeCallback callback)
{
    DeviceManager::GetInstance().AddDevChangeNotifyFunc(callback);
}

DEVICE_API void UnRegDeviceChangeCallback()
{
    DeviceManager::GetInstance().RemoveDevChangeNotifyFunc();
}

DEVICE_API void ScanDevice()
{
    DeviceManager::GetInstance().ScanDevice();
}