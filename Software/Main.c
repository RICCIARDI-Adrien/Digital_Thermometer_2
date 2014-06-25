/** @file Main.c
 * The digital thermometer version 2 main loop.
 * @author Adrien RICCIARDI
 * @version 1.0 : 09/06/2014
 */
#include <system.h>
#include "Processor.h"
#include "Screen.h"
#include "Temperature.h"

//--------------------------------------------------------------------------------------------------
// Private constants
//--------------------------------------------------------------------------------------------------
// PIC18F13K22 fuses
#pragma DATA _CONFIG1H, _IESO_OFF_1H & _FCMEN_OFF_1H & _PCLKEN_OFF_1H & _PLLEN_OFF_1H & _FOSC_IRC_1H // Disable Oscillator Switchover mode, disable Fail-Safe Clock Monitor, PLL and primary clock are under software control, select Internal RC Oscillator
#pragma DATA _CONFIG2L, _BORV_27_2L & _BOREN_NOSLP_2L & _PWRTEN_OFF_2L // Set Brown-out Reset voltage to 2.5V, enable Brown-out Reset in hardware only and disable it in Sleep mode, disable Power-up Timer
#pragma DATA _CONFIG2H, _WDTEN_OFF_2H // Disable Watchdog Timer
#pragma DATA _CONFIG3H, _MCLRE_OFF_3H & _HFOFST_OFF_3H // Enable RA3 pin, wait for the oscillator to become stable before booting the CPU core
#pragma DATA _CONFIG4L, _DEBUG_OFF_4L & _XINST_OFF_4L & _BBSIZ_OFF_4L & _LVP_OFF_4L & _STVREN_OFF_4L // Disable debug, CPU in legacy mode, 512-word boot block size, disable Low-Voltage Programming, disable stack related interrupts
#pragma DATA _CONFIG5L, _CP1_OFF_5L & _CP0_OFF_5L // Disable all blocks code protection
#pragma DATA _CONFIG5H, _CPD_OFF_5H & _CPB_OFF_5H // Disable data EEPROM and Boot Block code protection
#pragma DATA _CONFIG6L, _WRT1_OFF_6L & _WRT0_OFF_6L // Disable all blocks write protection
#pragma DATA _CONFIG6H, _WRTD_OFF_6H & _WRTB_OFF_6H & _WRTC_OFF_6H // Enable writing to data EEPROM, disable Boot Block and Configuration Registers write protections
#pragma DATA _CONFIG7L, _EBTR0_OFF_7L & _EBTR1_OFF_7L // Disable all table read protections
#pragma DATA _CONFIG7H, _EBTRB_OFF_7H // Disable Boot Block table read protection

// Clock frequency
#pragma CLOCK_FREQ 1000000

// The available leds
#define PIN_LED_MAXIMUM_TEMPERATURE porta.RA5
#define PIN_LED_CURRENT_TEMPERATURE porta.RA4
#define PIN_LED_MINIMUM_TEMPERATURE portb.RB7

// The button
#define PIN_BUTTON porta.RA0

//--------------------------------------------------------------------------------------------------
// Private types
//--------------------------------------------------------------------------------------------------
/** All the state machine states. */
typedef enum
{
	STATE_MAXIMUM_TEMPERATURE,
	STATE_CURRENT_TEMPERATURE,
	STATE_MINIMUM_TEMPERATURE,
	STATE_SLEEP
} TState;

//--------------------------------------------------------------------------------------------------
// Private variables
//--------------------------------------------------------------------------------------------------
/** The state machine current state. */
static TState Current_State = STATE_CURRENT_TEMPERATURE;

/** The three types of sampled temperatures. */
static signed char Maximum_Temperature = -128, Current_Temperature, Minimum_Temperature = 127;

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Convert a signed 8-bit value to the two character codes representing it.
 * @param Value The value to convert.
 * @param Pointer_Left_Character On output, will contain the left character code.
 * @param Pointer_Right_Character On output, will contain the right character code.
 * @note Displays "--" if the provided value is lesser than -9 or greater than 99.
 */
