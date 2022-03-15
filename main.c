#include <msp430.h>
#include <__cross_studio_io.h>
#include <hal_lcd.h>
extern int IncrementVcore(void);
extern int DecrementVcore(void);

  int finalData, avgData;
  char printData[] = {"Data:      "};
  int printScreenCount = 0;
  int screenLightToggle = 1;

void main(void){

  UCSCTL1 = DCORSEL_1; // Select range of to 1MHz
  UCSCTL2 = 31; // Set frequency to (30+1)x32.768kHz = 1MHz
  UCSCTL3 = SELREF__XT1CLK; // Use internal oscillator crystal
  UCSCTL4 = SELM__DCOCLK+SELS__DCOCLK; // Use DCO clock for MCLK and SMCLK

  P11DIR |= BIT0; // P11.0 set up for ACLK
  P11SEL |= BIT0; // Set P11.0 as a peripheral pin, not an I/O pin
  P11DIR |= BIT1; // P11.1 is the MCLK pin. Set it to output a signal
  P11SEL |= BIT1; // Set P11.1 as a peripheral pin, not an I/O pin
  P11DIR |= BIT2; // P11.2 is the SMCLK pin. Set it to output a signal
  P11SEL |= BIT2; // Set P11.2 as a peripheral pin, not an I/O pin
  
  REFCTL0 |= REFON+REFVSEL_3; // Vref  = 2.5V
  // Count to 256, IE., Enable Conv., auto iter. through MEM
  ADC12CTL0 |= ADC12ON+ADC12SHT03+ADC12OVIE+ADC12TOVIE+ADC12MSC;

  // ADC_CLK/8,choose SMCLK,single channel, A7 analog input
  ADC12CTL1 |= ADC12DIV_7+ADC12SSEL_3+ADC12CONSEQ_1+ADC12SHP;
  ADC12CTL2 |= ADC12RES_3;

  // ADC memory write order
  ADC12MCTL0 |= ADC12SREF_1+ADC12INCH_7; 
  ADC12MCTL1 |= ADC12SREF_1+ADC12INCH_7; 
  ADC12MCTL2 |= ADC12SREF_1+ADC12INCH_7; 
  ADC12MCTL3 |= ADC12SREF_1+ADC12INCH_7; 
  ADC12MCTL4 |= ADC12SREF_1+ADC12INCH_7; 
  ADC12MCTL5 |= ADC12SREF_1+ADC12INCH_7; 
  ADC12MCTL6 |= ADC12SREF_1+ADC12INCH_7; 
  // End of sequence, do print stuff
  ADC12MCTL7 |= ADC12SREF_1+ADC12INCH_7+ADC12EOS;
  ADC12IE |= ADC12IE7;

  P8DIR |= BIT5; // 3.3V output signal
  P8OUT |= BIT5;
  P6SEL |= BIT7;

  P2OUT |= BIT6;
  P2IE  |= BIT6; // P2.6 interrupt enabled
  P2REN |= BIT6; // Tie button to VCC
  P2IES |= BIT6; // P2.6 Hi/lo edge, when button is released.
 
  P2OUT |= BIT7;
  P2IE  |= BIT7; // P2.7 interrupt enabled
  P2REN |= BIT7; // Tie button to VCC
  P2IES |= BIT7; // P2.7 Hi/lo edge, when button is released

  TA1CTL |= TASSEL__ACLK+TACLR+ID_3+MC__STOP+TAIE; // Timer A1 counts w/ clock, clear the count
  //TA1EX0 |= TAIDEX_7; // divide clock by 8, make it SLOW, 4096 Hz
  TA1CCTL0 |= SCS; // Rising edge, sync w/ clock, interrupt enabled
  TA1CCR0 = 4096; // 4096Hz * 5 sec. = 13107 clocks 

  P2IFG  = 0;
  halLcdInit();
  halLcdClearScreen();
  halLcdBackLightInit();
  halLcdSetBackLight(0);
  halLcdSetContrast(90);


// SENDING DATA TO SD CARDD
  P3DIR |= BIT4; // cs pin, make low read data tx
  P3OUT |= BIT4;
  UCA1CTL0 |= UCCKPL+UCMST+UCMODE2+UCSYNC;
  UCA1CTL1 |= UCSSEL_SMCLK;



  _EINT();
  TA1CTL |= MC__UPDOWN;
  LPM0;
}

// Uses P2IV - When S1 button is pressed
// Joysticks Left/Right lower and raise LCD backlight brightness
// Button 1: Toggle LCD backlight
// Busson 2: Start Temp ADC conversaion and sample it
// toggleButton starts timer SS
void buttonISR (void) __interrupt[PORT2_VECTOR]{
  switch(P2IV){
    case 0x00:
      break;
    case 0x04:
      break;
    case 0x06:
      break;
    case 0x0E:
      if(screenLightToggle == 1){
        halLcdShutDownBackLight();
        screenLightToggle ++;
        screenLightToggle = 0;
        halLcdInit();
      }
      else{
        halLcdBackLightInit();
        halLcdSetBackLight(10);
        screenLightToggle = 1;
      }
      break;
     case 0x10:
      ADC12CTL0 |= ADC12SC+ADC12ENC; // Start conversion/sample
      P8OUT |= BIT5;
      break;
     default: 
      break;
  }
}

// Uses ADC12_VECTOR - When ADC values are sampled/written
// Case 1 and 2: Overflow errors, toggle LED's
// Case 10: ADC has been written to memory 8 times, 
//         Get Avg. ADC reading, convert to Temp.(C), and print to LCD
void dataReadingISR (void) __interrupt[ADC12_VECTOR]{
  switch(ADC12IV){
    case 0x00: break;
    case 0x02:
      P1OUT ^=  BIT0; //toggle LED1
      break;
    case 0x04: 
      P1OUT ^=  BIT2; //toggle LED2
      break;
    case 0x05: 
      break;
    case 0x14:
      avgData = ADC12MEM0+ADC12MEM1+ADC12MEM2+ADC12MEM3+ADC12MEM4+ADC12MEM5+ADC12MEM6+ADC12MEM7;
      avgData = avgData/8;

      finalData = avgData;

      P8OUT ^= ~BIT5; // Toggle output PIN signal for O-Scope testing

      printData[sizeof(printData)-5] = '0' + finalData/1000;
      printData[sizeof(printData)-4] = '0' + (finalData/10/100)%10;
      printData[sizeof(printData)-3] = '0' + (finalData/10)%10;
      printData[sizeof(printData)-2] = '0' + finalData%10;

      if (printScreenCount < 9){
        halLcdPrintLine(printData,printScreenCount,0);
        printScreenCount++;
      }
      else{
       printScreenCount = 0;
       halLcdPrintLine(printData,printScreenCount,0);
       halLcdClearScreen();
      }
      P3OUT &= ~BIT4; // make CS low for data transfer
      UCA1TXBUF = finalData; // send data
      P3OUT &= ~BIT4; // make CS high, stop data transfer
      break;
    default: 
     break;
  }
  
}


void clockCaptureISR (void) __interrupt[TIMER1_A1_VECTOR]{
  TA1CTL |= MC__STOP; // Stop the clock for capture 
  switch(TA1IV){
  case 0: break;
  case 14:
    ADC12CTL0 |= ADC12SC+ADC12ENC; // Start conversion/sample
    break;
  default:
    break;
  }
  TA1CTL |= TACLR; // Clear the counter
  TA1CTL |= MC__UP;
}