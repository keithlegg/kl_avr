#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 16000000

#define HIGH 1
#define LOW 0

#define LSBFIRST 1

/*
 code to send serial bits to an hc595 shift register
 
 dataPin  - DS    (pin 1)  (blue wire)
 clockPin - SH_CP (pin 2)  (yellow wire)
 latchPin - ST_CP (pin 3)  (green wire) 

*/

uint8_t buffer = 0; //buffer for portb outputs
   
void pinMode(uint8_t pin,uint8_t value){

     //1 OUTPUT - 0 INPUT 
     
     
     //DDRB |= (1 << 0); // Set LED as output
     DDRB = value;
     //DDRB = 0xff;
     
     
     //set PORTB as input
     //DDRB = 0x00;
  
}


  
/***************/
// Sets or clears the bit in position 'position' 
// either high or low (1 or 0) to match 'value'.
// Leaves all other bits in PORTB unchanged.


int set_PORTB_bit(int position, int value)
{
        
        if (value == 0)
        {
                PORTB &= ~(1 << position);      // Set bit # 'position' low
        }
        else
        {
                PORTB |= (1 << position);       // Set bit # 'position' high
        }
        return 1;
}


uint8_t getmaskrr(uint8_t output,uint8_t pin,uint8_t value){

  

  if (value==1){
    if(pin==1){
      output |= (1 << 0); 
    }
    
    if(pin==2){
      output |= (1 << 1); 
    }

    if(pin==3){
      output |= (1 << 2); 
    }
    if(pin==4){
      output |= (1 << 3); 
    }
    if(pin==5){
      output |= (1 << 4); 
    }
    if(pin==6){
      output |= (1 << 5); 
    }
    if(pin==7){
      output |= (1 << 6); 
    }
    if(pin==8){
      output |= (1 << 7); 
    }
  }
  if (value==0){

    if(pin==1){
      output &=~(1 << 0); 
    }
    
    if(pin==2){
      output &=~(1 << 1); 
    }
    
    if(pin==3){
      output &=~(1 << 2); 
    }
    if(pin==4){
      output &=~(1 << 3); 
    }
    if(pin==5){
      output  &=~(1 << 4); 
    }
    if(pin==6){
      output  &=~(1 << 5); 
    }
    if(pin==7){
      output &=~(1 << 6); 
    }
    if(pin==8){
      output &=~(1 << 7); 
    }
  
  } 
  return output;
}

/***************/

//8 pins for now 
void digitalWrite(uint8_t pin,uint8_t value){
   buffer= getmaskrr(buffer,pin,value);
   PORTB = buffer;
   //return buffer;
}

/***************/
//8 pins for now 
int8_t digitalRead(uint8_t pin){
   int8_t outread ;
   
   //buffer= getmaskrr(buffer,pin,value);
   outread = PINB;
   
   return outread;
}


/*
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder) {
	uint8_t value = 0;
	uint8_t i;

	for (i = 0; i < 8; ++i) {
		digitalWrite(clockPin, HIGH);
		if (bitOrder == LSBFIRST)
			value |= digitalRead(dataPin) << i;
		else
			value |= digitalRead(dataPin) << (7 - i);
		digitalWrite(clockPin, LOW);
	}
	return value;
}
*/


/***************/


void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
	uint8_t i;


	for (i = 0; i < 8; i++)  {
		if (bitOrder == LSBFIRST)
		    digitalWrite(dataPin, !!(val & (1 << i)));
		else	
		  digitalWrite(dataPin, !!(val & (1 << (7 - i))));
			
		digitalWrite(clockPin, HIGH);
		digitalWrite(clockPin, LOW);		
		
	}
}

/***************/
//shift out with auto latch pin 
void send_byte(uint8_t byte){
     digitalWrite(3, LOW); //LATCH PIN 
     shiftOut(1,2,1,byte);
     digitalWrite(3, HIGH); //LATCH PIN      
}


/***************/

int main (void)
{
    DDRD = 0x01;//PORT D OUTPIT
    
   pinMode(99,0b11110000); //hacked - turns all pins to value
	
    uint16_t delay = 500;

   for (;;)
   {
     /*

      digitalWrite(1, HIGH);
      _delay_ms(100);
      digitalWrite(1, LOW);
      _delay_ms(100);  
     */
    uint8_t temp = digitalRead(0);
     if (temp==0x01){
       PORTD = 0x01; 
     }else{
       PORTD = 0x00;      
     }
     

   }


} 
