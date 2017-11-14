//for P-Channel MOSFETs

// define 'functions' for clearer code, by column
#define setRed0(val) (OCR2B = val)
#define setRed1(val) (OCR1A = val)
#define setGreen0(val) (OCR0B = val)
#define setGreen1(val) (OCR1B = val)
#define setBlue0(val) (OCR0A = val)
#define setBlue1(val) (OCR2A = val)

// same with rows
#define Leds3off PORTD |= _BV(7)
#define Leds1on PORTD &= ~_BV(2)
#define Leds1off PORTD |= _BV(2)
#define Leds2on PORTD &= ~_BV(4)
#define Leds2off PORTD |= _BV(4)
#define Leds3on PORTD &= ~_BV(7)

//multiplex counter
volatile uint8_t count;

// array to be displayed (aka 'video memory')
volatile uint8_t colors[6][3] = {
  {255, 255, 255},
  {255, 255, 255},
  {255, 255, 255},
  {255, 255, 255},
  {255, 255, 255},
  {255, 255, 255}};

// sine wave lookup table for the justLights() function
const uint8_t lights[256]={                                                             
  0,   0,   0,   0,   0,   0,   1,   1,      //8                                   
  2,   3,   3,   4,   5,   6,   7,   8,      //16                                   
  9,  10,  12,  13,  15,  16,  18,  19,                                         
 21,  23,  25,  27,  28,  31,  33,  35,      //32                                   
 37,  39,  41,  44,  46,  49,  51,  54,                                         
 56,  59,  62,  64,  67,  70,  73,  75,                                         
 78,  81,  84,  87,  90,  93,  96,  99,                                         
102, 105, 108, 112, 115, 118, 121, 124,      //64                                   
127, 130, 133, 137, 140, 143, 146, 149,                                         
152, 155, 158, 161, 164, 167, 170, 173,                                         
176, 179, 182, 185, 187, 190, 193, 196,                                         
198, 201, 203, 206, 208, 211, 213, 215,                                         
217, 220, 222, 224, 226, 228, 230, 232,                                         
233, 235, 237, 238, 240, 241, 243, 244,                                         
245, 246, 247, 248, 249, 250, 251, 252,                                         
252, 253, 254, 254, 254, 255, 255, 255,                                         
255, 255, 255, 255, 254, 254, 254, 253,                                         
252, 252, 251, 250, 249, 248, 247, 246,                                         
245, 244, 243, 241, 240, 238, 237, 235,                                         
233, 232, 230, 228, 226, 224, 222, 220,                                         
217, 215, 213, 211, 208, 206, 203, 201,                                         
198, 196, 193, 190, 187, 185, 182, 179,                                         
176, 173, 170, 167, 164, 161, 158, 155,                                         
152, 149, 146, 143, 140, 137, 133, 130,                                         
127, 124, 121, 118, 115, 112, 108, 105,                                         
102,  99,  96,  93,  90,  87,  84,  81,                                         
 78,  75,  73,  70,  67,  64,  62,  59,                                         
 56,  54,  51,  49,  46,  44,  41,  39,                                         
 37,  35,  33,  31,  28,  27,  25,  23,                                         
 21,  19,  18,  16,  15,  13,  12,  10,                                         
  9,   8,   7,   6,   5,   4,   3,   3,                                         
  2,   1,   1,   0,   0,   0,   0,   0                                          
};          

//variable arrays for the justLights() function
boolean LedOn[18];
uint8_t fast[18];
uint16_t falseColor[18];
uint16_t lightscount[18];

// timer 2 overflow interrupt function
ISR (TIMER2_OVF_vect)
{
// first, switch off the mosfets, as they will display the value buffered on previous interrupt
  PORTD |= 0b10010100; //set pins 2, 4, 7 HIGH

// turn on the right row
  switch (count) {
    case 0: Leds1on; break;
    case 1: Leds2on; break;
    case 2: Leds3on; break;
    default: break;
  }

// increment the counter
  count++;
  count%=3;

// write new values to the buffers
setRed0 (colors[count][0]);
setGreen0 (colors[count][1]);
setBlue0 (colors[count][2]);
setRed1 (colors[count+3][0]);
setGreen1 (colors[count+3][1]);
setBlue1 (colors[count+3][2]);
}