static void ConvertIntegerToCharacterCodes(signed char Value, unsigned char *Pointer_Left_Character, unsigned char *Pointer_Right_Character)
{
	unsigned char Divided_By_Ten;
	
	// Check if value is in bounds
	if ((Value < -9) || (Value > 99))
	{
		*Pointer_Left_Character = SCREEN_CHARACTER_CODE_MINUS;
		*Pointer_Right_Character = SCREEN_CHARACTER_CODE_MINUS;
		return;
	}
	
	// Convert leftmost character
	if (Value < 0)
	{
		*Pointer_Left_Character = SCREEN_CHARACTER_CODE_MINUS;
		Value = -Value; // Set value as positive
	}		
	else if (Value >= 10)
	{
		// Do a simple euclidian division to get the decades
		Divided_By_Ten = Value / 10;
		*Pointer_Left_Character = Divided_By_Ten;
		Value -= (Divided_By_Ten << 3) + (Divided_By_Ten << 1); // Remove the decades from the original value by substracting (decades * 10)
	}
	else *Pointer_Left_Character = SCREEN_CHARACTER_CODE_EMPTY;
	
	*Pointer_Right_Character = Value;	
}

/** Read the temperature and update peaks. */
static void ReadTemperature(void)
{
	Current_Temperature = TemperatureReadValue();
	if (Current_Temperature < Minimum_Temperature) Minimum_Temperature = Current_Temperature;
	else if (Current_Temperature > Maximum_Temperature) Maximum_Temperature = Current_Temperature;
}

/** Display the temperature corresponding to the requested state.
 * @param State_To_Display Which state to display.
 */
static void DisplayStateTemperature(TState State_To_Display)
{
	unsigned char Left_Character, Right_Character;
	signed char Temperature_To_Display;
	
	// Select the requested temperature to display
	switch (State_To_Display)
	{
		case STATE_MAXIMUM_TEMPERATURE:
			Temperature_To_Display = Maximum_Temperature;
			break;
			
		case STATE_MINIMUM_TEMPERATURE:
			Temperature_To_Display = Minimum_Temperature;
			break;
			
		default:
			Temperature_To_Display = Current_Temperature;
			break;
	}
	
	// Display the requested temperature
	ConvertIntegerToCharacterCodes(Temperature_To_Display, &Left_Character, &Right_Character);
	ScreenSetDisplayedCharacters(Left_Character, Right_Character);
}

/** Suppress spurious button contacts. */
static void ButtonDebounceTimer(void)
{
	while (PIN_BUTTON); // Wait until the button is released
	delay_ms(50); // First debounce timer
	
	while (PIN_BUTTON) delay_ms(1); // As many other debounce timer as needed
}

