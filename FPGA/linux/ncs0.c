#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/arch-at91rm9200/at91rm9200_sys.h>

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

	base = mmap (0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, AT91_SMC_CSR(2) & ~MAP_MASK);
	if (base == (void *) -1)
	{
		printf ("Cannot mmap.\n");
		return -1;
	}
	printf ("Memory mapped at address %p.\n", base);

	virt_addr = base + (AT91_SMC_CSR(2) & MAP_MASK) + 0x0000;

/*

	SMC:
	-    RWHLD2 RWHLD1 RWHLD0 -     RWSTP2 RWSTP1 RWSTP0 
	-    -      -      -      -     -      ACSS1  ACSS0
	DRP  DBW1   DBW0   BAT    TDF3  TDF2   TDF1   TDF0
	WSEN NWS6   NWS5   NWS4   NWS3  NWS2   NWS1   NWS0

   DBW:  Data Bus Width:		01: 16 bits, 		10: 8  bits
   BAT:  Byte Access type:		0: 8 bit device, 	 1: 16 bits
	TDF:  Data Float Time
	WSEN: Wait State Enable		1: enable
	NWS:	Number of Wait States

*/

	// 1 WS, 16 bits,  
	*virt_addr = 0x00003081; 
//	*virt_addr = 0X7700508f;

	if (munmap (base, MAP_SIZE) == -1)
	{
		printf ("Cannot munmap.\n");
		return -1;
	}
	
	
	

	return 0;
}

