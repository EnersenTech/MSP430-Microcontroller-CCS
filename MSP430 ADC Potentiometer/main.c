#include <msp430.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
// ADC in MSP430 by drselim
// Plese don't forget to give credits while sharing this code
// for the video description for the code:
// https://youtu.be/ZOqMVR4tNbI
int datatosend[] = {0b00000001, 0b00000011, 0b00000111, 0b00001111, 0b00011111, 0b00111111, 0b01111111, 0b11111111};
char vt_char[5];
char rm_char[5];
char mv_char[5];
char charmemval[] = "ADC10MEM Value: ";
char volt[] = "Voltage of the Potentiometer is: ";
char newline[] = " \r\n";
char dot[] = ".";
#define SERIAL BIT0 //   P1.0
#define SRCLK BIT4  //   P1.4
#define RCLK BIT5   //   P1.5

void SRCLK_Pulse (void);  //To create a clock pulse for Shift Reg
void RCLK_Pulse (void);   //To create a clock pulse for Storage Reg
void Send_Bit (unsigned int value);     //For sending 1 or zero
void ser_output(char *str);
void main(void)
{
    int x;
    int y;
    int i;
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    BCSCTL1= CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    // PxDIR 0 means Input mode, 1 means Output mode
    // SERIAL P1.0 = SERIAL INPUT Pin connected: Digital Data write to SErial Input Pin of IC chip
    // SRCLK P1.1 = STORAGE REGISTER CLOCK Control Pin
    // RCLK P1.2 = SHIFT REGISTER CLOCK Control Pin
    P1DIR |= SERIAL + SRCLK + RCLK;


    // PxSEL, PxSEL2 = 1,1 = secondary peripheral module function select P1.1 and P1.2
    // What is the 2nd peripheral module at pin1.1 and pin1.2? The answer is down below :
    // P1.1 2nd peripheral module is "UCA0RX0"; RXD
    // P1.2 2nd peripheral module is "UCA0TX0"; TXD
    P1SEL = BIT1|BIT2;
    P1SEL2 = BIT1|BIT2;

    // UART0 configuration
    // UCAxCTL = Universal serial communication interface
    // UCAxCTL0 Register and UCAxCTL1 Register exist
    // Two registers are both configuration modules.
    // UCAxCTL0 set Asynchronous or Synchronous. default is Asynchronous. So we skipped the UCA0CTL0 setup in this code.
    // If I want to use I2C module or other synchronous communication, i have to set the bit of UCA0CTL0. p.453

    UCA0CTL1 |= UCSWRST+UCSSEL_2; // Configure serial communication. p.454
    // UCSWRST = 0x001 = Enable USCI logic held in reset state = Software reset enable


    // Baud-Rate control. Baud-Rate = UCA0BR0 + UCA0BR1 * 256
    // https://electronics.stackexchange.com/questions/411004/msp430-baud-rate-generation-datasheet-discrepancy
    UCA0BR0 = 52; // UCAxBR0 Register = USCI_Ax Baud-Rate control
    UCA0BR1 = 0; // UCAxBR1 Register = UCSI_Ax Baud-Rate control

    // UCA00MCTL = USCI_A0 modulation control
    // page 456
    // UCA0MCTL = UCBRS_0 means second modulation stage select (BITCLK이 baud rate관련한 것인데 잘 모르겠음)
    // BITCLK Modulation Pattern UCBRS_0 = 0000'0000 (뭔지 확실히 모르겠음)
    UCA0MCTL = UCBRS_0;
    // Enable serial communication again.
    UCA0CTL1 &= ~UCSWRST;

    volatile float voltage;
    ADC10CTL0 = SREF_0|ADC10SHT_2|ADC10ON;
    ADC10CTL1 = INCH_3|SHS_0|ADC10DIV_0|ADC10SSEL_0|CONSEQ_0;
    ADC10AE0 = BIT3;  //P1.3 also above, INCH_3
    ADC10CTL0 |= ENC;


    while(1){
        ADC10CTL0 |= ADC10SC;
        while(ADC10CTL1 & ADC10BUSY);
        int memval = ADC10MEM;
        voltage = ((ADC10MEM*3.3)/1023); // mapping values
        int volt_int = floor(voltage);
        int rmndr = floor((voltage-volt_int)*1000);

        ltoa(memval, mv_char,10);
        ltoa(volt_int,vt_char,10);
        ltoa(rmndr,rm_char,10);
        ser_output(charmemval); ser_output(mv_char); ser_output(newline);
        ser_output(volt); ser_output(vt_char); ser_output(dot); ser_output(rm_char); ser_output(newline);
        if ( voltage <= 0.412 ) y=0;
        else if ( voltage <= 0.825 ) y=1;
        else if ( voltage <= 1.236 ) y=2;
        else if ( voltage <= 1.648 ) y=3;
        else if ( voltage <= 2.06 ) y=4;
        else if ( voltage <= 2.472 ) y=5;
        else if ( voltage <= 2.884 ) y=6;
        else y=7;
        for (i=0;i<8;i++){
                    x = (datatosend[y] & (1 << i));
                    Send_Bit(x);
                    SRCLK_Pulse();

                }
        RCLK_Pulse();
        __delay_cycles(100000);
    }
}

//
void ser_output(char *str){
    while(*str != 0){
        // Interrupt occurs (data input is not yet filled)
        // If data input is filled on the buffer, then send the data. Ready!!!
        while (!(IFG2&UCA0TXIFG));
        // send data one by one
        UCA0TXBUF = *str++;
    }
}
void SRCLK_Pulse (void)
{
  P1OUT |= SRCLK;
  P1OUT ^= SRCLK;
}
void RCLK_Pulse (void)
{
  P1OUT |= RCLK;
  P1OUT ^= RCLK;
}
void Send_Bit (unsigned int value)
{
    if (value != 0){
        P1OUT |= SERIAL;}
    else {
        P1OUT &= ~SERIAL;
    }
}
