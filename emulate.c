/*

###
PoC to decompress Desko's Honeywell N56XX firmware using Unicorn engine
###

IOActive
Warcodes II - The Desko Case
https://labs.ioactive.com

---------
Compile:
$ gcc emulate.c -o emulate -lunicorn
Usage:
$ ./emulate decompress_function.bin compressed_firmware.bin OUTPUT_FILE
---------
For Research Purposes Only.

Ruben Santamarta
*/ 

#include <unicorn/unicorn.h>
#include <string.h>


#define FIRM_SIZE       0x1332F0


#define ADDRESS_CODE    0x0
#define ADDRESS_FIRM    0x8A000000
#define ADDRESS_STACK   0x500000
#define ADDRESS_DECOMP  0x80000000


void emulate_function(char *path_code, char *path_blob, char *path_output)
{
    uc_engine *uc;
    uc_err err;

    unsigned int r0;     
    unsigned int r1;	  
    unsigned int r2;     
    unsigned int r3;     
    unsigned int sp;
 
	size_t fileSize, n, codelen;
	
	unsigned char *buff, *dest;

	FILE *fIn = NULL;
	
	fIn = fopen(path_code, "rb");

	if(!fIn)
    {
    	printf("[+] Error loading code to emulate\n");
    	return;
	}
	fseek(fIn, 0, SEEK_END); 
	fileSize = ftell(fIn); 
	fseek(fIn, 0, SEEK_SET); 

  	buff = (unsigned char*)calloc(fileSize + 50, sizeof(unsigned char));
  	
  	
    codelen = fread(buff, 1, fileSize, fIn);
    
    printf("[+] Loaded %zx bytes of code to be emulated. Host memory %p\n",codelen, buff);

    fclose(fIn);
    
   
    printf("[+] Initializing ARM code emulator\n");


    err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &uc);
    if (err) {
        printf("Failed on uc_open() with error returned: %u (%s)\n",
                err, uc_strerror(err));
        return;
    }
	printf("[+] Mapping 'decompress function' code at %x - result %x\n", ADDRESS_CODE,
                                                                         uc_mem_map(uc, ADDRESS_CODE, 1 * 1024 * 1024, UC_PROT_ALL) );
    
	printf("[+] Copying 'decompress' function ( %zx bytes ) to emulator memory at 0x%X\n", codelen, ADDRESS_CODE);
    
    uc_mem_write(uc, ADDRESS_CODE, buff, codelen);
    free(buff);


	printf("[+] Loading compressed firmware...");

    fIn = fopen(path_blob, "rb");

    if(!fIn)
    {
    	printf("[+] Error loading firmware to decompress\n");
    	return;
	}
	fseek(fIn, 0, SEEK_END); 
	fileSize = ftell(fIn); 
	fseek(fIn, 0, SEEK_SET); 

  	buff = (unsigned char*)calloc( fileSize + 50, sizeof(unsigned char));
  	dest = (unsigned char*)calloc((8 * 1024 * 1024) + 0x200, sizeof(unsigned char));
  	
    	
	n = fread(buff,1, fileSize,fIn);
	printf(" %zx bytes loaded at host memory %p\n", n, buff);
	fclose(fIn);
	

    printf("[+] Allocating memory for the compressed firmware blob at 0x%X -> result %x\n", ADDRESS_FIRM,
                                                                                            uc_mem_map(uc, ADDRESS_FIRM, 2 * 1024 * 1024, UC_PROT_ALL));

    printf("[+] Copying the compressed firmware ( %zx bytes ) to emulator memory at 0x%X -> result %x\n", n, ADDRESS_FIRM,
                                                                                    uc_mem_write(uc, ADDRESS_FIRM, buff, FIRM_SIZE));
	
	
    printf("[+] Mapping stack at 0x%X -> result %x\n" , ADDRESS_STACK,
                                                        uc_mem_map(uc, ADDRESS_STACK, 1 * 1024 * 1024, UC_PROT_ALL));
  
 
    printf("[+] Mapping memory at the emulator to hold the decompressed firmware at 0x%X -> result %x\n", ADDRESS_DECOMP
                                                                                                        , uc_mem_map(uc, ADDRESS_DECOMP, 8 * 1024 * 1024, UC_PROT_ALL));
    
	// init context
	r0 = ADDRESS_FIRM;               // compressed firmware
    r1 = FIRM_SIZE;                  // compressed len
	r2 = ADDRESS_DECOMP    + 0x1000; // dest
	r3 = ADDRESS_STACK     + 0x100;  // flag
	sp = ADDRESS_STACK     + 0x10000;
	
    
    printf("[+] Initializing Registers\n");
    uc_reg_write(uc, UC_ARM_REG_R0, &r0);
    uc_reg_write(uc, UC_ARM_REG_R1, &r1);
    uc_reg_write(uc, UC_ARM_REG_R2, &r2);
    uc_reg_write(uc, UC_ARM_REG_R3, &r3);
    uc_reg_write(uc, UC_ARM_REG_SP, &sp);
    

   
    printf("[+] Starting emulation at 0x%X\n", ADDRESS_CODE);

    err = uc_emu_start(uc, ADDRESS_CODE, ADDRESS_CODE + codelen, 0, 0);


    printf("[+] Emulation done\n");


    printf("[+] Dumping decompressed firmware at 0x%X...\n", ADDRESS_DECOMP + 0x1000);
	uc_mem_read(uc, ADDRESS_DECOMP + 0x1000, dest, 4 * 1024 * 1024);
	
	fIn = fopen(path_output, "wb+");

	fwrite(dest, 1, 4 * 1024 * 1024, fIn);
	fclose(fIn);

    uc_close(uc);
}



int main(int argc, char **argv, char **envp)
{

    if(argc != 4) {

        printf("Usage: ./emulate decompress_function.bin compressed_firmware.bin OUTPUT_FILE\n");
        return 0;
    }
    
    emulate_function(argv[1],argv[2],argv[3]);

    
    return 0;
}
