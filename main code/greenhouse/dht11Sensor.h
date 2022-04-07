// dht11Sensor.h
// dht11 sensor code
// Made By: William Glass
// Based On: Selim Gullulu dht11_MSP430G2553 Code
// Link to dht11_MSP430G2553 Code:
// https://github.com/selimg76/microcontroller/blob/master/dht11_MSP430G2553

#ifndef DHT11_SENSOR_H
#define DHT11_SENSOR_H

// SMCLK initiate to 1 MHz
void clock_init(void);

// Simple func to set timer and capture for initial run.
void dht11_cap_init(void);

// triggers dht11 sensor to send data to MSP430
void dht11_startReading(void);

// Used in main to trigger and interpret dht11 by one function call

// Returns temperature data (float)
float readTemp(void);

// returns humidity data (char)
char readHumid(void);

//void button_init(void);

//void dht11_trigger(void);

//int interpret_timeDiff(void);

#endif
