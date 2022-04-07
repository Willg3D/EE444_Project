// dht11Sensor.c
// dht11 sensor code
// Made By: William Glass
// Based On: Selim Gullulu dht11_MSP430G2553 Code
// Link to dht11_MSP430G2553 Code:
// https://github.com/selimg76/microcontroller/blob/master/dht11_MSP430G2553

#include <msp430.h>


// Arrays to hold falling edge time captures
volatile int timeCap[48];   // array for captured data (extra byte of data added)
volatile int timeDiff[48];  // array for time stamp differences (extra byte of data added)

// Position Variables for Arrays
volatile unsigned int timeCap_pos=0;
volatile unsigned int timeDiff_pos=0;

// Recorded Values (Determined from timeDiff[])
volatile unsigned char h_wholeNum = 0;
volatile unsigned char h_deciNum = 0;
volatile unsigned char t_wholeNum = 0;
volatile unsigned char t_deciNum = 0;
volatile unsigned char checksum = 0;

// Calculated Value
volatile unsigned char calc_checksum = 0;

// SMCLK initiate to 1 MHz
void clock_init(void)
{
    UCSCTL1 = DCORSEL_1;        // DCO SEL Value for ideal CLK range
    UCSCTL2 = 30;               // DCO Div Value to change 32 KHz to 1 MHz
    UCSCTL3 = SELREF__XT1CLK;   // Set Ref Clock
    UCSCTL4 = SELM__DCOCLK + SELS__DCOCLK;// Selects DCOCLK for MCL and SMCLK
    P11DIR |= BIT2;             // Enable SMCLK test Pins
    P11SEL |= BIT2;
}

// Using Timer A1 for dht11 (clear it, SMCLK source, and continue mode)
void timer_init(void)
{
    TA1CTL |= TACLR;            // clear timer
    TA1CTL |= TASSEL__SMCLK;    // choose SMCLK
    TA1CTL |= MC__CONTINUOUS;   // Set to Cont Mode;
}

// Sets up capture registers
void capture_init(void)
{
    TA1CCTL0 |= CAP;            // Puts in Capture Mode
    TA1CCTL0 |= CCIE;           // Capture/compare interrupt enable
    TA1CCTL0 |= CCIS_1;         // Capture input select: 1 - CCI1B */
    TA1CCTL0 |= CM_2;           // Capture mode: 1 - neg. edge
    TA1CCTL0 |= SCS;            // Capture sychronize
}

// Simple func to set timer and capture for initial run.
void dht11_cap_init(void)
{
    timer_init();
    capture_init();
}


void button_init(void)
{
    P2DIR &=  ~BIT6 + ~BIT7;    // Set Button (Port 2.6 & 2.7) as input
    P2REN |= BIT6 + BIT7;       // Pull resistor;
    P2OUT |= BIT6 + BIT7;       // Enable Pull Up Resistor

    P2IE  |= BIT6 + BIT7;       // Enables interrupts on Port 2.6 & 2.7
    P2IES |= BIT6 + BIT7;       // High to Low transition on interrupt edge
    P2IFG &= ~BIT6 + BIT7;      // Clears interrupt register for Port 2.6 & 2.7
}

// triggers dht11 sensor to send data to MSP430
void dht11_trigger(void)
{
    timeCap_pos = 0;               // need to reset for capture
    capture_init();


    P8SEL &= ~BIT5;             // set up 8.5 for i/o
    P8DIR |= BIT5;              // set 8.5 as output port
    P8OUT &= ~BIT5;             // set port 8.5 to low
    __delay_cycles(20000);      // for 20 msec delay
    P8OUT |= BIT5;              // Pull High
    __delay_cycles(20);         // for 20 microsec delay
    P8DIR &= ~BIT5;             // set 8.5 as input port
    P8SEL |= BIT5;              // setup 8.5 for capture
}

int interpret_timeDiff(void)
{
    // Need to clear values
    h_wholeNum = 0;
    h_deciNum = 0;
    t_wholeNum = 0;
    t_deciNum = 0;
    checksum = 0;

    // Quick Check Less than 5 bytes (40 bits) of data
    // if more than a bad reading Occurred
    if (timeCap_pos >= 40){
        // Whole Humidity Values (1st byte)
        // ignoring timeDiff[0] because bad data
        for (timeDiff_pos = 1; timeDiff_pos <= 8; timeDiff_pos++){
            if (timeDiff[timeDiff_pos] >= 110)
                h_wholeNum |= (0x01 << (8-timeDiff_pos));
         }
        // Decimal Humidity Values (2nd byte)
        // Not really need for dht11 since it does not return any values
         for (timeDiff_pos = 9; timeDiff_pos <= 16; timeDiff_pos++){
             if (timeDiff[timeDiff_pos] >= 110)
                 h_deciNum |= (0x01 << (16-timeDiff_pos));
         }
         // Whole Temp Values (3rd byte)
         for (timeDiff_pos = 17; timeDiff_pos <= 24; timeDiff_pos++){
             if (timeDiff[timeDiff_pos] >= 110)
                 t_wholeNum |= (0x01 << (24-timeDiff_pos));
         }
         // Decimal Temp Values (4th byte)
         for (timeDiff_pos = 25; timeDiff_pos <= 32; timeDiff_pos++){
             if (timeDiff[timeDiff_pos] >= 110)
                 t_deciNum |= (0x01 << (32-timeDiff_pos));
          }
         // Checksum Values (5th byte)
          for (timeDiff_pos = 33; timeDiff_pos <= 40; timeDiff_pos++){
              if (timeDiff[timeDiff_pos] >= 110)
                  checksum |= (0x01 << (40-timeDiff_pos));
          }
          calc_checksum = h_wholeNum + h_deciNum + t_wholeNum + t_deciNum;
               if (calc_checksum != checksum)
                   return 1;
          }
    else{
        return 1;
    }

    return 0;
}

// Used in main to trigger and interpret dht11 by one function call
void dht11_startReading(void)
{
    // Note: DHT11 can not continuously sample (must have a delay between samples)
    dht11_trigger();
    __delay_cycles(200000);      // for 200 msec delay
    interpret_timeDiff();
}

// Returns temperature data (float)
float readTemp(void)
{
    float tempValue = t_deciNum;
    tempValue/=100;
	return t_wholeNum+tempValue;
}

// returns humidity data (char)
char readHumid(void)
{
	return h_wholeNum;
}


#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer_A0(void){
    timeCap[timeCap_pos] = TA1CCR0;
    timeCap_pos++;
    if (timeCap_pos>=2){
        timeDiff[timeCap_pos-2] = timeCap[timeCap_pos-1]-timeCap[timeCap_pos-2];
        //                   ^-- Changed from 1 to 2 do to small voltage fall triggering
        //                       bad interrupt response
    }
    TA1CCTL0 &= ~CCIFG; // Clear interrupt flag
}

/*
#pragma vector = PORT2_VECTOR
__interrupt void buttonS1Interrupt(void){
    switch(P2IV){
      case 0x0E: // switch 1 case
        dht11_trigger();
        break;
      case 0x10: // switch 2 case
         interpret_timeDiff();
        break;
      default:
        break;
    }
      P2IFG &= ~BIT6 + ~BIT7; // clear button flags
}
*/
