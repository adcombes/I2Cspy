// I2Cspy project is released under GNU GPL3.0 Licence
// refer to GNU documentation
//
// connect UNO port D8 (eg ATMEGA328P port PB0) to SDA under test
// connect UNO port D9 (eg ATMEGA328P port PB1) to SCL under test
// don't connect UNO port D7. it is used for debgging purpose
// connect UNO port D6 (eg ATMEGA328P port PD6) to GND. refer to documentation for trigger usage.

#include <Arduino.h>

// uncomment next line if you need PULLUP on SCL and SDA lines
//#define PULLUP_NEEDED

// uncomment next line if you use D6 trigger
//#define USE_D6_TRIGGER

// SCL and SDA states
typedef enum {E00, E01, E10, E11} SclSda_t;

// helpers
#define READ_LINES (PINB & B00000011)     // read SCL and SDA lines
#define DEBUG_1 PORTD |= B10000000;       // set debug line high
#define DEBUG_0 PORTD &= B01111111;       // set debug line low
#define READ_TRIGGER (PIND & B01000000)   // read trigger line state

// protocol decoder states
typedef enum {Pzz, Pbita0, Pbita1, Pbitb0, Pbitb1 } Pstate_t;
const String stateStr[] = {"Pzz", "Pbita0", "Pbita1", "Pbitb0", "Pbitb1"};

// as much memory as we can
#define LG_DATA 1200
unsigned char dataPool[LG_DATA];

// functions prototypes
long acquireData(void);
void dump(void);
char getData(int index);

//entry point of code
void setup() {
#ifdef PULLUP_NEEDED
  pinMode(8,INPUT_PULLUP);
  pinMode(9,INPUT_PULLUP);
#else
  pinMode(8,INPUT);
  pinMode(9,INPUT);
#endif
  pinMode(7,OUTPUT);
#ifdef USE_D6_TRIGGER
  pinMode(6,INPUT_PULLUP);
#endif
  Serial.begin(115200);
}

void loop() {

  noInterrupts();
  acquireData();
  interrupts();
  delay(100);
//  rawdump();
  dump();
  while (1) {
    delay(100);
  }

}

// synchronize then save SCL SDA transitions in dataPool
long acquireData() {

volatile unsigned char state;
volatile unsigned char oldState;
unsigned char temp;
unsigned char * dataPtr;
unsigned char * dataLimit = dataPool + LG_DATA;
int count;

// optionaly synchronize on trigger
#ifdef USE_D6_TRIGGER
  do {
    state = READ_TRIGGER
  } while (state > 0);
#endif
// synchronize on start
  while (1) {
    // synchronize on protocole idle for at least 10 µs
    do {
      state = READ_LINES;
    } while(state != E11);
    count = 20; // 20 * 700ns = 15µs
    do {
      state = READ_LINES;
      count --;
    } while ((state == E11) && (count));
    if (count > 0) {
      continue;
    }
    // look for a start condition on I2C
    do {
      state = READ_LINES;
    } while(state == E11);
    if (state == E10) {
      break;
    }
  }
// start condition met. begin to acquire data
  dataPtr = dataPool;
  oldState = B00000010;
  *dataPtr = B11111110;
  dataPtr++;
  while (dataPtr < dataLimit ) {
    do {
      state = READ_LINES;
    } while (state == oldState);
    temp = state;
    oldState = state;
    do {
      state = READ_LINES;
    } while (state == oldState);
    temp <<= 2;
    temp |= state;
    oldState = state;
    do {
      state = READ_LINES;
    } while (state == oldState);
    temp <<= 2;
    temp |= state;
    oldState = state;
    do {
      state = READ_LINES;
    } while (state == oldState);
    temp <<= 2;
    temp |= state;
    oldState = state;
    *dataPtr = temp;
    dataPtr ++;
  }
}

// retrieve data from dataPool
char getData(int index) {
  int raw;
  int column;
  
  raw = index >> 2;
  column = (3 - (index & 0x3)) * 2;
  if (raw >=  LG_DATA) {
    return -1;
  }
  return (dataPool[raw] >> column) & B00000011;
}

// print state/transition on error
void printError(Pstate_t state,char transition) {
  Serial.print("error state=");
  Serial.print(stateStr[state]);
  Serial.print(" transition=");
  Serial.println(transition,HEX);
}

// print address or data according to I2C protocol
void printAddressOrData(int addFlag,unsigned char current,unsigned char ack) {
  if (addFlag) {
    Serial.print("0x");
    Serial.print((unsigned int)current >> 1,HEX);
    if (current & 1) {
      Serial.print(" read");
    } else {
      Serial.print(" write");
    }
  } else {
    Serial.print("0x");
    Serial.print((unsigned int)current,HEX);
  }
  if (ack) {
    Serial.println(" nack");
  } else {
    Serial.println(" ack");
  }
}

// decoding I2C protocol and print result on console
void dump(void) {
  Pstate_t state = Pzz;
  char cc;
  int index = 0;
  int numBit;
  int addFlag = 0;
  unsigned char current;
  unsigned char ack;

  while (1) {
    cc = getData(index);
    index ++;
    if (cc < 0) {
      return;
    }
    switch (state) {
      case Pzz:
        switch(cc) {
          case E10: state = Pbitb0; Serial.println("start"); addFlag = 1; numBit = 8; break;
          case E11: state = Pzz; break;
          default: printError(state,cc); return;
        }; break;
      case Pbita0:
        switch (cc) {
          case E01: state = Pbita1;break;
          case E10: if (numBit > 0) {
              current <<= 1; numBit--;
            } else {
              ack = 0; printAddressOrData(addFlag,current,ack); addFlag = 0; numBit = 8;
            }; state = Pbitb0; break;
          default: printError(state,cc); return;
        }; break;
      case Pbita1:
        switch (cc) {
          case E00: state = Pbita0; break;
          case E11: if (numBit > 0) {
              current <<= 1; current |= 1; numBit--;
            } else {
              ack = 1; printAddressOrData(addFlag,current,ack); addFlag = 0; numBit = 8;
            };state = Pbitb1;  break;
          default: printError(state,cc); return;
        }; break;
      case Pbitb0:
        switch (cc) {
          case E00: state = Pbita0; break;
          case E11: state = Pzz; Serial.println("stop"); break;
          default: printError(state,cc); return;
        }; break;
      case Pbitb1:
        switch (cc) {
          case E00: state = Pbita0; break;
          case E01: state = Pbita1; break;
          case E10: state = Pbitb0; Serial.println("start"); addFlag = 1; numBit = 8; break;
          default: printError(state,cc); return;
        }; break;
      default: printError(state,cc); return;
    }
  }
  
}
