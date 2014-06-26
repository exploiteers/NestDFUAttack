/*  $Id:  Exp $
 *
 * omap3_usbload, an USB download application for OMAP3 processors
 * Copyright (C) 2008 Martin Mueller <martinmm@pfump.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 *
 */

/* 

  OMAP USB loader.  Using the version of x-loader with CONFIG_USB, you can
now boot up the BeagleBoard completely into Linux with a ramdisk via USB.

  Steps:

1. You will need libusb-dev on your host box:

sudo apt-get install libusb-dev

2. Modify u-boot to boot directly if it can't find mmc:

change "run nandboot;" to "run ramboot;"

and add:

"ramboot=setenv bootargs console=ttyO2,115200n8 mem=256M no_console_suspend ramdisk_size=80000 root=/dev/ram0; bootm\0"

 in this file u-boot/include/configs/omap3_beagle.h

3. Run minicom to the serial port, unplug USB OTG and brick power to
   the BeagleBoard, remove the SD Card and then run this program like this:

cd scripts
sudo omap3_usbload/omap3_usbload -f ../tftpboot/x-load.bin \
 -a 0x80008000 -f ../tftpboot/u-boot.bin \
 -a 0x82000000 -f ../tftpboot/uMulti -j 0x80008000 -v

  Then plug in the USB OTG cable to your host to power the board.

Usage: omap3_usbload -f file1 [-a addr2 -f file2] [-a addr3 -f file3] [etc] [-j addr] [-v]

 NOTE: first file (file1) has no addr
 Load address MUST come before each file2-N

  uMulti is a combined kernel/filesystem.  You can substitute
uImage for uMulti and it will at least boot into the kernel,
but you won't have a filesystem.

  Tested on BeagleBoardXM Rev C and Beagleboard Rev B4

  The Rev B4 Beagleboard saves u-boot environment var's in NAND so you will have to interrupt u-boot
and correct them.  Also on Rev B4 you will have to use an older kernel, this one won't work, then
interrupt u-boot and do:

setenv bootargs console=ttyS2,115200n8 mem=128M no_console_suspend ramdisk_size=80000 root=/dev/ram0; bootm 0x82000000

  You could load uImage at one address, then the ramdisk file at yet another address (at say 0x830000000) and do:

setenv bootargs console=ttyS2,115200n8 mem=128M no_console_suspend ramdisk_size=80000 root=/dev/ram0 initrd=0x830000000,8M; bootm 0x82000000


*/

#include <getopt.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <usb.h>

#define VERSION "1.1"

#if __BYTE_ORDER == __LITTLE_ENDIAN
# define cpu_to_le32(x)	(x)
# define le32_to_cpu(x)	(x)
#else
# define cpu_to_le32(x)	bswap_32 (x)
# define le32_to_cpu(x)	bswap_32 (x)
#endif

#define VENDOR_ID_TI       0x0451
#define DEVICE_ID          0xD000
#define DEVICE_ID_MASK     0xff00

#define OMAP3_BOOT_CMD     0xF0030002  /* see page 3515 sprugn4c.pdf */

#define PACK4(a,b,c,d)     (((d)<<24) | ((c)<<16) | ((b)<<8) | (a))
#define USBLOAD_CMD_FILE PACK4('U', 'S', 'B', 's')  /* send file size */
#define USBLOAD_CMD_JUMP PACK4('U', 'S', 'B', 'j')  /* go where I tell you */
#define USBLOAD_CMD_FILE_REQ PACK4('U', 'S', 'B', 'f')  /* file request */
#define USBLOAD_CMD_ECHO_SZ PACK4('U', 'S', 'B', 'n')  /* echo file size */
#define USBLOAD_CMD_REPORT_SZ PACK4('U', 'S', 'B', 'o')  /* report file size */
#define USBLOAD_CMD_MESSAGE PACK4('U', 'S', 'B', 'm')  /* message for debug */

#define EP_BULK_IN         0x81
#define EP_BULK_OUT        0x01
#define BUF_LEN             128
#define BUF_LEN_SM          256 /* we seem to need this exact size ?? */

#define USB_TIMEOUT 1000  /* time in ms */

#define MAX_FILES 10  /* the max addr/file pairs */
struct UPLOAD_DATA
	{
	int file_cntr;
	unsigned long addrs[MAX_FILES];
	int fd[MAX_FILES];
	int filesize[MAX_FILES];
	const char *filename[MAX_FILES];  /* file names */
	unsigned long jump_addr;
	usb_dev_handle *udev;
	int verbose;  /* be verbose? */
	char *p_file;
	};

struct UPLOAD_DATA upload_data =
	{
	.verbose = 0,  /* be verbose? */
	.filename[0] = NULL,
	.jump_addr = 0x80008000,
	.p_file = NULL,
	};


  /* A string listing valid short options letters.  */
const char* const short_options = "a:f:j:v";
  /* An array describing valid long options.  */
const struct option long_options[] = {
    { "load_addr",     1, NULL, 'a' },
    { "file",     1, NULL, 'f' },
    { "jump",      1, NULL, 'j' },
    { "verbose",   0, NULL, 'v' },
    { NULL,        0, NULL, 0   }   /* Required at end of array.  */
  };

