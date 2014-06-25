/** @file Temperature.c
 * @see Temperature.h for description.
 * @author Adrien RICCIARDI
 */
#include <system.h>
#include "Temperature.h"

//--------------------------------------------------------------------------------------------------
// Private functions
//--------------------------------------------------------------------------------------------------
/** Initialize Fixed Voltage Reference device at 1,024V. */
inline void TemperatureEnableFixedVoltageReference(void)
{
	vrefcon1 = 0; // Disable DAC1
	vrefcon0 = 0x90; // Enable FVR at 1,024V
	while (!vrefcon0.FVR1ST); // Wait for the voltage to become stable
}

/** Disable the Fixed Voltage Reference device to save power. */
inline void TemperatureDisableFixedVoltageReference(void)
{
	vrefcon0.FVR1EN = 0;
}

/** Read a 10-bit value from the temperature sensor ADC channel. */
inline unsigned short TemperatureADCReadValue(void)
{
	unsigned short Result;

	// Sample analog value
	adcon0.GO = 1; // Start conversion
	while (adcon0.GO); // Wait for the conversion to finish

	return (adresh << 8) | adresl;
}

//--------------------------------------------------------------------------------------------------
// Public functions
//--------------------------------------------------------------------------------------------------
void TemperatureInitialize(void)
{
	// Initialize ADC pin as analog input
	ansel.AN1 = 1;
	trisa.RA1 = 1;
	
	// Configure ADC module
	adcon2 = 0x88; // Conversion result is right justified, charge the sampling capacitor for almost 2 Tad, use a Fosc/2 conversion clock
	adcon1 = 0x08; // Positive reference voltage comes from Fixed Voltage Reference, negative voltage is Vss
	adcon0 = 0x04; // Select channel 1
	
	// Configure Timer 3 to generate a 1Hz interrupt
	// The timer is clocked from the internal frequency divided by 4, so 250KHz
	// Dividing this frequency by 65536 and a prescaler of 4 gives a 250000 / 65536 / 4 = 0.954 Hz
	t3con = 0x20; // Select internal clock and a prescaler of 4, do not enable the timer
	
	// Enable module
	TemperatureSetLowPowerMode(0);
}

signed char TemperatureReadValue(void)
{
	signed short Value;
	
	// Sample voltage
	Value = (signed short) TemperatureADCReadValue();
	
	// The TMP36 generates an output voltage of 10mV/°C with an offset of 500mV for 0°C
	// The ADC is configured to sample voltages from 0 to 1,024V by mapping these values from 0 to 1023
	// Thus, the thermometer can theoritically measures temperatures from -50°C to 102°C
	Value -= 500; // Remove TMP36 offset of 500mV
	Value /= 10; // Convert to Celsius degrees
	
	return (signed char) Value;
}

void TemperatureSetLowPowerMode(unsigned char Is_Low_Power_Enabled)
{
	if (Is_Low_Power_Enabled)
	{
		// Disable timer 3
		t3con.TMR3ON = 0; // Stop timer
		pie2.TMR3IE = 0; // Disable timer interrupt
		
		TemperatureDisableFixedVoltageReference();
		adcon0.ADON = 0; // Stop ADC module
	}
	else
	{
		TemperatureEnableFixedVoltageReference();
		adcon0.ADON = 1; // Enable ADC module
		
		// Reenable temperature sampling timer (force the interrupt triggering to immediately sample the temperature)
		tmr3h = 255;
		tmr3l = 255;
		pir2.TMR3IF = 0; // Reset timer interrupt flag to avoid spurious interrupt triggering
		pie2.TMR3IE = 1; // Enable timer interrupt
		t3con.TMR3ON = 1;
	}
}