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
		     MAP_SHARED, fd, 0xFFFFF800 & ~MAP_MASK);
	if (base == (void *) -1)
	{
		printf ("Cannot mmap.\n");
		return -1;
	}
	printf ("Memory mapped at address %p.\n", base);

	virt_addr = base + (0xFFFFF800 & MAP_MASK) + 0x0000;	// PIO enable
	*virt_addr = (1 << 16);

	virt_addr = base + (0xFFFFF800 & MAP_MASK) + 0x0010;	// output enable
	*virt_addr = (1 << 16);

	virt_addr = base + (0xFFFFF800 & MAP_MASK) + 0x0034;	// PC16 = 0
	*virt_addr = (1 << 16);

	virt_addr = base + (0xFFFFF800 & MAP_MASK) + 0x0030;	// PC16 = 1
	*virt_addr = (1 << 16);

	if (munmap (base, MAP_SIZE) == -1)
	{
		printf ("Cannot munmap.\n");
		return -1;
	}

	return 0;
}

