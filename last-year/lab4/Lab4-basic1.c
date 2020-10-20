#include <msp430.h>
#define LED1 BIT0
#define LED2 BIT6
#define B1 BIT3
#include <intrinsics.h> // Intrinsic functions
#define LED1 BIT0
volatile unsigned int flag0 = 0;
volatile unsigned int flag2 = 0;
volatile unsigned int k0 = 1;
// k0用來判斷是不是剛從mode1跳到mode2，或是相反
volatile unsigned int mode = 1;
volatile unsigned int temp;

void main(void) {


  WDTCTL = WDTPW + WDTHOLD;    // Stop WDT
  // H&S time 16x, interrupt enabled
  ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE + SREF_1 + REFON;
  ADC10CTL1 = INCH_10;    // Input from A1
  ADC10AE0 |= 0x02; // Enable pin A1 for analog in
  ADC10CTL0 |= ENC + ADC10SC; // Start sampling
  
  P1DIR |= 0x41;    // Set P1.0 to output
  
  BCSCTL3 |= LFXT1S_2;

  // timer1 setting
  TA1CTL = MC_1|ID_2|TASSEL_1|TACLR; //Setup Timer0_A
  TA1CCR0 = 299; // Upper limit of count for TA0R
  TA1CCTL0 = CCIE; // Enable interrupts

  TA0CCR0 = 7200-1;     // Sampling period
  TA0CTL = MC_1|ID_3|TASSEL_2|TACLR;
  TA0CCTL0 = CCIE; // Enable interrupts

  __enable_interrupt(); // Enable interrupts (intrinsic)
  for (;;)  { }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void) {
    TA0CTL |= TACLR;
    TA0CTL &= ~TAIFG;  // Clear overflow flag
    ADC10CTL0 |= ENC + ADC10SC; // enable sampling
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void){
    temp = ADC10MEM;
    if (ADC10MEM < 750){
        mode = 1;
    }
    else{
        mode = 0;
    }
}

// 控制LED燈的閃爍時間
#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void) {
  if (mode == 1) {
      if(k0==1){
          P1OUT &= ~0x41;
          P1OUT |= 0x01;
          k0 = 0;
      }
      if(flag0 == 0){
          flag0 = 1;
      }
      else if(flag0 == 1){
          flag0 = 2;
      }
      else if(flag0 == 2){
          flag0 = 3;
      }
      else if(flag0 == 3){
          flag0 = 4;
      }
      else if(flag0 == 4){
          flag0 = 5;
      }
      else if(flag0 == 5){
          flag0 = 6;
      }
      else if(flag0 == 6){
          P1OUT ^= 0x41;
          flag0 = 7;
      }
      else if(flag0 == 7){
          flag0 = 8;
      }
      else if(flag0 == 8){
          flag0 = 9;
      }
      else if(flag0 == 9){
          P1OUT ^= 0x41;
          flag0 = 0;
      }
      TA0R = 0;
  }
  else{
      if(k0==1){
          P1OUT &= ~0x41;
          k0 = 0;
      }
      if(flag0 == 0){
          P1OUT ^= 0x41;
          flag0 = 1;
      }
      else if(flag0 == 1){
          flag0 = 2;
      }
      else if(flag0 == 2){
          flag0 = 3;
      }
      else if(flag0 == 3){
          flag0 = 4;
      }
      else if(flag0 == 4){
          flag0 = 5;
      }
      else if(flag0 == 5){
          P1OUT ^= 0x41;
          flag0 = 6;
      }
      else if(flag0 == 6){
          flag0 = 7;
      }
      else if(flag0 == 7){
          flag0 = 8;
      }
      else if(flag0 == 8){
          flag0 = 9;
      }
      else if(flag0 == 9){
          flag0 = 0;
      }
  }
}
