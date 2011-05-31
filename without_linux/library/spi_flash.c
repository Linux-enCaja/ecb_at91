/*******************************************************************************
 *
 * Filename: spi_flash.c
 *
 * Instantiation of SPI flash control routines supporting AT45DB161B
 *
 * Revision information:
 *
 * 17JAN2005	kb_admin	initial creation
 *				adapted from external sources
 *				tested for basic operation only!!!
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

#include "spi_flash.h"
#include "prompt.h"

/* ********************** PRIVATE FUNCTIONS/DATA ******************************/


static spiCommand_t	spi_command;
static char		tx_commandBuffer[8], rx_commandBuffer[8];
static parse_function_t	spiread_function, spiwrite_function;


/*
 * .KB_C_FN_DEFINITION_START
 * void SendCommand(spiCommand_t *pCommand)
 *  Private function sends 8-bit value to the device and returns the 8-bit
 * value in response.
 * .KB_C_FN_DEFINITION_END
 */
static void SendCommand(spiCommand_t *pCommand) {

	unsigned	value;

	AT91C_BASE_SPI->SPI_PTCR = AT91C_PDC_TXTDIS + AT91C_PDC_RXTDIS;

	AT91C_BASE_SPI->SPI_RPR = (unsigned)pCommand->rx_cmd;
	AT91C_BASE_SPI->SPI_TPR = (unsigned)pCommand->tx_cmd;

	AT91C_BASE_SPI->SPI_RCR = pCommand->rx_cmd_size;
	AT91C_BASE_SPI->SPI_TCR = pCommand->tx_cmd_size;

	if (pCommand->tx_data_size != 0) {

		AT91C_BASE_SPI->SPI_TNPR = (unsigned)pCommand->tx_data;
		AT91C_BASE_SPI->SPI_TNCR = pCommand->tx_data_size;
		AT91C_BASE_SPI->SPI_RNPR = (unsigned)pCommand->rx_data;
		AT91C_BASE_SPI->SPI_RNCR = pCommand->rx_data_size;
	}

	AT91C_BASE_SPI->SPI_PTCR = AT91C_PDC_TXTEN + AT91C_PDC_RXTEN;

	// wait for completion
	while (!((value = AT91C_BASE_SPI->SPI_SR) & AT91C_SPI_SPENDRX)) ;
}


/*
 * .KB_C_FN_DEFINITION_START
 * char GetFlashStatus(void)
 *  Private function to return device status.
 * .KB_C_FN_DEFINITION_END
 */
static char GetFlashStatus(void) {

	p_memset(tx_commandBuffer, 0, 8);
	tx_commandBuffer[0] = STATUS_REGISTER_READ;
	p_memset(rx_commandBuffer, 0, 8);
	spi_command.tx_data_size = 0;
	spi_command.rx_data_size = 0;
	spi_command.tx_cmd = tx_commandBuffer;
	spi_command.rx_cmd = rx_commandBuffer;
	spi_command.rx_cmd_size = 2;
	spi_command.tx_cmd_size = 2;
	SendCommand(&spi_command);

	return (rx_commandBuffer[1]);
}


/*
 * .KB_C_FN_DEFINITION_START
 * void WaitForDeviceReady(void)
 *  Private function to poll until the device is ready for next operation.
 * .KB_C_FN_DEFINITION_END
 */
static void WaitForDeviceReady(void) {
	while (!(GetFlashStatus() & 0x80)) ;
}


/*
 * .KB_C_FN_DEFINITION_START
 * void ProgramBuffer(unsigned pageAddress, unsigned byteAddress,
 *			 unsigned src_addr, unsigned size);
 *  Private function to program Flash through the buffer.  Note this
 * will corrupt any data on the same page not intended for modification.
 * To preserve this data, read it first, modify it in RAM, then write back
 * data in FLASH_PAGE_SIZE intervals.  This also corrupts the data in RAM.
 * .KB_C_FN_DEFINITION_END
 */