/* send a value to x-loader */
static int send_val (struct UPLOAD_DATA *p_data, unsigned int code, unsigned int val1, unsigned int val2)
{
	int	res;
	int buffer[3];

	buffer[0]= code;
	buffer[1] = cpu_to_le32 (val1);
	buffer[2] = cpu_to_le32 (val2);

	if (p_data->verbose)
		printf ("send_val code = 0x%x, val1 = 0x%x, val2 = 0x%x\n",(int) buffer[0], (int) buffer[1], (int) buffer[2]);
	res= usb_bulk_write (p_data->udev, EP_BULK_OUT, (const char *) buffer, sizeof (buffer), USB_TIMEOUT);
	if (res < sizeof (buffer))
	{
		printf ("Error in usb_bulk_write1: %d/8\n", res);
		return 1;
	}

	return 0;
}

void print_usage (FILE* stream, int exit_code)
  {
  fprintf (stream, "Usage: omap3_usbload -f file1 [-a addr2 -f file2] [-a addr3 -f file3] [etc] [-j addr] [-v]\n");
  fprintf (stream,
           "  -f  --file file           File to load, first file (file1) has no addr\n"
           "  -a  --load_addr addr      Load address, MUST come before each file2-N\n"
           "  -j  --jump addr           Jump address[0x80008000]\n");
  exit (exit_code);
}

enum {  /* subroutine return codes */
	RET_OK,
	RET_ERR,
	RET_IN_LOOP
};

int do_files(struct UPLOAD_DATA *p_data)
	{
	int res, sz, file_cnt, file_len, size, usb_cnt;
	int dat_buf[BUF_LEN];
	unsigned int command;
	int this_file = 1;  /* start with the 2nd file */
	char *p_file;

	file_cnt = usb_bulk_read(p_data->udev, EP_BULK_IN, (char *) dat_buf, sizeof (dat_buf), USB_TIMEOUT);
	if (file_cnt <= 0)
		{
		printf("could not get ASIC ID\n");
		return(-1);
		}

	command = OMAP3_BOOT_CMD;
	if (sizeof(command) != usb_bulk_write(p_data->udev, EP_BULK_OUT, (char*) &command, sizeof(command), USB_TIMEOUT))
		{
		printf("could not send peripheral boot command\n");
		return(-1);
		}

	file_len = p_data->filesize[0];
	if (sizeof(file_len) != usb_bulk_write(p_data->udev, EP_BULK_OUT, (char*) &file_len, sizeof(file_len), USB_TIMEOUT))
		{
		printf("could not send length\n");
		return(-1);
		}

	do
		{
		file_cnt = read(p_data->fd[0], dat_buf, sizeof (dat_buf));
		if (file_cnt > 0)
			{
			usb_cnt = usb_bulk_write(p_data->udev, EP_BULK_OUT, (char *) dat_buf, file_cnt, USB_TIMEOUT);            

			if (usb_cnt != file_cnt)
				{
				printf("could not write to usb usb_cnt = %d file_cnt = %d\n", usb_cnt, file_cnt );
				return(-1);
				}
			}
		} while (file_cnt > 0);

	if (p_data->verbose)
		printf("download ok\n");
	close(p_data->fd[0]);
	p_data->fd[0] = 0;  /* mark as closed */

	/* now wait for x-loader */
	while ((size = p_data->filesize[this_file]) > 0)
		{
		if (p_data->verbose)
			printf ("while loop size = %d\n", size);
		p_file = p_data->p_file = malloc (p_data->filesize[this_file]);
		if (p_file == NULL)
			{
			printf ("Out of memory requesting %d bytes\n", p_data->filesize[this_file]);
			return 1;
			}
		/* read whole file in */
		if ((p_data->filesize[this_file] = read (p_data->fd[this_file], p_file, p_data->filesize[this_file])) < 0)
			{
			perror ("read file");
			return 1;
			}
		if (p_data->verbose)
			printf ("filesize = %d\n", p_data->filesize[this_file]);
		close (p_data->fd[this_file]);

		while (size > 0)
			{
			res= usb_bulk_read (p_data->udev, EP_BULK_IN, (char *) dat_buf, sizeof (dat_buf), USB_TIMEOUT);
			if (res < 0)
				{
				printf("Error in usb_bulk_read: %d\n", res);
				return 1;
				}
			if (res < 8)
				continue;

			switch (dat_buf[0])
				{
				case USBLOAD_CMD_FILE_REQ:
					if (p_data->verbose)
						printf("f: size = %d, addr = 0x%x\n", size, (int) p_data->addrs[this_file]);
					if (send_val (p_data, USBLOAD_CMD_FILE, size, p_data->addrs[this_file]))
						size= -1;
					break;

				case USBLOAD_CMD_REPORT_SZ:  /* he tells us how many btyes are left */
					size -= le32_to_cpu (dat_buf[1]);
					if (size > 0)
						size= -1;
					break;

				case USBLOAD_CMD_ECHO_SZ:  /* he echo's the file size */
					sz = le32_to_cpu (dat_buf[1]);
					if (p_data->verbose)
						printf("sending file, size %d %d\n", sz, res);
					while (sz > 0)
						{
						res = usb_bulk_write(p_data->udev, EP_BULK_OUT, p_file, BUF_LEN_SM, USB_TIMEOUT);
						if (res != BUF_LEN_SM) {
							printf("Error in usb_bulk_write3: %d/%d\n", res, sz);
							size = -1;
							break;
							}
						p_file += res;
						sz -= res;
						}
					break;

				case USBLOAD_CMD_MESSAGE:
					printf("Message: %s\n", (char *) &dat_buf[1]);
					break;

				default:
					printf("Unknown packet type '%s'\n", (char *) dat_buf[1]);
					break;
				}
			}
		if (size == 0 && p_data->verbose)
			printf("Program stored in memory: %d bytes\n", p_data->filesize[this_file]);
		this_file++;  /* next */
		free (p_data->p_file);
		p_data->p_file = NULL;  /* mark as freed */
		}
	if (p_data->jump_addr)
		{
		send_val (p_data, USBLOAD_CMD_JUMP, p_data->jump_addr, 0);  /* go to address */
		if (p_data->verbose)
			printf("Sending jump 0x%x\n", (int) p_data->jump_addr);
		}
	return 0;
	}

