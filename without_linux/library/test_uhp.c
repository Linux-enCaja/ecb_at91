/*******************************************************************************
 *
 * Filename: test_uhp.c
 *
 * Instantiation of the loopback UHP function adapted from external sources.
 *
 * Revision information:
 *
 * 20JAN2005	kb_admin	initial creation - development status only!
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

#include "AT91RM9200.h"
#include "ohci.h"
#include "debug_io.h"
#include "prompt.h"


static parse_function_t	uhptest_function;

typedef struct _AT91S_TIMEOUT {
		unsigned int tick;
		unsigned int second;
} AT91S_TIMEOUT, *AT91PS_TIMEOUT;


__inline void AT91F_PIO_CfgOutput(
	AT91PS_PIO pPio,             // \arg pointer to a PIO controller
	unsigned int pioEnable)      // \arg PIO to be enabled
{
	pPio->PIO_PER = pioEnable; // Set in PIO mode
	pPio->PIO_OER = pioEnable; // Configure in Output
}

__inline void AT91F_PIO_ClearOutput(
	AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
	unsigned int flag) // \arg  output to be cleared
{
	pPio->PIO_CODR = flag;
}

__inline void AT91F_PIO_SetOutput(
	AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
	unsigned int flag) // \arg  output to be set
{
	pPio->PIO_SODR = flag;
}

__inline void AT91F_PMC_EnablePeriphClock (
	AT91PS_PMC pPMC, // \arg pointer to PMC controller
	unsigned int periphIds)  // \arg IDs of peripherals to enable
{
	pPMC->PMC_PCER = periphIds;
}

__inline void AT91F_UHP_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_UHP));
}

__inline void AT91F_UDP_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_UDP));
}

void AT91F_InitTimeout(AT91PS_TIMEOUT pTimeout, unsigned int second)
{
	pTimeout->tick = *(AT91C_ST_CRTR);
	pTimeout->second = second;
}

int AT91F_TestTimeout(AT91PS_TIMEOUT pTimeout)
{
	if ((pTimeout->tick != *(AT91C_ST_CRTR)) && (pTimeout->second != 0)) {
		pTimeout->tick = *(AT91C_ST_CRTR);
		--(pTimeout->second);
	}
	return pTimeout->second;
}


// =====================================
// Memory allocation for UHP:
// =====================================
// UHP HCCA memory location:
AT91S_UHP_HCCA HCCA __attribute__ ((aligned(32)));
// UHP transfer descriptors:
AT91S_UHP_ED pUHPEd[1] __attribute__ ((aligned(32)));
AT91S_UHP_ED pUHPTd[4] __attribute__ ((aligned(32)));
// UHP data area
const char pUHPSetup[8] = {
	0x08,
	0x06,
	0x00,
	0x01,
	0x00,
	0x00,
	0x40,
	0x00
};
#define DataSIZE 0x12
char pUHPData[DataSIZE];
// =====================================
// Memory allocation for UDP:
// =====================================
// UDP data area
char pUDPSetup[8];
const char pUDPData[DataSIZE] = {
	0x12,
	0x01,
	0x00,
	0x01,
	0x00,
	0x00,
	0x00,
	0x08,
	0x7B,
	0x05,
	0x00,
	0x00,
	0x04,
	0x03,
	0x01,
	0x02,
	0x00,
	0x01
};

#define AT91C_PRDSTRTVAL 0x2240
#define AT91C_FRINTERVAL 0x2710
#define AT91C_FSMAXPKTSZ (((AT91C_FRINTERVAL * 6) / 7) - 180)

#define AT91C_PRDSTRT    AT91C_PRDSTRTVAL
#define AT91C_FMINTERVAL ((AT91C_FSMAXPKTSZ << 16) | AT91C_FRINTERVAL)

void UHP_Test(int iterations) {
	AT91PS_UHP      pUhp  = AT91C_BASE_UHP;
	AT91PS_UDP      pUdp  = AT91C_BASE_UDP;

	unsigned int i;
	AT91S_TIMEOUT timeout;

	iterations = 1; /* multiple iterations test not yet implemented */
	DebugPrint("\n\n\r-I- ======================================\n\r");
	DebugPrint("-I- AT91RM9200 basic UHP example\n\r");
	DebugPrint("-I- --------------------------------------\n\r");
	DebugPrint("-I- Connect the UDP port to a UHP port...\n\r");
	DebugPrint("-I- ======================================\n\r");

	/* ************************************************ */
	/* Deactivate UDP pull up                           */
	/* ************************************************ */
	AT91F_PIO_CfgOutput(AT91C_BASE_PIOB, AT91C_PIO_PB22);
	AT91F_PIO_ClearOutput(AT91C_BASE_PIOB, AT91C_PIO_PB22);

	/* ************************************************ */
	/* Open UDP+UHP clocks                              */
	/* ************************************************ */
	// Open clocks for USB host + device
	AT91F_UHP_CfgPMC();
	AT91F_UDP_CfgPMC();
	AT91C_BASE_PMC->PMC_SCER |= (AT91C_PMC_UHP | AT91C_PMC_UDP);

	/* ************************************************ */
	/* Configure the UHP                                */
	/* ************************************************ */
	// Desactivate all IT
	pUdp->UDP_IDR = (unsigned int) -1;
	// Disable all pending IT
	pUdp->UDP_ICR = (unsigned int) -1;
	// RESET UDP
	pUdp->UDP_RSTEP  = 0;
	pUdp->UDP_GLBSTATE = 0;

	// Forcing UHP_Hc to reset
	pUhp->UHP_HcControl = 0;

	// Writing the UHP_HCCA
	pUhp->UHP_HcHCCA = (unsigned int) &HCCA;

	// Enabling list processing
	pUhp->UHP_HcControl = 0;

	// Set the frame interval
	pUhp->UHP_HcFmInterval = AT91C_FMINTERVAL;
	pUhp->UHP_HcPeriodicStart = AT91C_PRDSTRT;

	// Create a default endpoint descriptor
	AT91F_CreateEd(
		(unsigned int) pUHPEd, // ED Address
		8,      // Max packet
		0,      // TD format
		0,      // Skip
		0,      // Speed
		0x0,    // Direction
		0x0,    // Endpoint
		0x0,    // Func Address
		(unsigned int) &pUHPTd[3],    // TDQTailPointer
		(unsigned int) &pUHPTd[0],    // TDQHeadPointer
		0,      // ToggleCarry
		0x0);   // NextED

	// Setup PID
	AT91F_CreateGenTd(
		(unsigned int) &pUHPTd[0],    // TD Address
		2,      // Data Toggle
		0x7,    // DelayInterrupt
		0x0,    // Direction
		1,      // Buffer Rounding
		(unsigned int) pUHPSetup,  // Current Buffer Pointer
		(unsigned int) &pUHPTd[1],    // Next TD
		8);     // Buffer Length

	// Data IN
	AT91F_CreateGenTd(
		(unsigned int) &pUHPTd[1],    // TD Address
		0,      // Data Toggle
		0x7,    // DelayInterrupt
		0x2,    // Direction
		1,      // Buffer Rounding
		(unsigned int) pUHPData,  // Current Buffer Pointer
		(unsigned int) &pUHPTd[2],    // Next TD
		DataSIZE);  // Buffer Length

	// Status OUT
	AT91F_CreateGenTd(
		(unsigned int) &pUHPTd[2],    // TD Address
		3,      // Data Toggle
		0x7,    // DelayInterrupt
		0x1,    // Direction
		1,      // Buffer Rounding
		0x0,    // Current Buffer Pointer
		(unsigned int) &pUHPTd[3],   // Next TD
		0x0);   // Buffer Length

	AT91F_CreateGenTd(
		(unsigned int) &pUHPTd[3],    // TD Address
		3,      // Data Toggle
		0x7,    // DelayInterrupt
		0x1,    // Direction
		1,      // Buffer Rounding
		0x0,    // Current Buffer Pointer
		(unsigned int) 0,   // Next TD
		0x0);   // Buffer Length

	// Programming the BHED
	pUhp->UHP_HcControlHeadED = (unsigned int) pUHPEd;

	// Programming the BCED
	pUhp->UHP_HcControlCurrentED = (unsigned int) pUHPEd;


	// Initializing the UHP_HcDoneHead
	pUhp->UHP_HcBulkDoneHead   = 0x00;
	HCCA.UHP_HccaDoneHead = 0x0000;

	// Forcing UHP_Hc to Operational State
	pUhp->UHP_HcControl = 0x80;


	// Enabling port power
	pUhp->UHP_HcRhPortStatus[0] = 0x00000100;
	pUhp->UHP_HcRhPortStatus[1] = 0x00000100;

	pUhp->UHP_HcRhStatus = 0x00010000;


	/* ************************************************ */
	/* Activate UDP pull up                             */
	/* ************************************************ */
	// UDP: Connect a pull-up
	AT91F_PIO_SetOutput(AT91C_BASE_PIOB, AT91C_PIO_PB22);

	/* ************************************************ */
	/* Detect a connected deviced, generate a reset...  */
	/* ************************************************ */
	// UHP: Detect the device on one port, generate a reset and enable the port
	AT91F_InitTimeout(&timeout, 2);
	while (1) {
		if ( (pUhp->UHP_HcRhPortStatus[0] & 0x01) ) {
			DebugPrint("-I- Device detected on port 0\n\r");
			pUhp->UHP_HcRhPortStatus[0] = (1 << 4); // SetPortReset
			while (pUhp->UHP_HcRhPortStatus[0] & (1 << 4)); // Wait for the end of reset
			pUhp->UHP_HcRhPortStatus[0] = (1 << 1); // SetPortEnable
			break;
		}
		else if ( (pUhp->UHP_HcRhPortStatus[1] & 0x01) ) {
			pUhp->UHP_HcRhPortStatus[1] = (1 << 4); // SetPortReset
			while (pUhp->UHP_HcRhPortStatus[1] & (1 << 4)); // Wait for the end of reset
			pUhp->UHP_HcRhPortStatus[1] = (1 << 1); // SetPortEnable
			break;
		}
		else if ( !AT91F_TestTimeout(&timeout) ) {
			DebugPrint("-E- Please connect the UHP port to the UDP port\n\r");
			goto error;
		}
	}
	// UHP: UHP is now operational and control list processing is enabled
	pUhp->UHP_HcControl = 0x90;

	// UDP: Wait for end bus reset
	AT91F_InitTimeout(&timeout, 2);
	while ( !(pUdp->UDP_ISR & AT91C_UDP_ENDBUSRES)) {
		if ( !AT91F_TestTimeout(&timeout)) {
			DebugPrint("-E- End of bus reset not received\n\r");
			goto error;
		}
	}

	pUdp->UDP_ICR = AT91C_UDP_ENDBUSRES;
	pUdp->UDP_CSR[0] = (AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_CTRL);

	DebugPrint("-I- A reset has been detected by the UDP\n\r");

	/* ************************************************ */
	/* Generate traffic between UHP and UDP             */
	/* ************************************************ */
	// UHP: Notify the Hc that the Control list is filled
	pUhp->UHP_HcCommandStatus = 0x02;

	// UDP: Wait for a Setup packet
	AT91F_InitTimeout(&timeout, 2);
	while ( !(pUdp->UDP_CSR[0] & AT91C_UDP_RXSETUP)) {
		if ( !AT91F_TestTimeout(&timeout)) {
			DebugPrint("-E- No setup packet has been received by the UDP\n\r");
			goto error;
		}
	}
	while (iterations--) {
		DebugPrint("Iteration: ");
		DebugPrintHex(2, iterations);
		DebugPrint("\n\r");

		for (i = 0; i < 8; ++i)
			pUDPSetup[i] = pUdp->UDP_FDR[0];
		pUdp->UDP_CSR[0] |= AT91C_UDP_DIR; // Data stage will be DATA IN transactions
		pUdp->UDP_CSR[0] &= ~(AT91C_UDP_RXSETUP);

		DebugPrint("-I- A setup packet has been sent by UDP and received by UHP\n\r");

		// UDP: Send several Data packets
		for (i = 0; i < DataSIZE; ++ i) {
			pUdp->UDP_FDR[0] = pUDPData[i];
			// UDP: Detect a packet frontier, send it and wait for the end of transmition
			if ( !((i+1) % 8) || (i == (DataSIZE - 1))) {
				pUdp->UDP_CSR[0] |= AT91C_UDP_TXPKTRDY;
				AT91F_InitTimeout(&timeout, 2);
				while ( !(pUdp->UDP_CSR[0] & AT91C_UDP_TXCOMP)) {
					if ( !AT91F_TestTimeout(&timeout)) {
						DebugPrint("-E- A data packet has not been acknowledged by the UHP\n\r");
						goto error;
					}
				}
				pUdp->UDP_CSR[0] &= ~AT91C_UDP_TXCOMP;
				DebugPrint("-I- A data packet has been sent by UDP and received by UHP\n\r");
			}
		}

		// UDP: Wait for the status sent by the host
		AT91F_InitTimeout(&timeout, 2);
		while ( !(pUdp->UDP_CSR[0] & AT91C_UDP_RX_DATA_BK0)) {
			if ( !AT91F_TestTimeout(&timeout)) {
				DebugPrint("-E- No status packet has been sent by the UHP\n\r");
				goto error;
			}
		}
		pUdp->UDP_CSR[0] = ~AT91C_UDP_RX_DATA_BK0;

		DebugPrint("-I- A status data packet has been sent by UHP and received by UDP\n\r");

		/* ************************************************ */
		/* Compare data sent and received                   */
		/* ************************************************ */
		DebugPrint("-I- Compare sent/received setup packet ...");
		for (i = 0; i < 8; ++i) {
			if (pUHPSetup[i] != pUDPSetup[i]) {
				DebugPrint("Failed\n\r");
				goto error;
			}
		}
		DebugPrint(" Success\n\r");

		DebugPrint("-I- Compare sent/received data packet ...");
		for (i = 0; i < DataSIZE; ++i) {
			if (pUHPData[i] != pUDPData[i]) {
				DebugPrint("Failed\n\r");
				goto error;
			}
		}
		DebugPrint(" Success\n\r");
	}

	DebugPrint("-I- Test successfull...\n\r");
	return ;

error:
	DebugPrint("-F- Test failed...\n\r");
}


/*
 * .KB_C_FN_DEFINITION_START
 * int uhptest_function_parse(int argc, char *argv[])
 *  This global function parses text from the command line.
 * .KB_C_FN_DEFINITION_END
 */
int uhptest_function_parse(int argc, char *argv[]) {

	UHP_Test(1);

	return (0);
}


/*
 * .KB_C_FN_DEFINITION_START
 * void uhptest_function_help(void)
 *  This global displays help info for the corresponding command.
 * .KB_C_FN_DEFINITION_END
 */
void uhptest_function_help(void) {
	DebugPrint("\tuhp loopback test\n\r");
	DebugPrint("\t\tuhp\n\r");
}


/*
 * .KB_C_FN_DEFINITION_START
 * void InitUHP(void)
 *  Global function to initialize the UHP loopback function/test.
 * .KB_C_FN_DEFINITION_END
 */
void InitUHP(void) {

	uhptest_function.f_string = "uhp";
	uhptest_function.parse_function = uhptest_function_parse;
	uhptest_function.help_function = uhptest_function_help;
	RegisterFunction(&uhptest_function);
}
