// soil_light.h
// cap soil moisture sensor v1.2 code
// and photoresistor code
// Made By: William Glass
// Based On: Adam Owens ADC Code


#include <msp430.h>
#include "soil_light.h"

    int calibratedSlope = 0; // NOT USED YET
    int avgSoilData = 0;
    int avgLightData = 0;



void soil_light_startReading(void){
    //avgSoilData = 0;
    //avgLightData = 0;

    P8DIR |= BIT6; // 3.3V output signal
    P8OUT |= BIT6;
    //P7SEL |= BIT5; // A13

  REFCTL0 |= REFON+REFVSEL_3; // Vref  = 2.5V

  // turn on ADC, 256 clk per sample, multiple sample conversion,
  // conversion overflow interrupt en, memory overflow interrupt en
  ADC12CTL0 |= ADC12ON + ADC12SHT03 + ADC12MSC +  ADC12TOVIE + ADC12OVIE;

  // choose SMCLK,single channel, A7 analog input
  ADC12CTL1 |= ADC12SSEL_3 + ADC12CONSEQ_1 + ADC12SHP;
  ADC12CTL2 |= ADC12RES_3;

  // Soil ADC Range
  // ADC12INCH_7 is A7 (port 6.7)
  ADC12MCTL0 |= ADC12SREF_1+ADC12INCH_7;
  ADC12MCTL1 |= ADC12SREF_1+ADC12INCH_7;
  ADC12MCTL2 |= ADC12SREF_1+ADC12INCH_7;
  ADC12MCTL3 |= ADC12SREF_1+ADC12INCH_7;


  // Light ADC Range
  // ADC12INCH_7 is A13 (port 7.5)
  ADC12MCTL4 |= ADC12SREF_1+ADC12INCH_13;
  ADC12MCTL5 |= ADC12SREF_1+ADC12INCH_13;
  ADC12MCTL6 |= ADC12SREF_1+ADC12INCH_13;
  // End of sequence
  ADC12MCTL7 |= ADC12SREF_1+ADC12INCH_7+ADC12EOS;
  ADC12IE |= ADC12IE7; // en interrupt at mem7



  ADC12CTL0 |= ADC12SC+ADC12ENC; // Start conversion/sample

  ADC12CTL0 &= ~ADC12ON; // Turn Off ADC for power Saving

}

// returns soil value (not calibrated)
int readSoil(void){
    avgSoilData = avgSoilData/4;
    return avgSoilData;
}

// returns light value (not calibrated)
int readLight(void){
    avgLightData = avgLightData/4;
    return avgLightData;
}

// Uses ADC12_VECTOR - When ADC values are sampled/written
#pragma vector = ADC12_VECTOR
__interrupt void dataReadingISR(void){
  switch(ADC12IV){
    case 0x14: // interrupt and ADC12MCTL7
        // average Soil
        avgSoilData = ADC12MEM0+ADC12MEM1+ADC12MEM2+ADC12MEM3;

        // average Light
        avgLightData = ADC12MEM4+ADC12MEM5+ADC12MEM6+ADC12MEM7;
      break;
    default:
     break;
  }

}

