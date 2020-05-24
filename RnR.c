
#include <stdio.h>
#include "RnR-API.h"
#include "hidapi.h"
#include <string.h>

void PrintBin(char s);
void PrintSignal(char s);

#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <Windows.h>
#define sleep Sleep
#else
#include <unistd.h>
#define sleep(x) usleep(x*1000)
#endif
#include "hidapi.h"

#define MAX_STR 255
#define MAXDEVICES 30

hid_device * HIDs[MAXDEVICES];
void AutoassignIDs();

// Duplicate hid_open
int get_hid_devices(unsigned short vendor_id, unsigned short product_id, hid_device** hids)
{
	// TODO: Merge this functions with the Linux version. This function should be platform independent.
	struct hid_device_info *devs, *cur_dev;
	const char *path_to_open = NULL;
	hid_device *handle = NULL;
	int i = 0;
	
	devs = hid_enumerate(vendor_id, product_id);
	cur_dev = devs;
	while (cur_dev) 
	{
		if (cur_dev->vendor_id == vendor_id && cur_dev->product_id == product_id) 
		{
			/* Open the device */		
			path_to_open = cur_dev->path;
			printf("device path = %s\n",path_to_open);
			if (path_to_open) 
			{
				hids[i++] = hid_open_path(path_to_open);
			}
		}
		cur_dev = cur_dev->next;
	}

	hid_free_enumeration(devs);
	
	return i;
}

void print_device_info(hid_device *handle)
{
	wchar_t wstr[MAX_STR];
	int res = 0;
//		hid_set_nonblocking(handle, 0);
	// Read the Manufacturer String
	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
	wprintf(L"Manufacturer String: %s\n", wstr);

		// Read the Product String
	res = hid_get_product_string(handle, wstr, MAX_STR);
	wprintf(L"Product String: %s\n", wstr);

	// Read the Serial Number String
	res = hid_get_serial_number_string(handle, wstr, MAX_STR);
	wprintf(L"Serial Number String: (%d) %s\n", wstr[0], wstr);

	// Read Indexed String 1
	res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
	wprintf(L"Indexed String 1: %s\n", wstr);
}

void scan_hid_devices(int vid, int pid)
{
	// TODO: Merge this functions with the Linux version. This function should be platform independent.
	struct hid_device_info *devs, *cur_dev;
	
	devs = hid_enumerate(vid, pid);
	cur_dev = devs;
	while (cur_dev) 
	{
		
		printf("- - - - - - - - - - - - - \nDevice path: %s\n", cur_dev->path);
		printf("Device VID/PID: %04x:%04x\n", cur_dev->vendor_id, cur_dev->product_id);
		if ( cur_dev->serial_number )
			wprintf(L"Serial Number String: (%d) %s\n", cur_dev->serial_number[0], cur_dev->serial_number);
		printf("Device Version: %d\n", cur_dev->release_number);
		wprintf(L"Manufacturer String: %s\n", cur_dev->manufacturer_string);
		wprintf(L"Product String: %s\n", cur_dev->product_string);
		printf("Interface Number: %d\n", cur_dev->interface_number);
			
//			unsigned short usage_page; /** Usage Page for this Device/Interface (Windows/Mac only). */
//			unsigned short usage; /** Usage for this Device/Interface (Windows/Mac only).*/

	// Read Indexed String 1
//	res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
//	wprintf(L"Indexed String 1: %s\n", wstr);
		cur_dev = cur_dev->next;
	}
	printf("- - - - - - - - - - - - - \n");
	hid_free_enumeration(devs);
}


void print_help()
{
	printf("\n 4x4 R'n'R USB device utility\n\n");
//	printf("    -id for re-counting all devices and pitting new id's\n");
	printf("    -help - print this help\n");
	printf("    -led n  - write n value to LED register\n");
	printf("    -relay n  - write n value to Realy register ( OUT ) \n");
	printf("    -wdt n  - write n value to WDT register \n");
	printf("    -status - displays internal RnR registers \n");
	printf("    -mode n select LED display mode \n");
	printf("      where n:\n");
	printf("      0 - LED indicator displays LED (see bellow) value\n");
	printf("      1 - indicates Relays states\n");
	printf("      2 - indicates Rays states\n");
	printf("      3 - indicates activity on R'n'R pins and on USB interface\n");
	printf("      4 - indicates internal state\n");
	printf("      7 - rolling LED\n");
	printf("      8..255 - error\n");
	printf("    -init set all devices into factory defaults\n");
	printf("    -scan list all attached USB HID devices\n");
	printf("    -list list all attached RnR devices\n");
	printf("    -stress - test cable quality\n");
	printf("    -test (requires 2 devices)\n\n");
	fflush(stdout);
}

