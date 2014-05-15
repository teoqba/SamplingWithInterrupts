// Asynchronous interrupt driven ADC.
// Performs ADC conversion, while the result from previous one 
// is being send over Serial. 
// In such a way the bottleneck for sampling frequency is 
// the speed of data transfer. 

// ADC conversion is triggerd by Timer1 interrupt
// The Time1 interrupt frequency (the overall sampling frequency)
// cannot be higher than the speed of Serial.print transmission,
// as we will loose data.
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

unsigned long counter = 0;
unsigned long nreps = 50000;
unsigned long t1,t2;

void setup(){
  byte analogPin = 0;
  Serial.begin(115200*2);
  //set up ADC to internal 5V reference and selected pin
  ADMUX = bit(REFS0) | (analogPin & 0x07); // equivalent to 10000000   
  
  cli(); // disable interrupts
  
/*  //Set Timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 1000;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= bit(WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= bit(CS12) | bit(CS10);  
  // enable timer compare interrupt
  TIMSK1 |= bit(OCIE1A); */
  
  // Set Timer 2
  TCCR2A = 0; //clear the registers
  TCCR2B = 0;
  TCNT2  = 0; //initialize counter to 0
  clockPrescaler(3); //set the prescaler
  OCR2A = 249; //set the comparator //x = (fclock/(fsampling*prescaler)) - 1 
  TCCR2A |= bit(WGM21); //enable CTC mode
  TIMSK2 |= bit(OCIE2A); //enable compare interrupt on clock 2

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
ISR (TIMER2_COMPA_vect){
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
   if (transmitData && counter < nreps+1){
     for (int i=0;i<3;i++){
        while ( !(UCSR0A &(1<<UDRE0)));
        UDR0 = bufHi[prevBufferIndex][i];
        while ( !(UCSR0A &(1<<UDRE0)));
        UDR0 = bufLo[prevBufferIndex][i];   
     }
     transmitData = false;
     counter ++;
     if (counter == nreps) t2 = micros();
   }

  if (counter == nreps+1){
    Serial.print("Time: ");
    unsigned long delta = t2-t1;
    float freq = 1.0/((delta/((float)nreps))*pow(10,-6));
    Serial.print(delta);
    Serial.print("   Frequency:");
    Serial.println(freq,4);
    counter = nreps+2;
  }   
}
    
void clockPrescaler(byte prescaler){
  // Set perscaler for the clock, see ATMega data-sheet, p. 162
  // 0 - no clock; 
  // 1 - 1; 2 - 8; 3 - 32; 4 - 64; 5 - 128; 6 - 256; 7 - 1024; 

  if (prescaler >7) prescaler = 7;  
  TCCR2B |= prescaler;
}