enum {  /* need to have at least one file to load */
	ARG_PROGNAME,
	ARG_BINFILE,
	NUM_ARGS
};

int main(int argc, char **argv)
	{
	struct UPLOAD_DATA *p_data;
	int next_option;
	struct usb_bus *bus;
	int file_cntr = 0, res = RET_IN_LOOP;

	p_data = &upload_data;
	do
		{
		next_option = getopt_long (argc, argv, short_options,
			long_options, NULL);
		switch (next_option)
			{
			case 'a':   /* -a or --load_addr */
			/* This option takes an argument, the address to load the file into.  */
				p_data->addrs[file_cntr] = strtoul (optarg, NULL, 0);
				break;
			case 'f':   /* -f or --file */
			/* This option takes an argument, the name of the file. */
				p_data->filename[file_cntr] = optarg;
				p_data->fd[file_cntr] = open(p_data->filename[file_cntr], O_RDONLY);
				if (p_data->fd[file_cntr] < 0)
					{
					perror("Can't open file");
					return 1;
					}
				p_data->filesize[file_cntr] = lseek (p_data->fd[file_cntr], 0, SEEK_END);
				if (p_data->filesize[file_cntr] < 0  ||  lseek (p_data->fd[file_cntr], 0, SEEK_SET) < 0)
					{
					perror ("lseek binfile");
					close (p_data->fd[file_cntr]);
					return 1;
					}
				if (p_data->verbose)
					printf("Opening %s fd = %d, size = %d\n", p_data->filename[file_cntr], p_data->fd[file_cntr], p_data->filesize[file_cntr]);
				file_cntr++;  /* go to next file */
				break;
			case 'j':   /* -j or --jump */
			/* This option takes an argument, the jump address. */
				p_data->jump_addr = strtoul (optarg, NULL, 0);
				break;
			case 'v':   /* -v or --verbose */
				p_data->verbose = 1;
				break;
			case '?':   /* The user specified an invalid option.  */
			/* Print usage information to standard error, and exit with exit
				 code one (indicating abonormal termination).  */
				print_usage (stderr, 1);
				break;
			case -1:    /* Done with options.  */
				break;
			default:    /* Something else: unexpected.  */
				print_usage (stderr, 1);
			}
		}
	while (next_option != -1);

	if (argc < NUM_ARGS) {
		print_usage (stderr, 1);
		return(-1);
		}

	usb_init();
	usb_find_busses();

	while (res == RET_IN_LOOP)
		{
		printf(".");
		fflush(stdout);

		if (usb_find_devices())
			{
			for (bus = usb_get_busses(); bus; bus = bus->next)
				{
				struct usb_device *dev;

				for (dev = bus->devices; dev; dev = dev->next)
					{
					if (dev->descriptor.idVendor == VENDOR_ID_TI &&
						(dev->descriptor.idProduct & DEVICE_ID_MASK) == DEVICE_ID)
						{
						p_data->udev = usb_open(dev);

						if (p_data->udev)
							{
							if (p_data->verbose)
								printf("\n\nfound device 0x%04x:0x%04x\n", dev->descriptor.idVendor, dev->descriptor.idProduct);

							if (usb_set_configuration(p_data->udev, 1) < 0)
								{
								printf("could not set configuration\n");
								res = RET_ERR;
								}
							if (do_files(p_data))  /* if an error occurred */
								res = RET_ERR;
							else
								res = RET_OK;
							}
						}
					}
				}
			}
		usleep(500 * 1000); /* wait until we put another dot */
		}
	for (file_cntr = 0; file_cntr < MAX_FILES; file_cntr++)  /* close all files */
		if (p_data->fd[file_cntr])
			close(p_data->fd[file_cntr]);
	if (p_data->p_file)
		free (p_data->p_file);
	usb_close(p_data->udev);
	return(res);
	}
