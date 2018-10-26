#include <avr/io.h>
#include <stdlib.h> //for ABS()

#define F_CPU 16000000UL
#define FOSC 16000000UL

#include <util/delay.h>

#include "SPI.h"
#include "SPI.cpp"

#include "DAC_MCP49xx.h"
#include "DAC_MCP49xx.cpp"

#define SS_PIN 10
#define LDAC_PIN 7

/****************************/
//rendering properties

#define OFFSETX 120                 
#define OFFSETY 175              
#define OFFSETZ 30


//SIMPLE CUBE GEOMETRY - XYZ vertecies are stored in 3 seperate "zipped" arrays

const int spx = 50; //size of cube in 3 dimensions 

const int aa[9]={  spx ,-spx ,-spx,spx   ,  spx,-spx,-spx ,spx   };  // x data for shape vertex
const int bb[9]={  spx ,spx  ,-spx,-spx  ,  spx,spx ,-spx ,-spx  };  // y data for shape vertex
const int cc[9]={ -spx ,-spx ,-spx,-spx  ,  spx,spx ,spx  ,spx   };  // z data for shape vertex

//construct the edges of the "polygons" in 2 seperate zipped arrays
const int ff[12]={1,2,3,4,  5,6,7,8, 1,2,3,4}; // start vertex for lines
const int gg[12]={2,3,4,1,  6,7,8,5, 5,6,7,8}; // end vertex for lines

/****************************/


/****************************/
//DELAY USEC - 1us delay -30ms,200us - 250ms sawtooth
void ADC_init(uint8_t adctouse)
{
    ADMUX = adctouse;   
    ADMUX |= (1 << REFS0);   
    ADMUX &= ~(1 << ADLAR);  
    ADCSRA|=  (1 << ADPS0); 
    ADCSRA|= (1 << ADPS1);    
}

/*****************************/
int ADCsingleREAD(uint8_t adctouse)
/* read a voltage on pin */
{
    uint16_t ADCval;
    ADCSRA |= (1 << ADEN);  
    ADCSRA |= (1 << ADSC);
    while(ADCSRA & (1 << ADSC));          
    ADCval = ADCL;
    ADCval = (ADCH << 8) + ADCval; 
    return ADCval;
}

/*****************************/
void mydelay(uint16_t count) {
  while(count--) {
    _delay_us(1);

  }
} 


/*****************************/

/*
const int cube_object(){
    const int vx[]={  10 ,-10 ,-10,10   ,  10,-10,-10 ,10   };  // x data for shape vertex
    const int vy[]={  10 ,10  ,-10,-10  ,  10,10 ,-10 ,-10  };  // y data for shape vertex
    const int vz[]={ -10 ,-10 ,-10,-10  ,  10,10 ,10  ,10   };  // z data for shape vertex
    const int ed1[]={1,2,3,4,  5,6,7,8, 1,2,3,4};            // start vertex for lines
    const int ed2[]={2,3,4,1,  6,7,8,5, 5,6,7,8};            // end vertex for lines
}
*/

/*****************************/


void bresenham(int x0, int x1, int y0, int y1,uint16_t color){

    int dx =  abs(x1-x0);int sx = x0<x1 ? 1 : -1;
    int dy = -abs(y1-y0);int sy = y0<y1 ? 1 : -1;

    int err = dx+dy, e2; 
    for (;;){ 

        /****/
        //draw each pixel on the line , or latch an analog voltage in this case
        //dac.outputA(a); //X voltage
        //dac.outputB(b); //Y voltage 
        //dac.latch(); 

        //Tft.setPixel(x0,y0,color);
        /****/

        e2 = 2*err;
        if (e2 >= dy) { /* e_xy+e_x > 0 */
            if (x0 == x1) break;
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) { /* e_xy+e_y < 0 */
            if (y0 == y1) break;
            err += dx;
            y0 += sy;
        }
    }
}


/*****************************/
void test( DAC_MCP49xx dacchip)
{
   int a,b = 0;
   int stepsize = 10;
  
   while(1)
   {
       for (a=0;a<1024;a=a+stepsize)
       {
         //PORTD ^=0x04;//TOGGLE LED ON EACH NEW CYCLE
         for (b=0;b<1024;b=b+stepsize)
         {
           dacchip.outputA(a);
           dacchip.outputB(b);
           dacchip.latch(); 
           //ADCvalue = ADCsingleREAD(0);
           //mydelay(ADCvalue); 
         }

       }
       for (a=1024;a>0;a=a-stepsize)
       {
         //PORTD ^=0x04;//TOGGLE LED ON EACH NEW CYCLE
         for (b=1024;b>0;b=b-stepsize)
         {
           dacchip.outputA(a);
           dacchip.outputB(b);
           dacchip.latch(); 
           //ADCvalue = ADCsingleREAD(0);
           //mydelay(ADCvalue); 
         }

       }

    }

}

