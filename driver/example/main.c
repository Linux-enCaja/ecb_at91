/*******************************************************************************
 *
 * Filename: main.c
 *
 * Simple application intended to run within Linux environment from
 *  a mounted filesystem: e.g., loaded from FLASH or via NFS.
 *
 * Revision information:
 *
 * 31DEC2004	kb_admin	initial creation
 *
 * BEGIN_KBDD_BLOCK
 * No warranty, expressed or implied, is included with this software.  It is
 * provided "AS IS" and no warranty of any kind including statutory or aspects
 * relating to merchantability or fitness for any purpose is provided.  All
 * intellectual property rights of others is maintained with the respective
 * owners.  This software is not copyrighted and is intended for reference
 * only.
 * END_BLOCK
 ******************************************************************************/

#include "stdio.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"

int main(void) {

	int	fileNum, bytes;
	char	data[40];

	fileNum = open("/dev/basic", O_RDWR);

	if (fileNum < 0) {
		printf(" Unable to open file\n");
		exit(1);
	}

	printf(" Opened device\n");

	bytes = read(fileNum, data, sizeof(data));

	if (bytes > 0)
		printf("%s", data);

	close(fileNum);

	return (0);
}
