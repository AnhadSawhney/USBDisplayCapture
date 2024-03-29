// USBDisplayCapture.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <assert.h>
#include "libusb.h"

#define BITS_PER_PIXEL 24
#define BITS_PER_CHANNEL 8
#define WIDTH 128
#define HEIGHT 64

#define CAPTUREX 300
#define CAPTUREY 300

struct RGB {
	uint8_t R;
	uint8_t G;
	uint8_t B;
};

typedef struct RGB RGB_t;

#define PIXEL(f, x, y) f[y * WIDTH + x]

BITMAPINFOHEADER bi;

using namespace std;

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

void printdev(libusb_device* dev) {
		libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
				cout << "failed to get device descriptor" << endl;
				return;
		}
		cout << "Number of possible configurations: " << (int)desc.bNumConfigurations << "  ";
		cout << "Device Class: " << (int)desc.bDeviceClass << "  ";
		cout << "VendorID: " << hex << desc.idVendor << dec << "  ";
		cout << "ProductID: " << hex << desc.idProduct << dec << endl;
		libusb_config_descriptor * config;
		libusb_get_config_descriptor(dev, 0, &config);
		cout << "Interfaces: " << (int)config->bNumInterfaces << endl;
		const libusb_interface * inter;
		const libusb_interface_descriptor * interdesc;
		const libusb_endpoint_descriptor * epdesc;
		for (int i = 0; i < (int)config->bNumInterfaces; i++) {
				inter = &config->interface[i];
				cout << "Number of alternate settings: " << inter->num_altsetting << " | ";
				for (int j = 0; j < inter->num_altsetting; j++) {
						interdesc = &inter->altsetting[j];
						cout << "Interface Number: " << (int)interdesc->bInterfaceNumber << " | ";
						cout << "Number of endpoints: " << (int)interdesc->bNumEndpoints << " | ";
						for (int k = 0; k < (int)interdesc->bNumEndpoints; k++) {
								epdesc = &interdesc->endpoint[k];
								cout << "Descriptor Type: " << (int)epdesc->bDescriptorType << " | ";
								cout << "EP Address: " << (int)epdesc->bEndpointAddress << " | ";
						}
				}
				cout << endl;
		}
		cout << endl << endl << endl;
		libusb_free_config_descriptor(config);
}

struct libusb_device_handle* usb_init(struct libusb_context* context) {
	struct libusb_device** device_list;
	struct libusb_device_handle* handle = NULL;

	int deviceCount = libusb_get_device_list(context, &device_list);

	int i;
	for (i = 0; i < deviceCount; i++) {
		struct libusb_device* device = device_list[i];
		struct libusb_device_descriptor desc;
		libusb_get_device_descriptor(device, &desc);
		cout << "Vendor:Device = " << hex << desc.idVendor << " " << desc.idProduct << dec << endl;
		if (desc.idVendor == 0x0483 && desc.idProduct == 0x5740) {
			cout << libusb_open(device, &handle) << endl;
			break;
		}
	}

	libusb_free_device_list(device_list, 1);

	return handle;
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

	//RGB_t* R = (RGB_t*)calloc(WIDTH * HEIGHT, sizeof(RGB_t));

	RGB_t arr[WIDTH * HEIGHT];
	unsigned char out[WIDTH * HEIGHT * 3];

	libusb_device* dev;
	libusb_device_handle* handle;
	libusb_context* context = NULL;
	libusb_init(&context); //initialize a library session
	libusb_set_debug(context, 3); //set verbosity level to 3, as suggested in the documentation'

	//handle = libusb_open_device_with_vid_pid(NULL, 0x0483, 0x5740);
	handle = usb_init(context);

	cout << handle << " " << endl;
	assert(handle != NULL);
	dev = libusb_get_device(handle);
	printdev(dev);
	cout << libusb_claim_interface(handle, 0) << endl;
	
	unsigned char request[] = "Test", response[8];
	memset(response, 0, sizeof(response));

	int ret, bytes_transferred;

	while (1) {
		GetArray(arr);

		for (uint8_t color = 0; color < 3; color++) {
			uint32_t offset = HEIGHT * WIDTH * color;
			uint32_t i = 0;
			for (uint32_t y = 0; y < HEIGHT; y++) {
				for (int x = WIDTH - 1; x >= 0; x--) {
					if (color == 0) out[offset + i] = PIXEL(arr, x, y).R;
					if (color == 1) out[offset + i] = PIXEL(arr, x, y).G;
					if (color == 2) out[offset + i] = PIXEL(arr, x, y).B;
					i++;
				}
			}
		}

		// output array is 3 full frames sequentially, for each color channel

		ret = libusb_bulk_transfer(handle, 0x01, out, WIDTH*HEIGHT*3, &bytes_transferred, 0); //libusb will automatically divide into packets
		switch (ret) {
			case 0:
				printf("%d bytes sent\n", bytes_transferred);
				break;
			case LIBUSB_ERROR_TIMEOUT:
				printf("ERROR in bulk write: %d Timeout\n", ret);
				break;
			case LIBUSB_ERROR_PIPE:
				printf("ERROR in bulk write: %d Pipe\n", ret);
				break;
			case LIBUSB_ERROR_OVERFLOW:
				printf("ERROR in bulk write: %d Overflow\n", ret);
				break;
			case LIBUSB_ERROR_NO_DEVICE:
				printf("ERROR in bulk write: %d No Device\n", ret);
				break;
			default:
				printf("ERROR in bulk write: %d Unknown\n", ret);
				break;
		}
		//libusb_bulk_transfer(handle, 0x82, response, sizeof(response), &bytes_transferred, 0);

		drawArray(arr);
	}

	libusb_release_interface(handle, 0);
	libusb_close(handle);
	libusb_exit(NULL); //close the session
}