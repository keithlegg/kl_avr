// http://www.avrfreaks.net/index.php?name=PNphpBB2&file=printview&t=81312&start=0

//Cool it works now - It was PIND instead of PORTD. 


ISR(INT0_vect){
   if( PORTD & ( 1 << PIND2 ) ){
      PORTA &= ~( 1 << PA1 );
      TCNT2 = 0x01;
      TIMSK |= ( 1 << TOIE2 );
      Timer2_Overflow_Counter = 0;
   }
   else{
      PORTA &= ~( 1 << PA2 );
      TIMSK &= ~( 1 << TOIE2 );
                Timer2_Overflow_Counter = 0;
   }
}


ISR(TIMER2_OVF_vect){
   PORTA &= ~( 1 << PA0 );
   TCNT2 = 0x01;
   TIFR &= ~( 1 << TOV2 );
   Timer2_Overflow_Counter++;
}


int main(void){
  
   DDRD = 0b01000000;
   PORTD |= ( 1 << PD2 );
   
   TCCR2 = 0x07;    
   MCUCR |= ( 1 << ISC00 );
   MCUCR &= ~( 1 << ISC01 );
   GICR |= ( 1 << INT0 );
   SREG |= ( 1 << SREG_I );
   while(1){
   }
} 