#include <msp430.h>
#define LED1 BIT0
#define LED2 BIT6
#define B1 BIT3
#include <intrinsics.h> // Intrinsic functions
#define LED1 BIT0
volatile unsigned int flag = 0;
volatile unsigned int flag2 = 0;
volatile unsigned int k = 1;
volatile unsigned int time = 0;
void main(void) {
  WDTCTL = WDTPW|WDTHOLD; // Stop watchdog timer
  P1OUT &= ~0x41;       P1DIR = LED1 | LED2;
  P1REN = B1;
  P1OUT |= B1;
  TA0CCR0 = 1199; // Upper limit of count for TA0R

  BCSCTL3 |= LFXT1S_2;
  TA0CTL = MC_1|ID_0|TASSEL_1|TACLR; //Setup Timer0_A
  for (;;) {

      if((P1IN & B1) == 0){  //Is button down?
          if(time > 20){
              if(k==0){
                  P1OUT &= ~0x41;
                  P1OUT ^= 0x41;
                  k = 1;
                  flag = 0;
              }
              if(flag2==0){
                  while (!(TA0CTL & TAIFG)) {} // Wait for time up
                  TA0CTL &= ~TAIFG;  // Clear overflow flag
                                  flag2 = 1;
                              }
                              else if(flag2==1){
                                  while (!(TA0CTL & TAIFG)) {} // Wait for time up
                                  TA0CTL &= ~TAIFG;  // Clear overflow flag
                                  flag2 = 2;
                              }
                              else if(flag2==2){
                                  while (!(TA0CTL & TAIFG)) {} // Wait for time up
                                  TA0CTL &= ~TAIFG;  // Clear overflow flag
                                  flag2 = 3;
                              }
                              else if(flag2==3){
                                  while (!(TA0CTL & TAIFG)) {} // Wait for time up
                                  TA0CTL &= ~TAIFG;  // Clear overflow flag
                                  flag2 = 4;
                              }
                              else if(flag2==4){
                                  while (!(TA0CTL & TAIFG)) {} // Wait for time up
                                  TA0CTL &= ~TAIFG;  // Clear overflow flag
                                  flag2 = 5;
                              }
                              else if(flag2==5){
                                  while (!(TA0CTL & TAIFG)) {} // Wait for time up
                                  TA0CTL &= ~TAIFG;  // Clear overflow flag
                                  flag2 = 6;
                              }
                              else if(flag2==6){
                                  while (!(TA0CTL & TAIFG)) {} // Wait for time up
                                  TA0CTL &= ~TAIFG;  // Clear overflow flag
                                  P1OUT ^= 0x41;
                                  flag2 = 7;
                              }
                              else if(flag2==7){
                                  while (!(TA0CTL & TAIFG)) {} // Wait for time up
                                  TA0CTL &= ~TAIFG;  // Clear overflow flag
                                  flag2 = 8;
                              }
                              else if(flag2==8){
                                  while (!(TA0CTL & TAIFG)) {} // Wait for time up
                                  TA0CTL &= ~TAIFG;  // Clear overflow flag
                                  flag2 = 9;
                              }
                              else if(flag2==9){
                                  while (!(TA0CTL & TAIFG)) {} // Wait for time up
                                  TA0CTL &= ~TAIFG;  // Clear overflow flag
                                  P1OUT ^= 0X41;
                                  flag2 = 0;
                              }
                  }
                  else{
                      if(flag==0){
                          while (!(TA0CTL & TAIFG)) {} // Wait for time up
                          TA0CTL &= ~TAIFG;  // Clear overflow flag
                                P1OUT ^= 0x01;
                                flag = 1;
                            }
                            else if(flag==1){
                                while (!(TA0CTL & TAIFG)) {} // Wait for time up
                                TA0CTL &= ~TAIFG;  // Clear overflow flag
                                P1OUT ^= 0x01;
                                flag = 2;
                            }
                            else if(flag==2){
                                while (!(TA0CTL & TAIFG)) {} // Wait for time up
                                TA0CTL &= ~TAIFG;  // Clear overflow flag
                                P1OUT ^= 0x01;
                                flag = 3;
                            }
                            else if(flag==3){
                                while (!(TA0CTL & TAIFG)) {} // Wait for time up
                                TA0CTL &= ~TAIFG;  // Clear overflow flag
                                P1OUT ^= 0x01;
                                flag = 4;
                            }
                            else if(flag==4){
                                while (!(TA0CTL & TAIFG)) {} // Wait for time up
                                TA0CTL &= ~TAIFG;  // Clear overflow flag
                                P1OUT ^= 0x01;
                                flag = 5;
                            }
                            else if(flag==5){
                                while (!(TA0CTL & TAIFG)) {} // Wait for time up
                                TA0CTL &= ~TAIFG;  // Clear overflow flag
                                P1OUT ^= 0x01;
                                flag = 6;
                            }
                            else if(flag==6){
                                while (!(TA0CTL & TAIFG)) {} // Wait for time up
                                TA0CTL &= ~TAIFG;  // Clear overflow flag
                                P1OUT ^= 0x40;
                                flag = 7;
                            }
                            else if(flag==7){
                                while (!(TA0CTL & TAIFG)) {} // Wait for time up
                                TA0CTL &= ~TAIFG;  // Clear overflow flag
                                P1OUT ^= 0x40;
                                flag = 0;
                            }
                      time = time + 1;
                  }
          }
            else{
                if(k==1){
                    time = 0;
                    P1OUT &= ~0x41;
                    k = 0;
                    flag2 = 0;
                }
                if(flag==0){
                    while (!(TA0CTL & TAIFG)) {} // Wait for time up
                    TA0CTL &= ~TAIFG;  // Clear overflow flag
                          P1OUT ^= 0x01;
                          flag = 1;
                      }
                      else if(flag==1){
                          while (!(TA0CTL & TAIFG)) {} // Wait for time up
                          TA0CTL &= ~TAIFG;  // Clear overflow flag
                          P1OUT ^= 0x01;
                          flag = 2;
                      }
                      else if(flag==2){
                          while (!(TA0CTL & TAIFG)) {} // Wait for time up
                          TA0CTL &= ~TAIFG;  // Clear overflow flag
                          P1OUT ^= 0x01;
                          flag = 3;
                      }
                      else if(flag==3){
                          while (!(TA0CTL & TAIFG)) {} // Wait for time up
                          TA0CTL &= ~TAIFG;  // Clear overflow flag
                          P1OUT ^= 0x01;
                          flag = 4;
                      }
                      else if(flag==4){
                          while (!(TA0CTL & TAIFG)) {} // Wait for time up
                          TA0CTL &= ~TAIFG;  // Clear overflow flag
                          P1OUT ^= 0x01;
                          flag = 5;
                      }
                      else if(flag==5){
                          while (!(TA0CTL & TAIFG)) {} // Wait for time up
                          TA0CTL &= ~TAIFG;  // Clear overflow flag
                          P1OUT ^= 0x01;
                          flag = 6;
                      }
                      else if(flag==6){
                          while (!(TA0CTL & TAIFG)) {} // Wait for time up
                          TA0CTL &= ~TAIFG;  // Clear overflow flag
                          P1OUT ^= 0x40;
                          flag = 7;
                      }
                      else if(flag==7){
                          while (!(TA0CTL & TAIFG)) {} // Wait for time up
                          TA0CTL &= ~TAIFG;  // Clear overflow flag
                          P1OUT ^= 0x40;
                          flag = 0;
                      }
            }

  }
}
