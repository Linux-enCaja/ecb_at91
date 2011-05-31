#include "debug_io.h"
#include "at91rm9200_lowlevel.h"


int main(void) {

	char		l_char;

	DebugPrint("\n\rEntry: main\n\r");

	while (1) {
		DebugPrint("Waiting for input\n\r");

		/* wait for input */
		if (WaitForChar(&l_char, 1)) {

			DebugPrint("  -- Received input\n\r");
		}
	}

	return (1);
}