/*****************************/

void circle( DAC_MCP49xx dacchip)
{
   int a,b = 0;
   int stepsize = 10;
  
   while(1)
   {
       for (b=0;b<360;b=b+stepsize)
       {
           dacchip.outputA( (sin(b)*1024)+1024 );
           dacchip.outputB( (cos(b)*1024)+1024 );
           dacchip.latch(); 

       }

    }//while  

}


/*****************************/
void cube(DAC_MCP49xx dacchip, float rotx,float roty,float rotz)     
{
    int newx[8];                // translated screen x coordinates for vertex
    int newy[8];                // translated screen y coordinates for vertex

    int i,ii,loopi;             // temp variable for loops

                                // lots of work variables
    float xt,yt,zt;
    float x,y,z;
    float sinax,cosax,sinay;
    float cosay,sinaz,cosaz;    
    int   sx,sy,ex,ey;                  
    int   vertex;
   
    float xpos=0;               // position for object in 3d space, in x
    float ypos=0;               // y
    float zpos=0;               // and z values
 
    xpos=xpos+0.0;         
    ypos=ypos+0.0;         
    zpos=zpos+0.0;         

    rotx=rotx+.1;           
    roty=roty+.1;           
    rotz=rotz+0;           

    sinax=sin(rotx);  // precalculate the sin and cos values
    cosax=cos(rotx);  // for the rotation as this saves a
 
    sinay=sin(roty);  // little time when running as we
    cosay=cos(roty);  // call sin and cos less often
 
    sinaz=sin(rotz);  // they are slow routines
    cosaz=cos(rotz);  // and we dont want slow!

    for (i=0; i<8; i++)               // translate 3d vertex position to 2d screen position
    {
        x=aa[i];                  
        y=bb[i];                  
        z=cc[i];                 

        yt = y * cosax - z * sinax;    // rotate around the x axis
        zt = y * sinax + z * cosax;    // using the Y and Z for the rotation
        y = yt;
        z = zt;

        xt = x * cosay - z * sinay;    // rotate around the Y axis
        zt = x * sinay + z * cosay;    // using X and Z
        x = xt;
        z = zt;

        xt = x * cosaz - y * sinaz;    // finaly rotate around the Z axis
        yt = x * sinaz + y * cosaz;    // using X and Y
        x = xt;
        y = yt;

        x=x+xpos;            // add the object position offset
        y=y+ypos;            // for both x and y
        z=z+OFFSETZ-zpos;        // as well as Z

        newx[i]=(x*64/z)+OFFSETX;    // translate 3d to 2d coordinates for screen
        newy[i]=(y*64/z)+OFFSETY;    // drawing so we can see the cube

    }

    
    for (ii=0; ii<12; ii++)        // draw the lines that make up the object
    {
        vertex=ff[ii]-1;         // -1 temp = start vertex for this line
        sx=newx[vertex];        // set line start x to vertex[i] x position
        sy=newy[vertex];        // set line start y to vertex[i] y position
       
        vertex=gg[ii]-1;         // -1 temp = end vertex for this line
        ex=newx[vertex];    // set line end x to vertex[i+1] x position
        ey=newy[vertex];    // set line end y to vertex[i+1] y position

        //Tft.drawLine(sx, sy, ex, ey ,WHITE);// draw the line between these 2 vertex
        int sz = 3;
        
        //DRAW PIXEL 
        //Tft.setPixel(sx  ,sy  ,WHITE);
        //Tft.setPixel(sx-1,sy-1,WHITE); 
        
        dacchip.outputA( sx ); //X voltage
        dacchip.outputB( sy ); //Y voltage 
        dacchip.latch(); 

        dacchip.outputA( ex ); //X voltage
        dacchip.outputB( ey ); //Y voltage 
        dacchip.latch();

    }
    

 }//cube function

/*****************************/




int main (void)
{

   DDRD |= (1<<LDAC_PIN)| (0x04); //(LDAC - LED-PD2)

   //the DAC controller struct  
   DAC_MCP49xx dac(DAC_MCP49xx::MCP4912, SS_PIN, LDAC_PIN);

   //set up some options via the DAC_MCP49xx library 
   dac.setPortWrite(true);
   dac.setAutomaticallyLatchDual(true);

   //uint16_t ADCvalue =0;
   //ADC_init(0);
   
   while(1)
   {
       circle(dac);

       /*
        for (int c =0;c<360;c++)
        {
            cube(dac, 0,c,0);
            _delay_ms(10); 
        }
       */
   };
  

}




