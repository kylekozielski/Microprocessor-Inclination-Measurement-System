//*                          Final Project                         
//*                  Kyle Kozielski kozielsk 400128562             
//*                                
//*********************************************************************
//*********************************************************************
//*                   Student Assigned Requirements                             
//*                                                                   
//*                   Bus Speed = 8MHz                                                
//*                   Resolution = 10-bit                                          
//*                   Port = AN6, second port = AN7                             
//*                                                                   
//*                                                                   
//*********************************************************************
// Clock Speed is 6.25 MHz as written                                //
//*********************************************************************
//*                            References                   
//***********************************************************************

/* ---------Header files needed throughout the program------------- */
#include <hidef.h>      /* common defines and macros */
#include "derivative.h"  /* derivative information */
#include "SCI.h"

/*-----------------Global Variables---------------*/
char string[20];
unsigned short val;
int switch0, switch1;
unsigned short y_input;
unsigned short min_y, max_y;
unsigned short y_Angle;
unsigned short x_Angle;
unsigned short min_Angle, max_Angle, x_input, z_input, min_x, max_x;

  
/*---------------Function prototypes used in the main loop-------------*/
void Initialize_Clock(void);
void Initialize_ADC(void);
void Initialize_Ports(void);
void Initialize_Interrupts(void);
void Initialize_BaudRate(void);
void TurnOn_LEDs(unsigned short angle);
void OutCRLF(void);
void msDelay(unsigned int time);


void main(void) {

  Initialize_Clock();
  Initialize_ADC();
  Initialize_Ports();
  Initialize_Interrupts();
  Initialize_BaudRate(); //Sets baud rate to 38400
  
  
  //First Line Outputted in RealTerm
  SCI_OutString("Kyle Kozielski 400128562 kozielsk\n\r"); OutCRLF();
  
 
 //The next one assignment statement configures the ESDX to catch Interrupt Requests                                                              
  
	EnableInterrupts; //CodeWarrior's method of enabling interrupts
	
	/*------------------Set the values of global variables to avoid automatic assignment problems------------*/
	switch0 = 0;
	switch1 = 0;
	
	//Following variables are for linear interrpolation. min corresponds to 0 degrees, max corresponds to 90.
	min_y = 504;
	max_y = 606;
	min_Angle = 0;
	max_Angle = 90;
	
	/*-------------------Infinite for loop, write all of the code in it-----------------*/
	
	for(;;) {
	  
	  if(switch0 == 1){
      //Enable serial communication
      PTJ = 0x01;
      
      if(switch1 == 0){ //mode 0
        x_input = ATDDR0;
        z_input = ATDDR2;
        
        /*Four conditionals for full 360 degree detection in the x-axis.
        z axis is always 90 degrees ahead of x axis:
        Case 1: x b/w 0-90, z b/w 90-180
        Case 2: x b/w 90-180, z b/w 180-270
        Case 3: x b/w 180-270, z b/w 270-360
        Case 4: x b/w 270-360, z b/w 0-90.
        
        In each case the combination of degrees correspond to a unique pair of inequality
        in input digital values for both x and z, which was found experimentally.
        
        I will ignore equal to bounds for simplicity. 
        */
      
        //Linear Interpolation: y = y0 + (x-x0)(y1-y0)/(x1-x0). Y is angle, x is digital value of the ADC
        //y0 is min, y1 is max, same for x. For each conditional, y0 assigned min angle for that quadrant
        //x0 and x1 are experimental range for that quadrant and y1 is max angle for that region.
        
        //Case 1
        if(x_input>=511 && x_input<=618 && z_input>=522 && z_input<=620){
          x_Angle = min_Angle + (abs(x_input-511)*(max_Angle-min_Angle)/(618-511));
        } 
        
        //Case 2
        else if(x_input>=520 && x_input<=618 && z_input>=418 && z_input<=522){
          x_Angle = 90 + (abs(x_input-618)*(180-90)/abs(520-618));
        }
        
        //Case 3
        else if(x_input>=411 && x_input<=520 && z_input>=418 && z_input<=518){
          x_Angle = 180 + (abs(x_input-520)*(270-180)/abs(411-520));
        }
        
        //Case 4
        else if(x_input>=411 && x_input<=511 && z_input>=518 && z_input<=620){
          x_Angle = 270 + (abs(x_input-411)*(360-270)/(511-411));
        }
        
        SCI_OutUDec(x_Angle);
        
        msDelay(100);
        SCI_OutChar(CR);
        
        TurnOn_LEDs(x_Angle);
        
      } 
      else{ //mode 1 by default
        y_input = ATDDR1;
        
        //Linear Interpolation: y = y0 + (x-x0)(y1-y0)/(x1-x0)
        //SCI_OutString(" and the y_Angle reading is: ");
        y_Angle = min_Angle + (abs(y_input-min_y)*(max_Angle-min_Angle)/abs(max_y-min_y));

        SCI_OutUDec(y_Angle);
        
        msDelay(100);
        SCI_OutChar(CR);
        
        TurnOn_LEDs(y_Angle); 
      }
	  } 
	  
	  else{
	    //Disable Serial communciation
	    PTJ = 0x00;
	    PT1AD = 0x00;
	    PT0AD = 0x00;
	  }
  } 
}


