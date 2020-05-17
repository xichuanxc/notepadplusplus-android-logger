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
#include "Plugin.h"
#include "PluginInterface.h"
#include "resource.h"
#include <vector>
#include <Shellapi.h>
#include "Window.h"
#include "FilerPanel.h"
#include "ShellPanel.h"
#include "SettingsDialog.h"
#include "AndroidDevice.h"

#define IsValidLevel(l) (l[0] == L'V' || l[0] == L'D' || l[0] == L'I' || l[0] == L'W' || l[0] == L'E')

/** Main Window handle for Notepad++ */
HWND g_hwndNpp = nullptr;

/** The plugin data that Notepad++ needs */
FuncItem g_menuFuncs[MAX_MENU_ITEMS];
std::vector<std::string> g_keyWords;

/** The data of Notepad++ that you can use in your plugin commands */
NppData g_nppData;
AndroidDevice gAndroidDevice;

SettingsDialog	m_settingsDialog;
FilerPanel *m_filerPanel = nullptr;
ShellPanel *m_shellPanel = nullptr;

PluginSettings g_pluginSettings;

/** log share buffer **/
int       g_curBufferID = 0;
int       g_loggLexerID = 0;

HMENU     g_mainMenuHandle;
HINSTANCE g_hInstance = NULL;
HBITMAP   g_iconsOfToolbar[4]; // 0: AP LOG 1: CP LOG 2: SHELL 3: CAPTRUE

/**
 * Internal functions
 */
void pluginInit(HINSTANCE hModule);
void pluginCleanup();

bool menuAddItem(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit);
void menuInit();
void hideFoldMargin(int bufferID, int langType);
int  findMyLexerMenu();

int  getCurAllAdbCmds(std::vector<std::string> &cmds);
int  getCurMonkeyCmds(std::vector<std::string> &cmds);


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
extern "C" BOOL APIENTRY DllMain( HINSTANCE hModule, DWORD  reasonForCall, LPVOID lpReserved )
{
    switch (reasonForCall)
    {
    case DLL_PROCESS_ATTACH:
        g_hInstance = hModule;
        pluginInit(hModule);
        break;

    case DLL_PROCESS_DETACH:
        pluginCleanup();
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}

/**
 * notepad++ cb
 */
void setInfo(NppData notpadPlusData){
	menuInit();

	g_nppData = notpadPlusData;
    g_mainMenuHandle = ::GetMenu(notpadPlusData._nppHandle);
    g_hwndNpp = notpadPlusData._nppHandle;
    
    m_filerPanel = new FilerPanel();
	m_shellPanel = new ShellPanel();
    m_filerPanel->Create(notpadPlusData._nppHandle, notpadPlusData._nppHandle, 0, 4);
	m_shellPanel->Create(notpadPlusData._nppHandle, notpadPlusData._nppHandle, 0, 6);

	loadSettings(&g_pluginSettings);
}

/**
 * notepad++ cb
 */
const TCHAR * getName(){
    return PLUGIN_NAME_UNICODE;
}

/**
 * notepad++ cb
 */
FuncItem * getFuncsArray(int *nbF){
    *nbF = MAX_MENU_ITEMS;
    return g_menuFuncs;
}

/**
 * notepad++ cb
 */
void beNotified(SCNotification *notifyCode)
{
    switch (notifyCode->nmhdr.code) 
	{
    case NPPN_SHUTDOWN:
        if ( m_filerPanel != nullptr ) {
            m_filerPanel->Destroy();
            delete m_filerPanel;
        }

		if ( m_shellPanel != nullptr) {
			m_shellPanel->Destroy();
            delete m_shellPanel;
		}
        return;

    case NPPN_BUFFERACTIVATED: {
        int langType = ::SendMessage(g_nppData._nppHandle, NPPM_GETBUFFERLANGTYPE, notifyCode->nmhdr.idFrom, 0);
        if ( langType > L_EXTERNAL ) {
            hideFoldMargin( notifyCode->nmhdr.idFrom, langType);
            return;
        }
    }
    break;

    case NPPN_WORDSTYLESUPDATED:
    case NPPN_LANGCHANGED: {
        int langType = ::SendMessage(g_nppData._nppHandle, NPPM_GETBUFFERLANGTYPE, notifyCode->nmhdr.idFrom, 0);
        if ( langType > L_EXTERNAL ) {
            hideFoldMargin( notifyCode->nmhdr.idFrom, langType);
            return;
        }
    }
    return;

    case NPPN_TBMODIFICATION: {
        toolbarIcons tbiIcons;
        
        tbiIcons.hToolbarBmp = g_iconsOfToolbar[0];
        tbiIcons.hToolbarIcon = NULL;
        ::SendMessage(g_nppData._nppHandle, NPPM_ADDTOOLBARICON, (WPARAM)g_menuFuncs[0]._cmdID, (LPARAM)&tbiIcons);
        
        tbiIcons.hToolbarBmp = g_iconsOfToolbar[1];
        tbiIcons.hToolbarIcon = NULL;
        ::SendMessage(g_nppData._nppHandle, NPPM_ADDTOOLBARICON, (WPARAM)g_menuFuncs[1]._cmdID, (LPARAM)&tbiIcons);
        
        tbiIcons.hToolbarBmp = g_iconsOfToolbar[2];
        tbiIcons.hToolbarIcon = NULL;
        ::SendMessage(g_nppData._nppHandle, NPPM_ADDTOOLBARICON, (WPARAM)g_menuFuncs[2]._cmdID, (LPARAM)&tbiIcons);
        
        tbiIcons.hToolbarBmp = g_iconsOfToolbar[3];
        tbiIcons.hToolbarIcon = NULL;
        ::SendMessage(g_nppData._nppHandle, NPPM_ADDTOOLBARICON, (WPARAM)g_menuFuncs[3]._cmdID, (LPARAM)&tbiIcons);
    }
    return;

    default:
        return;
    } //switch
}

/**
 * notepad++ cb
 */
LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam) {
    return TRUE;
}

