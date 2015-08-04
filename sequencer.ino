/*

  code by sam galison, summer of 2015
  This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

  NOTES:
  this code uses timer1, which controls pwm on pins 11 & 12, so don't use those pins

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
#include <Beatkeeper.h>

using namespace std;

#define size( x )  ( sizeof( x ) / sizeof( *x ) )

boolean debug = true; // set false to disable serial

const int smoothing = 5;

boolean newtick = true,
  newbeat = true,
  newbar = true,
  newmeasure = true;

int previousState = LOW,
  subdivisions = 64,
  subdivsPerBeat = 64,
  beatsPerBar = 3,
  barsPerMeasure = 4
  ;

volatile int ticks = 1, // these change in timer routines
  beats = 1,
  bars = 1
  ;

int LEDs[] = {3, 5, 7, 8, 9, 13}; // pins that the leds are on

int sensors[] = {A0};
int proximities[size(sensors)];

Pattern patterns[6];


void setup()
{
  for (int i = 0; i < size(LEDs); i++) {
    pinMode(LEDs[i], OUTPUT);
  }

  for (int i = 0; i < size(sensors); i++) {
    pinMode(sensors[i], INPUT);
  }

  setupTimer();

  Serial.begin(9600);

  if (debug) {


    int n = size(LEDs);
    Serial.println("--- setup ---");
    Serial.print(n);
    Serial.print(" LEDs on pins ");
    for (int j = 0; j < n; j++) {
      Serial.print(LEDs[j]);
      if (j < n - 1) Serial.print(", ");
    }
    Serial.println();
    Serial.print(size(patterns));
    Serial.println(" patterns total");
    patterns[0].set(LEDs[0], String("10000000"));
    patterns[1].set(LEDs[1], String("01"));
    patterns[2].set(LEDs[2], String("000010000"));
    patterns[3].set(LEDs[3], String("100"));
    patterns[4].set(LEDs[4], String("1000"));
    patterns[5].set(LEDs[5], String("10"));



  }


}

/* ISR(TIMER1_OVF_vect)        // overflow ISR
   /* { */
/*   /\* TCNT1 = 3035;            // preload timer *\/ */
/*   state = !state; */
/*   digitalWrite(ledPin, state); */
/* } */

ISR(TIMER1_COMPA_vect) { // compare match ISR, occurs when match happens

  newtick = true  ;

  if (ticks < subdivsPerBeat) {
    ticks ++;
    subdiv();
  }
  else {
    ticks = 1;
    // new beat
    newbeat = true;
    beat();

    if (beats < beatsPerBar) beats ++;
    else {
      beats = 1;
      // new bar
      newbar = true;
      bar();

      if (bars < barsPerMeasure) bars ++;
      else {
        // new measure (cycle resets)
        newmeasure = true;
        measure();
        bars = 1;
      }
    }
  }
}


void loop() {

  readsensors();
  for (int i = 0; i < size(sensors); i++) {
    /* Serial.print(proximities[i]); */
    /* Serial.print('\t'); */
  }
  /* Serial.println(); */

}

void readsensors() {
  float singles[smoothing][size(sensors)];
  float deviations[smoothing][size(sensors)];
  float sum, sumDeviation, avg, avgDeviation;

  /* for each sensor */
  for (int y  = 0; y  < size(sensors); y ++) {
    if (debug) {
      Serial.print("s");
      Serial.print(y);
      Serial.print("\t");
    }

    /* gather a bunch of readings, count determined by "smoothing" variable */
    for (int x = 0; x < smoothing; x++) {
      /* put reading into "singles" array -- raw data */
      /* x is reading counter, y is sensor counter  */
      singles[x][y] = analogRead(sensors[y]);

      if (debug) {
        Serial.print(singles[x][y]);
        if(x < smoothing-1) Serial.print("\t");
      }

      if (x == 0) sum = 0; // reset summation

      // median filter
      sum += singles[x][y];

      if (x == smoothing-1) { // once we're at last value

        if (debug) {Serial.print("\n");}

        avg = sum / smoothing; // get average

        /* if (debug) { */
        /*   Serial.print("avg: "); */
        /*   Serial.print(avg); */
        /* } */


        if (debug) {Serial.print("devs:\t");}
        for (int read = 0; read < smoothing; read++) {
          if (read == 0) sumDeviation = 0; // reset sum to begin

          deviations[read][y] = abs(singles[read][y] - avg); // deviation of this sample


          if (debug) {
            Serial.print(deviations[read][y]);
            Serial.print("\t");
          }

          sumDeviation += deviations[read][y];

          if (read == smoothing - 1) {
            avgDeviation = sumDeviation / smoothing; // get avg deviation
            if (debug) {
              Serial.print("\tavgDev: ");
              Serial.print(avgDeviation);
            }

            int goodcount = 0, goodsum = 0, goodavg = 0;

            if (debug) {Serial.print("\ngood:\t");}

            for (int fr = 0; fr < smoothing; fr++) {
              if( deviations[fr][y] < avgDeviation ){
                goodcount++;
                goodsum += singles[fr][y];
                if (debug) {
                  Serial.print(singles[fr][y]);
                  Serial.print("\t");
                }
              }


            }

            goodavg = goodsum / goodcount;
            proximities[y] = goodavg;

            if (debug) {
              Serial.print("\nFINAL: ");
              Serial.println(proximities[y]);
            }
          }

        }
      }


    }




  }
  if (debug) {Serial.println();}

}



void subdiv() {

}

void beat() {

  for (int p = 0; p < size(patterns); p++) {
    patterns[p].display();
    patterns[p].advance();
  }

}

void bar() {


}

void measure() {


}

void setupTimer() {
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
}