/*-------------------- INTERRUPT SERIVE ROUTINES (ISRs) GO BELOW------------------------------*/
  
/*
 * This is the Interrupt Service Routine for TIC channel 0. It enables or disables the Serial Communciation                                                    
 */           
interrupt  VectorNumber_Vtimch0 void SerialCommunication(void)
{
  /* DECLARE ALL YOUR LOCAL VARIABLES BELOW*/
  /////////////////////////////////////////////////////
  unsigned int temp; //DON'T EDIT THIS
  
  //switch0 = 0 disables serial communication. switch0 = 1 enables serial communication.
  switch0++;
  
  if(switch0 >1){
    switch0 = 0;
  }
  
  /* RESETS INTERRUPT (Don't Edit)*/
  ////////////////////////////////////////////////////  
  temp = TC0;       //Refer back to TFFCA, we enabled FastFlagClear, thus by reading the Timer Capture input we automatically clear the flag, allowing another TIC interrupt
}

 /*
 * This is the Interrupt Service Routine for TIC channel 1. It sets the Mode of Operation.
 */           
interrupt  VectorNumber_Vtimch1 void Mode(void)
{
  /* DECLARE ALL YOUR LOCAL VARIABLES BELOW*/
  /////////////////////////////////////////////////////
  unsigned int temp; //DON'T EDIT THIS
  
  //switch1 = 0 means Mode 0, i.e. read x-axis. switch1 = 1 Means Mode 1 i.e. read y-axis.
  switch1++;
  
  if(switch1 > 1){
    switch1 = 0;
  }
 
  
  /* RESETS INTERRUPT (Don't Edit)*/
  ////////////////////////////////////////////////////  
  temp = TC1;       //Refer back to TFFCA, we enabled FastFlagClear, thus by reading the Timer Capture input we automatically clear the flag, allowing another TIC interrupt
}




/*----------------------Below are all of the function definitions----------------------*/

/*Initialize Clock Speed to 8MHz adapted from Week 8 Code*/

void Initialize_Clock(void){
  //VCOFRQ[1:0] = 0b00: 32MHz <= VCOCLK <= 48MHz
  //SYNDIV[5:0] = 0b000001: SYNDIV = 1
  //These are used in CPMUSYNR register
     
  //REFFRQ[1:0] = 0b10:  6MHz < fREF <= 12MHz
  //REFDIV[3:0] = 0b0000: REFDIV = 0
  //These are used in CPMUREFDIV register


  CPMUPROT = 0x26;    //Allows needed clock registers to be over written
  
  CPMUCLKS = 0x80;    //Set PLLSEL = 1
  
  CPMUOSC = 0x80;     //Set OSCE = 1, use oscillator clock of 8MHz since REFDIV = 0
  
  CPMUREFDIV = 0x80;  //Set REFFRQ = 0b10 and REFDIV = 0b0000
  
  CPMUSYNR = 0x01;    //Set SYNDIV = 1, VCOFRQ = 0. VCOCLK = 32MHz
  
  CPMUPOSTDIV = 0x01; //POSTDIV = 1. PLLCLK = 16MHz -> fBus = 8MHz
  
  while (CPMUFLG_LOCK == 0) {}  //Wait for PLL to achieve desired tolerance of target frequency. 
  //NOTE: For use when the source clock is PLL.
 
 CPMUPROT = 1;        //Protection for clock configuration is reenabled    
}

