/** @file Temperature.h
 * Sample the TMP36 temperature sensor value.
 * @author Adrien RICCIARDI
 * @version 1.0 : 13/04/2014
 */
#ifndef H_TEMPERATURE_H
#define H_TEMPERATURE_H

#include <system.h>

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------
/** Enable the temperature sampling interrupt. */
#define TEMPERATURE_ENABLE_INTERRUPT() pie2.TMR3IE = 1

/** Disable the temperature sampling interrupt. */
#define TEMPERATURE_DISABLE_INTERRUPT() pie2.TMR3IE = 0

//--------------------------------------------------------------------------------------------------
// Functions
//--------------------------------------------------------------------------------------------------
/** Initialize the ADC channel and FVR module needed to read the temperature sensor value. */
void TemperatureInitialize(void);

/** Sample the temperature from the sensor.
 * @return The read temperature converted to Celsius degrees.
 */
signed char TemperatureReadValue(void);

/** Put the temperature module in low power mode.
 * @param Is_Low_Power_Enabled Set to 1 to enable low power mode or to 0 to run the module.
 */
void TemperatureSetLowPowerMode(unsigned char Is_Low_Power_Enabled);

#endif