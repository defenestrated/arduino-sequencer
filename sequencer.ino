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

  tempo 120, 64th notes
  3/4 time

  counts per subdiv: 15624
  prescale: 8 // 0	1	0
  subdivs per beat	16
  beats per bar	3

  "measure" is my terminology for the cycle

*/


volatile int state = LOW;

boolean newtick = true,
  newbeat = true,
  newbar = true,
  newmeasure = true;

int previousState = LOW,
  subdivisions = 64,
  subdivsPerBeat = 16,
  beatsPerBar = 3,
  barsPerMeasure = 4,
  ticks = 1,
  beats = 1,
  bars = 1
  ;

int ctk = 1, ptk = 0;

float amplitude = 0,
  decaySpeed = 0.01;

int LEDs[] =      {3, 5, 7, 8, 9, 13};
int ledStates[] = {0, 0, 0, 0, 0, 0};


void setup()
{
  for (int i = 0; i < 6; i++) {
    pinMode(LEDs[i], OUTPUT);
  }

  // initialize timer1
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;

  TCCR1B |= (1 << WGM12); // turn on ctc mode

  /* TCNT1 = 3035;            // preload timer 65536-16MHz/256/2Hz */
  TCCR1B |= (0 << CS10);    // prescale register, see sheet for config
  TCCR1B |= (1 << CS11);    // prescale register, see sheet for config
  TCCR1B |= (0 << CS12);    // prescale register, see sheet for config

  /* TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt */
  TIMSK1 |= (1 << OCIE1A); // enable CTC interrupt



  OCR1A = 15624; // desired match to compare against (counts per note subdivision)

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

  newtick = true  ;

  if (ticks < subdivsPerBeat) ticks ++;
  else {
    ticks = 1;
    // new beat
    newbeat = true;
    ledStates[3] = !ledStates[3];

    if (beats < beatsPerBar) beats ++;
    else {
      beats = 1;
      // new bar
      newbar = true;
      ledStates[5] = !ledStates[5];

      if (bars < barsPerMeasure) bars ++;
      else {
        // new measure (cycle resets)
        newmeasure = true;
        ledStates[4] = !ledStates[4];
        bars = 1;
      }
    }
  }
}


void loop() {

  for (int i = 0; i < 6; i++) {
    digitalWrite(LEDs[i], ledStates[i]*255);
  }

}