static void ProgramBuffer(unsigned pageAddress, unsigned byteAddress,
			unsigned src_addr, unsigned size) {

	p_memset(tx_commandBuffer, 0, 8);
	tx_commandBuffer[0] = PROGRAM_THROUGH_BUFFER;
	tx_commandBuffer[1] = ((pageAddress >> 6) & 0x3F);
	tx_commandBuffer[2] = ((pageAddress << 2) & 0xFC) |
				((byteAddress >> 8) & 0x3);
	tx_commandBuffer[3] = (byteAddress & 0xFF);

	p_memset(rx_commandBuffer, 0, 8);

	spi_command.tx_cmd = tx_commandBuffer;
	spi_command.rx_cmd = rx_commandBuffer;
	spi_command.rx_cmd_size = 4;
	spi_command.tx_cmd_size = 4;

	spi_command.tx_data_size = size;
	spi_command.tx_data = (char*)src_addr;
	spi_command.rx_data_size = size;
	spi_command.rx_data = (char*)src_addr;

	SendCommand(&spi_command);

	WaitForDeviceReady();
}


/* ************************** GLOBAL FUNCTIONS ********************************/


/*
 * .KB_C_FN_DEFINITION_START
 * void SPI_ReadFlash(unsigned flash_addr, unsigned dest_addr, unsigned size)
 *  Global function to read the SPI flash device using the continuous read
 * array command.
 * .KB_C_FN_DEFINITION_END
 */
void SPI_ReadFlash(unsigned flash_addr, unsigned dest_addr, unsigned size) {

	unsigned	pageAddress, byteAddress;

	// determine page address
	pageAddress = flash_addr / FLASH_PAGE_SIZE;

	// determine byte address
	byteAddress = flash_addr % FLASH_PAGE_SIZE;

	p_memset(tx_commandBuffer, 0, 8);
	tx_commandBuffer[0] = CONTINUOUS_ARRAY_READ;
	tx_commandBuffer[1] = (pageAddress >> 6) & 0x3F;
	tx_commandBuffer[2] = ((pageAddress << 2) & 0xFC) |
				((byteAddress >> 8) & 0x3);
	tx_commandBuffer[3] = (byteAddress & 0xFF);

	p_memset(rx_commandBuffer, 0, 8);

	spi_command.tx_cmd = tx_commandBuffer;
	spi_command.rx_cmd = rx_commandBuffer;
	spi_command.rx_cmd_size = 8;
	spi_command.tx_cmd_size = 8;

	spi_command.tx_data_size = size;
	spi_command.tx_data = (char*)dest_addr;
	spi_command.rx_data_size = size;
	spi_command.rx_data = (char*)dest_addr;

	SendCommand(&spi_command);
}


/*
 * .KB_C_FN_DEFINITION_START
 * void SPI_WriteFlash(unsigned flash_addr, unsigned src_addr, unsigned size)
 *  Global function to program the SPI flash device.  Notice the warning
 * provided in lower-level functions regarding corruption of data in non-
 * page aligned write operations.
 * .KB_C_FN_DEFINITION_END
 */
void SPI_WriteFlash(unsigned flash_addr, unsigned src_addr, unsigned size) {

	unsigned	pageAddress, byteAddress, this_size;

	// determine page address
	pageAddress = flash_addr / FLASH_PAGE_SIZE;

	// determine byte address
	byteAddress = flash_addr % FLASH_PAGE_SIZE;

	while (size) {

		this_size = FLASH_PAGE_SIZE - byteAddress;
		if (this_size > size)
			this_size = size;

		// write through buffer to flash
		ProgramBuffer(pageAddress, byteAddress, src_addr, this_size);

		size -= this_size;
		src_addr += this_size;
		byteAddress = 0;
		++pageAddress;
	}
}


/*
 * .KB_C_FN_DEFINITION_START
 * int spiread_function_parse(int argc, char *argv[])
 *  This global function parses text from the command line.
 * .KB_C_FN_DEFINITION_END
 */
int spiread_function_parse(int argc, char *argv[]) {

	unsigned flash_addr, dest_addr, size;

	if (argc < 4) {
		DebugPrint("Missing parameter");
		return (1);
	}

	p_ASCIIToHex(argv[1], &flash_addr);
	p_ASCIIToHex(argv[2], &dest_addr);
	p_ASCIIToHex(argv[3], &size);
	SPI_ReadFlash(flash_addr, dest_addr, size);

	return (0);
}


/*
 * .KB_C_FN_DEFINITION_START
 * void spiread_function_help(void)
 *  This global displays help info for the corresponding command.
 * .KB_C_FN_DEFINITION_END
 */
void spiread_function_help(void) {
	DebugPrint("\tRead SPI Flash to memory\n\r");
	DebugPrint("\t\tspi_read <flash_addr> <dest_addr> <size>\n\r");
}


