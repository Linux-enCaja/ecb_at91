#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define MAP_SIZE 4096Ul
#define MAP_MASK (MAP_SIZE - 1)

int main ()
{	
	int fd;
	void *base;
	unsigned long *virt_addr;

	if ((fd = open ("/dev/mem", O_RDWR | O_SYNC)) == -1)
	{
		printf ("Cannot open /dev/mem.\n");
		return -1;
	}
	printf ("/dev/mem opened.\n");

	base = mmap (0, MAP_SIZE, PROT_READ | PROT_WRITE,
		     MAP_SHARED, fd, 0xFFFFF600 & ~MAP_MASK);
	if (base == (void *) -1)
	{
		printf ("Cannot mmap.\n");
		return -1;
	}
	printf ("Memory mapped at address %p.\n", base);

	virt_addr = base + (0xFFFFF600 & MAP_MASK) + 0x0004;
	*virt_addr = 0x08000000;

	if (munmap (base, MAP_SIZE) == -1)
	{
		printf ("Cannot munmap.\n");
		return -1;
	}

	base = mmap (0, MAP_SIZE, PROT_READ | PROT_WRITE,
		     MAP_SHARED, fd, 0xFFFFFC00 & ~MAP_MASK);
	if (base == (void *) -1)
	{
		printf ("Cannot mmap.\n");
		return -1;
	}
	printf ("Memory mapped at address %p.\n", base);

	virt_addr = base + (0xFFFFFC00 & MAP_MASK) + 0x44;
	*virt_addr = 0x00000006;	// PCK1 @ 45 MHz

	virt_addr = base + (0xFFFFFC00 & MAP_MASK) + 0x00;
	*virt_addr = 0x00000200;	// enable PCK1
	if (munmap (base, MAP_SIZE) == -1)
	{
		printf ("Cannot munmap.\n");
		return -1;
	}

	return 0;
}

