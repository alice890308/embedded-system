#include <msp430.h>
#define LED1 BIT0
#define FIRST_RED 0
#define B1 BIT3     //P1.3 to button
#define SECOND_RED 1
#define GREEN 2
#define BUTTON 3

volatile int light = 0;
volatile int state = FIRST_RED;

void main (void) {
    WDTCTL = WDTPW|WDTHOLD; // Stop watchdog timer
    P1OUT &= ~0x41;  //Set P1.3 to use pull-up resistor
    P1REN |= B1;   //Set P1.3 to use internal resistor
    P1DIR |= 0x41;
    TA0CCR0 = 5999;
    TA0CTL |= MC_1|ID_0|TASSEL_1|TACLR; //Setup Timer0_A
    TA0CCTL0 = CCIE; // Enable interrupts
    //up mode, divide clk by 8, use SMCLK, clr timer
    P1IE |= B1;              // P1.3 interrupt enabled
    P1IES |= B1;             // P1.3 Hi/lo edge
    P1IFG &= ~B1;            // P1.3 IFG cleared
    _BIS_SR(GIE);
    BCSCTL3 |= LFXT1S_2;
    __enable_interrupt();
    while(1);
}
#pragma vector=TIMER0_A0_VECTOR;
__interrupt void TA0_ISR(void) {
    switch(state) {
        case BUTTON:
            /*if (light == 0) {
                P1OUT &= ~0x41;
                P1OUT |= 0x41;     // Toggle LEDs
                TA0CCR0 = 5999;
                light = 1;
            }
            else if (light == 1) {
                P1OUT &= ~0x41;
                TA0CCR0 = 5999;
                light = 0;
            }*/
            P1OUT ^= 0x41;
            //TA0CTL &= ~TAIFG;  // Clear overflow flag
            break;
        case FIRST_RED:
            if (light == 0) {
                P1OUT &= ~0x41;
                P1OUT |= 0x01;     // Toggle LEDs
                TA0CCR0 = 5999;
                light = 1;
            }
            else if (light == 1) {
                P1OUT &= ~0x41;
                TA0CCR0 = 5999;
                light = 0;
                state = SECOND_RED;
            }
            TA0CTL &= ~TAIFG;  // Clear overflow flag
            break;
        case SECOND_RED:
            if (light == 0) {
                P1OUT &= ~0x41;
                P1OUT |= 0x01;     // Toggle LEDs
                TA0CCR0 = 5999;
                light = 1;
            }
            else if (light == 1) {
                P1OUT &= ~0x41;
                TA0CCR0 = 5999;
                light = 0;
                state = GREEN;
            }
            TA0CTL &= ~TAIFG;  // Clear overflow flag
            break;
        case GREEN:
            if (light == 0) {
                P1OUT &= ~0x41;
                P1OUT |= 0x40;     // Toggle LEDs
                TA0R = 0;
                TA0CCR0 = 10799;
                light = 1;
            }
            else if (light == 1) {
                P1OUT &= ~0x41;
                TA0CCR0 = 5999;
                light = 0;
                state = FIRST_RED;
            }
            TA0CTL &= ~TAIFG;  // Clear overflow flag
            break;
    } // Back arou
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
    P1IFG &= ~B1;
    if (P1IES & B1) {
        state = BUTTON;
        P1IES &= ~B1;
        TA0CTL |= TACLR;

        P1OUT |= 0x41;     // Toggle LEDs
        TA0CCR0 = 5999;
    }
    else {
        state = FIRST_RED;
        P1IES |= B1;
        TA0CTL |= TACLR;

        P1OUT &= ~0x41;
        P1OUT |= 0x01;
        light = 1;
    }
}

