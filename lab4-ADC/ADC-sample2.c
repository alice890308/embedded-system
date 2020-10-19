#include <msp430.h>;

REFON = 1;    // 表示enable internal reference, 0表示不行
REF2_5V = 1;  // 表示2.5V，如果是0表示1.5V
ADC10MEM;     // 10 bits out of ADC conversion
ADC10IE = 1;  // enable ADC10的interrupt，然後也要設定GIE
ADC10IFG;     //如果出現interrupt這個訊號就會被拉起來，進到處理interrupt的function後會自動被清掉，或是使用者也可以自己清
              // - if 沒有使用DTC(ADC的記憶體) => ADC10DTC1 = 0
              //   ADC10IFG在conversion完後就會直接拉起來
              // - if 有使用 => ADC10DTC1 > 0
              //   在整個block搬到DTC之後才會拉起來
ADC10AE0 = 1; //如果是1表示可以接受analog訊號
/*更多訊號可以看ADC ppt p.31*/

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // Stop WDT
    // TA1 trigger sample start
    // SHS_x: 設定要用哪個timer作為sample跟取樣的trigger
    // SHS_1: timerA.OUT1

    //CONSEQ_x: select conversion sequence mode
    // CONSEQ_2: repeat single channel
    ADC10CTL1 = SHS_1 + CONSEQ_2 + INCH_1;
    // SREF_1：用來設定高電位跟低電位的參考值，在p.31有完整的數值說明
    // REFON: 啟用SREF_X所設定的參考值，如果沒有把REFON打開的話就會還是用預設值
    ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE;
    ADC10CTL0 |= ENC;   // ADC10 Enable
    ADC10AE0 |= 0x02;   // P1.1 ADC10 option select
    P1DIR |= 0x01;      // Set P1.0 output
    TA0CCR0 = 2048 - 1; // Sampling period
    // Timer0_A CCR1 out mode 3: The output (OUT1) is set when the timer counts to the
    // TA0CCR1 value and is reset when the timer counts to the TA0CCR0 value
    TA0CCTL1 = OUTMOD_3;      // TA0CCR1 set/reset
    TA0CCR1 = 2046;           // TA0CCR1 OUT1 on time
    TA0CTL = TASSEL_1 + MC_1; // ACLK, up mode
    while (1);
}

// ADC10 interrupt service routine
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
    if (ADC10MEM < 0x155) // ADC10MEM = A1 > 0.5V?
        P1OUT &= ~0x01;   // Clear P1.0 LED off
    else
        P1OUT |= 0x01; // Set P1.0 LED on
}
