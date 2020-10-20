#include <msp430.h>
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
  // H&S time 16x, interrupt enabled

  // ADC10SHT表示sample & hold time
  // 0: 4 *ADC10CLKs, 1: 8.., 2: 16個clock cylces

  // ADC10ON: 把ADC10打開
  /* ADC10CTL0, ADC10CTL1兩個都是控制ADC10，但控制的內容不一樣 */
  ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE;
  ADC10CTL1 = INCH_1;         // Input from A1, 選擇input chennel來源
  ADC10AE0 |= 0x02;           // Enable pin A1 for analog in, 第二個bit表示對A1的控制
  P1DIR |= 0x01;              // Set P1.0 to output, 因為第一個是對0的控制
  ADC10CTL0 |= ENC + ADC10SC; // Start sampling, 可以參考p.29的圖，反正就是開始sampling ouo
  // ADC10SC是ADC內建的timer(?) 每隔一段時間就會自己觸發一次。
  // 但如果使用其他外部的timer，就不用設定這個了
  for (;;);
}
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  if (ADC10MEM < 0x1FF)
    P1OUT &= ~0x01;
  else
    P1OUT |= 0x01
  ADC10CTL0 |= ENC + ADC10SC; // enable sampling
}