#define ledPin 13

/*

code by sam galison, summer of 2015
This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

NOTES:
this code uses timer1, which controls pwm on pins 11 & 12, so don't use those pins

hardware: LED on pin 13

references:

AVR timer datasheet:
http://www.atmel.com/Images/doc2505.pdf

good tutorial:
https://arduinodiy.wordpress.com/2012/02/28/timer-interrupts/

timer calculator + reference:
https://docs.google.com/spreadsheets/d/1Kq19yXfzRZ7rHQM-4t8iTiXk9fSxJ60MgJrKHpM9SC4/pubhtml?gid=956017403&single=true


*/


volatile int state = LOW;

int previousState = LOW
    ;

float amplitude = 0,
decaySpeed = 0.01;


void setup()
{
  pinMode(ledPin, OUTPUT);

  // initialize timer1
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;

  TCCR1B |= (1 << WGM12); // turn on ctc mode

  /* TCNT1 = 3035;            // preload timer 65536-16MHz/256/2Hz */
  TCCR1B |= (1 << CS12);    // set prescaler to 256
  /* TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt */
  TIMSK1 |= (1 << OCIE1A); // enable CTC interrupt



  OCR1A = 31249; // desired match to compare against

  interrupts();             // enable all interrupts

  Serial.begin(9600);
}

/* ISR(TIMER1_OVF_vect)        // overflow ISR
/* { */
/*   /\* TCNT1 = 3035;            // preload timer *\/ */
/*   state = !state; */
/*   digitalWrite(ledPin, state); */
/* } */

ISR(TIMER1_COMPA_vect) { // compare match ISR, occurs when match happens
  state = !state;
  amplitude = 1;
  /* digitalWrite(ledPin, state); */
}


void loop() {
  /* if (state != previousState) { */
  /*   amplitude = 1; */
  /* } */

  if (amplitude >= decaySpeed) amplitude -= decaySpeed;
  analogWrite(ledPin, amplitude*255);

  Serial.println(state);
}
