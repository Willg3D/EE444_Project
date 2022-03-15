// William Glass
// EE 444x
#include <msp430.h>
extern int IncrementVcore(void);

int averageTemp;
int tempSlope;
int lowRef30;
int highRef85;
int adc_Conv;
int pressbutton = 0;
int textLoop = 0;


void
main(void)
{

  IncrementVcore();
  IncrementVcore();
  IncrementVcore();
  IncrementVcore();

  // Clock Freq to setup
  UCSCTL1 = DCORSEL_6;      // DCO SEL Value for ideal CLK range
  UCSCTL2 = 488;            // DCO Div Value to change 32 KHz to 16 MHz
  UCSCTL3 = SELREF__XT1CLK; // Set Ref Clock
  UCSCTL4 = SELM__DCOCLK + SELS__DCOCLK;   // sELECTS DCOCLK for MCL and SMCLK
  P11DIR |= BIT1;           // Enable MCLK test Pins
  P11SEL |= BIT1;
  P11DIR |= BIT2;           // Enable SMCLK test Pins
  P11SEL |= BIT2;

  //////////// Button Code ////////////
  P2DIR &=  ~BIT6 + ~BIT7; // Set Button (Port 2.6 & 2.7) as input
  P2REN |= BIT6 + BIT7; // pull resistor;
  P2OUT |= BIT6 + BIT7; // Enable Pull Up Resistor

  P2IE  |= BIT6 + BIT7; // Enables interrupts on Port 2.6 & 2.7
  P2IES |= BIT6 + BIT7; // High to Low transition on interrupt edge
  P2IFG &= ~BIT6 + BIT7; // Clears interrupt register for Port 2.6 & 2.7

  /////////// ADC Setup //////////////


  // turn on ADC, 256 clk per sample, multiple sample conversion,
  // conversion overflow interrupt en, memory overflow interrupt en
  ADC12CTL0 |= ADC12ON + ADC12SHT0_8 + ADC12MSC +  ADC12TOVIE + ADC12OVIE;
  // Div clk by 8, select SMCLK, sequence of channels conver mod sel, sample timer source
  ADC12CTL1 |= ADC12DIV_7 + ADC12SSEL_3 + ADC12CONSEQ_1 + ADC12SHP;
  // set reference v to 2.5V, ref on
  REFCTL0 |= REFVSEL_3 + REFON;
  //for 12-bit res
  ADC12CTL2 |= ADC12RES_3;



  //////// ADC MEM CTL ////////

  ADC12IE = ADC12IE7; // enable interrupt for mem 7 reg

  // input channel Temperature diode, set voltage ref
  ADC12MCTL0 |= ADC12INCH_7 + ADC12SREF_1;  //ADC12INCH_7 is port 6.7
  ADC12MCTL1 |= ADC12INCH_10 + ADC12SREF_1 + ADC12EOS;



  //////// Loop Setup //////////
  _EINT();  // Enables interrupts globally
  LPM0;
  //while(1){}
}


//////// ADC Interrupt //////////
//void adcInterrupt (void) __interrupt[ADC12_VECTOR]{
#pragma vector=ADC12_VECTOR
__interrupt void ADC_Routine(void){

  averageTemp = ADC12MEM0 + ADC12MEM1;
}


/////// SET Button INTERRUPT ROUTINE /////////
//void buttonS1Interrupt (void) __interrupt[PORT2_VECTOR]{
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void){
  switch(P2IV){
    case 0x0E: // switch 1 case
      ADC12CTL0 |= ADC12SC + ADC12ENC; // start conversion
      break;
    case 0x10: // switch 2 case
      break;
    default:
      break;
  }
    //ADC12CTL0 |= ADC12SC + ADC12ENC;   //start convert
    P2IFG &= ~BIT6 + ~BIT7; // clear button flags
}
