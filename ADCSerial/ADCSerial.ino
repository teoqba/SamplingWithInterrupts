// Interrupt driven ADC.
// Performs ADC conversion, while the result from previous one 
// is being send over Serial. 
// In such a way the bottleneck for sampling frequency is 
// the speed of data transfer. 
// Code based on http://www.gammon.com.au/forum/?id=11488
// Kuba Kaminski, 2014

volatile int sensorData;
volatile boolean adcCompleted = true;
volatile boolean sensorDataReady;

void setup(){
  byte analogPin = 0;  
  Serial.begin(115200);
  //set up ADC to internal 5V reference and selected pin
  ADMUX = bit(REFS0) | (analogPin & 0x07); // equivalent to 10000000   
}

//Interrupt routine
ISR (ADC_vect){
    //read ADCL first! 
  byte valADCL = ADCL;
  byte valADCH= ADCH;
  sensorData = (valADCH << 8) | valADCL;
  adcCompleted = true; 
}

void loop(){
   if (adcCompleted){
     //previous ADC cycle is completed, initialize new mesurment
     ADCSRA |= bit(ADSC) | bit(ADIE);
     adcCompleted = false;
     sensorDataReady = true;
   }
   if (sensorDataReady){
     Serial.println(sensorData);
     sensorDataReady = false;
   }
}

