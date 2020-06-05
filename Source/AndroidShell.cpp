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
#include "DeviceBase.h"
#include "AndroidDevice.h"
#include "FilerPanel.h"
#include "Plugin.h"
#include <time.h>
#include "DebugTrace.h"
#include "StringUtils.h"

bool AndroidShell::sendShellCmd() {
	if (mHandleSemaphore != INVALID_HANDLE_VALUE) {
		::WaitForSingleObject(mHandleSemaphore, INFINITE);
	}

	if (mShellCmds.empty()) {
		return false;
	}

	// plus one "\n", can get echo shell@android $
	std::string cmd = mShellCmds.front() + "\n";
	mShellCmds.erase(mShellCmds.begin());

	if (cmd.find("logcat") != std::string::npos) {
		mIsLogcat = true;
	} else {
		mIsLogcat = false;
	}

	mSocket->SendLine(cmd);
	_xxDebugTrace(TEXT("start RecvLine in sendShellCmd() 111"));
	std::string echo = mSocket->ReceiveLine();

	// disconnected by usr press menu or device lost
	if (echo.size() == 0) {
		//::MessageBox(nullptr, L"echo.size() == 0", MSGBOX_TITLE, MB_OK);
		return false;
	}

#if 0
	/**
	 * Occasionally, get echo only cmd without shell prompt
	 * >ls -l
	 * >shell@android $ ls -l 
	 */
	while (!SU::strStartWith(echo, mShellPrompt.c_str())) {
		echo = mSocket->ReceiveLine();
		if (echo.size() == 0) {
			return false;
		}
	}
#endif

	// oops: maybe empty command
	if (isalpha(cmd[0]) == L'\0') {
		return true;
	}

	cmd = cmd.substr(0, cmd.size()-1);
	mCurShellCmd = cmd;

	mIsCatCmd = false;
	if (cmd == "cat") {
		mIsCatCmd = true;
	}

	// sent shell cmd, get the echo: shell@android $ ls -l
	while (echo.find(cmd) == std::string::npos) {
		_xxDebugTrace(TEXT("start RecvLine in sendShellCmd() 222"));
		echo = mSocket->ReceiveLine();
		if (echo.size() == 0) {
			return false;
		}
	}

	std::string::size_type pos = echo.find("\r\r\n");
	if (pos != std::string::npos) {
		echo.replace(pos, 3, "\r\n");
	}
	mLogLines += echo;

	return true;
}

bool AndroidShell::startShell() {
    if ( !connect() ) {
		notifyListeners(WM_FILE_SHELLCMD, 0, 0);
        return false;
    }

	mIsLogcat = false;
	mLinesCount = 0;
    mIsContinue = true;
    mSocket->SetTimeOut(0, 10);

    std::string msg = formAdbRequest("shell:");
    mSocket->SendLine(msg);
        
    if (checkResult(mSocket, ID_OKAY)) {
        startLoop();
        return true;
    }
    return false;
}

bool AndroidShell::shellCmd(std::vector<std::string> &cmds) {
	if (mHandleSemaphore != INVALID_HANDLE_VALUE) {
		mShellCmds.clear();
		::ReleaseSemaphore(mHandleSemaphore, 1, NULL);
		popupMessage(L"Please wait a while shell mode exit!");
		return false;
	}

	mShellCmds = cmds;
	mHandleSemaphore = INVALID_HANDLE_VALUE;

    return startShell();
}

bool AndroidShell::startShellMode() {
	mHandleSemaphore = ::CreateSemaphore(NULL, 0, 2, NULL);
	if (mSocket != nullptr) {
		stop(); // stop other shell connection
	}
	return startShell();
}

void AndroidShell::stopShellMode() {
	if (mHandleSemaphore != INVALID_HANDLE_VALUE) {
		::ReleaseSemaphore(mHandleSemaphore, 1, NULL);
	}

	if (mSocket != nullptr) {
		stop();
	}
}

void AndroidShell::appendShellCmd(std::string &cmd) {
	if (mSocket == nullptr) {
		mHandleSemaphore = ::CreateSemaphore(NULL, 0, 2, NULL);
		mShellCmds.push_back(cmd);
		startShell();
	} else {
		mShellCmds.push_back(cmd);
		::ReleaseSemaphore(mHandleSemaphore, 1, NULL);
	}
}


