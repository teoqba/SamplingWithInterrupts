// Asynchronous interrupt driven ADC.
// Performs ADC conversion, while the result from previous one 
// is being send over Serial. 
// In such a way the bottleneck for sampling frequency is 
// the speed of data transfer. 
// Code based on http://www.gammon.com.au/forum/?id=11488
//
// ADC conversion is triggerd by Timer1 interrupt
// The Time1 interrupt frequency (the overall sampling frequency)
// cannot be higher than the speed of Serial.print transmission,
// as we will lose data.
//
// Performs 3 ADC conversions in sequence. Each conversion is triggered by
// interrupt.
// Using two buffers to store ADC Conversion data. They are swapped on each 
// mesurment cycle. During n cycle, data n-1 cycle is being send from one of 
// the buffers. On the first cycle no data is send. 
//
// Kuba Kaminski, 2014

volatile boolean sensorDataReady;
volatile boolean transmitData;

volatile byte bufLo1[3];
volatile byte bufLo2[3];
volatile byte bufHi1[3];
volatile byte bufHi2[3];
volatile byte *bufLo[2] = {bufLo1,bufLo2};
volatile byte *bufHi[2] = {bufHi1,bufHi2};
volatile byte bufferIndex = 0;
volatile byte prevBufferIndex = 1;
volatile byte adcIndex = 0;

int counter = 0;
unsigned long t1,t2;

void setup(){
  byte analogPin = 0;  
  Serial.begin(115200*2);
  //set up ADC to internal 5V reference and selected pin
  ADMUX = bit(REFS0) | (analogPin & 0x07); // equivalent to 10000000   
  
  cli(); // disable interrupts
  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= bit(WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= bit(CS12) | bit(CS10);  
  // enable timer compare interrupt
  TIMSK1 |= bit(OCIE1A);

  sei(); //allow interrupts
}

//ADC Interrupt routine
ISR (ADC_vect){
  //remember to read ADCL register first! 
  bufLo[bufferIndex][adcIndex] = ADCL;
  bufHi[bufferIndex][adcIndex] = ADCH;
  //sensorData = (valADCH << 8) | valADCL;
  adcIndex ++;
  // First ADC Converstion is triggered by Timer ISR
  // Next two are triggered in ADC ISR
  if (adcIndex < 3) ADCSRA |= bit(ADSC) | bit(ADIE);
  // Do only 3 conversions, after that, Transfer data.
  if (adcIndex == 3) sensorDataReady=true;  
}

//Timer1 interrupt routine
ISR (TIMER1_COMPA_vect){
  if (sensorDataReady) transmitData = true;
  //start first ADC conversion
  adcIndex = 0;
  //change the buffers
  if (bufferIndex==0){
    bufferIndex = 1;
    prevBufferIndex = 0;}
  else{
    bufferIndex = 0;
    prevBufferIndex = 1;
  }
    
  ADCSRA |= bit(ADSC) | bit(ADIE);
}

void loop(){
   if (!counter)  t1 = micros();
   if (transmitData && counter <101){
     for (int i=0;i<3;i++){
         Serial.println((bufHi[prevBufferIndex][i]<<8 )| bufLo[prevBufferIndex][i]);
     }
     transmitData = false;
     counter ++;
     if (counter == 100) t2 = micros();
   }

  if (counter == 101){
    Serial.print("Time: ");
    unsigned long delta = t2-t1;
    float freq = 1.0/((delta/100.0)*pow(10,-6));
    Serial.print(delta);
    Serial.print("   Frequency:");
    Serial.println(freq,4);
    counter = 102;
  }   
}
    
void clockPrescaler(byte prescaler){
  // Set perscaler for the clock, see ATMega data-sheet, p. 162
  // 0 - no clock; 
  // 1 - 1; 2 - 8; 3 - 32; 4 - 64; 5 - 128; 6 - 256; 7 - 1024; 

  if (prescaler >7) prescaler = 7;  
  TCCR2B |= prescaler;
}