//--------------------------------------------------------------------------------------------------
// Interrupts handler
//--------------------------------------------------------------------------------------------------
/** Handle all interrupts (they are classified by usage rate). */
void interrupt(void)
{
	// Screen refresh (Timer 0)
	if ((intcon.TMR0IE) && (intcon.TMR0IF))
	{
		ScreenRefresh();
		intcon.TMR0IF = 0;
	}
	
	// Temperature sampling (Timer 3)
    if ((pie2.TMR3IE) && (pir2.TMR3IF))
    {
		ReadTemperature();
		DisplayStateTemperature(Current_State); // Update displayed temperature (this function is called even if the temperature to display has not changed, but it is simpler this way)
		pir2.TMR3IF = 0;
    }
	
    // Wake-up timer (Timer 1)
    if ((pie1.TMR1IE) && (pir1.TMR1IF))
    {
		// Wake up processor and temperature modules
		ProcessorSetLowPowerMode(0); // Reenable processor full speed prior any other thing
		TemperatureSetLowPowerMode(0);
		
        ReadTemperature();
        
        // Reenable interrupt before returning to low power mode
        pir1.TMR1IF = 0;
        
        // Return to low power mode
        TemperatureSetLowPowerMode(1);
        ProcessorSetLowPowerMode(1);
    }
    
    // Button (triggered only when the system is in low power mode)
    if ((intcon.INT0IE) && (intcon.INT0IF))
    {
		// Reenable processor full speed
		ProcessorSetLowPowerMode(0); 
		intcon.INT0IF = 0;
	}	
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void main(void)
{
	// Initialize leds (configure leds' pins as digital outputs)
	ansel.AN3 = 0; // RA4
	trisa.RA4 = 0;
	trisa.RA5 = 0;
	trisb.RB7 = 0;
	
	// Indicate current temperature displaying
	PIN_LED_MAXIMUM_TEMPERATURE = 0;
	PIN_LED_CURRENT_TEMPERATURE = 1;
	PIN_LED_MINIMUM_TEMPERATURE = 0;
	
	// Initialize the button (configure the button pin as digital input)
	ansel.AN0 = 0;
	trisa.RA0 = 1;

	// Configure modules
	ScreenInitialize();
	TemperatureInitialize();

	// Enable interrupts
	intcon.PEIE = 1;
	intcon.GIE = 1;
     
	while (1)
	{	
		// Wait for the button to be pressed
		while (!PIN_BUTTON);
		
		Current_State++;
		if (Current_State > STATE_SLEEP) Current_State = STATE_MAXIMUM_TEMPERATURE;
		
		switch (Current_State)
		{
			case STATE_MAXIMUM_TEMPERATURE:
				PIN_LED_MAXIMUM_TEMPERATURE = 1;
				PIN_LED_CURRENT_TEMPERATURE = 0;
				PIN_LED_MINIMUM_TEMPERATURE = 0;
				break;
				
			case STATE_CURRENT_TEMPERATURE:
				PIN_LED_MAXIMUM_TEMPERATURE = 0;
				PIN_LED_CURRENT_TEMPERATURE = 1;
				PIN_LED_MINIMUM_TEMPERATURE = 0;
				break;
				
			case STATE_MINIMUM_TEMPERATURE:
				PIN_LED_MAXIMUM_TEMPERATURE = 0;
				PIN_LED_CURRENT_TEMPERATURE = 0;
				PIN_LED_MINIMUM_TEMPERATURE = 1;
				break;
				
			case STATE_SLEEP:
				// Clear the leds
				PIN_LED_MAXIMUM_TEMPERATURE = 0;
				PIN_LED_CURRENT_TEMPERATURE = 0;
				PIN_LED_MINIMUM_TEMPERATURE = 0;
				
				// Put the maximum modules in low power mode 
				ScreenSetLowPowerMode(1); // Clear the screen
				TemperatureSetLowPowerMode(1);
				
				// Debounce the button now to safely enable the button interrupt just after without spurious button press
				ButtonDebounceTimer();
				
				// Enable the button interrupt to allow to wake up the system
				intcon.INT0IF = 0; // Clear interrupt flag as it was previously set when pushing the button
				intcon.INT0IE = 1;
				
				// Put the whole system in idle mode, only an interrupt can wake it
				ProcessorSetLowPowerMode(1);
				
				// The following code is executed when the processor wakes up
				// The button interrupt occured
				intcon.INT0IE = 0; // Disable the button interrupt
				
				// Reenable all modules
				TemperatureSetLowPowerMode(0);
				ScreenSetLowPowerMode(0);
				
				// Force the next state temperature displaying to avoid a glitch which can occur during when the screen is reenabled and the 
				DisplayStateTemperature(STATE_MAXIMUM_TEMPERATURE);
				
				// Here the button is still pushed, so the state machine will be reentered and the next state will automatically be selected
				continue;
		}
		
		// Show the requested temperature
		TEMPERATURE_DISABLE_INTERRUPT(); // Use a "mutex" to avoid the sampling interrupt and this call changing both the display values in a random way
		DisplayStateTemperature(Current_State);
		TEMPERATURE_ENABLE_INTERRUPT();
		
		ButtonDebounceTimer();
	}
}