#include <msp430.h> // Button up/down ??LED on/off
#include <intrinsics.h> // Intrinsic functions

#define LED1 BIT0   // P1.0 to red LED
#define LED2 BIT6   // P1.6 to green LED
#define B1 BIT3     // P1.3 to button

int mode = 0;
int normal = 0, measure_low = 1, measure_high = 2 , measure_init = 3;
int start = 0;
int threshold = 10;
int event = 0;
int btn_up =0 , btn_down=1;
float time = 0;
int measure =0;
int index = 0;
volatile unsigned int val = 0;
volatile int average_temp = 0;
volatile int adc[5]={0};
int i= 0;
int Celsius = 0;
volatile unsigned int luke = 0;
void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;       // stop watchdog timer

    P1DIR = LED1 + LED2;            // Set pin with LED1 and LED2 to output
    P1OUT = B1 + LED1 + LED2;

     P1REN |= B1;                    // Set pin to use pull-up resistor
     P1IE |= B1;                     // B1 interrupt enabled
     P1IES |= B1;                    // B1 Hi/lo edge
     P1IFG &= ~B1;                   // B1 IFG cleared
     _BIS_SR(GIE);                   // Enter button interrupt

    // ACLK VLO
    BCSCTL1 &= ~XTS;                            // the VLO set can only be used when XTS = 0
    BCSCTL3 |= LFXT1S_2;                        // Enable VLO as MCLK or ACLK src

    // Timer 1 setting
    TA1CCR0 = 1199;                             // every 1200 times = 0.1 sec
    TA1CTL = MC_1 | ID_0 | TASSEL_1 | TACLR;    // count up, divider = 1, use the ACLK, clear the register
    TA1CCTL0 |= CCIE;                           // enable clock interrupt
    TA0CCR1 = 3125 - 2;

    // ADC10
    ADC10CTL0 = SREF_1 | ADC10SHT_2 | REFON | ADC10ON | ADC10IE;    // standard usage
    ADC10CTL1 = INCH_10 | SHS_1 | CONSEQ_2  | ADC10SSEL_3;          // temp sensor, S&H sensor source = TA.out1, repeat-single channel, use SMCLk
    ADC10CTL0 |= ENC + ADC10SC;                                     // enable sampling

    // SMLK DCO 1.5v
    DCOCTL = CALDCO_1MHZ;      // Set DCO as 1MHz
    BCSCTL1 = CALBC1_1MHZ;     // BCSCTL1 Calibration Data for 1MHz
    BCSCTL2 &= SELS;           // SELS = 0 -> select DCO as clk
    BCSCTL2 |= DIVS_3;         // use DCO as clk(default) source and divider = 8 => clk became 125KHz

    // Timer 0 setting
    TA0CCTL1 |= OUTMOD_3;                                           // OUTMOD3: The output (OUT1) is set when the timer counts to the TA0CCR1 value and is reset when the timer counts to the TA0CCR0 value.
    TA0CTL = MC_1 | ID_3 | TASSEL_2 | TACLR;                        // sel SMCLK ; mode = count_up ; divider = 3 ; Clear the current TAR0                                                                 // current clock become 125KHz / 8 = 15625 Hz
    TA0CCR0 = 3125 - 1;                                             // 0.2 sec = 3125 Hz

    _BIS_SR(GIE);
    ADC10DTC1 = 5;     // # of transfers
    ADC10SA = (int)adc; // Buffer starting address

    __enable_interrupt();
    for (;;) { }
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA0_ISR (void){
    time += 1.0;
    if(mode == normal){
        if(time <= 7.0){
            P1OUT &= ~0x41;
            P1OUT |= 0x01;
        }
        else if(time > 7.0 && time <= 10.0){
            P1OUT &= ~0x41;
            P1OUT |= 0x40;
            if(time == 10.0)
                time = 0;
        }
        for(i=0;i<5;i++)
            adc[i] = 0;
    }
    else if(mode == measure_low){
        if(time == 5.0 || time == 10.0){
            P1OUT ^= 0x41;
            if(time == 10.0)
                time = 0;
        }
    }
    else if(mode == measure_high){

        if(time == 2.5 || time == 5.0){
            P1OUT &= ~0x40;
            P1OUT ^= 0x01;
            if(time == 5.0)
                time = 0;
        }
    }

    if(event == btn_down){
        measure++;
    }
    else{
        luke = 0;
    }

    if(mode == normal){
        if(measure > 30){
            if(luke == 0){
                luke = 1;
                mode = measure_init;
                measure = 0;
                ADC10CTL0 |= ENC + ADC10SC; // enable sampling
            }
        }
        else{
            mode = normal;
        }
     }
    else if(mode == measure_low || mode == measure_high){
        ADC10CTL0 |= ENC + ADC10SC; // enable sampling
        if(measure > 20 ){
            if(luke==0){
                luke = 1;
                mode = normal;
            }
        }
    }
    TA1CTL |= TACLR;
    TA1CCTL0 &= ~CCIFG;
}


#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void){
    //val = ADC10MEM;
    if(mode == measure_low || mode == measure_high || mode == measure_init){
        double temp = 0;
        int sum=0;
        for(i=0;i<5;i++){
            temp = adc[i];
            temp = temp*1.5/1023;
            temp = (temp-0.986)/0.00355;
            sum += temp;
        }
        average_temp = sum/5;
        if(adc[0] && adc[1] && adc[2] &&adc[3] &&adc[4]){
            if(average_temp > threshold){
                mode = measure_high;
                P1OUT &= ~0x41;
                P1OUT |= 0x01;
                time = 0;
            }
            else{
                mode = measure_low;
                P1OUT &= ~0x41;
                P1OUT |= 0x41;
                time = 0;
            }
        }
        ADC10CTL0 |= ENC + ADC10SC; // enable sampling
    }
    else if(mode == normal){
        for(i=0;i<5;i++)
            adc[i] = 0;
    }
    ADC10SA = (int)adc;

}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
    P1IFG &= ~B1;             // B1 IFG cleared
    P1IES ^= B1;                // B1 Hi/lo edge

    if(event == btn_up){
        event = btn_down;
        measure = 0;

        TA1CTL &= ~TAIFG;                   // clear the interrupt flag
        TA1CTL |= TACLR;

        TA1CCTL0 = CCIE;                    // Enable Timer1 interrupts
        __enable_interrupt();               // Enable interrupts (intrinsic)
    }
    else if(event == btn_down){
        event = btn_up;
        TA0CTL &= ~TAIFG;
        TA0CTL |= TACLR;
        __enable_interrupt();
    }
}
