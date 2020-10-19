#include <msp430.h>

volatile unsigned int flag = 1;
volatile unsigned int count = 0;
unsigned int record;
unsigned int n = 0;
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    P1DIR |= 0x41; //設成1表示output，設成0表示input，default是0

    //button set
    //P1裡面有很多個裝置，而button是P1.3，所以我們設定都是設BIT3
    P1REN |= BIT3; //use resister，也就是要使用這個硬體所相對的reg，這樣才能控制這個設備
    P1OUT |= BIT3; //pull-up，
    P1IE |= BIT3; //P1.3 interrupt enable
    P1IES |= BIT3; //button_down interrupt，設成1的時候表示要偵測button按下的瞬間，設成0表示buuton放開的瞬間
    P1IFG &= ~BIT3; // 當P1註冊好的intterupt發生時，並且P1IE也是1(表示enable)的時候，就會被拉起成1。
                    //只是在處理interrupt的地方要自己把P1IFG關掉(設成0)
    //clk set
    TA0CTL |= MC_1|ID_3|TASSEL_1|TACLR;
    // MC系列：控制timer計時的模式
    // MC_0: 暫停timer, MC_1: 從0往上數到TA0CCR0
    // MC_2: 一直從0數到timer最大值(0FFFFh) ,MC_3:從0數到TA0CCR0再往下數到0(up and down)
    
    // ID系列：表示input clock要再除以多少(也就是clock divider)
    // ID_0: /1, ID_1: /2, ID_2: /4, ID_3: /8

    // TASSEL系列：選擇不同種類的clock source
    // TASSEL_0: TACLK, TASSEL_1: ACLK
    // TASSEL_2: SMCLK, TASSEL_3: INCLK

    // TACLR: 把timer的時間歸零
    TA1CTL |= MC_1 | ID_3 | TASSEL_2 | TACLR;
    TA1CCR0 = 0; // timer1要數到的上限

    /*BCSCTL是basic clock system control，可以控制一些clock的基本設定*/
    /*(講義L04 timer p.46)*/
    // BCSCTL1管ACLK, DCO
    // BCSCTL2管MCLK, SMCLK
    // BCSCTL3管LFT1/VLO

    /*這次lab裡面要求SMCLK source from DCO, SMCLK source from VLO running at 12Hz*/
    BCSCTL3 |= LFXT1S_2; // 使用VLO clock
    BCSCTL2 |= DIVS_3; // DIVS_x是SMCLK的divider，算法和前面的ID一樣
    TA0CCTL0 |= CCIE; // enable timer 0的interrupt
    TA1CCTL0 |= CCIE;// enable timer 1的interrupt
    TA0CCR0 = 749;


    P1OUT &= ~0x41;//控制LED全暗
    P1OUT |= 0x01; //red on
    __enable_interrupt();
    for(;;){}
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
    P1IFG &= ~BIT3; //rst

    if(P1IES & BIT3) //button_down mode
    {
        P1IES &= ~BIT3; //button_up interrupt
        //TA1CTL = MC_1 | ID_3 | TASSEL_2 | TACLR;
        TA1CCR0 = 171;
        count  = 0;
    }
    else //button_up mode
    {
        P1IES |= BIT3; //button_down interrupt
        record = count;

        TA1CTL |= TACLR;
        TA0CTL |= TACLR;

        if(count >= 300){
            P1OUT |= 0x41;
            TA0CCR0 = 749;
            flag = 0;
        }
    }
}
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void){
    if(!flag){
        if(record <= 0){
            flag = 1;
            n = 0;
            P1OUT &= ~0x41;
            P1OUT |= 0x01;
            TA0CCR0 = 749;
            TA0CTL |= TACLR;
        }else{
            record = record - 1;
        }
    }

    count = count + 1;
}
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR (void){

    if(!flag) //button_down
    {
        P1OUT ^= 0x41;
    }
    else //button_up
    {
        if(n == 6) n = 0;
        n = n + 1;
        TA0CTL |= TACLR;

        if(n < 4){
            P1OUT ^= 0x01;
        }else if(n == 4){
            TA0CCR0 = 1349;
             P1OUT ^= 0x40;
        }else if(n == 5){
            P1OUT ^= 0x40;
            TA0CCR0 = 749;
        }else if(n == 6){
            P1OUT |= 0x01;
        }
    }
}

