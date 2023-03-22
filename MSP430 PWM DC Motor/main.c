
#include<msp430.h>
void main(void)
{
    WDTCTL=WDTPW + WDTHOLD;

    ADC10CTL0 = ADC10ON + SREF_0 + ADC10SHT_2; //Enable ADC                                         //module, reference                                                                 //voltage, sample &                                                       //hold time
    ADC10CTL1 = INCH_3;                        //Select input channel 3
    ADC10AE0|= 0x08;                //Analog Enable channel 3
    P1DIR = BIT6 | BIT0 | BIT1;                //Make Port1-->Pin6 as output
    P1SEL |=BIT6;                //Select PWM Function
    P1SEL &= (~BIT0 | ~BIT1);
    TA0CCR0=1024;                   //Configure PWM time period
    TA0CCR1=1;                      //Configure PWM ON_TIME(Duty Cycle)
    ADC10CTL0|= ENC + ADC10SC;          //ADC enable conversion & start conversion
    TA0CCTL1|=OUTMOD_7 ;            //Select OUTPUT MODE 7; RESET/SET
    P1OUT |= BIT0;
    P1OUT &= ~BIT1;


    TA0CTL|= TASSEL_2 + ID_3 + MC_1 + TAIE;     //Configure Timer0,                                         //SMCLK,UP mode,                                            //Clock Source divide by 8,                                     //Interrupt Enable


    _BIS_SR(LPM0_bits + GIE);           //General Interrupt Enable
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void timer(void)
{
    ADC10CTL0|= ENC + ADC10SC;          //ADC enable conversion & start conversion
    TA0CCR1= ADC10MEM;              //Copy ADC buffer value into CCR1 to vary duty cycle
}