void setup() 
{

/* pins to OC registers:
 * 3  OCR2B
 * 5  OCR0B
 * 6  OCR0A
 * 9  OCR1A
 * 10 OCR1B
 * 11 OCR2A
 */

cli();

DDRD  |= 0b11111100; //set pins 2-7 to OUTPUT
PORTD |= 0b10010100; //set pins 2, 4, 7 HIGH
DDRB  |= 0b00001110; //set pins 9-11 to OUTPUT

//stop timers
GTCCR |= _BV(TSM) | _BV(PSRASY) | _BV(PSRSYNC);

//setup fast PWM
//timer0 A non-inverted | B non-inverted | fast PWM (WGM0 = 11)
TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);

//timer1 A non-inverted | B non-inderted | fast PWM 8 bit (WGM1 = 01 01)
TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM10);
TCCR1B = _BV(WGM12); // fast PWM cntd

//timer2 A non-inverted | B non-inderted | fast PWM (WGM2 = 11)
TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);

//set prescalers to 64
//TCCR0B = _BV(CS01) | _BV(CS00); // prescaler = 64 (CS0 = 011)
//TCCR1B |= _BV(CS11) | _BV(CS10); // prescaler = 64 (CS1 = 011) (keeping WGM bit intact)
//TCCR2B = _BV(CS22); // prescaler = 64 (CS2 = 100) 

//set prescalers to 256
TCCR0B = _BV(CS02) ; // prescaler = 256 (CS0 = 100)
TCCR1B |= _BV(CS12); // prescaler = 256 (CS1 = 100) (keeping WGM bit intact)
TCCR2B = _BV(CS22) | _BV(CS21) ; // prescaler = 256 (CS2 = 110)

//set all analog outputs to HIGH
OCR0A = 255;
OCR0B = 255;
OCR1A = 255;
OCR1B = 255;
OCR2A = 255;
OCR2B = 255;

//reset all counters
TCNT0 = 0;
TCNT1H = 0; // high byte
TCNT1L = 0; // low byte
TCNT2 = 0;

TIMSK2 |= _BV(TOIE2); // enable overflow interrupt on timer 2

sei();

GTCCR = 0; //start timers
}

void loop() 
{
//colors[0][0] = 0;
justLights(120, 15000);
//testFor(1234);
}

// alternative Delay (approx 62.5 ns per tick)
void justWait (uint32_t period)
{
  for (uint32_t z = 0; z<period; z++) __asm__("nop\n\t");   
}

//my old stuff, for demonstration purposes
void justLights(uint16_t rfactor, uint32_t dperiod)
{
for(uint8_t kcount=0; kcount<18; kcount++){

      if ((!LedOn[kcount])&& (random(rfactor)==1)){        
        LedOn[kcount]=true;
        fast[kcount]=random(8)+1;
        lightscount[kcount]=0;}
      if (LedOn[kcount]){
        falseColor[kcount]=lights[lightscount[kcount]];
        lightscount[kcount]=lightscount[kcount]+fast[kcount];
        if (lightscount[kcount]>255) {LedOn[kcount]=false;falseColor[kcount]=0;};
      }

   colors[kcount/3][kcount%3]=~falseColor[kcount];
  }
  justWait(dperiod);
}

//a simple test function turning on LEDs one by one
//with fades
void testFor(uint32_t dperiod)
{
 for (uint8_t y = 0; y<18; y++){
  for (uint16_t k = 255; k>0; k--) 
   {colors[y/3][y%3] = k;
    justWait(dperiod);
    }
  for (uint16_t k = 0; k<256; k++) 
   {colors[y/3][y%3] = k;
   justWait (dperiod);
   }
 }
}

