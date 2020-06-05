#include "StdInc.h"
#include "ShellPanel.h"
#include "Plugin.h"

const TCHAR * ShellPanel::ShellPanelCLASS = TEXT("AndroidLexerShellPanel");
WNDPROC ShellPanel::m_scintillaDefaultProc = nullptr;

ShellPanel::ShellPanel() :
	DockableWindow(ShellPanelCLASS),
	m_hScintilla(NULL) {
	m_style = 0;
	m_exStyle = 0;
}

ShellPanel::~ShellPanel() {
}

int ShellPanel::Create(HWND hParent, HWND hNpp, int MenuID, int MenuCommand) {
	SetTitle(TEXT("AndroidLogger"));
	SetInfo(TEXT("ShellPanel"));
	SetLocation(DWS_DF_CONT_BOTTOM);
	//HICON icon = ::LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_ICON_MESSAGES));
	//SetIcon(icon);

	int res = DockableWindow::Create(hParent, hNpp, MenuID, MenuCommand);
	if (res == -1) {
		return -1;
	}

	m_hScintilla = ::CreateWindowEx(WS_EX_CLIENTEDGE,
									TEXT("Scintilla"), TEXT(""),
									WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN,
									0, 0, 10, 10,
									m_hwnd, NULL, m_hInstance, NULL);

	m_scintillaDefaultProc = reinterpret_cast<WNDPROC>(
			::SetWindowLong(m_hScintilla, GWL_WNDPROC, reinterpret_cast<LONG>(scintillaStatic_Proc))
	);

	::SetWindowLongPtr(m_hScintilla, GWL_USERDATA, reinterpret_cast<LONG>(this));

	if (m_hScintilla == NULL) {
		DockableWindow::Destroy();
		return -1;
	}

	::SendMessage(m_hwnd, WM_SIZE, 0, 0);
	::ShowWindow(m_hScintilla, SW_SHOW);

	return 0;
}

int ShellPanel::Destroy() {
	DestroyWindow(m_hScintilla);
	return DockableWindow::Destroy();
}

int ShellPanel::Show(bool show) {
	if (show == true) {
		::SendMessage(m_hScintilla, SCI_SETTEXT, 1, (LPARAM)">");
		startShellMode();
	} else {
		stopShellMode();
	}
	return DockableWindow::Show(show);;
}

int ShellPanel::OnSize(int newWidth, int newHeight) {
	BOOL res = SetWindowPos(m_hScintilla, NULL, 0, 0, newWidth, newHeight, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOZORDER);
	if (res == FALSE) {
		return -1;
	}

	return 0;
}

int ShellPanel::RegisterClass() {
	WNDCLASSEX ShellPanelClass;
	ShellPanelClass.cbSize = sizeof(WNDCLASSEX);
	ShellPanelClass.style = CS_DBLCLKS;//|CS_NOCLOSE;
	ShellPanelClass.cbClsExtra = 0;
	ShellPanelClass.cbWndExtra = 0;
	ShellPanelClass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	ShellPanelClass.hCursor = LoadCursor(NULL,IDC_ARROW);
	ShellPanelClass.hbrBackground = (HBRUSH)COLOR_WINDOW+1;
	ShellPanelClass.lpszMenuName = NULL;
	ShellPanelClass.hIconSm = NULL;

	ShellPanelClass.lpfnWndProc = NULL;
	ShellPanelClass.hInstance = NULL;
	ShellPanelClass.lpszClassName = NULL;

	//register the class
	int ret = Window::RegisterClass(ShellPanelCLASS, ShellPanelClass);
	if (ret != 0) {
		return -1;
	}

	return 0;
}

LRESULT CALLBACK ShellPanel::scintillaStatic_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	ShellPanel *pShellPanel = (ShellPanel *)(::GetWindowLongPtr(hwnd, GWL_USERDATA));
	LRESULT ret = 1;

	ret = pShellPanel->ShellProc(uMsg, wParam, lParam);

	if (ret == 0) {
		return 0;
	}

	if (pShellPanel != nullptr) {
		return ::CallWindowProc(m_scintillaDefaultProc, hwnd, uMsg, wParam, lParam);
	}
	return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT ShellPanel::MessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	bool doDefaultProc = false;
	LRESULT result = 0;

	switch(uMsg) {
		case WM_SETFOCUS: {
			//Why restore focus here? This window should never be able to get focus in the first place
			HWND hPrev = (HWND)wParam;
			if (hPrev != NULL) {
				::SetFocus(hPrev);
			}
			break; }
		case WM_ERASEBKGND: {
			HDC hDC = (HDC) wParam;
			RECT rectClient;
			GetClientRect(m_hwnd, &rectClient);
			FillRect(hDC, &rectClient, ::GetSysColorBrush(COLOR_3DFACE));
			result = TRUE;
			break; }
		default:
			doDefaultProc = true;
			break;
	}

	if (doDefaultProc)
		result = DockableWindow::MessageProc(uMsg, wParam, lParam);

	return result;
}

LRESULT ShellPanel::ShellProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (wParam == VK_UP || wParam == VK_DOWN) {
		return 0;
	}

	if (wParam == VK_LEFT || wParam == VK_BACK) {
		int curPos = ::SendMessage(m_hScintilla, SCI_GETCURRENTPOS, 0, 0);
		int curCol = ::SendMessage(m_hScintilla, SCI_GETCOLUMN, curPos, 0);
		if (curCol == 1) {
			return 0;
		}
	}

	if (uMsg == WM_KEYUP && wParam == VK_RETURN) {
		if (!CheckIfEndLine()) {
			::SendMessage(m_hScintilla, SCI_SETSEL, -1, -1);
			return 0;
		}

		::SendMessage(m_hScintilla, SCI_APPENDTEXT, 1, (LPARAM)">");
		::SendMessage(m_hScintilla, SCI_SETSEL, -1, -1);

		std::string cmd;
		GetShellCmd(cmd);
		
		if (cmd.compare(0, 5, "clear") == 0 || cmd.compare(0, 3, "cls") == 0) {
			clearAllText();
			return 0;
		} else if (cmd.compare(0, 4, "exit") == 0) {
			stopShellMode();
			return 0;
		}else {
			appendShellCmd(cmd);
		}

		return 0; 
	}

	return 1;
}

int ShellPanel::GetShellCmd(std::string &cmd) {
    int lastLine = ::SendMessage(m_hScintilla, SCI_GETLINECOUNT, 0, 0) - 2;

	if (lastLine < 0) {
		lastLine = 0;
	}

	char line[256] = {0};
	int size = ::SendMessage(m_hScintilla, SCI_GETLINE, lastLine, (LPARAM)line);
	if (size == 0){
		return 0;
	}
	cmd.assign(line+1, size-3);
	return cmd.size();
}

bool ShellPanel::CheckIfEndLine() {
	int lineCount = ::SendMessage(m_hScintilla, SCI_GETLINECOUNT, 0, 0);
	int curPos = ::SendMessage(m_hScintilla, SCI_GETCURRENTPOS, 0, 0);
	int curLine = ::SendMessage(m_hScintilla, SCI_LINEFROMPOSITION, curPos, 0);

	if (curLine == (lineCount-1)) {
		return true;
	}
	return false;
}
