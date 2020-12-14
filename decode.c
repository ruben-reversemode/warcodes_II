/*

###
Extracting N5600's Winbond SPI Flash memory from a SALEAE exported file.
###

IOActive
Warcodes II - The Desko Case
https://labs.ioactive.com

---------
Compile:
$ gcc decode.c -o decode

Usage:
$ ./decode session_csv_file.txt memory.bin
---------
For Research Purposes Only.

Ruben Santamarta
*/ 

#include <stdlib.h>
#include <stdio.h>  
#include <string.h>

#define BUFFER_LEN 0x400

#define SPI_READ 0x3

typedef struct _SPI_PACKET_
{
	unsigned char bytes[BUFFER_LEN];
	unsigned int packet_id;
	unsigned char command[4];
	size_t 	counter;
} SPI_PACKET;

unsigned int get_packet_id(char *line)
{
	char *pt;
	char *line_shadow;

	line_shadow = strdup(line);
	pt = strtok (line_shadow,",");
  pt = strtok (NULL, ",");
  free(line_shadow);
  return  atoi(pt);  
}

unsigned char get_byte_from_line(char *line)
{

	char *pt;
	char *line_shadow;
	unsigned char payload;
	line_shadow = strdup(line);
	
	pt = strtok (line_shadow,","); // time
  pt = strtok (NULL, ",");      // packet id
  pt = strtok (NULL, ",");      // byte
 
  payload = strtoul(pt,NULL,16)&0xFF;
  
  free(line_shadow);

  return  payload;  
}

unsigned char get_command_from_line(char *line)
{

	char *pt;
	int i,b;
	unsigned char payload;
	char *line_shadow;
	
	line_shadow = strdup(line);
	pt = strtok (line_shadow,",");
  pt = strtok (NULL, ","); // packet id
  pt = strtok (NULL, ","); // byte
  pt = strtok (NULL, ","); // command
  
  payload = strtoul(pt,NULL,16)&0xFF;

  free(line_shadow);
    
  return  payload;  
}

void dump_packet(SPI_PACKET *spi,FILE *fpOut)
{

	
	if(spi->command[0] == SPI_READ) 
	{	
			
      if( !memcmp(spi->bytes,"\xFF\xFF\xFF\xFF",4) ) {
        
        printf("[+][ID %d] Writing %lx bytes from address 0x%02x%02x%02x\n",spi->packet_id,spi->counter-4,spi->command[1],spi->command[2],spi->command[3]);
      
      } else {
        
        printf("Glitches found in the dummy bytes at PacketID %d - C:%02x - %02x%02x%02x%02x -  Dumping data anyway\n",spi->packet_id,spi->command[0],spi->bytes[0],spi->bytes[1],spi->bytes[2],spi->bytes[3]);
      }
     
     fwrite(((unsigned char*)spi->bytes+4), 1, spi->counter-4, fpOut);
  }
}

SPI_PACKET *get_payload_from_packet(FILE *fp, SPI_PACKET *spi,FILE *fpOut)
{
  char *line_buf = NULL;
  char *pt;
  unsigned int packetId;
  size_t line_buf_size = 0;
  int line_count = 0;
  ssize_t line_size;
  unsigned int b=1;
  SPI_PACKET *newpacket;

  line_size = getline(&line_buf, &line_buf_size, fp);
  packetId = get_packet_id(line_buf);
  printf("processing %d\n",packetId);
  while ( line_size >=0 )
  {
  
   		if( packetId == spi->packet_id )
   		{
    	
    		spi->bytes[b] = get_byte_from_line(line_buf);
    		
    		
    		if( b < 4 )
    		{
    			spi->command[b] = get_command_from_line(line_buf);
    		}
    
  			
  	
  			if( b >= BUFFER_LEN ){ 
  				printf("bye bye! packet Id %d is too big!\n", packetId);
  				exit(0);
  			}
  			
  			b++;
  
  			
  		} else {
  				spi->counter = b;
  				dump_packet(spi,fpOut);
  				
  				free(spi);
  				
  				newpacket = (SPI_PACKET*)calloc(1,sizeof(SPI_PACKET));
  				newpacket->packet_id = packetId;
  				newpacket->command[0] = get_command_from_line(line_buf);
  				newpacket->bytes[0] = get_byte_from_line(line_buf);
  				return newpacket;
  		}	
  		
  		free(line_buf);
  		line_buf = NULL;
  		
  		line_size = getline(&line_buf, &line_buf_size, fp);

  		if( line_size < 0  ){ 
  			printf("[+] Done\n");
  			exit(0);
  			}

  		packetId = get_packet_id(line_buf);
  	}
  	
  return NULL;
}


int main(int argc, char **argv)
{

  unsigned int packetId, currentId;
  int i;
  char *line_buf = NULL;
  char *pt;
  size_t line_buf_size = 0;
  int line_count = 0;
  ssize_t line_size;
  char *line_shadow;
  FILE *fpOut;
 
  SPI_PACKET *current_packet;

  if( argc != 3 )
  {
    printf("Usage: ./decode session_csv_file.txt memory.bin\n");
    return 0;
  }
  
  FILE *fp = fopen(argv[1], "r");
  
  fpOut = fopen(argv[2], "wb+");
  
  if (!fp)
  {
    fprintf(stderr, "Error opening file '%s'\n", argv[1]);
    return EXIT_FAILURE;
  }

  line_size = getline(&line_buf, &line_buf_size, fp);
  current_packet = (SPI_PACKET*)calloc(1,sizeof(SPI_PACKET));
   
  current_packet->command[0] = get_command_from_line(line_buf);
  current_packet->bytes[0] = get_byte_from_line(line_buf);
  current_packet->packet_id= get_packet_id(line_buf);

 
  while (line_size >= 0)
  {
  	
   
    current_packet = get_payload_from_packet(fp, current_packet,fpOut); 
  
    if(!current_packet)
    {
    	printf("[+] Done\n");
    	exit(0);
    }

  }


  fclose(fpOut);
  fclose(fp);

  free(line_buf);
  line_buf = NULL;

  return 1;
}