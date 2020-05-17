/*
  AndroidLogger: Android Logger plugin for Notepad++
  Copyright (C) 2015 Simbaba at Najing <zhaoxi.du@gmail.com>

  ******************************************************
  Thanks for GedcomLexer & NppFtp plugin source code.
  ******************************************************
 
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/
#pragma once

#include "StdInc.h"
#include "Socket.h"
#include "DeviceBase.h"
#include "ImageUtil.h"


class AndroidShell: public DeviceBase {
public:
    AndroidShell();
    virtual ~AndroidShell();

public:
	virtual bool onStart();
    virtual bool onLoop();
    virtual void onExit();

protected:
    void sendShellCmds();
    bool startShell();

public:
	bool getShellPrompt();
	bool sendShellCmd();

    bool shellCmd(std::vector<std::string> &cmds);	

	bool startShellMode();
	void stopShellMode();
	void appendShellCmd(std::string &cmd);

	void release();

public://private:
    bool mIsLogcat;
	bool mIsCatCmd;
    std::vector<std::string> mShellCmds;
    int  mLinesCount;
    std::string mLogLines;
	std::string mShellPrompt;
	std::string mCurShellCmd;
	std::string mShellMode; // # root ? $ usr?

	HANDLE mHandleSemaphore;
	bool mInteractivie;
};

class AndroidFiler: public DeviceBase {
public:
    AndroidFiler();
    virtual ~AndroidFiler();

public://protected:
	virtual bool onStart();
    virtual bool onLoop();
    virtual void onExit();

    void release();

public: 
    bool listDir(const char* dir, void *args);

private:
    tstring _local;
};

class AndroidFile: public DeviceBase {
public:
    AndroidFile();
    virtual ~AndroidFile();

public://protected:
	virtual bool onStart();
    virtual bool onLoop();
    virtual void onExit();

    void release();

protected:
    bool onPullStart();
    bool onPullLoop();
    void onPullExit();
    
	bool onPushStart();
	bool onPushLoop();
    void onPushExit();

public:
    bool pullFile(const char* remote, const TCHAR* local, void* args);
    bool pushFile(const char* remote, const TCHAR* path, void* args);

public: //private
    tstring mLocalPath;
    bool mIsPull;
};

class AndroidCapture: public DeviceBase {
public:
    AndroidCapture();
    virtual ~AndroidCapture();

public:
	virtual bool onStart();
    virtual bool onLoop();
    virtual void onExit();

    bool capture();
};

class AndroidMonkey: public DeviceBase {
public:
    AndroidMonkey();
    virtual ~AndroidMonkey();

public://protected:
	virtual bool onStart();
    virtual bool onLoop();
    virtual void onExit();

    void release();

public://protected
    bool connectMonkey();
    bool sendMonkeyCmd(const char* monkey);
    void quitMonkey();

public:
    bool monkey(std::vector<std::string> &cmds);

public://private:
    std::vector<std::string> mMonkeyCmds;
};

class AndroidDevice {
public:
    AndroidDevice();
    ~AndroidDevice();

public:
	bool shellCmd(std::string cmd);
	bool shellCmd(std::string cmd, HWND hwnd);
    bool shellCmd(std::vector<std::string> &cmds);
	bool shellCmd(std::vector<std::string> &cmds, HWND hwnd);

	void startShellMode();
	void stopShellMode();
	void appendShellCmd(std::string &cmd);

    bool capture();
    bool pullFile(const char* remote, const TCHAR* local, void* args);
    bool pushFile(const char* remote, const TCHAR* local, void* args);
    bool lsDir(const char* dir, void* args);
    
    /*********************************************
    PRESS|{'name':'HOME','type':'downAndUp',}
    PRESS|{'name':'HOME','type':'down',}
    PRESS|{'name':'HOME','type':'up',}
    TOUCH|{'x':171,'y':1096,'type':'downAndUp',}
    DRAG|{'start':(115,512),'end':(576,512),'duration':1.0,'steps':10,}
    DRAG|{'start':(576,512),'end':(115,512),'duration':1.0,'steps':10,}
    TOUCH|{'x':468,'y':1170,'type':'downAndUp',}
    TYPE|{'message':'123456',}
    **********************************************/
    bool monkey(std::vector<std::string> &cmds);

    void closeAllClients();
};

enum {
    KEYCODE_HOME,
    KEYCODE_MENU,
    KEYCODE_BACK,
};

extern bool appendLogToNpp(std::string &log);

