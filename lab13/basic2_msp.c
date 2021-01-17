#include "msp430.h"
#include "stdio.h"
#define UART_TXD 0x02 // TXD on P1.1 (Timer0_A.OUT0)
#define UART_RXD 0x04 // RXD on P1.2 (Timer0_A.CCI1A)
#define UART_TBIT_DIV_2     (1000000 / (9600 * 2))
#define UART_TBIT           (1000000 / 9600)
#define B1 BIT3
unsigned int txData;  // UART internal TX variable
unsigned char rxBuffer; // Received UART character
int press = 0;
int print_count = 0, print = 0, i;
int light = 0;

char rxStr[20], num[5];
int strCnt = 0;

void TimerA_UART_init(void);
void TimerA_UART_tx(unsigned char byte);
void TimerA_UART_print(char *string);

void main(void) {
    WDTCTL = WDTPW + WDTHOLD;
    DCOCTL = 0x00;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    TA1CCR0 = 24000-1;
    TA1CTL |= MC_1|ID_0|TASSEL_1|TACLR; //Setup Timer0_A
    TA1CCTL0 = CCIE; // Enable interrupts

    P1OUT = 0x00 | B1;
    P1SEL = UART_TXD + UART_RXD;
    P1DIR = 0xFF & ~UART_RXD & ~B1;
    P1REN = B1;
    P1IE |= B1;
    P1IES |= B1;
    P1IFG &= ~B1;

    BCSCTL3 |= LFXT1S_2;

    _BIS_SR(GIE);
    __enable_interrupt();

    TimerA_UART_init();
    TimerA_UART_print("G2xx3 TimerA UART\r\n");
    TimerA_UART_print("READY.\r\n");
    for (;;) {
        if (press) {
            TimerA_UART_print("1");
            press = 0;
        }
    }
}

#pragma vector=TIMER1_A0_VECTOR;
__interrupt void TA0_ISR(void) {
    P1OUT &= ~0x41;
    light = 0;
    TA1CTL |= TACLR;
    TA1CTL &= ~TAIFG;
}

void TimerA_UART_print(char *string) {
    while (*string) TimerA_UART_tx(*string++);
}

void TimerA_UART_init(void) {
    TA0CCTL0 = OUT;
    TA0CCTL1 = SCS + CM1 + CAP + CCIE;
    TA0CTL = TASSEL_2 + MC_2;
}

void TimerA_UART_tx(unsigned char byte) {
    while (TA0CCTL0 & CCIE);
    TA0CCR0 = TA0R;
    TA0CCR0 += UART_TBIT;
    TA0CCTL0 = OUTMOD0 + CCIE;
    txData = byte;
    txData |= 0x100;
    txData <<= 1;
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void) {
    static unsigned char txBitCnt = 10;
    TA0CCR0 += UART_TBIT;
    if (txBitCnt == 0) {
      TA0CCTL0 &= ~CCIE;
      txBitCnt = 10;
    } else {
        if (txData & 0x01) {
            TA0CCTL0 &= ~OUTMOD2;
        } else {
            TA0CCTL0 |= OUTMOD2;
        }
        txData >>= 1;
        txBitCnt--;
    }
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A1_ISR(void) {
    static unsigned char rxBitCnt = 8;
    static unsigned char rxData = 0;
    switch (__even_in_range(TA0IV, TA0IV_TAIFG)) {
        case TA0IV_TACCR1:
            TA0CCR1 += UART_TBIT;
            if (TA0CCTL1 & CAP) {
                TA0CCTL1 &= ~CAP;
                TA0CCR1 += UART_TBIT_DIV_2;
            } else {
                rxData >>= 1;
                if (TA0CCTL1 & SCCI) {
                    rxData |= 0x80;
                }
                rxBitCnt--;
                if (rxBitCnt == 0) {
                    rxBuffer = rxData;
                    rxBitCnt = 8;
                    TA0CCTL1 |= CAP;

                    if (rxData == '7') {
                        light = 1;
                        TA1CTL |= TACLR;
                        P1OUT &= ~0x41;
                        P1OUT |= 0x01;     // Toggle red LEDs
                    }
                }
            }
        break;
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
    press = 1;
    P1IFG &= ~B1;
}