/*Initialize ADC which was adapted from Lab 5 Code*/

void Initialize_ADC(void){
  ATDCTL0 = 0x07;   //Set AN7 as the wrap around bit
  
  ATDCTL1 = 0x25;		// Set 10 bit resolution and change external trigger to AN5
  
	ATDCTL3 = 0x98;		// Right justified, three samples per sequence. [S8C:S1C] = 0011
	
	ATDCTL4 = 0x00;		// prescaler = 0; ATD clock = 8MHz / (2 * (0 + 1)) == 4MHz
	                  //[7:5]: Sample time is 4 ATD clk cycles = 1/1MHz = 1us 
	                  
	ATDCTL5 = 0x36;		// Continuous conversion on channel 6, enable multi-channel
	                  //sample mode. Choose 3 inputs to be channel 6 and 7 and 0. 
}

/*Set Data Direction ports for pins to be used*/

void Initialize_Ports(void){
  DDR1AD = 0x3E; // = 0b 0011 1110
  DDR0AD = 0xFF; // = 0b 1111 1111
  DDRS = 0xFF;   // = 0b 1111 1111
  DDRJ = 0x01;   // = 0b 0000 0001
  PTJ = 0x00; //Turns off on board LED to avoid confusion  
}

/*Initialize Interrupts which was adapted from Lab 4 Code*/

void Initialize_Interrupts(void){
 /*
 * The next six assignment statements configure the Timer Input Capture                                                   
 */           
  TSCR1 = 0x90;    //Timer System Control Register 1
                    // TSCR1[7] = TEN:  Timer Enable (0-disable, 1-enable)
                    // TSCR1[6] = TSWAI:  Timer runs during WAI (0-enable, 1-disable)
                    // TSCR1[5] = TSFRZ:  Timer runs during WAI (0-enable, 1-disable)
                    // TSCR1[4] = TFFCA:  Timer Fast Flag Clear All (0-normal 1-read/write clears interrupt flags)
                    // TSCR1[3] = PRT:  Precision Timer (0-legacy, 1-precision)
                    // TSCR1[2:0] not used

  TSCR2 = 0x04;    //Timer System Control Register 2
                    // TSCR2[7] = TOI: Timer Overflow Interrupt Enable (0-inhibited, 1-hardware irq when TOF=1)
                    // TSCR2[6:3] not used
                    // TSCR2[2:0] = Timer Prescaler Select: See Table22-12 of MC9S12G Family Reference Manual r1.25 (set for bus/1)
                    //busclock/16
                    
  TIOS = 0xFC;     //Timer Input Capture or Output capture
                    //set TIC[0] and TIC[1] as input (similar to DDR)
  PERT = 0x03;     //Enable Pull-Up resistor on TIC[0] and TIC[1]

  TCTL3 = 0x00;    //TCTL3 & TCTL4 configure which edge(s) to capture
  TCTL4 = 0x0A;    //Configured for falling edge on TIC[0] (channel 0) and TIC[1] (channel 1)

/*
 * The next one assignment statement configures the Timer Interrupt Enable                                                   
 */           
   
  TIE = 0x03;      //Timer Interrupt Enable. Bits correspond bit to bit with the bits in the tflg1 register
}

/*Initialize baud rate which was adapted from SCI.C using 8MHz Bus Speed*/

void Initialize_BaudRate(void){
	
  //MCLK = 8MHZ. Baud divisor = SBR[12:0] = MCLK / (16*baud rate) = 500,000/baud rate
  //baud rate = 500,000/Baud divisor.
  //13.02
  //Choose baud divisor = 13, baud rate = 500,000/13 = 38461.5 ~ 38400
  //Error = (38461.5 - 38400) / 38400 = 0.16025%
  
  SCI0BDH = 0;    //SCIOBDH[5:0] = SBR[12:8]
  SCI0BDL = 13;   //SCIOBDL[7:0] = SBR[7:0]
  SCI0CR1 = 0;    //See SCI.C for definition
  SCI0CR2 = 0x0C; //See SCI.C for definition
}

/*--------------Below are functions that aren't involved with initialization-----------*/