int crosstest()
{
	RnRUSBBuff wr0;
	RnRUSBBuff wr1;
	RnRUSBBuff rd0;
	RnRUSBBuff rd1;
	int i;

	// Test vectors
	const static char t0[] = {0,1,2,4,8,7,11,13,14,15,0x5,0xa};
	const static char t1[] = {0,8,4,2,1,14,13,11,7,15,0xa,0x5};

	int err = 0;
	int devs = 0;
	printf("\n");

	memset(&wr0,0, sizeof(wr0));
	memset(&wr1,0, sizeof(wr1));

	memset(&rd0,0, sizeof(rd0));
	memset(&rd1,0, sizeof(rd1));

	devs = get_hid_devices(0x16D0, 0x8080, HIDs);
	if(devs < 2)
	{
		printf("ERROR: insufficient number of devices has been connected %d!\n", devs);
		scan_hid_devices(0x16D0, 0x8080);
		return -1;
	}

	for( i=0; i<sizeof(t0); i++)
	{
		int ret;
		int localerror = 0;
		int num = 0;

		printf("case(%2d): ", i);
				
//		num = hid_read(HIDs[0], ((unsigned char *) & rd0)+1, sizeof(RnRUSBBuff)-1);
		wr0.Data = t0[i] & 0xf;
		wr0.UpdateFlag = 1;
		ret = hid_write(HIDs[0], (unsigned char *) & wr0, sizeof(RnRUSBBuff));	

//		num = hid_read(HIDs[1], ((unsigned char *) & rd1)+1, sizeof(RnRUSBBuff)-1);
		wr1.Data = t1[i] & 0xf;
		wr1.UpdateFlag = 1;
		ret = hid_write(HIDs[1], (unsigned char *) & wr1, sizeof(RnRUSBBuff));	
				
		sleep(1000);

		PrintSignal(t1[i]); printf(" -> ");

		num = hid_read(HIDs[0], ((unsigned char *) & rd0)+1, sizeof(RnRUSBBuff)-1);

		if (num <= 0) 
		{
			printf("********");
			localerror ++;
		}
		else
		{
			PrintSignal(rd0.Data); 
			if(t1[i] != rd0.Data)
			{
				localerror ++;
				err++;
			}
		}

		printf(" : ");
		PrintSignal(t0[i]); printf(" -> ");

		num = hid_read(HIDs[1], ((unsigned char *) & rd1)+1, sizeof(RnRUSBBuff)-1);
		if (num <= 0) 
		{
			printf("********");				
			localerror ++;
		}
		else
		{
			PrintSignal(rd1.Data); 
			if(t0[i] != rd1.Data)
			{
				localerror ++;
				err++;
			}
		}

		if ( localerror ) printf(" <--- ERROR");
		printf("\n");
	}

	if(err == 0)
		printf("\n SUCCESS !\n");
	else
		printf("\n FALURE --> %d ERROR(s)!\n\n", err);

	return 0;
}

