#include <msp430.h>
#include <stdio.h>
#include <string.h>

#define UART_TXD 0x02 // TXD on P1.1 (Timer0_A.OUT0)
#define UART_RXD 0x04 // RXD on P1.2 (Timer0_A.CCI1A)
#define UART_TBIT_DIV_2 (1000000 / (9600 * 2))
#define UART_TBIT (1000000 / 9600)
#define normal 0
#define enhanced 1

unsigned int txData;    // UART internal TX variable
char rxBuffer[20]; // Received UART character
char* ack = "Ack!\r\n";
int num = 0;
int state = 0;
char slen[200];
int hello = 0;
int check = 0;
int first = 0;
int check_input = 0;
volatile int Temp[5];
int adc_count = 0;
int i;
volatile double t = 0;
volatile double sum = 0;
volatile double average = 0;
int int_average = 0;
char char_average[20];
int send = 0;
int threshold = 740;
int led_state = 0;
// 0: green on, 1: green off, 2: red on, 3: red off

void TimerA_UART_init(void);
void TimerA_UART_tx(unsigned char byte);
void TimerA_UART_print(char *string);
void flash(char id, int on, int off);
void temp(int interval, int times);

void main(void)
{
    P1DIR |= 0x41; //設成1表示output，設成0表示input，default是0
    P1OUT &= ~0x41; //控制LED全暗

    WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer
    DCOCTL = 0x00;            // Set DCOCLK to 1MHz
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    P1OUT = 0x00;                // Initialize all GPIO
    P1SEL = UART_TXD + UART_RXD; // Use TXD/RXD pins
    P1DIR = 0xFF & ~UART_RXD;    // Set pins to output

    // timer 0 for sampling
    // TA0CTL |= MC_3|ID_3|TASSEL_2|TACLR;
    // TA0CCTL1 = OUTMOD_3;
    // TA0CCTL0 |= CCIE; // enable timer 0的interrupt
    // TA0CCR0 = 17187; // 1sec
    TA0CTL |= MC_1| ID_3 | TASSEL_1|TACLR;
    TA0CCTL1 = OUTMOD_3;
    TA0CCTL0 |= CCIE; // enable timer 0的interrupt
    TA0CCR0 = 12000; // 1sec

    // tiemr 1 for led
    BCSCTL3 |= LFXT1S_2; // 使用VLO clock
    TA1CTL |= MC_1 | TASSEL_1 | ID_0 | TACLR | ID_0;
    TA1CCTL0 |= CCIE; // enable timer 1的interrupt
    //TA1CCR0 = 1200;  //0.1 sec
    TA1CCR0 = 10800-1; // 0.9 sec
    TA1CCTL0 &= ~CCIFG;

    // ADC
    ADC10CTL0 = ADC10SHT_2 | ADC10ON | ADC10IE | SREF_1 | REFON;
    ADC10CTL0 &= ~REF2_5V;
    ADC10CTL1 = SHS_1 | CONSEQ_2 | INCH_10;    // Input from A1
    ADC10CTL0 |= ENC + ADC10SC; // open sampling

    __enable_interrupt();
    TimerA_UART_init(); // Start Timer_A UART
    TimerA_UART_print("G2xx3 TimerA UART\r\n");
    TimerA_UART_print("READY.\r\n");
    flash('1', 900, 500);
    temp(1, 2);
    for (;;)
    {
        if (state == normal) {
            // Wait for incoming character
            __bis_SR_register(LPM3_bits);
            sum = 0;
            sum += adc[adc_count];
            if (adc_count == 0) {
                sum += adc[63];
            }
            else {
                sum += adc[adc_count-1];
            }
            sum = sum * 1.5 / 1023.0;
            average = (sum/2 - 0.986) / 0.00355;
            if (average > threshold) {
                state = enhanced;
                flash('0', 200, 300);
                ADC10CTL0 &= ~ENC; // close sampling
                TimerA_UART_init(); // Start Timer_A UART
                TimerA_UART_print("Emergency!\r\n");
                int_average = average;
                sprintf(char_average, "%d\r\n", int_average);
            }
        }
        if ((check_input == 1) && (state == enhanced)) {
            if (strcmp(rxBuffer, ack) == 0) {
                state = normal;
                flash('1', 900, 500);
                temp(1, 2);
            }
            for(i = num-1; i >= 0; i--) {
                rxBuffer[i] = '\0';
            }
            num = 0;
            check_input = 0;
        }
    }
}

