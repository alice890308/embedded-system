#include <msp430.h> 
#define B1 BIT3     //P1.3 to button

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    P1DIR |= 0x41;
    P1OUT |= 0x00;

    P1REN |= B1;
    P1OUT |= B1;

    TA1CCR0 = 299;
    TA1CTL = MC_1|ID_3|TASSEL_1|TACLR;
    BCSCTL3 |= LFXT1S_2;
    TA1CCTL0 = CCIE;

    TA0CCTL1 = OUTMOD_3;
    TA0CCR0 = 38400-1;
    TA0CCR1 = 38400-2;
    TA0CTL = TASSEL_1 + MC_1;
    TA0CCTL0 = CCIE;

    ADC10CTL1 = SHS_1 + CONSEQ_0 + INCH_10;
    ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE;

    ADC10CTL0 |= ENC;
    ADC10AE0 |= 0x02;

    P1IE |= B1;
    P1IES |= B1;
    P1IFG &= ~B1;
    _BIS_SR(GIE);

    __bis_SR_register(LPM3_bits + GIE);
   // __enable_interrupt(); // Enable interrupts (intrinsic)
    for (;;) { } // Loop forever doing nothing
    return 0;

}

volatile unsigned int mode_0=0, mode_1=1;  // prevent optimization
volatile unsigned int button=0, tem=0;
volatile unsigned int mode=0, pattern_1=0, pattern_2=2;
volatile unsigned int state=0, Normal=0, Emergency=1;
volatile float LPM3_time=0.0, LPM0_time=0.0;
volatile unsigned int LED_mode=0;

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void){
    tem=ADC10MEM;
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR (void){
    if(mode == pattern_1){  //Is button down?
        if(LED_mode == mode_0){
            P1OUT &= ~0x41;
            P1OUT |= 0x01;
            TA1CTL &= ~TAIFG;
            TA1CCR0 = 1199;
            LED_mode = mode_1;
            LPM3_time+=0.4;
        }
        else if(LED_mode == mode_1){
            P1OUT &= ~0x41;
            P1OUT |= 0x40;
            TA1CTL &= ~TAIFG;
            TA1CCR0 = 1799;
            LED_mode = mode_0;
            LPM3_time+= 0.6;
        }

    } // Yes, turn red LED off
    else if (mode == pattern_2){
         if(LED_mode == mode_0){
            P1OUT &= ~0x41;
            P1OUT |= 0x41;
            TA1CCR0 = 899;
            TA1CTL &= ~TAIFG;
            LED_mode = mode_1;
            LPM0_time+=0.3;
        }
         else if(LED_mode == mode_1){
            P1OUT &= ~0x41;
            TA1CCR0 = 599;
            TA1CTL &= ~TAIFG;  // Clear overflow flag
            LED_mode = mode_0;
            LPM0_time+=0.2;
        }
    }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void) {
    if(tem<805){
        mode=pattern_1;
        TA0CCR0 = 38400-1;
        TA0CCR1 = 38400-2;
        state = Normal;
        _BIC_SR(LPM0_EXIT) ;
        __bis_SR_register(LPM3_bits + GIE);
    }
    else {
        mode = pattern_2;
        TA0CCR0 = 12800-1;
        TA0CCR1 = 12800-2;
        state = Emergency;
        _BIC_SR(LPM3_EXIT) ;
        __bis_SR_register(LPM0_bits + GIE);
    }
    TA0CTL |= TACLR;
    TA0CTL &= ~TAIFG;
    ADC10CTL0 |= ENC + ADC10SC;
}


/*

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR (void){

    mode_0++;
    mode_1++;
    if(mode == pattern_1){  //Is button down?
        mode_1=0;
        if(mode_0<=7){
            P1OUT &= ~0x41;     // Toggle LEDs
            P1OUT |= 0x01;     // Toggle LEDs
        }
        else if(mode_0<=10){
            P1OUT &= ~0x41;     // Toggle LEDs
            P1OUT |= 0x40;     // Toggle LEDs

        }
        else{
            mode_0=0;
        }
    } // Yes, turn red LED off
    else if (mode == pattern_2){
        mode_0=0;
         if(mode_1<=5){
            P1OUT &= ~0x41;     // Toggle LEDs
            P1OUT |= 0x41;     // Toggle LEDs
        }
         else if(mode_1<=10){
            P1OUT &= ~0x41;     // Toggle LEDs
        }
         else{
             mode_1=0;
         }
    }
    TA1CTL &= ~TAIFG;  // Clear overflow flag
    TA1R=0;

}

*/