/*
 * .KB_C_FN_DEFINITION_START
 * int spiwrite_function_parse(int argc, char *argv[])
 *  This global function parses text from the command line.
 * .KB_C_FN_DEFINITION_END
 */
int spiwrite_function_parse(int argc, char *argv[]) {

	unsigned flash_addr, src_addr, size;

	if (argc < 4) {
		DebugPrint("Missing parameter");
		return (1);
	}

	p_ASCIIToHex(argv[1], &flash_addr);
	p_ASCIIToHex(argv[2], &src_addr);
	p_ASCIIToHex(argv[3], &size);
	SPI_WriteFlash(flash_addr, src_addr, size);

	return (0);
}


/*
 * .KB_C_FN_DEFINITION_START
 * void spiwrite_function_help(void)
 *  This global displays help info for the corresponding command.
 * .KB_C_FN_DEFINITION_END
 */
void spiwrite_function_help(void) {
	DebugPrint("\tWrite memory to SPI Flash\n\r");
	DebugPrint("\t\tspi_read <flash_addr> <src_addr> <size>\n\r");
}


/*
 * .KB_C_FN_DEFINITION_START
 * void SPI_InitFlash(void)
 *  Global function to initialize the SPI flash device/accessor functions.
 * .KB_C_FN_DEFINITION_END
 */
void SPI_InitFlash(void) {

	AT91PS_PIO	pPio;
	AT91PS_SPI	pSPI = AT91C_BASE_SPI;
	unsigned	value;
	char		initStatus;

	// enable CS0, CLK, MOSI, MISO
	pPio = (AT91PS_PIO)AT91C_BASE_PIOA;
	pPio->PIO_ASR = (((unsigned)AT91C_PA3_NPCS0) |
		((unsigned)AT91C_PA1_MOSI) |
		((unsigned)AT91C_PA0_MISO) |
		((unsigned)AT91C_PA2_SPCK));
	pPio->PIO_BSR = 0;
	pPio->PIO_PDR = (((unsigned)AT91C_PA3_NPCS0) |
		((unsigned)AT91C_PA1_MOSI) |
		((unsigned)AT91C_PA0_MISO) |
		((unsigned)AT91C_PA2_SPCK));

	// enable clocks to SPI
	AT91C_BASE_PMC->PMC_PCER = ((unsigned) 1 << AT91C_ID_SPI);

	// reset the SPI
	pSPI->SPI_CR = AT91C_SPI_SWRST;

	pSPI->SPI_MR = ((0xf << 24) | AT91C_SPI_MSTR | AT91C_SPI_MODFDIS | (0xE << 16));

	pSPI->SPI_CSR[0] = (AT91C_SPI_CPOL | (AT91C_SPI_DLYBS & (0x4 <<16 )) | (3 << 8));
	pSPI->SPI_CR = AT91C_SPI_SPIEN;

	pSPI->SPI_PTCR = AT91C_PDC_TXTDIS;
	pSPI->SPI_PTCR = AT91C_PDC_RXTDIS;
	pSPI->SPI_RNPR = 0;
	pSPI->SPI_RNCR = 0;
	pSPI->SPI_TNPR = 0;
	pSPI->SPI_TNCR = 0;
	pSPI->SPI_RPR = 0;
	pSPI->SPI_RCR = 0;
	pSPI->SPI_TPR = 0;
	pSPI->SPI_TCR = 0;
	pSPI->SPI_PTCR = AT91C_PDC_RXTEN;
	pSPI->SPI_PTCR = AT91C_PDC_TXTEN;

	value = pSPI->SPI_RDR;
	value = pSPI->SPI_SR;

	if (((initStatus = GetFlashStatus()) & 0xFC) != 0xAC) {
		DebugPrint(" Unexpected SPI flash status: ");
		DebugPrintHex(2, initStatus);
		DebugPrint("\n\r");
	}

	spiread_function.f_string = "spi_read";
	spiread_function.parse_function = spiread_function_parse;
	spiread_function.help_function = spiread_function_help;
	RegisterFunction(&spiread_function);

	spiwrite_function.f_string = "spi_write";
	spiwrite_function.parse_function = spiwrite_function_parse;
	spiwrite_function.help_function = spiwrite_function_help;
	RegisterFunction(&spiwrite_function);
}
