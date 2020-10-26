#include <msp430.h>
#include <intrinsics.h> // Intrinsic functions
#define normal 1
#define measure_low 2
#define measure_high 3

volatile unsigned int state = normal;
volatile unsigned int led_state = 0;
// 0: r-on 1: r-off 2: r-on 3: r-off 4: g-on 5: g-off
// 6: both-on 7: both-off  8: r-on 0.25sec 9: r-of 0.25sec
volatile unsigned int flag = 0; // 0: low-> high, 1: high->low
volatile int change_to_measure_low = 0;
volatile int change_to_measure_high = 0;
volatile int change_to_normal = 0;
volatile int measuring = 0;
volatile int i; // 計算平均溫度用
volatile int average = 0;
volatile int adc[4]={0};
volatile int threshold = 4;
volatile double temp = 0;
volatile int sum = 0;
volatile int count = 0; // for button time

int main()
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    P1DIR |= 0x41; //設成1表示output，設成0表示input，default是0

    // button
    P1REN |= BIT3;
    P1OUT |= BIT3;
    P1IE |= BIT3;
    P1IES |= BIT3;
    P1IFG &= ~BIT3;
    //_BIS_SR(GIE);

    ADC10CTL0 = ADC10SHT_2 | ADC10ON | ADC10IE | SREF_1 | REFON;
    ADC10CTL1 = SHS_1 | CONSEQ_2 | INCH_10;    // Input from A1
    ADC10CTL0 &= ~ENC; // close sampling
    // DTC
    ADC10DTC1 = 4; // number of transfers
    ADC10SA = (int)adc; // buffer starting address

    // timer 1 for led
    TA1CTL |= MC_1|ID_0|TASSEL_1|TACLR;
    TA1CCTL0 |= CCIE;// enable timer 1的interrupt
    TA1CCR0 = 5999; // 0.5 sec

    // timer 0 for sampling
    TA0CTL |= MC_1|ID_3|TASSEL_2|TACLR;
    TA0CCTL1 = OUTMOD_3;
    TA0CCTL0 |= CCIE; // enable timer 0的interrupt
    TA0CCR0 = 42968; // 2.5sec

    BCSCTL2 |= DIVS_3; // DIVS_x是SMCLK的divider，算法和前面的ID一樣，已經除以16，所以是125KHz
    BCSCTL3 |= LFXT1S_2; // 使用VLO clock
    BCSCTL1 &= ~XTS;     // the VLO set can only be used when XTS = 0

    P1OUT &= ~0x41;//控制LED全暗
    P1OUT |= 0x01; //red on

    led_state = 0;
    __enable_interrupt();

    for(;;) {
        if (measuring == 1) {
            measuring = 0;
            temp = 0;
            sum = 0;
            for (i = 0; i < 4; i++) {
                temp = adc[i];
                sum += temp;
            }
            sum = sum * 1.5/1023;
            average = (sum/4 - 0.986) / 0.00355;
            if (average < threshold) {
                if (state != measure_low) {
                    change_to_measure_low = 1;
                }
            }
            else {
                if (state != measure_high) {
                    change_to_measure_high = 1;
                }
            }
        }
        if (change_to_normal == 1) {
            change_to_normal = 0;
            led_state = 0;
            count = 0;
            state = normal;
            ADC10CTL0 &= ~ENC; // 關掉ADC
        }
        if (change_to_measure_low == 1) {
            change_to_measure_low = 0;
            led_state = 6;
            state = measure_low;
        }
        if (change_to_measure_high == 1) {
            change_to_measure_high = 0;
            led_state = 8;
            state = measure_high;
        }

        if (led_state == 0) {
            TA1CCR0 = 5999;//0.5 sec
            P1OUT &= ~0x41;
            P1OUT |= 0x01;
        } else if (led_state == 1) {
            P1OUT &= ~0x41;
        } else if (led_state == 2) {
            P1OUT |= 0x01;
        } else if (led_state == 3) {
            P1OUT &= ~0x41;
        } else if (led_state == 4) {
            TA1CCR0 = 10799; // 0.9 sec
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
        } else if (led_state == 8) {
            TA1CCR0 = 2400-1;
            P1OUT &= ~0x41;
            P1OUT |= 0x01;
        } else if (led_state == 9) {
            P1OUT &= ~0x41;
        }
    }
}

// sensor sampling
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
    ADC10SA = (int)adc;
    if (state == measure_high || state == measure_low) {
        measuring = 1;
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR (void){
    if (count == 1) count++;
    else if (count == 2){
        change_to_measure_low = 1;
        TA0CCR1 = 4296; //0.25sec, set timer 0
        TA0CCR0 = 4297;
        TA0CTL |= TACLR;
        ADC10CTL0 |= ENC; // 打開ADC
        count = 0;
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
    P1IFG &= ~BIT3; //rst
    if(P1IES & BIT3) {//button_down mode
        P1IES &= ~BIT3; //button_up interrupt
        //set timer1 for measure button down time
        TA0CCR0 = 42968; // 2.5 sec
        count  = 1; // 表示button是被按下的狀態，如果沒有被按都會是0
        TA0CTL |= TACLR;
    } else {//button_up mode
        P1IES |= BIT3; //button_down interrupt
        change_to_normal = 1;
    }
}

// LED flash
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void)
{
    if (state == normal) {
        if (led_state == 0) led_state = 1;
        else if (led_state == 1) led_state = 2;
        else if (led_state == 2) led_state = 3;
        else if (led_state == 3) led_state = 4;
        else if (led_state == 4) led_state = 5;
        else if (led_state == 5) led_state = 0;
    }
    else if (state == measure_low) {
        if (led_state == 6) led_state = 7;
        else if (led_state == 7) led_state = 6;
    } else {
        if (led_state == 8) led_state = 9;
        else if (led_state == 9) led_state = 8;
    }
    TA1CTL |= TACLR;
}
