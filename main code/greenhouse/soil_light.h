// soil_light.h
// cap soil moisture sensor v1.2 code
// and photoresistor code
// Made By: William Glass

#ifndef SOIL_LIGHT_H_
#define SOIL_LIGHT_H_


//int capSoilSensor_capture(void);

void soil_light_startReading(void);

int readSoil(void);

int readLight(void);

#endif /* SOIL_LIGHT_H_ */
