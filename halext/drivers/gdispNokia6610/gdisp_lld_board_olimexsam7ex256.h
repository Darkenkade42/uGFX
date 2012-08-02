/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes ChibiOS/RT, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
*/
/*
   Concepts and parts of this file have been contributed by:
		Joel Bodenmann aka Tectu	-> Maintainer
		Andrew Hannam aka inmarket	-> framework
		Badger						-> console implementation and FSMC
		Abhishek 					-> font rendering
		Ben William					-> fastMath and lcdDrawEllipse()
		Dongxu Li aka dxli			-> lcdDrawEllipse() filled option
*/

/**
 * @file    gdispNokia6610/gdisp_lld_board_olimexsam7ex256.h
 * @brief   GDISP Graphic Driver subsystem board interface for the Olimex SAM7-EX256 board.
 *
 * @addtogroup GDISP
 * @{
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

// ******************************************************
// Pointers to AT91SAM7X256 peripheral data structures
// ******************************************************
volatile AT91PS_PIO pPIOA = AT91C_BASE_PIOA;
volatile AT91PS_PIO pPIOB = AT91C_BASE_PIOB;
volatile AT91PS_SPI pSPI = AT91C_BASE_SPI0;
volatile AT91PS_PMC pPMC = AT91C_BASE_PMC;
volatile AT91PS_PDC pPDC = AT91C_BASE_PDC_SPI0;

/**
 * @brief   Initialise the board for the display.
 * @notes	Performs the following functions:
 *			1. initialise the spi port used by your display
 *			2. initialise the reset pin (initial state not-in-reset)
 *			3. initialise the chip select pin (initial state not-active)
 *			4. initialise the backlight pin (initial state back-light off)
 *
 * @notapi
 */
static __inline void gdisp_lld_init_board(void) {
	// *********************************************************************************************
	// InitSpi( )
	//
	// Sets up SPI channel 0 for communications to Nokia 6610 LCD Display
	//
	// I/O ports used:	PA2  = LCD Reset (set to low to reset)
	//					PA12 = LCD chip select (set to low to select the LCD chip)
	//					PA16 = SPI0_MISO Master In - Slave Out (not used in LCD interface)
	//					PA17 = SPI0_MOSI Master Out - Slave In pin (Serial Data to LCD slave)
	//					PA18 = SPI0_SPCK Serial Clock (to LCD slave)
	//					PB20 = backlight control (normally PWM control, 1 = full on)
	//
	// *********************************************************************************************}

	/* This code should really use the ChibiOS driver for these functions */

	// Pin for backlight
	pPIOB->PIO_CODR   = PIOB_LCD_BL_MASK;    // Set PB20 to LOW
	pPIOB->PIO_OER    = PIOB_LCD_BL_MASK;    // Configure PB20 as output

	// Reset pin
	pPIOA->PIO_SODR   = PIOA_LCD_RESET_MASK;     // Set PA2 to HIGH
	pPIOA->PIO_OER    = PIOA_LCD_RESET_MASK;     // Configure PA2 as output

	// CS pin - this seems to be ignored
	// pPIOA->PIO_SODR   = BIT12;     // Set PA2 to HIGH
	// pPIOA->PIO_OER    = BIT12;     // Configure PA2 as output

	// Init SPI0
	// Disable the following pins from PIO control (will be used instead by the SPI0 peripheral)
	// BIT12 = PA12 -> SPI0_NPCS0 chip select
	// BIT16 = PA16 -> SPI0_MISO Master In - Slave Out (not used in LCD interface)
	// BIT17 = PA17 -> SPI0_MOSI Master Out - Slave In pin (Serial Data to LCD slave)
	// BIT18 = PA18 -> SPI0_SPCK Serial Clock (to LCD slave)
	pPIOA->PIO_PDR = BIT12 | BIT16 | BIT17 | BIT18;
	pPIOA->PIO_ASR = BIT12 | BIT16 | BIT17 | BIT18;
	pPIOA->PIO_BSR = 0;

	//enable the clock of SPI
	pPMC->PMC_PCER = 1 << AT91C_ID_SPI0;

	// Fixed mode
	pSPI->SPI_CR      = 0x81;               //SPI Enable, Sowtware reset
	pSPI->SPI_CR      = 0x01;               //SPI Enable

	//pSPI->SPI_MR      = 0xE0019;            //Master mode, fixed select, disable decoder, FDIV=1 (MCK), PCS=1110
	pSPI->SPI_MR      = 0xE0011;          //Master mode, fixed select, disable decoder, FDIV=0 (MCK), PCS=1110

	//pSPI->SPI_CSR[0]  = 0x01010C11;           //9bit, CPOL=1, ClockPhase=0, SCLK = 48Mhz/32*12 = 125kHz
	pSPI->SPI_CSR[0]  = 0x01010311;        //9bit, CPOL=1, ClockPhase=0, SCLK = 48Mhz/8 = 6MHz if using commented MR line above
}

/**
 * @brief   Set or clear the lcd reset pin.
 *
 * @param[in] state		TRUE = lcd in reset, FALSE = normal operation
 * 
 * @notapi
 */
static __inline void gdisp_lld_setpin_reset(bool_t state) {
	if (state)
		palClearPad(IOPORT1, PIOA_LCD_RESET);
//		pPIOA->PIO_CODR = PIOA_LCD_RESET_MASK;
	else
		palSetPad(IOPORT1, PIOA_LCD_RESET);
//		pPIOA->PIO_SODR = PIOA_LCD_RESET_MASK;
}

/**
 * @brief   Set or clear the lcd back-light pin.
 *
 * @param[in] state		TRUE = lcd back-light on, FALSE = lcd back-light off
 * 
 * @notapi
 */
static __inline void gdisp_lld_setpin_backlight(bool_t state) {
	if (state)
		palSetPad(IOPORT2, PIOB_LCD_BL);
//		pPIOB->PIO_SODR = PIOB_LCD_BL_MASK;
	else
		palClearPad(IOPORT2, PIOB_LCD_BL);
//		pPIOB->PIO_CODR = PIOB_LCD_BL_MASK;
}

/**
 * @brief   Send a 9 bit command/data to the lcd.
 *
 * @param[in] data		The data to send
 * 
 * @notapi
 */
static __inline void gdisp_lld_write_spi(uint16_t data) {
	// wait for the previous transfer to complete
	while((pSPI->SPI_SR & AT91C_SPI_TXEMPTY) == 0);
	// send the data
	pSPI->SPI_TDR = data;
}

#if GDISP_HARDWARE_READPIXEL || GDISP_HARDWARE_SCROLL || defined(__DOXYGEN__)
/**
 * @brief   Read data from the lcd.
 *
 * @return	The data from the lcd
 * 
 * @notapi
 */
static __inline uint16_t gdisp_lld_read_spi(void) {
	#error "gdispNokia6610: GDISP_HARDWARE_READPIXEL and GDISP_HARDWARE_SCROLL are not supported on this board"
	return 0;
}
#endif

#endif /* _GDISP_LLD_BOARD_H */
/** @} */
