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

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN  //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件:
#include <windows.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "PluginInterface.h"
#include <string>
#include <vector>
#include "AndroidDevice.h"

#define PLUGIN_NAME          "AndroidLogger"
#define PLUGIN_NAME_UNICODE  L"AndroidLogger"
#define LEXER_NAME           "AndroidLog Lexer"
#define MSGBOX_TITLE         L"AndroidLogger v1.2.7"

#define MAX_MENU_ITEMS       10
#define MAX_LOG_LINES        200000

struct PluginSettings {
	TCHAR workingDirectory[MAX_PATH]; // default = D:\AndroidLogger
	TCHAR logcatLevel[2]; // default = V
	TCHAR logcatTag[256]; // default = *
	int autoShowCapture; // default = 1
	int asTag; // default = 0
	int wholeWord; // default = whole = 1
};

extern NppData       g_nppData;
extern FuncItem      g_menuFuncs[MAX_MENU_ITEMS];
extern HINSTANCE     g_hInstance;
extern AndroidDevice gAndroidDevice;
extern int           g_curBufferID;
extern int           g_loggLexerID;


void saveSettings(PluginSettings *settings);
void loadSettings(PluginSettings *settings);

void appendShellCmd(std::string &cmd);
void startShellMode();
void stopShellMode();

bool shellCmd(std::string cmd);
bool shellCmd(std::string cmd, HWND hwnd);

bool shellCmd(std::vector<std::string> &cmds);
bool shellCmd(std::vector<std::string> &cmds, HWND hwnd);

void pullFile(const char* dir, const TCHAR* local, void* args);
void pushFile(const char* dir, const TCHAR* local, void* args);
void getFileList(const char* curDir, void *filer);

void tryApplyLexer();
bool appendLogToNpp(std::string &log);
bool logFilter(std::string &line);
void clearAllText();

const TCHAR* getWorkingDirectory();
bool isAutoShowCapture();

