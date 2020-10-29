#include <msp430.h>
#define normal 1
#define measure_high 2
#define measure_low 3

volatile unsigned int state = normal;
volatile unsigned int led_state = 0;
// 0: r-on 1: r-off 2: r-on 3: r-off 4: g-on 5: g-off 6: both-on 7: both-off
volatile unsigned int flag = 0; // 0: low-> high, 1: high->low
volatile double temp = 0.0;
volatile double time = 0.0;
volatile int threshold = 28;
volatile int adc[4] = {0};
volatile int click = 0;

int main()
{
    WDTCTL = WDTPW + WDTHOLD; // stop watchdog timer

    P1DIR |= 0x41; //設成1表示output，設成0表示input，default是0

    ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE + SREF_1 + REFON;
    ADC10CTL1 = INCH_10;        // Input from A1
    ADC10AE0 |= 0x02;           // Enable pin A1 for analog in
    ADC10CTL0 |= ENC + ADC10SC; // Start sampling
    ADC10CTL0 &= ~REF2_5V;
    //DTC
    ADC10DTC1 = 4;      // number of transfers
    ADC10SA = (int)adc; // buffer starting address

    // timer 1 for led
    TA1CTL |= MC_1 | ID_0 | TASSEL_1 | TACLR;
    TA1CCTL0 |= CCIE; // enable timer 1的interrupt
    TA1CCR0 = 5999;   //0.5 sec
    TA1CCTL0 &= ~CCIFG;

    // timer 0 for sampling and button
    TA0CTL |= MC_1 | ID_3 | TASSEL_1 | TACLR;
    TA0CCTL0 |= CCIE; // enable timer 0的interrupt
    //TA0CCR0 = 300-1; //0.25sec
    TA0CCR0 = 600 - 1; //0.5sec

    // button
    P1REN |= BIT3;
    P1OUT |= BIT3;
    P1IE |= BIT3;
    P1IES |= BIT3;
    P1IFG &= ~BIT3;

    BCSCTL3 |= LFXT1S_2; // 使用VLO clock

    P1OUT &= ~0x41; //控制LED全暗
    P1OUT |= 0x01;  //red on

    led_state = 0;
    //__enable_interrupt();
    _BIS_SR(GIE + LPM3_bits);
    for (;;)
    {
    }
}

// timer0 trigger sampling
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void)
{
    TA0CTL |= TACLR;
    TA0CTL &= ~TAIFG; // Clear overflow flag
    if (state == normal)
    {
        if (click == 1)
        { // double click 超時
            click = 0;
            P1IES |= BIT3; //button_down interrupt
        }
    }
    else
    {
        ADC10CTL0 |= ENC + ADC10SC; // enable sampling
    }
}

#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void)
{
    P1IFG &= ~BIT3; //rst
    if (P1IES & BIT3)
    { //button_down mode
        //set timer1 for measure button down time
        if (click == 0)
        {                      // 第一次按下按鈕
            TA0CCR0 = 600 - 1; //0.5sec
            click = 1;
            TA0CTL |= TACLR;
        }
        else
        {                   //第二次按下
            P1IES &= ~BIT3; //button_up interrupt
        }
    }
    else
    { // button_up mode
        if (click == 1)
        {
            state = measure_low;
            TA0CTL |= TACLR;   // reset timerA-0
            TA0CCR0 = 300 - 1; //0.25sec
            _BIS_SR(LPM3_EXIT);
            __bis_SR_register(LPM0_bits + GIE);
        }
        click = 0;
        P1IES |= BIT3; //button_down interrupt
    }
}

// sensor sampling
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
    ADC10SA = (int)adc;
    for (i = 0; i < 4; i++)
    {
        temp = adc[i];
        sum += temp;
    }
    sum = sum * 1.5 / 1023.0;
    average = (sum / 4 - 0.986) / 0.00355;
    if (average > threshold)
    {
        if (state != measure_high) {
            state = measure_high;
            led_state = 8;
            _BIS_SR(LPM3_EXIT);
            __bis_SR_register(LPM0_bits + GIE);
        }
    }
    else
    {
        if (state != measure_low) {
            state = measure_low;
            led_state = 6;
            _BIS_SR(LPM3_EXIT);
            __bis_SR_register(LPM0_bits + GIE);
        }
    }
}

// LED flash
#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void)
{
    if (state == normal)
    {
        if (led_state == 0)
        {
            led_state = 1;
            TA1CCR0 = 5999;    //0.5 sec
            P1OUT &= ~0x41;
            P1OUT |= 0x01;
        }
        else if (led_state == 1)
        {
            led_state = 2;
            P1OUT &= ~0x41;
        }
        else if (led_state == 2)
        {
            led_state = 3;
            P1OUT |= 0x01;
        }
        else if (led_state == 3)
        {
            led_state = 4;
            P1OUT &= ~0x41;
        }
        else if (led_state == 4)
        {
            led_state = 5;
            TA1CCR0 = 10799; //0.9 sec for green led
            P1OUT &= ~0x41;
            P1OUT |= 0x40;
        }
        else if (led_state == 5)
        {
            led_state = 0;
            TA1CCR0 = 5999; //0.5 sec
            P1OUT &= ~0x41;
        }
    }
    else if (state == measure_low)
    {
        if (led_state == 6)
        {
            TA1CCR0 = 3600 - 1; //0.3 sec for led
            P1OUT &= ~0x41;
            P1OUT |= 0x41;
            led_state = 7;
        }
        else if (led_state == 7)
        {
            TA1CCR0 = 2400 - 1; //0.2 sec
            P1OUT &= ~0x41;
            led_state = 6;
        }
    }
    else {
        if (led_state == 8)
        {
            TA1CCR0 = 3600 - 1; //0.3 sec for led
            P1OUT &= ~0x41;
            P1OUT |= 0x01;
            led_state = 9;
        }
        else if (led_state == 9)
        {
            TA1CCR0 = 2400 - 1; //0.2 sec
            P1OUT &= ~0x41;
            led_state = 8;
        }
    }
    TA1CTL &= ~TAIFG;
    TA1CTL |= TACLR;
}