int RnR(int argc, char* argv[])
{
	hid_device *handle;
	int dev_count = 0;
	int devs = 0;
	int i;

	RnRUSBBuff read_buf;
	RnRUSBBuff write_buf;
	// Initialize the hidapi library
	
	if(argc > 1 && 0 == strcmp( "-help", argv[1] ) )
	{
		print_help();
		return 0;
	}

	if(argc > 1 && 0 == strcmp( "-scan", argv[1] ) )
	{
		scan_hid_devices(0,0);
		return 0;
	}

	if(argc > 1 && 0 == strcmp( "-list", argv[1] ) )
	{
		scan_hid_devices(0x16D0, 0x8080);
		return 0;
	}
	
	if(argc > 1 && 0 == strcmp( "-test", argv[1] ) )
	{
		return crosstest();
	}

	if(argc > 1 && 0 == strcmp( "-stress", argv[1] ) )
	{
		printf(" STRESS test is not implemented yet\n");
		return 0;
	}

	if(argc > 1 && 0 == strcmp( "-id", argv[1] ))
	{
		AutoassignIDs();
		return 0;
	}

	devs = get_hid_devices(0x16D0, 0x8080, HIDs);

	for ( i = 0; i< devs; i++)
	{
		// Open the device using the VID, PID,
		// and optionally the Serial number.
		handle = HIDs[i];
		print_device_info(handle);

		if(argc > 2 && 0 == strcmp( "-led", argv[1] ) )
		{
			printf("\nSetting LEDs to 0x%0x \n",  atoi(argv[2]));
			write_buf.LED = atoi(argv[2]);
//			write_buf.ID = i;
			write_buf.UpdateFlag = 2;
			hid_write(handle, (unsigned char *) & write_buf, sizeof(RnRUSBBuff));
			continue;
		}

		if(argc > 2 && 0 == strcmp( "-relay", argv[1] ) )
		{
			printf("\nSetting relays to to 0x%0x \n",  atoi(argv[2]));
			write_buf.Data = atoi(argv[2]);
			write_buf.UpdateFlag = 1;
			hid_write(handle, (unsigned char *) & write_buf, sizeof(RnRUSBBuff));
			continue;
		}

		if(argc > 2 && 0 == strcmp( "-mode", argv[1] ))
		{
			printf("\nSetting Display Mode to %d \n",atoi(argv[2]));
			write_buf.DisplayMode = atoi(argv[2]);
			write_buf.UpdateFlag = 4;
			hid_write(handle, (unsigned char *) & write_buf, sizeof(RnRUSBBuff));
			continue;
		}

		if(argc > 2 && 0 == strcmp( "-wdt", argv[1] ))
		{
			printf("\nSetting Display Mode to %d \n",atoi(argv[2]));
			write_buf.WDTime = atoi(argv[2]);
			write_buf.UpdateFlag = 8;
			hid_write(handle, (unsigned char *) & write_buf, sizeof(RnRUSBBuff));
			continue;
		}

		if(argc > 1 && 0 == strcmp( "-status", argv[1] ))
		{
			int res = 0;
			printf("\nStatus of device\n" );
			res = hid_read(handle, ((unsigned char *) & read_buf)+1, sizeof(RnRUSBBuff)-1);
			if (res <= 0)
			{
				printf("Reading Error\n");
				break;
			}
			printf("ID: %02u:\nRays:   ", read_buf.ID);
			PrintSignal( read_buf.Data );
			printf("\nRelays: ");
			PrintSignal( read_buf.Data2 );
			printf("\nLED:    ");
			PrintSignal( read_buf.LED );
			printf("\nLED Mode: %02u: ", read_buf.DisplayMode );
			printf("\nWDT: %08u ", read_buf.WDTime);
			printf("\nWDTLeft: %08u ", read_buf.WDTimeLeft);
			printf("\nUptime: %08u \n", read_buf.Uptime);
			continue;
		}
		if(argc > 1 && 0 == strcmp( "-init", argv[1] ))
		{
			printf("Setting facroty defaults\n");
			write_buf.DisplayMode = 0;
			write_buf.LED = 0;
			write_buf.Data = 0;
			write_buf.WDTime = 0;
			write_buf.UpdateFlag = 15;
			hid_write(handle, (unsigned char *) & write_buf, sizeof(RnRUSBBuff));
			continue;
		}

		if(argc > 1)
		{
			printf("INVALID ARGUMENTS!\n");
			print_help();
			break;
		}
	}
	return 0;
}

void AutoassignIDs()
{
	int dev_count = 0;
	int devs = 0;
	int i;

	RnRUSBBuff write_buf;

	devs = get_hid_devices(0x16D0, 0x8080, HIDs);

	for ( i = 0; i< devs; i++)
	{
		switch(i)
		{
			case 0:
				write_buf.LED = 0x10;
				break;
			case 1:
				write_buf.LED = 0x30;
				break;
			case 2:
				write_buf.LED = 0x70;
				break;
			case 3:
				write_buf.LED = 0xf0;
				break;
			default:
				write_buf.LED = '\x50';
				break;
		}
		write_buf.ID = i;
		write_buf.UpdateFlag = 2;
		hid_write(HIDs[i], (unsigned char *) & write_buf, sizeof(RnRUSBBuff));
	}
}


void Listen()
{
	int dev_count = 0;
	int devs = 0;
	int i;

	RnRUSBBuff read_buf;

	devs = get_hid_devices(0x16D0, 0x8080, HIDs);

	for ( i = 0; i< devs; i++)
	{
		printf("Device HID[%d] = %x\n",i,HIDs[i]);
	}
	while (1)
	{
		for ( i = 0; i< devs; i++)
		{
			int res = 0;
			res = hid_read(HIDs[i], ((unsigned char *) & read_buf), sizeof(RnRUSBBuff)-1);
			if (res < 0) break;
			if(res > 0)
			{
				printf("%02u: ", read_buf.ID);
				PrintSignal( read_buf.Data );
				printf(" %08x ", read_buf.Uptime);
			}
//			write_buf.UpdateFlag = 1;
//			hid_write(handle, (unsigned char *) & write_buf, sizeof(CUSBBuff));

//			// switch led 1 on
//			buf.Data = ++led[i] & 0xf;
//		buf.UpdateFlag = 1;
			sleep(1);
		}
		printf("\r");
	}
}

int main(int argc, char* argv[])
{
	int res;

	if ( argc < 2)
	{
		print_help();
		Listen();
	}

	res = hid_init();

	RnR( argc, argv);
	
	res = hid_exit(); // Finalize the hidapi library

	return 0;
}


void PrintBin(char s)
{
	int i;
	for(i=7; i>=0; i--)
		printf("%d", (s >> i) & 1);
}

void PrintSignal(char s)
{
	int i;
	for( i=7; i>=0; i--)
	{
		if ( (s >> i) & 1 )
		{
			printf("-");
		}
		else
		{
			printf("_");
		}
	}
}
