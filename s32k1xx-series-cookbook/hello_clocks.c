/*
 * Copyright (c) 2014 - 2016, Freescale Semiconductor, Inc.
 * Copyright (c) 2016 - 2018, NXP.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!
 * # Hello World + Clocks
 *
 * ## Description
 *
 * - This project provides common initialization for clocks and an LPIT channel counter function.
 *
 * - Core clock is set to 80 MHz.
 *
 * - LPIT0 channel 0 is configured to count one second of SPLL clocks.
 *
 * - Software polls the channels timeout flag and toggles the GPIO output to the LED when the flag sets.
 */

/* Include peripheral declarations S32K144 */
#include "device_registers.h"  
#include "clocks_and_modes.h"

/*!
 * LPIT0 timeout counter
 */
int lpit0_ch0_flag_counter = 0; 

void PORT_init (void)
{
	/*!
	 * ## Pins definitions
	 *
	 * | Pin number | Function        |
	 * | -----------| --------------- |
	 * | PTD0       | GPIO [BLUE LED] |
	 */

	/* Enable clock for PORT D */
	PCC->PCCn[PCC_PORTD_INDEX] = PCC_PCCn_CGC_MASK;

	/* Port D0: data direction = output */
	PTD->PDDR |= 1 << 0;
	/* Port D0: MUX = ALT1, GPIO (to blue LED on EVB) */
	PORTD->PCR[0] |= PORT_PCR_MUX(1);
}

void LPIT0_init(void)
{
	/*!
	 * ## LPIT Clocking
	 */
	/* Clock source = 6 (SPLL2_DIV2_CLK)*/
	PCC->PCCn[PCC_LPIT_INDEX] = PCC_PCCn_PCS(6);
	/* Enable clock to LPIT0 regs */
	PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK;

	/*!
	 * - LPIT Initialization:
	 */
	/*
	 * DBG_EN = 0: Timer chans stop in Debug mode
	 * DOZE_EN = 0: Timer chans are stopped in DOZE mode
	 * SW_RST = 0: SW reset does not reset timer chans, regs
	 * M_CEN = 1: enable module clk (allows writing other LPIT0 regs)
	 */
	LPIT0->MCR |= LPIT_MCR_M_CEN_MASK;
	/* Chan 0 Timeout period: 40M clocks */
	LPIT0->TMR[0].TVAL = 40000000;
	/*
	 * T_EN = 1: Timer channel is enabled
	 * CHAIN = 0: channel chaining is disabled
	 * MODE = 0: 32 periodic counter mode
	 * TSOT = 0: Timer decrements immediately based on restart
	 * TSOI = 0: Timer does not stop after timeout
	 * TROT = 0 Timer will not reload on trigger
	 * TRG_SRC = 0: External trigger soruce
	 * TRG_SEL = 0: Timer chan 0 trigger source is selected
	 */
	LPIT0->TMR[0].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK;
}

void WDOG_disable(void)
{
	/* Unlock watchdog */
	WDOG->CNT=0xD928C520;
	/* Maximum timeout value */
	WDOG->TOVAL=0x0000FFFF;
	/* Disable watchdog */
	WDOG->CS = 0x00002100;
}

int main(void)
{
	/*!
	 * ## Initialization
	 */
	/* Disable WDOG */
	WDOG_disable();
	PORT_init();
	/* Initialize system oscilator for 8 MHz xtal */
	SOSC_init_8MHz();
	/* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	SPLL_init_160MHz();
	/* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
	NormalRUNmode_80MHz();
	/* Initialize PIT0 for 1 second timeout  */
	LPIT0_init();

	/*!
	 * ## Infinite loop
	 */
	while (1) {
		/* Toggle output to LED every LPIT0 timeout */
		/* Wait for LPIT0 CH0 Flag */
		while (0 == (LPIT0->MSR & LPIT_MSR_TIF0_MASK))
			;
		/* Increment LPIT0 timeout counter */
		lpit0_ch0_flag_counter++;
                /* Toggle output on port D0 (blue LED) */
		PTD->PTOR |= 1 << 0;
		/* Clear LPIT0 timer flag 0 */
		LPIT0->MSR |= LPIT_MSR_TIF0_MASK;
	}
}
