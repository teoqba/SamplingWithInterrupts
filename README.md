Arduino sampling by timing ADC with interrupts

- ADCSerial - run ADC conversion, while results from previous conversion is
  being send over Serial

- ADCTimer - run ADC conversion asynchronously, timing with Timer2 interrupt

- calcComparatorValue.py - simple script to calculate value of comparator 
  register for given sampling frequency and clock prescaler 
