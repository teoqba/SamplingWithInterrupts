// Asynchronous interrupt driven ADC.
// Performs ADC conversion, while the result from previous one 
// is being send over Serial. 
// In such a way the bottleneck for sampling frequency is 
// the speed of data transfer. 
// Code based on http://www.gammon.com.au/forum/?id=11488
//
// ADC conversion is triggerd by Timer2 interrupt
// The Time2 interrupt frequency (the overall sampling frequency)
// cannot be higher than the speed of Serial.print transmission,
// as we will lose data.
//
// Kuba Kaminski, 2014

volatile int sensorData;
volatile boolean adcCompleted = true;
volatile boolean sensorDataReady;
volatile boolean sampleTime = true;

void setup(){
  byte analogPin = 0;  
  Serial.begin(115200);
  //set up ADC to internal 5V reference and selected pin
  ADMUX = bit(REFS0) | (analogPin & 0x07); // equivalent to 10000000   
  
  //set up Timer2
  cli(); // disable interrupts
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
  byte valADCL = ADCL;
  byte valADCH= ADCH;
  sensorData = (valADCH << 8) | valADCL;
  adcCompleted = true; 
}

//Timer2 interrupt routine
ISR (TIMER2_COMP_vect){
  sampleTime = true;
}

void loop(){
   if (adcCompleted && sampleTime) {
     //previous ADC cycle is completed, initialize new mesurment
     ADCSRA |= bit(ADSC) | bit(ADIE);
     adcCompleted = false;
     sensorDataReady = true;
     sampleTime = false;
   }
   
   if (sensorDataReady){
     Serial.println(sensorData);
     sensorDataReady = false;
   }
}

void clockPrescaler(byte prescaler){
  // Set perscaler for the clock, see ATMega data-sheet, p. 162
  // 0 - no clock; 
  // 1 - 1; 2 - 8; 3 - 32; 4 - 64; 5 - 128; 6 - 256; 7 - 1024; 

  if (prescaler >7) prescaler = 7;  
  TCCR2B |= prescaler;
}

