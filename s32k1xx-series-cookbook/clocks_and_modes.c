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

/* include peripheral declarations S32K144 */
#include "device_registers.h"
#include "clocks_and_modes.h"

/*!
 * # Clocks and Modes
 */

void SOSC_init_8MHz(void)
{
	/*!
	 * ## SOSC Initialization (8 MHz)
	 */
  	/* SOSCDIV1 & SOSCDIV2 = 1: divide by 1*/
	SCG->SOSCDIV = SCG_SOSCDIV_SOSCDIV1(1) | SCG_SOSCDIV_SOSCDIV2(1);
	/*
	 * Range = 2: Medium freq (SOSC between 1MHz-8MHz)
	 * HGO = 0: Config XTAL OSC for low power
	 * EREFS = 1: Input is external XTAL
	 */
	SCG->SOSCCFG =	SCG_SOSCCFG_RANGE(2) | SCG_SOSCCFG_EREFS_MASK;

 	/* Ensure SOSCCSR unlocked */
	while(SCG->SOSCCSR & SCG_SOSCCSR_LK_MASK)
		;
	/*
	 * LK = 0: SOSCCSR can be written
	 * SOSCCMRE = 0: OSC CLK monitor IRQ if enabled
	 * SOSCCM = 0: OSC CLK monitor disabled
	 * SOSCERCLKEN = 0: OSC 3V ERCLK output clock disabled
	 * SOSCLPEN = 0: Sys OSC disabled in VLP modes
	 * SOSCSTEN = 0: Sys OSC disabled in Stop modes
	 * SOSCEN = 1: Enable oscillator
	 */
	SCG->SOSCCSR = SCG_SOSCCSR_SOSCEN_MASK;
	/* Wait for system OSC clock valid */
	while(!(SCG->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK))
		;
}

void SPLL_init_160MHz(void)
{
	/*!
	 * ## SPLL Initialization (160 MHz)
	 */
 	/* Ensure SPLLCSR unlocked */
	while (SCG->SPLLCSR & SCG_SPLLCSR_LK_MASK)
		;
  	/* SPLLEN = 0: SPLL is disabled (default) */
	SCG->SPLLCSR &= ~SCG_SPLLCSR_SPLLEN_MASK;

	/*
	 * SPLLDIV1 divide by 2
  	 * SPLLDIV2 divide by 4
	 */
	SCG->SPLLDIV |= SCG_SPLLDIV_SPLLDIV1(2) | SCG_SPLLDIV_SPLLDIV2(3);

	/* PREDIV = 0: Divide SOSC_CLK by 0 + 1 = 1 */
	/* MULT = 24: Multiply system PLL by 16 + 24 = 40 */
	/* SPLL_CLK = 8MHz / 1 * 40 / 2 = 160 MHz */
	SCG->SPLLCFG = SCG_SPLLCFG_MULT(24);

 	/* Ensure SPLLCSR unlocked */
	while(SCG->SPLLCSR & SCG_SPLLCSR_LK_MASK)
		;
	/*
	 * LK = 0:        SPLLCSR can be written
	 * SPLLCMRE = 0:  SPLL CLK monitor IRQ if enabled
	 * SPLLCM = 0:    SPLL CLK monitor disabled
	 * SPLLSTEN = 0:  SPLL disabled in Stop modes
	 * SPLLEN = 1:    Enable SPLL
	 */
	SCG->SPLLCSR |= SCG_SPLLCSR_SPLLEN_MASK;

	/* Wait for SPLL valid */
	while(!(SCG->SPLLCSR & SCG_SPLLCSR_SPLLVLD_MASK))
		;
}

void NormalRUNmode_80MHz(void)
{
	/*!
	 * ## Slow IRC
	 *
	 * - Slow IRC is enabled with high range (8 MHz) in reset.
	 *
	 * 	- Enable SIRCDIV2_CLK and SIRCDIV1_CLK, divide by 1 = 8MHz
	 *	  asynchronous clock source.
	 */
	SCG->SIRCDIV = SCG_SIRCDIV_SIRCDIV1(1) | SCG_SIRCDIV_SIRCDIV2(1);

	/*!
	 * ## Change to normal RUN mode with 8MHz SOSC, 80 MHz PLL
	 */
	/* Select PLL as clock source
	 * DIVCORE = 1, div. by 2: Core clock = 160/2 MHz = 80 MHz
	 * DIVBUS = 1, div. by 2: bus clock = 40 MHz
	 * DIVSLOW = 2, div. by 3: SCG slow, flash clock= 26 2/3 MHz
	 */
	SCG->RCCR = SCG_RCCR_SCS(6) | SCG_RCCR_DIVCORE(0b01)
		| SCG_RCCR_DIVBUS(0b01) | SCG_RCCR_DIVSLOW(0b10);

	/* Wait for SYS CLK source = SPLL */
	while (((SCG->CSR & SCG_CSR_SCS_MASK) >> SCG_CSR_SCS_SHIFT) != 6)
		;
}
