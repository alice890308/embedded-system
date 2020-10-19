#include <msp430.h>
#define LED1 BIT0
void main (void) {
  WDTCTL = WDTPW|WDTHOLD; // Stop watchdog timer
  BCSCTL3 |= LFXT1S_2;
  P1DIR |= 0x41;
  P1OUT = 0;
  P1OUT = ~LED1;
  TA0CCR0 = 1199;
  TA0CTL = MC_1|ID_0|TASSEL_1|TACLR; //Setup Timer0_A
  int flag = 0;
  //up mode, divide clk by 8, use SMCLK, clr timer
  for (;;) { // Loop forever

      if(flag==0){
          while (!(TA0CTL & TAIFG)) {} // Wait for time up
          TA0CTL &= ~TAIFG;  // Clear overflow flag
          P1OUT = 0x40;
          flag = 1;
      }
      else if(flag==1){
          while (!(TA0CTL & TAIFG)) {} // Wait for time up
          TA0CTL &= ~TAIFG;  // Clear overflow flag
          P1OUT = 0x40;
          flag = 2;
      }
      else if(flag==2){
          while (!(TA0CTL & TAIFG)) {} // Wait for time up
          TA0CTL &= ~TAIFG;  // Clear overflow flag
          P1OUT = 0x40;
          flag = 3;
      }
      else if(flag==3){
          while (!(TA0CTL & TAIFG)) {} // Wait for time up
          TA0CTL &= ~TAIFG;  // Clear overflow flag
          P1OUT = 0x40;
          flag = 4;
      }
      else if(flag==4){
          while (!(TA0CTL & TAIFG)) {} // Wait for time up
          TA0CTL &= ~TAIFG;  // Clear overflow flag
          P1OUT = 0x40;
          flag = 5;
      }
      else if(flag==5){
          while (!(TA0CTL & TAIFG)) {} // Wait for time up
          TA0CTL &= ~TAIFG;  // Clear overflow flag
          P1OUT = 0x40;
          flag = 6;
      }
      else if(flag==6){
          while (!(TA0CTL & TAIFG)) {} // Wait for time up
          TA0CTL &= ~TAIFG;  // Clear overflow flag
          P1OUT = 0x40;
          flag = 7;
      }
      else if(flag==7){
          while (!(TA0CTL & TAIFG)) {} // Wait for time up
          TA0CTL &= ~TAIFG;  // Clear overflow flag
          P1OUT = 0x01;
          flag = 8;
      }
      else if(flag==8){
          while (!(TA0CTL & TAIFG)) {} // Wait for time up
          TA0CTL &= ~TAIFG;  // Clear overflow flag
          P1OUT = 0x01;
          flag = 9;
      }
      else if(flag==9){
          while (!(TA0CTL & TAIFG)) {} // Wait for time up
          TA0CTL &= ~TAIFG;  // Clear overflow flag
          P1OUT = 0x01;
          flag = 0;
      }

  } // Back around infinite loop
}
