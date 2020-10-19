#include <msp430.h>
#define LED1 BIT0
#define LED2 BIT6
#define B1 BIT3
#include <intrinsics.h> // Intrinsic functions
#define LED1 BIT0
volatile unsigned int flag0 = 0;
volatile unsigned int flag1 = 0;
volatile unsigned int flag2 = 0;
volatile unsigned int timer = 0;
volatile unsigned int mode = 0;
volatile unsigned int k2 = 1;
volatile unsigned int k1 = 1;
volatile unsigned int k0 = 1;
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
    if(mode == 2 & timer > 0){
        if(k2 == 1){
            P1OUT &= ~0x41;
            P1OUT ^= 0x01;
            k2 = 0;
        }
        if(timer == 1){
            mode = 0;
            k0 = 1;
        }

        if(flag2 == 0){
            flag2 = 1;
            timer--;
        }
        else if(flag2 == 1){
            flag2 = 2;
            timer--;
        }
        else if(flag2 == 2){
            P1OUT ^= 0x01;
            flag2 = 3;
            timer--;
        }
        else if(flag2 == 3){
            flag2 = 4;
            timer--;
        }
        else if(flag2 == 4){
            P1OUT ^= 0x01;
            flag2 = 0;
            timer--;
        }
    }
    else if(mode == 1 & timer > 0){
        if(k1 == 1){
            P1OUT &= ~0x41;
            P1OUT ^= 0x41;
            k1 = 0;
        }
        if(timer == 1){
            mode = 0;
            k0 = 1;
        }

        if(flag1 == 0){
            flag1 = 1;
            timer--;
        }
        else if(flag1 == 1){
            flag1 = 2;
            timer--;
        }
        else if(flag1 == 2){
            flag1 = 3;
            timer--;
        }
        else if(flag1 == 3){
            flag1 = 4;
            timer--;
        }
        else if(flag1 == 4){
            flag1 = 5;
            timer--;
        }
        else if(flag1 == 5){
            flag1 = 6;
            timer--;
        }
        else if(flag1 == 6){
            P1OUT ^= 0x41;
            flag1 = 7;
            timer--;
        }
        else if(flag1 == 7){
            flag1 = 8;
            timer--;
        }
        else if(flag1 == 8){
            flag1 = 9;
            timer--;
        }
        else if(flag1 == 9){
            P1OUT ^= 0x41;
            flag1 = 0;
            timer--;
        }
    }
    else if(mode == 0){
        if((P1IN & B1) == 0){
            timer++;
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
        }
        else{
            if(timer > 50){
                flag0 = 0;
                k2 = 1;
                mode = 2;
            }
            else if(timer > 0){
                flag0 = 0;
                k1 = 1;
                mode = 1;
            }
            else{
                if(k0==1){
                    flag1 = 0;
                    flag2 = 0;
                    P1OUT &= ~0x41;
                    P1OUT ^= 0x40;
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
            }
        }
    }
}
