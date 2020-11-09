#include <msp430.h>
#include <stdio.h>

#define UART_TXD 0x02 // TXD on P1.1 (Timer0_A.OUT0)
#define UART_RXD 0x04 // RXD on P1.2 (Timer0_A.CCI1A)
#define UART_TBIT_DIV_2 (1000000 / (9600 * 2))
#define UART_TBIT (1000000 / 9600)
#define normal 0
#define enhanced 1
unsigned int txData;    // UART internal TX variable
unsigned char rxBuffer; // Received UART character


void TimerA_UART_init(void);
void TimerA_UART_tx(unsigned char byte);
void TimerA_UART_print(char *string);
void flash(int id, int on, int off);
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

    // timer1 for led
    BCSCTL3 |= LFXT1S_2; // 使用VLO clock
    TA1CTL |= MC_1 | TASSEL_1 | TACLR | ID_3;
    TA1CCTL0 |= CCIE; // enable timer 1的interrupt

    // timer0 for sampling and communication
    TA0CTL |= MC_1| ID_3 | TASSEL_1|TACLR;
    TA0CCTL1 = OUTMOD_3;
    TA0CCTL0 |= CCIE; // enable timer 0的interrupt

    // ADC
    ADC10CTL0 = ADC10SHT_2 | ADC10ON | ADC10IE | SREF_1 | REFON;
    ADC10CTL0 &= ~REF2_5V;
    ADC10CTL1 = SHS_1 | CONSEQ_2 | INCH_10;    // Input from A1
    ADC10CTL0 |= ENC + ADC10SC; // open sampling

    __enable_interrupt();
    TimerA_UART_init(); // Start Timer_A UART
    TimerA_UART_print("G2xx3 TimerA UART\r\n");
    TimerA_UART_print("READY.\r\n");

    flash(1, 1350, 750);
    temp(1500, 1);
    for (;;)
    {
        
    }
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void){
    ADC10SA = (int)Temp;
    avgNow = Temp[0];
    avg = (avgNow + avgPre) / 2;


    if(avg > 740){ // 要連續2秒測到
        ADC10DTC1 = 0;
        flash(0, 300, 450);
        TimerA_UART_init();     // Start Timer_A UART
        TimerA_UART_print("Emergency!\r\n");
        receiveCorrect = 1;
        _BIC_SR(LPM3_EXIT);
    }

    avgPre = avgNow;
    ADC10CTL0 |= ENC + ADC10SC;
}

void temp(int interval, int times) {
    TA0CTL |= MC_1| ID_3 | TASSEL_1|TACLR;
    TA0CCTL1 = OUTMOD_3;
    TA0CCTL0 |= CCIE; // enable timer 0的interrupt
    TA0CCR0 = interval; // 1sec
    TA0CCR1 = interval-1;
    
    ADC10SA = (int)Temp;
    ADC10DTC1 = times;
    ADC10CTL0 |= ENC + ADC10SC;
}

void flash(int id, int on, int off) {
    TA1CTL |= TACLR;
    if (id == 0) {
        P1OUT |= 0x01; // red led
        led_state = 2;
        TA1CCR0 = on; // 0.2 sec
    }
    else {
        P1OUT |= 0x40; // green led
        led_state = 0;
        TA1CCR0 = on; // 0.9 sec
    }
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
        TA1CCR0 = 750;  //0.5 sec
    }
    else if (led_state == 1) {
        P1OUT |= 0x40; // green led
        led_state = 0;
        TA1CCR0 = 1350; // 0.9 sec
    }
    else if (led_state == 2) {
        P1OUT &= ~0x41; //控制LED全暗
        led_state = 3;
        TA1CCR0 = 450;  //0.3 sec
        send++;
    }
    else if (led_state == 3) {
        P1OUT |= 0x01; // red led
        led_state = 2;
        TA1CCR0 = 300; // 0.2 sec
        send++;
    }
    else {
        P1OUT |= 0x41; // error
    }
    if ((send == 4) && (state == enhanced)) {
        TimerA_UART_print(char_average);
        send = 0;
    }
    _BIC_SR(LPM3_EXIT);
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
    txData |= 0x100;           // Add stop bit to TXData
    txData <<= 1;              // Add start bit
}

#pragma vector = TIMER0_A0_VECTOR // TXD interrupt
__interrupt void Timer_A0_ISR(void)
{
    static unsigned char txBitCnt = 10;
    TA0CCR0 += UART_TBIT; // Set TA0CCR0 for next intrp
    if (txBitCnt == 0)
    {                      // All bits TXed?
        TA0CCTL0 &= ~CCIE; // Yes, disable intrpt
        txBitCnt = 10;     // Re-load bit counter
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
                rxBuffer = rxData; // Store in global
                rxBitCnt = 8;      // Re-load bit counter
                TA0CCTL1 |= CAP;   // Switch to capture
                __bic_SR_register_on_exit(LPM0_bits);
                // Clear LPM0 bits SR
            }
        }
        break;
    }
}
