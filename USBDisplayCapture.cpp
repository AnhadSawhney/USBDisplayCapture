// USBDisplayCapture.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>

#define BITS_PER_PIXEL 24
#define BITS_PER_CHANNEL 8
#define WIDTH 64
#define HEIGHT 64

#define CAPTUREX 100
#define CAPTUREY 200

struct RGB {
	uint8_t R;
	uint8_t G;
	uint8_t B;
};

typedef struct RGB RGB_t;

#define PIXEL(f, x, y) f[y * WIDTH + x]

BITMAPINFOHEADER bi;

void drawArray(RGB_t arr[]) {
	COLORREF* cref = (COLORREF*)calloc(WIDTH * HEIGHT, sizeof(COLORREF));

	for (int j = 0; j < HEIGHT; j++)
		for (int i = 0; i < WIDTH; i++)
			cref[(HEIGHT - j - 1) * WIDTH + i] = RGB(PIXEL(arr, i, j).R, PIXEL(arr, i, j).G, PIXEL(arr, i, j).B);

	HDC hdc = GetDC(NULL);
	HDC src = CreateCompatibleDC(hdc); // hdc - Device context for window, I've got earlier with GetDC(hWnd) or GetDC(NULL);
	// Creating temp bitmap
	HBITMAP map = CreateBitmap(WIDTH, HEIGHT,
		1, // Color Planes, unfortanutelly don't know what is it actually. Let it be 1
		8 * 4, // Size of memory for one pixel in bits (in win32 4 bytes = 4*8 bits)
		(void*)cref); // pointer to array
	SelectObject(src, map); // Inserting picture into our temp HDC
	// Copy image from temp HDC to window
	BitBlt(hdc, // Destination
		10,  // x and
		10,  // y - upper-left corner of place, where we'd like to copy
		WIDTH, // width of the region
		HEIGHT, // height
		src, // source
		0,   // x and
		0,   // y of upper left corner  of part of the source, from where we'd like to copy
		SRCCOPY); // Defined DWORD to juct copy pixels. Watch more on msdn;

	//clean up
	free(cref);
	DeleteObject(map);
	DeleteDC(hdc);
	DeleteDC(src);
}

void GetArray(RGB_t arr[]) {
	//int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	//int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	HWND hDesktopWnd = GetDesktopWindow();
	HDC hDesktopDC = GetDC(hDesktopWnd);
	HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);
	HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDesktopDC,
		WIDTH, HEIGHT);
	SelectObject(hCaptureDC, hCaptureBitmap);
	BitBlt(hCaptureDC, 0, 0, WIDTH, HEIGHT, hDesktopDC, CAPTUREX, CAPTUREY, SRCCOPY | CAPTUREBLT);

	GetDIBits(hCaptureDC, hCaptureBitmap, 0, HEIGHT, arr, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

	ReleaseDC(hDesktopWnd, hDesktopDC);
	DeleteDC(hCaptureDC);
	DeleteObject(hCaptureBitmap);
}

int main() {
    CoInitialize(nullptr); // NULL if using older VC++

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = WIDTH;
	bi.biHeight = HEIGHT;
	bi.biPlanes = 1;
	bi.biBitCount = BITS_PER_PIXEL;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	RGB_t* R = (RGB_t*)calloc(WIDTH * HEIGHT, sizeof(RGB_t));

	//clear
	while (1) {
		
		//memset(R, 0, WIDTH* HEIGHT * sizeof(R[0]));

		/*for (int i = 0; i < WIDTH; i++) {
			for (int j = 0; j < i; j++) {
				R[i][j][0] = 255; //BGR
				R[i][j][1] = 0;
				R[i][j][2] = 0;
			}
		}*/
		GetArray(R);
		drawArray(R);
	}

    std::cout << "Hello World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