/*--------------------Output the Angles to the LEDs in BCD-------------------*/

void TurnOn_LEDs(unsigned short angle){
   /*b9 b8  b7 b6 b5  b4   b3 b2 b1 b0   ----> binary assignment
     T4 T3  A8 A9 A10 A11  A1 A2 A3 A4  ----> Analog pins assigned
     
     b9 b8 = PTT[4:3]
     b7 b6 b5 b4= PT0AD[3:0]
     b3 b2 b1 b0 = PT1AD[1:4] --> Warning this one is opposite            
     
     PT1AD[6:7] and PT1AD[0] are inputs*/
     
     //Following are local variables. hundreds is MSB, ones is LSB and tens is LSB+1.
     int hundreds,tens, ones, i;
     
     if(angle>99){
       ones = angle%10;
       i = angle/10;
       tens = i%10;
       hundreds = i/10;
      
     } 
     else {
       hundreds = 0;
       tens = angle/10;
       ones = angle%10;
     }
     
     //The following 10 conditionals are for setting the 4 LSBs LEDs ON accordingly
     if(ones==0){
       PT1AD = 0b00000000;
     } 
     else if(ones==1){
       PT1AD = 0b00010000;
     }
     else if(ones==2){
       PT1AD = 0b00001000;
     }
     else if(ones==3){
       PT1AD = 0b00011000;
     }
     else if(ones==4){
       PT1AD = 0b00000100;
     }
     else if(ones==5){
       PT1AD = 0b00010100;
     }
     else if(ones==6){
       PT1AD = 0b00001100;
     }
     else if(ones==7){
       PT1AD = 0b00011100;
     }
     else if(ones==8){
       PT1AD = 0b00000010;
     }
     else if(ones==9){
       PT1AD = 0b00010010;
     }
     
     //The following 10 conditionals are for setting the 4 middle LEDs ON accordingly 
     if(tens==0){
       PT0AD = 0b00000000;
     } 
     else if(tens==1){
       PT0AD = 0b00000001;
     }
     else if(tens==2){
       PT0AD = 0b00000010;
     }
     else if(tens==3){
       PT0AD = 0b00000011;
     }
     else if(tens==4){
       PT0AD = 0b00000100;
     }
     else if(tens==5){
       PT0AD = 0b00000101;
     }
     else if(tens==6){
       PT0AD = 0b00000110;
     }
     else if(tens==7){
       PT0AD = 0b00000111;
     }
     else if(tens==8){
       PT0AD = 0b00001000;
     }
     else if(tens==9){
       PT0AD = 0b00001001;
     } 
     
     //The following four conditionals are for setting the two LEDs associated with the MSB
     if(hundreds==0){
       PTS = 0b00000000;
     } 
     else if(hundreds==1){
       PTS = 0b00000100;
     }
     else if(hundreds==2){
       PTS = 0b00001000;
     }
     else if(hundreds==3){
       PTS = 0b00001100;
     }
}

/*---------------------OutCRLF---------------------*/
// Output a CR,LF to SCI to move cursor to a new line
// Input: none
// Output: none
// Toggle LED each time through the loop

void OutCRLF(void){
  SCI_OutChar(CR);
  SCI_OutChar(LF);
  PTJ ^= 0x20;          // toggle LED D2
}

/*-------------------Lazy Delay Function Adapted from Lab 5------------------*/

void msDelay(unsigned int time)
{
	unsigned int j,k;
	for(j=0;j<time;j++)
		for(k=0;k<1322;k++);  //8/6.25 = 1.28*1032 = 1320.96 -> need 1321 iterations
}

/*void msDelay(unsigned int time){
  
  unsigned int i; //loop control variable
  
  TSCR1 = 0x90;   //enable timer and fast timer flag clear
  
  TSCR2 = 0x00;   //Disable timer interrupt, set prescaler=1
  
  TIOS |= 0x01;   //Enable OC0 (not necessary)
  
  TC0 = TCNT + 8000;
   
  for(i=0;i<time;i++) {
    TFLG2 = 0x80; //clear the TOF flag
    while (!(TFLG1_C0F));
    TC0 += 8000;
  }
  
  TIOS &= -0x01; //Disable OC0 (not necessary) 
  
}*/
