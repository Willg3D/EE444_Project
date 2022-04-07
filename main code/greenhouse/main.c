#include <msp430.h>
#include <soil_light.h>
#include "dht11Sensor.h"
#include "hal_MSP-EXP430F5438.h"

volatile float temp = 0.0f;
volatile char humid = 0;
volatile int soil = 0;
volatile int light = 0;


// Set Baud Rate and UART set up
void baudRate(void)
{
    P5SEL = BIT6 + BIT7;      // P5.6 (TxD) & P5.7 (RxD)
    UCA1CTL1 |= UCSWRST + UCSSEL_2; // Software reset en, SMCLK source
    UCA1BR0 = 104;              // 9600 bps from 1MHz CLK
    UCA1BR1 = 0;              // 9600 bps from 17MHz CLK
    UCA1MCTL |= UCBRS_1 + UCBRF_0 + UCOS16; // Modulation and Oversampling
    UCA1CTL1 &=  ~UCSWRST;    // Initialize USCI State Machine
    UCA1IE |= UCRXIE;         // Enable USCI_A1 Rx Interrupts
}


int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	clock_init();
	dht11_cap_init();
	halLcdInit();

	TA0CTL |= TASSEL__SMCLK + TACLR + ID_3 + MC__STOP + TAIE; // Timer A1 counts w/ clock, clear the count
	  //TA1EX0 |= TAIDEX_7; // divide clock by 8, make it SLOW, 4096 Hz
	TA0CCTL0 |= SCS; // Rising edge, sync w/ clock, interrupt enabled
	TA0CCR0 = 4096; // 4096Hz * 5 sec. = 13107 clocks


    //TA1CTL |= TACLR;            // clear timer
    //TA1CTL |= TASSEL__SMCLK;    // choose SMCLK
    //TA1CTL |= MC__CONTINUOUS;   // Set to Cont Mode;

    _EINT();

    while(1){
    //    __delay_cycles(2000000);      // for 200 msec delay

    }
	//LPM0;
}


#pragma vector = TIMER0_A0_VECTOR
__interrupt void clockCaptureISR(void){
  TA0CTL |= MC__STOP; // Stop the clock for capture
    //ADC12CTL0 |= ADC12SC+ADC12ENC; // Start conversion/sample
      dht11_startReading();
      humid = readHumid();
      temp = readTemp();

      soil_light_startReading();
      soil = readSoil();
      light = readLight();

      TA0CTL |= TACLR; // Clear the counter
      TA0CTL |= MC__UP;
}

// Outline of UART interrupt
#pragma vector = USCI_A1_VECTOR
__interrupt void USCO_A1_ISR(void){
  switch(UCA1IV) {
    case 0: break;
    case 2:                     // data received
      if (UCA1RXBUF != 0x0D){   // Not equal return ascii
        while(!(UCA1IFG & UCTXIFG));    // Is TX buff ready?
        UCA1TXBUF = UCA1RXBUF;          // Places RX into TX
      }
      else{
        //messageLength(charCount);       // Function to edit message
        while(!(UCA1IFG & UCTXIFG));    // Is TX buf ready?
        UCA1TXBUF = 0x0D;               // Return ascii to clear line
        UCA1TXBUF = 0x0D;               // Return ascii completely clear line
        //for(pos = 0; pos != messLength; pos++){ // loop to place message char into TX Buf
        //UCA1TXBUF = message[pos];
        //}
        //pos = 0;                        // restart pos to 0
      }
    case 4: break; // transmit buffer empty
    default: break;
  }
}
