/** @file Processor.c
 * @see Processor.h for description.
 * @author Adrien RICCIARDI
 */
#include <system.h>
#include "Processor.h"

//--------------------------------------------------------------------------------------------------
// Public functions
//--------------------------------------------------------------------------------------------------
void ProcessorSetLowPowerMode(unsigned char Is_Low_Power_Enabled)
{
	// Go to idle mode
	if (Is_Low_Power_Enabled)
	{	
    	// Enable timer 1
        tmr1h = 0;
    	tmr1l = 0;
    	pir1.TMR1IF = 0; // Reset timer interrupt flag to avoid spurious triggering
    	pie1.TMR1IE = 1; // Enable timer 1 interrupt
    	t1con = 0x31; // 8x prescaler
    	
    	// Change oscillator frequency to 31KHz and enable idle mode
    	osccon = 0x80;

    	// Go to idle mode
    	asm sleep;
	}
	// Wake up system
	else
	{
	    // Disable timer 1
	    t1con = 0;
	    pie1.TMR1IE = 0; // Disable timer 1 interrupt
	
	    // Change oscillator frequency to 1MHz
    	osccon = 0x30;
	}
}