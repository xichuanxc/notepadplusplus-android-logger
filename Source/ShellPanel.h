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
#ifndef SHELLPANLE_H
#define SHELLPANLE_H

#include "DockableWindow.h"

class ShellPanel : public DockableWindow {
public:

public:
							ShellPanel();
	virtual					~ShellPanel();

	virtual int				Create(HWND hParent, HWND hNpp, int MenuID, int MenuCommand);
	virtual int				Destroy();

	virtual int				Show(bool show);
	virtual int				OnSize(int newWidth, int newHeight);

	static int				RegisterClass();

	virtual LRESULT			MessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT         ShellProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	
protected:
	int GetShellCmd(std::string &cmd);
	static LRESULT CALLBACK scintillaStatic_Proc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
	bool CheckIfEndLine();

protected:
	HWND					m_hScintilla;
	static WNDPROC          m_scintillaDefaultProc;
	static const TCHAR * ShellPanelCLASS;
	
};

#endif