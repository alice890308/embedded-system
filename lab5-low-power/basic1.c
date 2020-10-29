#include <msp430.h>
#define normal 1
#define emergency 2


volatile unsigned int state = normal;
volatile unsigned int led_state = 0;
// 0: r-on 1: r-off 2: r-on 3: r-off 4: g-on 5: g-off 6: both-on 7: both-off
volatile unsigned int flag = 0; // 0: low-> high, 1: high->low
volatile double temp = 0.0;
volatile double time = 0.0;
volatile int threshold = 28;

int main()
{
    WDTCTL = WDTPW + WDTHOLD;   // stop watchdog timer

    P1DIR |= 0x41; //設成1表示output，設成0表示input，default是0

    ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE + SREF_1 + REFON;
    ADC10CTL1 = INCH_10;    // Input from A1
    ADC10AE0 |= 0x02; // Enable pin A1 for analog in
    ADC10CTL0 |= ENC+ADC10SC; // Start sampling
    ADC10CTL0 &= ~REF2_5V;

    // timer 1 for led
    TA1CTL |= MC_1|ID_0|TASSEL_1|TACLR;
    TA1CCTL0 |= CCIE;// enable timer 1的interrupt
    TA1CCR0 = 5999;//0.5 sec
    TA1CCTL0 &= ~CCIFG;

    // timer 0 for sampling
    TA0CTL |= MC_1|ID_3|TASSEL_1|TACLR;
    TA0CCTL0 |= CCIE; // enable timer 0的interrupt
    TA0CCR0 = 960-1; //0.8sec

    BCSCTL3 |= LFXT1S_2; // 使用VLO clock

    P1OUT &= ~0x41;//控制LED全暗
    P1OUT |= 0x01; //red on

    led_state = 0;
    //__enable_interrupt();
    _BIS_SR(GIE+LPM3_bits) ;
    for(;;) {}
}

// timer0 trigger sampling
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void) {
    TA0CTL |= TACLR;
    TA0CTL &= ~TAIFG;  // Clear overflow flag
    ADC10CTL0 |= ENC+ADC10SC; // enable sampling
}

// sensor sampling
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
    temp = ADC10MEM * 1.5 / 1023.0;
    temp = (temp - 0.986) / 0.00355;
    if (temp > threshold) {
        state = emergency;
        if (flag == 0) {
            flag = 1;
            led_state = 6;
            _BIS_SR(LPM0_EXIT) ;
            __bis_SR_register(LPM3_bits + GIE);
        }
    }
    else{
        state = normal;
        if (flag == 1){
            led_state = 0;
            flag = 0;
            _BIS_SR(LPM0_EXIT) ;
            __bis_SR_register(LPM3_bits + GIE);
        }
    }
}

// LED flash
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void)
{
    if (state == normal) {
        if (led_state == 0 || led_state == 1 || led_state == 2 || led_state == 3 || led_state == 5) {
            time += 0.5;
        }
        else if ( led_state == 4) {
            time += 0.9;
        }

        if (led_state == 0){
            led_state = 1;
            TA1CCR0 = 5999; //0.5 sec
            TA0CCR0 = 960-1; //0.8sec for sampling
            P1OUT &= ~0x41;
            P1OUT |= 0x01;
        } else if (led_state == 1) {
            led_state = 2;
            P1OUT &= ~0x41;
        } else if (led_state == 2){
            led_state = 3;
            P1OUT |= 0x01;
        } else if (led_state == 3) {
            led_state = 4;
            P1OUT &= ~0x41;
        } else if (led_state == 4) {
            led_state = 5;
            TA1CCR0 = 10799; //0.9 sec for green led
            P1OUT &= ~0x41;
            P1OUT |= 0x40;
        } else if (led_state == 5){
            led_state = 0;
            TA1CCR0 = 5999; //0.5 sec
            P1OUT &= ~0x41;
        }
    }
    else {
        if (led_state == 6) {
            time += 0.3;
            TA1CCR0 = 3600-1; //0.3 sec for led
            TA0CCR0 = 480-1;//0.4sec for sampling
            P1OUT &= ~0x41;
            P1OUT |= 0x41;
        }
        else if (led_state == 7) {
            time += 0.2;
            TA1CCR0 = 2400-1; //0.2 sec
            P1OUT &= ~0x41;
        }

        if (led_state == 6) led_state = 7;
        else if (led_state == 7) led_state = 6;
    }
    TA1CTL &= ~TAIFG;
    TA1CTL |= TACLR;
}