#include "AndroidDevice.h"
#include "Plugin.h"

bool AndroidCapture::capture() {
	if ( !connect() ) {
		return false;
	}

	std::string msg = formAdbRequest("framebuffer:");
	mSocket->SendLine(msg);
		
	if (checkResult(mSocket, ID_OKAY)) {
		startLoop();
		return true;
	}
	return true;
}

raw_image_t* getFrameBufferInfo(const BYTE* head)
{
    raw_image_t *raw = new raw_image;
    int *lhead = (int*)head;

    raw->version      = lhead[0];
    raw->bpp          = lhead[1];
    raw->size         = lhead[2];
    raw->width        = lhead[3];
    raw->height       = lhead[4];
    raw->red_offset   = lhead[5];
    raw->red_length   = lhead[6];
    raw->blue_offset  = lhead[7];
    raw->blue_length  = lhead[8];
    raw->green_offset = lhead[9];
    raw->green_length = lhead[10];
    raw->alpha_offset = lhead[11];
    raw->alpha_length = lhead[12];

    return raw;
}

bool AndroidCapture::onStart() {
	return true;
}

bool AndroidCapture::onLoop() {
	HBITMAP hbmpCapture = (HBITMAP)INVALID_HANDLE_VALUE;
	BYTE *head = nullptr;
	BYTE *rgbData = nullptr;
	raw_image_t *raw = nullptr;

	do {
		head = getBytes(52);

		// decode framebuffer header
		if ( head == nullptr ) {
			break;
		}

		raw = getFrameBufferInfo(head);
		delete head;

		if ( raw == nullptr ) {
			break;
		} else if ( raw->version < 0 ) {
			break;
		} else {
			raw->data = getBytes(raw->size);
		}

		stop();
	
		if ( raw->data == nullptr) {
			break;
		}

		rgbData = Convert2RGB888(raw);
		
		BITMAPINFOHEADER bih;
		ConstructBih(raw->width, -raw->height, bih);

		BITMAPINFO bi;
		bi.bmiHeader = bih;

		hbmpCapture = ::CreateDIBitmap(::GetDC(g_nppData._nppHandle), &bih, CBM_INIT, rgbData, &bi, DIB_RGB_COLORS);

		tstring capture = getWorkingDirectory();
		capture += L"\\device.bmp";

		SaveDIB2Bmp(rgbData, raw->width, raw->height, capture.c_str());

		if (isAutoShowCapture()) {
			::ShellExecute(NULL, L"open", capture.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
	} while(0);

	DeleteObject(hbmpCapture);

	freeif(raw->data);
	freeif(raw);
	freeif(rgbData);
	
	return false;
}

void AndroidCapture::onExit() {
    // check off menu
	release();
}

AndroidCapture::AndroidCapture():DeviceBase(){
}

AndroidCapture::~AndroidCapture() {
}