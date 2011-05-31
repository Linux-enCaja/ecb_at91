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
	unsigned long *virt_addr;

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
	if(*virt_addr != 0x00003081){
   	*virt_addr  = 0x00003081;
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
	unsigned short i, j;
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



	printf("Writing Memory..\n");
	for (i = 0; i < 1024; i++)
	{
		   virt_addr[i] = i;
	}


	printf("Reading Memory..\n");
	for (i = 0; i < 1024; i++)
	{
	   j = virt_addr[i];
		printf("%x = %x\n", i, j );
	}

	if (munmap (base, MAP_SIZE) == -1)
	{
		printf ("Cannot munmap.\n");
		return -1;
	}

	return 0;
}

