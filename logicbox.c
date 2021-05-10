/*------------------------
PROGRAMME TO PRESENT LOGICAL OPERATORS IN A HANDS-ON WAY
BY FACELESSMOOK - YOU ARE FREE TO USE THIS IN ANY WAY YOU WANT - CC0, "NO RIGHTS RESERVED"

USES AN ATTINY24 - A DIP-14, 2KB FLASH MCU FROM ATMEL

THREE MOMENTARY PUSH BUTTONS WILL LATCH THE RELEVANT LOGICAL OPERATORS (OR; AND; XOR)
-THIS IS READ VIA PIN CHANGE INTERRUPTS AND SETS THE RELEVANT LED TO 'ON'
TWO TOGGLE SWITCHES SET THE INPUTS A & B AND PROCESS THEM USING THE SELECTED LOGICAL OPERATOR
-THIS IS READ VIA PIN CHANGE INTERRUPTS
ONE FURTHER TOGGLE SWITCH REPRESENTS NOT AND WILL INVERT THE OUTPUT OF THE LOGICAL OPERATOR
-THIS IS READ VIA THE SAME PIN CHANGE INTERRUPT AS THE INPUTS A & B

DEV NOTES:
-CODE ORGINALLY HAD MORE FUNCTIONS AND ENCAPSULATION BUT ISRs DONT SEEM TO LIKE TOO ...
 MANY LAYERS OF ABSTRACTION REGARDING FUNCTIONS (I.E ONE FUNCTION CALLING ANOTHER)
-CASE STATEMENTS COULD BE USED TO SET THE LOGICAL OPERATORS
-I ELECTED TO SNAPSHOT THE REGISTER WHEN THE LOGICAL ISR WAS FIRED SO THAT BOUNCE WOULD BE MINIMISED..
 OR AT LEAST MADE MORE CONSISTENT - CASE STATEMENTS WOULD PROBABLY ELIMINATE THE NEED FOR THIS..
 IF REQUIRED AT ALL
-THE 'TIMEWASTE' VARIABLE CAN PROBABLY BE REMOVED, I ORIGINALLY IMPLEMENTED WHEN I WAS TRYING TO DEBUG..
 AS THE 'OR' INPUT DIDN'T WANT TO LATCH SO I TRIED TO GIVE THE MCU TIME TO CATCH UP AFTER THE ISR FIRE..
 , FOR WHICH THERE IS NO BASIS, I JUST THOUGHT 'LETS TRY IT'
-I LIKE HANDLING ALL OF MY BEHAVIOUR USING INTERRUPTS RATHER THAN IN THE 'WHILE' LOOP - IT JUST SEEMS..
 MORE EFFICIENT

--------------------------*/
#define OR_IN PB0
#define OR_OUT PA0

#define AND_IN PB1
#define AND_OUT PA1

#define XOR_IN PB2
#define XOR_OUT PA2

#define NOT_IN PA7

#define INPUT_A PA6
#define INPUT_B PA5
#define OUTPUT_X PA3

#include <avr/io.h>
#include <avr/interrupt.h>

volatile unsigned char a = 0;
volatile unsigned char b = 0;
volatile unsigned char timewaste = 0;

//READS THE RELATIVE LOGICAL OPERATOR PIN AND XORS IT WITH 'NOT' RESULT
unsigned char calc_output(void)
{
    unsigned char not = (PINA & (1 << NOT_IN)) ? 1 : 0;
    if (PORTA & (1 << OR_OUT))
    {
        return ((a | b) ^ not );
    }
    else if (PORTA & (1 << AND_OUT))
    {
        return ((a & b) ^ not );
    }
    else if (PORTA & (1 << XOR_OUT))
    {
        return ((a ^ b) ^ not );
    }
    else
    {
        return 0;
    }
}

//SETS THE OUTPUT
void set_output(void)
{
    PORTA = (PORTA & ~(1 << OUTPUT_X));
    PORTA = (PORTA | (calc_output() << OUTPUT_X));
}

//DETECTS AND LATCHES THE RELEVANT LOGICAL OPERATOR
ISR(PCINT1_vect)
{
    timewaste ^= 1;
    volatile unsigned char pinb_copy = PINB;

    if (pinb_copy & (1 << OR_IN))
    {
        PORTA = (PORTA | (1 << OR_OUT));
        PORTA = (PORTA & ~(1 << AND_OUT) & ~(1 << XOR_OUT));
        set_output();
        reti();
    }
    if (pinb_copy & (1 << AND_IN))
    {
        PORTA = (PORTA | (1 << AND_OUT));
        PORTA = (PORTA & ~(1 << OR_OUT) & ~(1 << XOR_OUT));
        set_output();
        reti();
    }
    if (pinb_copy & (1 << XOR_IN))
    {
        PORTA = (PORTA | (1 << XOR_OUT));
        PORTA = (PORTA & ~(1 << AND_OUT) & ~(1 << OR_OUT));
        set_output();
        reti();
    }
}

//DETECTS AND PROCESSES INPUT PINS
ISR(PCINT0_vect)
{
    unsigned char pina_copy = PINA;
    a = (pina_copy & (1 << INPUT_A)) ? 1 : 0;
    b = (pina_copy & (1 << INPUT_B)) ? 1 : 0;
    set_output();
}

int main(void)
{
    DDRA |= (1 << OR_OUT) | (1 << AND_OUT) | (1 << XOR_OUT) | (1 << OUTPUT_X);
    //SETS PIN CHANGE INTERRUPT GROUPS - 0 AND 1
    GIMSK |= (1 << PCIE1) | (1 << PCIE0);
    //ENABLES OR & AND & XOR INPUT PIN CHANGE INTERRUPTS SPECIFICALLY
    PCMSK1 |= (1 << PCINT8) | (1 << PCINT9) | (1 << PCINT10);
    //ENABLES NOT & INPUT A & INPUT B PIN CHANGE INTERRUPTS SPECIFICALLY
    PCMSK0 |= (1 << PCINT7) | (1 << PCINT6) | (1 << PCINT5);
    sei();

    while (1)
    {
        //EMPTY - EVERYTHING IS HANDLED BY INTERRUPTS
    }
}
