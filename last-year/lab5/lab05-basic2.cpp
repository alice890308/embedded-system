#include <intrinsics.h>
#include <msp430.h> 
#define B1 BIT3     //P1.3 to button

int adc[6];
int c_adc[6];
int threshold = 20;
volatile unsigned int k=0;
volatile unsigned int button=0, tem=0;
volatile unsigned int average=0, c_average=0;
volatile unsigned int mode=0, pattern_1=0, pattern_2=1;
volatile unsigned int state=0, Normal=0, Emergency=1;
volatile float LPM3_time=0.0, LPM0_time=0.0;
volatile unsigned int LED_mode=0, mode_0=0, mode_1=1;
volatile unsigned int temperture=0, below=0, above=1;//Temperture

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    P1DIR |= 0x41;
    P1OUT |= 0x00;

    P1REN |= B1;
    P1OUT |= B1;

    TA1CTL = MC_1|ID_0|TASSEL_1|TACLR;
    TA1CCR0 = 11000-1;
    TA1CCTL0 = CCIE;
    BCSCTL3 |= LFXT1S_2;


    TA0CCR0 = 14000;
    TA0CCR1 = 14000-1;
    TA0CTL =  MC_1|ID_0|TASSEL_1|TACLR;
    TA0CCTL1 = OUTMOD_3;
//    TA0CCTL0 = CCIE;

    ADC10CTL1 = SHS_1 + INCH_10;
    ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE;
    ADC10CTL0 |= ENC+ADC10SC;    // ADC10 Enable
//    ADC10DTC1 = 6;
//    ADC10SA = (int)adc;

    _BIS_SR(LPM3_bits + GIE);
    __enable_interrupt(); // Enable interrupts (intrinsic)
    for (;;) { } // Loop forever doing nothing
    return 0;

}


#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void){
    if(state == Normal){
        tem = ADC10MEM;
        c_average = ((tem*1.5/1023) - 0.986)/0.00355;
        ADC10CTL0 |= ENC + ADC10SC;
        if(c_average > threshold){
            ADC10DTC1 = 6;
            TA0CCR0 = 4800;
            TA0CCR1 = 4800 - 1;
            ADC10CTL1 = SHS_1 + CONSEQ_2 + INCH_10;
            temperture=above;
            state = Emergency;
            mode=pattern_2;
            _BIC_SR(LPM3_EXIT) ;
            _BIS_SR(LPM0_bits + GIE);
        }
    }
    else if(state == Emergency){
        volatile int total=0;
        for(k=0;k<6;k++){
            total += adc[k];
            c_adc[k]=((adc[k]*1.5/1023)-0.986)/0.00355;
        }
        ADC10SA = (int)adc;
        average = total/6;
        c_average= ((average*1.5/1023)-0.986)/0.00355;

        if(c_average < threshold){
            ADC10DTC1 = 0;
            TA0CCR0 = 14000;
            TA0CCR1 = 14000 - 1;
            ADC10CTL1 = SHS_1 + INCH_10;
            temperture=below;
            state = Normal;
            mode=pattern_1;
            _BIC_SR(LPM0_EXIT) ;
            _BIS_SR(LPM3_bits + GIE);
        }
    }


}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR (void){
    if(mode == pattern_1){
        if(LED_mode == mode_0){
            P1OUT &= ~0x41;
            P1OUT |= 0x01;
            TA1CTL &= ~TAIFG;
            TA1CCR0 = 7200 - 1;
            LED_mode = mode_1;
            LPM3_time += 0.6;

        }
        else if(LED_mode == mode_1){
            P1OUT &= ~0x41;
            P1OUT |= 0x40;
            TA1CTL &= ~TAIFG;
            TA1CCR0 = 4800 -1;
            LED_mode = mode_0;
            LPM3_time += 0.4;
        }
    }
    else if (mode == pattern_2){
         if(LED_mode == mode_0){
            P1OUT &= ~0x41;
            P1OUT |= 0x41;
            TA1CCR0 = 2400 - 1;
            TA1CTL &= ~TAIFG;
            LED_mode = mode_1;
            LPM0_time += 0.2;
        }
         else if(LED_mode == mode_1){
            P1OUT &= ~0x41;
            TA1CCR0 = 3600 - 1;
            TA1CTL &= ~TAIFG;
            LED_mode = mode_0;
            LPM0_time += 0.3;
        }
    }

}
//#pragma vector=TIMER0_A0_VECTOR
//__interrupt void TA0_ISR(void) {
//    TA0CTL &= ~TAIFG;
//    ADC10CTL0 |= ENC + ADC10SC;
//}