bool AndroidShell::getShellPrompt() {
	sendLine("\0");

	std::string echo;
	std::string prompt;

	getLine(echo);
	getLine(mShellPrompt);
	
	_xxDebugTrace(TEXT("echo=[%s], size=%d, mShellPrompt=[%s] size=%d"), SU::Utf8ToTChar(echo.c_str()), echo.size(), SU::Utf8ToTChar(mShellPrompt.c_str()), mShellPrompt.size());

	if (mShellPrompt.empty()){
		return false;
	}

	/** shell@andoroid: /\s$\s\r\r\n **/

	// $ or #
	mShellMode = mShellPrompt[mShellPrompt.size() - 5];

	//shell@andoroid:
	mShellPrompt = mShellPrompt.substr(0, mShellPrompt.find_first_of('/'));

	_xxDebugTrace(TEXT("mShellPrompt2=[%s] size2=%d"), SU::Utf8ToTChar(mShellPrompt.c_str()), mShellPrompt.size());
	return mShellPrompt.size() > 0;
}

/**
 * release socket & resources
 */
void AndroidShell::release() {
    DeviceBase::release();
    freeif(mSocket);
    
	mShellCmds.clear();
    mLogLines.clear();
    
	mLinesCount = 0;
    mLastClock = 0;

    mIsContinue = false;

	if (mHandleSemaphore != INVALID_HANDLE_VALUE) {
		::CloseHandle(mHandleSemaphore);
		mHandleSemaphore = INVALID_HANDLE_VALUE;
	}
}

bool AndroidShell::onStart() {
	getShellPrompt();
    sendShellCmd();
    return true;
}

void AndroidShell::onExit() {
    appendLogToNpp(std::string("\n------------ Exit ------------\n"));
    release();
    ::PostMessage(g_nppData._nppHandle, NPPM_SETMENUITEMCHECK, (WPARAM)g_menuFuncs[0]._cmdID, (LPARAM)false);
    ::PostMessage(g_nppData._nppHandle, NPPM_SETMENUITEMCHECK, (WPARAM)g_menuFuncs[1]._cmdID, (LPARAM)false);
    ::PostMessage(g_nppData._nppHandle, NPPM_SETMENUITEMCHECK, (WPARAM)g_menuFuncs[2]._cmdID, (LPARAM)false);

	notifyListeners(WM_FILE_SHELLCMD, 0, 0);
}

//shell@android:/sdcard $
bool AndroidShell::onLoop() {
    std::string line;

	_xxDebugTrace(TEXT("start RecvLine in sendShellCmd() 333"));
    int size = getLine(line);
	bool doubleCrEnd = (size > 3) && (line[size-3] == '\r');

	//shell@andoroid: /\s$\s\r\r\n
	if (doubleCrEnd) {
		bool isPromtLine = SU::strStartWith(line, mShellPrompt.c_str());

		if ( isPromtLine || mIsCatCmd == false ) {
			
			std::string test;
			_xxDebugTrace(TEXT("start RecvLine in sendShellCmd() 444"));
			test = mSocket->ReceiveLines();		

			std::string::size_type pos = line.find("\r\r\n");
			if (pos != std::string::npos) {
				line.replace(pos, 3, "\r\n");
			}
			
			mLogLines += line;
			appendLogToNpp(mLogLines);

			mLogLines.clear();
			mLastClock = clock();
			return sendShellCmd();
		} 
		
		 // cat file : if text file already including \r(+\r\n)
		if ( mIsCatCmd ) {
			line[size-2] = '\n';
			line.pop_back();
		}
	}

    double cost = clock() - mLastClock;

    /** timeout, device lost?! **/
    size += mLogLines.size();
    if ( size == 0 ) {

        /** try again **/
        if (cost < 200) {
            return true;
        }

        /** device lost ? **/
        if ( !isDeviceOn() ) { 
            return false;
        }
    }

	_xxDebugTrace(TEXT("mIsLogcat=[%d], logFilter(line)=%d"), mIsLogcat, logFilter(line));
	if ( (!mIsLogcat) || (mIsLogcat && logFilter(line)) ) {
		mLogLines += line;
		mLinesCount += 1;
	}

    /** interval too short **/
	_xxDebugTrace(TEXT("cost=[%f], mIsContinue=%d"), cost, mIsContinue);
    if ( cost < 20 ) {
        return mIsContinue;
    }

    /** current buffer maybe hide! **/
    if ( !appendLogToNpp(mLogLines) ){
        return false;
    }

    /** too many lines ! **/
    if ( mLinesCount > 990000 ) {
        return false;
    }

    mLogLines.clear();
    mLastClock = clock();

    return mIsContinue;
}

AndroidShell::AndroidShell():DeviceBase(){
	mHandleSemaphore = INVALID_HANDLE_VALUE;
	mLinesCount   = 0;
    mLastClock    = 0;
    mIsContinue   = false;
	mShellPrompt  = "";
	mShellMode    = "";
}

AndroidShell::~AndroidShell() {
}