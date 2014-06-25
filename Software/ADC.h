/** @file ADC.h
 * Read the battery and the temperature sensor values using two Analog to Digital Converter channels.
 * @author Adrien RICCIARDI
 * @version 1.0 : 09/06/2014
 */
#ifndef H_ADC_H
#define H_ADC_H

/** Initialize the ADC module. */
void ADCInitialize(void);

/** Put the ADC module in sleep mode to save power.
 * @param Is_Low_Power_Required Set to 1 to enable low power mode or to 0 to wake up the module.
 */
void ADCSetPowerMode(unsigned char Is_Low_Power_Required);

/** Sample the temperature sensor voltage value.
 * @return The temperature sensor value in range [0..1023].
 */
unsigned short ADCReadTemperatureValue(void);

/** Sample the battery voltage value.
 * @return The battery value in range [0..1023].
 */
unsigned short ADCReadBatteryVoltageValue(void);

#endif