// sensor sampling
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
    adc[adc_count] = ADC10MEM;
    if (adc_count != 63) {
        adc_count++;
    }
    else {
        adc_count = 0;
    }
    _BIC_SR(LPM3_EXIT);
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void)
{
    //P1OUT ^= 0x01; //控制LED全暗

    TA1CTL &= ~TAIFG;
    TA1CTL |= TACLR;
    if (led_state == 0) {
        P1OUT &= ~0x41; //控制LED全暗
        led_state = 1;
        TA1CCR0 = 6000-1;  //0.5 sec
    }
    else if (led_state == 1) {
        P1OUT |= 0x40; // green led
        led_state = 0;
        TA1CCR0 = 10800-1; // 0.9 sec
    }
    else if (led_state == 2) {
        P1OUT &= ~0x41; //控制LED全暗
        led_state = 3;
        TA1CCR0 = 3600;  //0.3 sec
        send++;
    }
    else if (led_state == 3) {
        P1OUT |= 0x01; // red led
        led_state = 2;
        TA1CCR0 = 2400; // 0.2 sec
        send++;
    }
    else {
        P1OUT |= 0x41; // error
    }
    if ((send == 4) && (state == enhanced)) {
        TimerA_UART_print(char_average);
        send = 0;
    }
}

void TimerA_UART_print(char *string)
{
    while (*string)
        TimerA_UART_tx(*string++);
}

void TimerA_UART_init(void)
{
    TA0CCTL0 = OUT;                    // Set TXD idle as '1'
    TA0CCTL1 = SCS + CM1 + CAP + CCIE; // CCIS1 = 0
    // Set RXD: sync, neg edge, capture, interrupt
    TA0CTL = TASSEL_2 + MC_2; // SMCLK, continuous mode
}

void TimerA_UART_tx(unsigned char byte)
{
    while (TA0CCTL0 & CCIE);   // Ensure last char TX'd
    TA0CCR0 = TA0R;            // Current count of TA counter
    TA0CCR0 += UART_TBIT;      // One bit time till 1st bit
    TA0CCTL0 = OUTMOD0 + CCIE; // Set TXD on EQU0, Int
    txData = byte;             // Load char to be TXD
    txData |= 0x300;           // Add stop bit to TXData
    txData <<= 1;              // Add start bit
}

#pragma vector = TIMER0_A0_VECTOR // TXD interrupt
__interrupt void Timer_A0_ISR(void)
{
    static unsigned char txBitCnt = 11;
    TA0CCR0 += UART_TBIT; // Set TA0CCR0 for next intrp
    if (txBitCnt == 0)
    {                      // All bits TXed?
        TA0CCTL0 &= ~CCIE; // Yes, disable intrpt
        txBitCnt = 11;     // Re-load bit counter
    }
    else
    {
        if (txData & 0x01)
        {                         // Check next bit to TX
            TA0CCTL0 &= ~OUTMOD2; // TX '1’ by OUTMODE0/OUT
        }
        else
        {
            TA0CCTL0 |= OUTMOD2;
        } // TX '0‘
        txData >>= 1;
        txBitCnt--;
    }
}

#pragma vector = TIMER0_A1_VECTOR // RXD interrupt
__interrupt void Timer_A1_ISR(void)
{
    static unsigned char rxBitCnt = 8;
    static unsigned char rxData = 0;
    switch (__even_in_range(TA0IV, TA0IV_TAIFG))
    {
    case TA0IV_TACCR1:        // TACCR1 - UART RXD
        TA0CCR1 += UART_TBIT; // Set TACCR1 for next int
        if (TA0CCTL1 & CAP)
        {                               // On start bit edge
            TA0CCTL1 &= ~CAP;           // Switch to compare mode
            TA0CCR1 += UART_TBIT_DIV_2; // To middle of D0
        }
        else
        { // Get next data bit
            rxData >>= 1;
            if (TA0CCTL1 & SCCI)
            { // Get bit from latch
                rxData |= 0x80;
            }
            rxBitCnt--;
            if (rxBitCnt == 0)
            {                      // All bits RXed?
                rxBuffer[num++] = rxData; // Store in global
                rxBitCnt = 8;      // Re-load bit counter
                TA0CCTL1 |= CAP;   // Switch to capture
                if (rxData == '\r') {
                    rxBuffer[num++] = '\n';
                    check_input = 1;
                }
            }
        }
        break;
    }
}

void flash(char id, int on, int off) {
    TA1CTL |= TACLR;
    if (id == '0') {
        P1OUT |= 0x01; // red led
        led_state = 2;
        TA1CCR0 = 2400; // 0.2 sec
    }
    else {
        P1OUT |= 0x40; // green led
        led_state = 0;
        TA1CCR0 = 10800-1; // 0.9 sec
    }
}

void temp(int interval, int times) {
    // TA0CTL |= TACLR;
    // TA0CCR0 = 17187; // 1sec
    // TA0CCR1 = 17186;
    // ADC10CTL0 |= ENC; // start sampling

    TA0CTL |= MC_1| ID_0 | TASSEL_1|TACLR;
    TA0CCTL1 = OUTMOD_3;
    TA0CCTL0 |= CCIE; // enable timer 0的interrupt
    TA0CCR0 = 12000; // 1sec
    TA0CCR1 = 12000-1;
    
    ADC10SA = (int)Temp;
    ADC10DTC1 = times;
    ADC10CTL0 |= ENC + ADC10SC;
}