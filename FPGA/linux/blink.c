#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <asm/arch-at91rm9200/at91rm9200_sys.h>



#define MAP_SIZE 4096Ul
#define MAP_MASK (MAP_SIZE - 1)


int io_map(int address){
	int fd;
	void *base;
	unsigned long *virt_addr;

	if ((fd = open ("/dev/mem", O_RDWR | O_SYNC)) == -1)
	{
		printf ("Cannot open /dev/mem.\n");
		return -1;
	}
	printf ("/dev/mem opened.\n");

	base = mmap (0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, address & ~MAP_MASK);
	if (base == (void *) -1)
	{
		printf ("Cannot mmap.\n");
		return -1;
	}
	printf ("Memory mapped at address %p.\n", base);

	virt_addr = base + (address & MAP_MASK) + 0x0000;


	// 1 WS, 16 bits
	if(*virt_addr != 0x00003081){
   	*virt_addr = 0x00003081;
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



int main ()
{	
	int i, fd;
	void *base;
	unsigned char *virt_addr;

	io_map(AT91_SMC_CSR(2));

	if ((fd = open ("/dev/mem", O_RDWR | O_SYNC)) == -1)
	{
		printf ("Cannot open /dev/mem.\n");
		return -1;
	}
	printf ("/dev/mem opened.\n");

	base = mmap (0, MAP_SIZE, PROT_READ | PROT_WRITE,
		     MAP_SHARED, fd, 0x30000000 & ~MAP_MASK);
	if (base == (void *) -1)
	{
		printf ("Cannot mmap.\n");
		return -1;
	}
	printf ("Memory mapped at address %p.\n", base);

	virt_addr = base + (0x30000000 & MAP_MASK) + 0x0000;
	for (i = 0; i < 0xf; i++)
	{
		*virt_addr = i;
		usleep(25000);
	}

	if (munmap (base, MAP_SIZE) == -1)
	{
		printf ("Cannot munmap.\n");
		return -1;
	}

	return 0;
}

