// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 DEVICE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// DEVICE_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef DEVICE_EXPORTS
#define DEVICE_API extern "C" __declspec(dllexport)
#else
#define DEVICE_API extern "C" __declspec(dllimport)
#endif

typedef void(__stdcall *fnDeviceChangeCallback)(int event, const wchar_t* location, const wchar_t* model, const wchar_t* uvcInterface);

DEVICE_API int test(void);

DEVICE_API void Initialize();

DEVICE_API void UnInitialize();

DEVICE_API void RegDeviceChangeCallback(fnDeviceChangeCallback callback);

DEVICE_API void UnRegDeviceChangeCallback();

DEVICE_API bool WriteAx327XIspProperty(const wchar_t* location, int parameter, void* receivedBuffer, int dataSize);

DEVICE_API bool ReadAx327XIspProperty(const wchar_t* location, int parameter, void* dataBuffer, int dataSize);

DEVICE_API bool WriteAx327XLcdProperty(const wchar_t* location, int parameter, void* receivedBuffer, int dataSize);

DEVICE_API bool ReadAx327XLcdProperty(const wchar_t* location, void* dataBuffer);

DEVICE_API bool GetFlashBinData(const wchar_t* location, int parameter, void* dataBuffer, int dataSize);

DEVICE_API bool Ax327XCutRaw(const wchar_t* location, char* rawFilePath);

DEVICE_API void ScanDevice();