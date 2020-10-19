#include <msp430.h>
#define LED1 BIT0
#define LED2 BIT6
#define B1 BIT3
#include <intrinsics.h> // Intrinsic functions
#define LED1 BIT0
volatile unsigned int flag = 0;
volatile unsigned int flag2 = 0;
volatile unsigned int k = 1;
void main(void) {
  WDTCTL = WDTPW|WDTHOLD; // Stop watchdog timer
  P1OUT &= ~0x41;       P1DIR = LED1 | LED2;
  P1REN = B1;
  P1OUT |= B1;
  TA0CCR0 = 1199; // Upper limit of count for TA0R
  TA0CCTL0 = CCIE; // Enable interrupts
  BCSCTL3 |= LFXT1S_2;
  TA0CTL = MC_1|ID_0|TASSEL_1|TACLR; //Setup Timer0_A
  // Up mode, divide clock by 8, clock from SMCLK, clear
  __enable_interrupt(); // Enable interrupts (intrinsic)
  for (;;) { } // Loop forever doing nothing
}

// Interrupt service routine for CCR0 of Timer0_A3
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR (void){
    if((P1IN & B1) == 0){  //Is button down?
              if(k==0){
                  P1OUT &= ~0x41;
                  P1OUT ^= 0x41;
                  k = 1;
                  flag = 0;
              }
              if(flag2==0){
                                  flag2 = 1;
                              }
                              else if(flag2==1){
                                  flag2 = 2;
                              }
                              else if(flag2==2){
                                  flag2 = 3;
                              }
                              else if(flag2==3){

                                  flag2 = 4;
                              }
                              else if(flag2==4){

                                  flag2 = 5;
                              }
                              else if(flag2==5){

                                  flag2 = 6;
                              }
                              else if(flag2==6){
                                  P1OUT ^= 0x41;
                                  flag2 = 7;
                              }
                              else if(flag2==7){

                                  flag2 = 8;
                              }
                              else if(flag2==8){

                                  flag2 = 9;
                              }
                              else if(flag2==9){
                                  P1OUT ^= 0X41;
                                  flag2 = 0;
                              }
           } // Yes, turn red LED off
          else{
              if(k==1){
                  P1OUT &= ~0x41;
                  P1OUT ^= 0x40;
                  k = 0;
                  flag2 = 0;
              }
              if(flag==0){
                        flag = 1;
                    }
                    else if(flag==1){
                        flag = 2;
                    }
                    else if(flag==2){
                        flag = 3;
                    }
                    else if(flag==3){

                        flag = 4;
                    }
                    else if(flag==4){

                        flag = 5;
                    }
                    else if(flag==5){

                        flag = 6;
                    }
                    else if(flag==6){
                        P1OUT ^= 0x41;
                        flag = 7;
                    }
                    else if(flag==7){

                        flag = 8;
                    }
                    else if(flag==8){

                        flag = 9;
                    }
                    else if(flag==9){
                        P1OUT ^= 0X41;
                        flag = 0;
                    }
          }
}