/**
 * notepad++ cb
 */
#ifdef UNICODE
BOOL isUnicode(){
    return TRUE;
}
#endif //UNICODE

/**
 * Initialize your plugin data here
 * It will be called while plugin loading
 */
void pluginInit(HINSTANCE hModule){
    g_iconsOfToolbar[0] = (HBITMAP)::LoadImage(g_hInstance, MAKEINTRESOURCE(IDB_AP_LOG), IMAGE_BITMAP, 16,16, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
    g_iconsOfToolbar[1] = (HBITMAP)::LoadImage(g_hInstance, MAKEINTRESOURCE(IDB_RIL_LOG), IMAGE_BITMAP, 16,16, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
    g_iconsOfToolbar[2] = (HBITMAP)::LoadImage(g_hInstance, MAKEINTRESOURCE(IDB_CMD), IMAGE_BITMAP, 16,16, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
    g_iconsOfToolbar[3] = (HBITMAP)::LoadImage(g_hInstance, MAKEINTRESOURCE(IDB_CAPTURE), IMAGE_BITMAP, 16,16, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);

    Window::SetDefaultInstance(hModule);
    FilerPanel::RegisterClass();
	ShellPanel::RegisterClass();
}

/**
 * Here you can do the clean up, save the parameters (if any) for the next session
 */
void pluginCleanup(){
    for ( int i = 0; i < 4; i++ ) {
        DeleteObject(g_iconsOfToolbar[i]);
    }
}

/**
 * This function help you to initialize your plugin commands
 */
bool menuAddItem(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit)
{
    if (index >= MAX_MENU_ITEMS || !pFunc ) {
        return false;
    }

    lstrcpy(g_menuFuncs[index]._itemName, cmdName);
    g_menuFuncs[index]._pFunc = pFunc;
    g_menuFuncs[index]._init2Check = check0nInit;
    g_menuFuncs[index]._pShKey = sk;

    return true;
}

void onMenuAbout() {
    ::MessageBox(nullptr, L"Simbaba(ÐÀ°Ö) 2015.1.10 @ ÄÏ¾©", MSGBOX_TITLE, MB_OK);
}

void onShellMode() {
	m_shellPanel->Show(true);
}

void onMenuSettings() {
	m_settingsDialog.Create(g_nppData._nppHandle, g_hInstance, &g_pluginSettings);
}

void onMenuUpdate() {
    ::ShellExecute(NULL, TEXT("open"), TEXT("https://sourceforge.net/p/androidlogger"), NULL, NULL, SW_SHOWNORMAL);
}

/**
 * main & system
 * logcat -v time TAG:E *:S
 */
void onMenuLogApp() {
	m_shellPanel->Show(false);

	std::string cmd = "logcat -v time ";

	if (IsValidLevel(g_pluginSettings.logcatLevel))
	{
		char *tag = nullptr;

		if ( g_pluginSettings.asTag && g_pluginSettings.logcatTag[0] != L'\0') {
			tag = SU::TCharToUtf8(g_pluginSettings.logcatTag);
			cmd += tag;
		} else {
			cmd += "*";
		}

		cmd += ":";

		char *level = SU::TCharToUtf8(g_pluginSettings.logcatLevel);
		cmd += level;

		if (tag != nullptr && tag[0] != '*') {
			cmd += " *:S";
		}

		freeif(tag);
		delete level;
	}

    if (shellCmd(cmd)) {
        ::SendMessage(g_nppData._nppHandle, NPPM_SETMENUITEMCHECK, (WPARAM)g_menuFuncs[0]._cmdID, (LPARAM)true);
    }
}

/**
 * radio
 */
void onMenuLogRadio() {
	m_shellPanel->Show(false);

	std::string cmd = "logcat -b radio -v time ";

	if (IsValidLevel(g_pluginSettings.logcatLevel)) {
		char *level = SU::TCharToUtf8(g_pluginSettings.logcatLevel);
		cmd += "*:";
		cmd += level;
		delete level;
	}

    if (shellCmd(cmd)){
        ::SendMessage(g_nppData._nppHandle, NPPM_SETMENUITEMCHECK, (WPARAM)g_menuFuncs[1]._cmdID, (LPARAM)true);
    }
}

void onMenuAdbCmd() {
	m_shellPanel->Show(false);

    std::vector<std::string> cmds;
    getCurAllAdbCmds(cmds);

    if ( cmds.size() <= 0 ) {
        return;
    }

    if (shellCmd(cmds)){
        ::SendMessage(g_nppData._nppHandle, NPPM_SETMENUITEMCHECK, (WPARAM)g_menuFuncs[2]._cmdID, (LPARAM)true);
    }
}

void onMenuCapture() {
    gAndroidDevice.capture();
}

void onMenuFiler() {
    m_filerPanel->Show(true);
}

void onMenuMonkeyRunner() {
    std::vector<std::string> cmds;
    getCurMonkeyCmds(cmds);

    if ( cmds.size() <= 0 ) {
        return;
    }
    gAndroidDevice.monkey(cmds);
}

/**
 * Initialization of your plugin commands
 * You should fill your plugins commands here
 */
void menuInit()
{
	// 0-3 shouldn't change order, according toolbar icon
    menuAddItem(0, L"App Log", onMenuLogApp, nullptr, false);
    menuAddItem(1, L"Radio Log", onMenuLogRadio, nullptr, false);
    menuAddItem(2, L"Android Shell", onMenuAdbCmd, nullptr, false);
    menuAddItem(3, L"Capture", onMenuCapture, nullptr, false);

    menuAddItem(4, L"Filer", onMenuFiler, nullptr, false);
    menuAddItem(5, L"Settings", onMenuSettings, nullptr, false);
	menuAddItem(6, L"Shell Mode", onShellMode, nullptr, false);
	menuAddItem(7, L"About", onMenuAbout, nullptr, false);
    menuAddItem(8, L"Update", onMenuUpdate, nullptr, false);
    menuAddItem(9, L"MonkeyRunner(beta)", onMenuMonkeyRunner, nullptr, false);
}

/////////////////// Util functions ////////////////////////////
void hideFoldMargin(int bufferID, int langType){
    if ( g_loggLexerID && g_loggLexerID == langType ) {
        ::SendMessage(g_nppData._scintillaMainHandle, SCI_SETMARGINWIDTHN, 2, 0);
        ::SendMessage(g_nppData._scintillaSecondHandle, SCI_SETMARGINWIDTHN, 2, 0);
        return;
    }

    TCHAR name[20] = {0};
    ::SendMessage(g_nppData._nppHandle, NPPM_GETLANGUAGENAME, langType, (LPARAM)name);

    if (lstrcmp(name, PLUGIN_NAME_UNICODE) == 0) {
        ::SendMessage(g_nppData._scintillaMainHandle, SCI_SETMARGINWIDTHN, 2, 0);
        ::SendMessage(g_nppData._scintillaSecondHandle, SCI_SETMARGINWIDTHN, 2, 0);
        g_loggLexerID = langType;
    }
}

int findMyLexerMenu() {
    TCHAR buffer[65] = {0};
    int i = 0;

    do {
        GetMenuString(g_mainMenuHandle, 46065 + i, buffer, 64, MF_BYCOMMAND);
        if ( lstrcmp(PLUGIN_NAME_UNICODE, buffer) == 0 ) {
            break;
        }
        i++;
    } while ( i < 14 );

    if ( i == 14 ) {
        return -1;
    }

    return i;
}

bool logFilter(std::string &curLine) {
	if (curLine.size() == 0) {
		return true;
	}
	
	int size = g_keyWords.size();

	if (size == 0) {
		return true;
	}

	if (g_keyWords[0].size() == 0) {
		return true;
	}

	std::string line = curLine; 
	for(int i = 0; i < size; i++) {
		std::transform(line.begin(), line.end(), line.begin(), tolower);
		size_t pos = line.find(g_keyWords[i]);

		if ( pos == std::string::npos) {
			continue;
		}

		if (g_pluginSettings.wholeWord == 0) {
			return true;
		}

		bool heading = false;
		bool ending = false;
		size_t len = g_keyWords[i].size();

		if (pos == 0 || isalpha(line[pos-1]) == 0) {
			heading = true;
		}

		if ( (pos + len) == size || isalpha(line[pos+len]) == 0) {
			ending = true;
		}

		if (heading && ending) {
			return true;
		}
	}

	return false;
}

bool appendLogToNpp(std::string &log) {
    if (log.empty()){
        return true;
    }

    int bufID = ::SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
    if (g_curBufferID != bufID) {
        return false;
    }

    // Get the current scintilla
    int which = -1;
    ::SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1) {
        return false;
    }
    HWND curScintilla = (which == 0)?g_nppData._scintillaMainHandle:g_nppData._scintillaSecondHandle;

    ::SendMessage(curScintilla, SCI_APPENDTEXT, log.length(), (LPARAM)log.c_str());
    ::PostMessage(curScintilla, SCI_SCROLLTOEND, 0, 0);
    return true;
}

void tryApplyLexer() {
    int myLexerMenuID = findMyLexerMenu();
    if ( myLexerMenuID >=0 ) {
        ::SendMessage(g_nppData._nppHandle, NPPM_MENUCOMMAND, 0, 46065 + myLexerMenuID );
    }

    ::SendMessage(g_nppData._scintillaMainHandle, SCI_SETMARGINWIDTHN, 2, 0);
    ::SendMessage(g_nppData._scintillaSecondHandle, SCI_SETMARGINWIDTHN, 2, 0);
}

/**
 * Get all shell cmds line by line, until invalid line
 * Shell cmds should started with '>', '#', or empty line
 */
int getCurAllAdbCmds(std::vector<std::string> &cmds) {
    int bufID = ::SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
    int which = ::SendMessage(g_nppData._nppHandle, NPPM_GETPOSFROMBUFFERID, bufID, 0);
    int lines = 0;

    which >>= 30;
    if (which==0){
        lines = ::SendMessage(g_nppData._scintillaMainHandle, SCI_GETLINECOUNT, 0, 0);
    } else if (which==1){
        lines = ::SendMessage(g_nppData._scintillaSecondHandle, SCI_GETLINECOUNT, 0, 0);
    } else {
        return 0;
    }

    HWND curScintilla = (which == 0)?g_nppData._scintillaMainHandle:g_nppData._scintillaSecondHandle;

    for ( int i = 0; i < lines; i++ ) {
        char line[256] = {0};
        ::SendMessage(curScintilla, SCI_GETLINE, i, (LPARAM)line);

        if (line[0] == '>') {
            int len = strlen(line) - 1;
            while(line[len] == '\r' || line[len] == '\n' ) {
                line[len] = '\0';
                len--;
            }

            if ( strstr(line, "logcat") ) {
                tryApplyLexer();
            }
            cmds.push_back(&line[1]);
        } else if (line[0] == '#') {
            continue;
        } else if (line[0] == '\r' || line[0] == '\n') {
            continue;
        } else {
            break;
        }
    }

    return cmds.size();
}

int getCurMonkeyCmds(std::vector<std::string> &cmds) {
    int bufID = ::SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
    int which = ::SendMessage(g_nppData._nppHandle, NPPM_GETPOSFROMBUFFERID, bufID, 0);
    int lines = 0;

    which >>= 30;
    if (which==0){
        lines = ::SendMessage(g_nppData._scintillaMainHandle, SCI_GETLINECOUNT, 0, 0);
    } else if (which==1){
        lines = ::SendMessage(g_nppData._scintillaSecondHandle, SCI_GETLINECOUNT, 0, 0);
    } else {
        return 0;
    }

    HWND curScintilla = (which == 0)?g_nppData._scintillaMainHandle:g_nppData._scintillaSecondHandle;

    for ( int i = 0; i < lines; i++ ) {
        char line[256] = {0};
        ::SendMessage(curScintilla, SCI_GETLINE, i, (LPARAM)line);

        std::string cmd(line);

        if(SU::strStartWith(cmd, "PRESS") || SU::strStartWith(cmd, "TOUCH") 
            || SU::strStartWith(cmd, "TYPE") || SU::strStartWith(cmd, "WAIT")|| SU::strStartWith(cmd, "DRAG")) {
            cmds.push_back(line);
        }
    }

    return cmds.size();
}

void clearAllText() {
    // Get the current scintilla
    int which = -1;
    ::SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1) {
        return;
    }
    HWND curScintilla = (which == 0)?g_nppData._scintillaMainHandle:g_nppData._scintillaSecondHandle;

    ::SendMessage(curScintilla, SCI_CLEARALL, 0, 0);
}

void split(std::string& s, std::string& delim,std::vector<std::string> &ret){
	size_t last = 0;
	size_t index=s.find_first_of(delim, last);

	while (index != std::string::npos){
		std::string sub = s.substr(last, index-last);

		int i = 0;
		int size = sub.size();

		for(i = 0; i < size; i++) {
			if (isspace(sub[i]) == 0) {
				break;
			}
		}

		if (i != size) {
			std::transform(sub.begin(), sub.end(), sub.begin(), tolower);
			ret.push_back(sub);
		}

		last  = index+1;
		index = s.find_first_of(delim, last);
	}

	if (last != s.size() && index-last > 0) {
		std::string sub = s.substr(last, index-last);
		std::transform(sub.begin(), sub.end(), sub.begin(), tolower);
		ret.push_back(s.substr(last, index-last));
	}
}

void updateFilter(const TCHAR *tKeyWords) {
	g_keyWords.clear();
	char* keyWords = SU::TCharToUtf8(tKeyWords);
	std::string keys = keyWords;
	std::string delim = ",";
	split(keys, delim, g_keyWords);
	delete(keyWords);
}

void loadSettings(PluginSettings *settings) {
	TCHAR configDir[MAX_PATH] = {0};
	::SendMessage(g_nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, (WPARAM)MAX_PATH, (LPARAM)configDir);
	
	tstring configFile(configDir);
	configFile += L"\\AndroidLogger.ini";

	::GetPrivateProfileString(PLUGIN_NAME_UNICODE, L"WorkingDirectory", L"D:\\AndroidLogger", settings->workingDirectory, MAX_PATH, configFile.c_str());
	::GetPrivateProfileString(PLUGIN_NAME_UNICODE, L"LogcatLevel", L"V", settings->logcatLevel, 2, configFile.c_str());
	::GetPrivateProfileString(PLUGIN_NAME_UNICODE, L"LogcatKeywords", L"", settings->logcatTag, 64, configFile.c_str());

	settings->autoShowCapture = ::GetPrivateProfileInt(PLUGIN_NAME_UNICODE, L"AutoShowCapture", 1, configFile.c_str());
	settings->asTag = ::GetPrivateProfileInt(PLUGIN_NAME_UNICODE, L"AsTag", 0, configFile.c_str());
	settings->wholeWord = ::GetPrivateProfileInt(PLUGIN_NAME_UNICODE, L"WholeWord", 1, configFile.c_str());

	updateFilter(settings->logcatTag);

	::_tmkdir(settings->workingDirectory);
}

void saveSettings(PluginSettings *settings) {
	TCHAR configDir[MAX_PATH] = {0};
	::SendMessage(g_nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, (WPARAM)MAX_PATH, (LPARAM)configDir);
	
	tstring configFile(configDir);
	configFile += L"\\AndroidLogger.ini";

	::WritePrivateProfileString(PLUGIN_NAME_UNICODE,  L"WorkingDirectory", settings->workingDirectory, configFile.c_str());
	::WritePrivateProfileString(PLUGIN_NAME_UNICODE,  L"LogcatLevel", settings->logcatLevel, configFile.c_str());
	::WritePrivateProfileString(PLUGIN_NAME_UNICODE,  L"LogcatKeywords", settings->logcatTag, configFile.c_str());
	::WritePrivateProfileString(PLUGIN_NAME_UNICODE,  L"AutoShowCapture", settings->autoShowCapture?L"1":L"0", configFile.c_str());
	::WritePrivateProfileString(PLUGIN_NAME_UNICODE,  L"AsTag", settings->asTag?L"1":L"0", configFile.c_str());
	::WritePrivateProfileString(PLUGIN_NAME_UNICODE,  L"WholeWord", settings->wholeWord?L"1":L"0", configFile.c_str());

    ::_tmkdir(settings->workingDirectory);
	updateFilter(settings->logcatTag);
}

const TCHAR* getWorkingDirectory() {
	return g_pluginSettings.workingDirectory;
}

bool isAutoShowCapture() {
	return g_pluginSettings.autoShowCapture != 0;
}
