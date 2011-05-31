/*******************************************************************************
 *
 * Filename: main.c
 *
 ******************************************************************************/

#include "stdio.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"

int main(void) {

	int	fileNum, bytes;
	char	data[40];

	fileNum = open("/dev/fpga", O_RDWR);

	if (fileNum < 0) {
		printf(" Unable to open file\n");
		exit(1);
	}

	printf(" Opened device\n");

	bytes = read(fileNum, data, sizeof(data));

	if (bytes > 0)
		printf("%x\n", data);

	close(fileNum);

	return (0);
}
