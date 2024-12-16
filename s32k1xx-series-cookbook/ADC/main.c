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
 * # ADC - SW Trigger
 *
 * ## Description
 *
 * - The ADC is initialized to convert two channels using software triggers that are configured for one-shot conversions.
 *
 * - Each conversion requires its own software trigger.
 *
 * - One channel (AD12) connects to a potentiometer on the EVB the other to VREFSH.
 * 
 * - The results are scaled 0 to 5000 mV.
 */

#include "device_registers.h" 
#include "clocks_and_modes.h"
#include "ADC.h"

/* Port D0: FRDM EVB output to blue LED 	*/
#define PTD0 0
/* Port D15: FRDM EVB output to red LED 	*/
#define PTD15 15
/* Port D16: FRDM EVB output to green LED 	*/
#define PTD16 16

void PORT_init (void)
{
	/*!
	 * ## Pins definitions
	 *
	 * | Pin number | Function         |
	 * | ---------- |----------------- |
	 * | PTD0       | GPIO [BLUE LED]  |
	 * | PTD15      | GPIO [RED LED]   |
	 * | PTD16      | GPIO [GREEN LED] |
	 */
	/* Enable clock for PORTD */
	PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;
	/* Port D0, D15, D16: MUX = GPIO  */
	PORTD->PCR[PTD0] = PORT_PCR_MUX(1);
	PORTD->PCR[PTD15] = PORT_PCR_MUX(1);
	PORTD->PCR[PTD16] = PORT_PCR_MUX(1);

	/* Port D0, D15, D16: data direction = output */
	PTD->PDDR |= 1 << PTD0| 1 << PTD15 | 1 << PTD16;
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
	 * ADC0 Result in mili volts
	 */
	uint32_t adcResultInMv = 0;

	/*!
	 * ## Initialization
	 */
	/* Disable WDOG */
	WDOG_disable();
	/* Initialize system oscilator for 8 MHz xtal */
	SOSC_init_8MHz();
	/* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	SPLL_init_160MHz();
	/* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
	NormalRUNmode_80MHz();
	PORT_init();
	/* Init ADC resolution 12 bit */
	ADC_init();

	/*!
	 * ## Infinite loop
	 */
	while (1) {
		/* Convert Channel AD12 to pot on EVB 	*/
		convertAdcChan(12);
		/* Wait for conversion complete flag 	*/
		while(adc_complete()==0)
			;
		/* Get channel's conversion results in mv */
		adcResultInMv = read_adc_chx();

		/* If result > 3.75V 		*/
		if (adcResultInMv > 3750) {
			/* Turn off blue, green LEDs */
			PTD->PSOR |= 1 << PTD0 | 1 << PTD16;
			/* Turn on red LED */
			PTD->PCOR |= 1 << PTD15;
		}
		/* If result > 2.50V */
		else if (adcResultInMv > 2500) {
			/* Turn off blue, red LEDs */
			PTD->PSOR |= 1<<PTD0 | 1<<PTD15;
			/* Turn on green LED */
			PTD->PCOR |= 1<<PTD16;
		}
		/* If result > 1.25V */
		else if (adcResultInMv >1250) {
			/* Turn off red, green LEDs */
			PTD->PSOR |= 1<<PTD15 | 1<<PTD16;
			/* Turn on blue LED */
			PTD->PCOR |= 1<<PTD0;
		} else
			/* Turn off all LEDs */
			PTD->PSOR |= 1 << PTD0 | 1 << PTD15 | 1 << PTD16;

		/* Convert chan 29, Vrefsh */
		convertAdcChan(29);
		/* Wait for conversion complete flag */
		while (adc_complete() == 0)
			;
		/* Get channel's conversion results in mV */
		adcResultInMv = read_adc_chx();
	}
}
