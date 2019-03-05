// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� DEVICE_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// DEVICE_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
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