#pragma once

#include "StdInc.h"
#include "Socket.h"

#define INVALID_HWND (HWND)INVALID_HANDLE_VALUE
#define freeif(p) {if(p)delete p; p=nullptr;}
unsigned __stdcall loop(void *arg);

#define ID_FAIL "FAIL"
#define ID_OKAY "OKAY"
#define ID_STAT "STAT"
#define ID_RECV "RECV"
#define ID_DATA "DATA"
#define ID_DONE "DONE"
#define ID_SEND "SEND"


class DeviceBase{
public:
    DeviceBase();
    virtual ~DeviceBase();

public:
    bool connect();
    void release();

public:
	virtual bool onStart() = 0;
    virtual bool onLoop()  = 0;
    virtual void onExit()  = 0;
	static unsigned __stdcall run(void *device);

	virtual void startLoop();	
    virtual void stop();

public://protected:
    int    getLine(std::string &lines);
    bool   sendLine(const char *cmd);
    int    sendBytes(const char* bytes, int count);
    BYTE*  getBytes(int count);

    bool sendAdbCmdOnce(std::string cmd);
    void notifyListeners(UINT msg, WPARAM wparam, LPARAM lparam);

protected:
    static void CALLBACK waitStopTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
    static void   asyncRun(void *arg);

public:
    static void   executeAsyncCommand(DeviceBase* device, const char* cmd);

    static SocketClient* getDeviceChannel(LPCSTR device, int timeout=5000);
    static char*  getDeviceSerial();
    static bool   isDeviceOn();

    static bool  createForward(int localPort, int remotePort, const char *serail);
    static bool  executeRemoteCommand(std::string cmd, std::string serial, int timeout=5000);

    static std::string formAdbRequest(const char *req);

public:
    static DeviceBase *mShellClient;
    static DeviceBase *mCaptureClient;
    static DeviceBase *mFilerClient;
    static DeviceBase *mFileClient;
    static DeviceBase *mMonkeyClient;

public: //private
    bool     mIsContinue;
    double   mLastClock;
    HANDLE   mThread;

	std::vector<HWND> mListeners;

    UINT     mTimerId;
    std::string mDeviceSerial;

public:
    void *mLoopArgs;
    SocketClient *mSocket;
};

void popupMessage(LPCTSTR message);
bool checkResult(SocketClient *s, const char* expected);
bool checkResult(const char *value, const char* expected);