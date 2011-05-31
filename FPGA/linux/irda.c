#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <asm/arch-at91rm9200/at91rm9200_sys.h>



#define MAP_SIZE 4096Ul
#define MAP_MASK (MAP_SIZE - 1)


int io_map(off_t address){
	int fd;
	void *base;
	unsigned short *virt_addr;

	if ((fd = open ("/dev/mem", O_RDWR | O_SYNC)) == -1)
	{
		printf ("Cannot open /dev/mem.\n");
		return -1;
	}
	printf ("/dev/mem opened.\n");
	
	printf("WRITE TO: %x\n", address);

	base = mmap (0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, address & ~MAP_MASK);
	if (base == (void *) -1)
	{
		printf ("Cannot mmap.\n");
		return -1;
	}
	printf ("Memory mapped at address %p.\n", base);

	virt_addr = base + (address & MAP_MASK);


	// 1 WS, 16 bits
	if(*virt_addr != 0x00003082){
   	*virt_addr  = 0x00003082;
      printf("Configuring CS2 16 bits and 1 WS\n");
   }
   else
     printf("CS2, already configured\n");

	if (munmap (base, MAP_SIZE) == -1)
	{
		printf ("Cannot munmap.\n");
		return -1;
	}
	else
	  printf ("Memory unmapped at address %p.\n", base);
	return 0;
}

//0xFFFFFF78   CS2

int main ()
{	
	int fd;
	unsigned short i;
	void *base;
	unsigned short *virt_addr;

	io_map(0xFFFFFF78);
	off_t address = 0x30000000;

	if ((fd = open ("/dev/mem", O_RDWR | O_SYNC)) == -1)
	{
		printf ("Cannot open /dev/mem.\n");
		return -1;
	}
	printf ("/dev/mem opened.\n");

	base = mmap (0, MAP_SIZE, PROT_READ | PROT_WRITE,
		     MAP_SHARED, fd, address & ~MAP_MASK);
	if (base == (void *) -1)
	{
		printf ("Cannot mmap.\n");
		return -1;
	}
	printf ("Memory mapped at address %p.\n", base);

	virt_addr = base + (address & MAP_MASK);




	printf("IREG_LP: %x, ISRCF:%x, IRQEn:%x\n", virt_addr[0x20], virt_addr[0x21], virt_addr[0x22]);

   virt_addr[0x12] = 0xFFFF;    // Enable UART0 IRQ Interrupt
   virt_addr[0x01] = 0x0004;    // Enable RX interrupt on UART0

   for(i=0; i<10;i++){
	  printf("Sending data..\n");
     virt_addr[0] = i;
     virt_addr[0 + 1*8] = i*2;
     virt_addr[0 + 2*8] = i*3;
     virt_addr[0 + 3*8] = i*4;

/*  // Loopback test
	  usleep(1000);
	  printf("Data0(%d) = %d\n", i, virt_addr[0x00]);
	  printf("Data1(%d) = %d\n", i, virt_addr[0x00 + 1*8]);
	  printf("Data2(%d) = %d\n", i, virt_addr[0x00 + 2*8]);
	  printf("Data3(%d) = %d\n", i, virt_addr[0x00 + 3*8]);
*/
	}


	if (munmap (base, MAP_SIZE) == -1)
	{
		printf ("Cannot munmap.\n");
		return -1;
	}

	return 0;
}

