#include <msp430.h>
#define measure_low 1
#define measure_high 2


volatile unsigned int state = measure_low;
volatile unsigned int led_state = 0;
// 0: r-on 1: r-off 2: r-on 3: r-off 4: g-on 5: g-off 6: both-on 7: both-off 
volatile unsigned int flag = 0; // 0: low-> high, 1: high->low

int main()
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    P1DIR |= 0x41; //設成1表示output，設成0表示input，default是0

    ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE + SREF_1 + REFON;
    ADC10CTL1 = INCH_10;    // Input from A1
    ADC10AE0 |= 0x02; // Enable pin A1 for analog in
    ADC10CTL0 |= ENC + ADC10SC; // Start sampling

    // timer 1 for led
    TA1CTL |= MC_1|ID_0|TASSEL_1|TACLR;
    TA1CCTL0 |= CCIE;// enable timer 1的interrupt
    TA1CCR0 = 5999;

    // timer 0 for sampling 
    TA0CTL |= MC_1|ID_3|TASSEL_2|TACLR;
    TA0CCTL0 |= CCIE; // enable timer 0的interrupt
    TA0CCR0 = 13680-1;

    BCSCTL3 |= LFXT1S_2; // 使用VLO clock

    P1OUT &= ~0x41;//控制LED全暗
    P1OUT |= 0x01; //red on

    led_state = 0;
    __enable_interrupt();
    for(;;) {
        if (led_state == 0) {
            TA1CCR0 = 5999;
            P1OUT &= ~0x41;
            P1OUT |= 0x01;
        } else if (led_state == 1) {
            P1OUT &= ~0x41;
        } else if (led_state == 2) {
            P1OUT |= 0x01;
        } else if (led_state == 3) {
            P1OUT &= ~0x41;
        } else if (led_state == 4) {
            TA1CCR0 = 10799;
            P1OUT &= ~0x41;
            P1OUT |= 0x40;
        } else if (led_state == 5) {
            P1OUT &= ~0x41;
        } else if (led_state == 6) {
            TA1CCR0 = 4199;
            P1OUT &= ~0x41;
            P1OUT |= 0x41;
        } else if (led_state == 7) {
            P1OUT &= ~0x41;
        }
    }
}

// timer0 trigger sampling
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void) {
    TA0CTL |= TACLR;
    TA0CTL &= ~TAIFG;  // Clear overflow flag
    ADC10CTL0 |= ENC + ADC10SC; // enable sampling
}

// sensor sampling
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
    if (ADC10MEM < 740) {
        state = measure_low;
        if (flag == 1){
          led_state = 0;
          flag = 0;
        }
    }
    else{
        state = measure_high;
        if (flag == 0) {
            flag = 1;
            led_state = 6;
        }
    }
}

// LED flash
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void)
{
    if (state == measure_low) {
        if (led_state == 0) led_state = 1;
        else if (led_state == 1) led_state = 2;
        else if (led_state == 2) led_state = 3;
        else if (led_state == 3) led_state = 4;
        else if (led_state == 4) led_state = 5;
        else if (led_state == 5) led_state = 0;
    }
    else {
        if (led_state == 6) led_state = 7;
        else if (led_state == 7) led_state = 6;
    }
    TA1CTL |= TACLR;
}