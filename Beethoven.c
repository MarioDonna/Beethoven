#include <xc.h>
#include <PIC12F683.h>
#include <Configuration_bits.h>


#define blue GP0  //GP0 pin, blue led
#define green GP4 //GP4 pin, green led
#define red GP5   //GP5 pin, green led
#define pwm GP1   //GP1 pin, PWM out
#define on 1      //GP5 pin, green led
#define off 0     //GP5 pin, green led



unsigned char cuentas=0,duty_cycle=0, ccp_count=0,tl1=0,tl2=0,th1=0,th2=0,contador1=0,contador2=0,contador3=0,flag=0;
unsigned int tiempo1=0,tiempo2=0,tiempo_final=0,frecuencia=0,T2=0;
unsigned char rojo=0,azul=0,verde=0,modo_sleep=0;


void Config_init() //Function to initialize common modules on PIC12F683
{
    OSCCONbits.IRCF=0b111; //8MHz
    CMCON0=0x07;//Comparator off
    ANSEL=0;//Make all pins digitital I/O
    GIE=1;//Enable global interrupts
    PEIE=1;//Enable peripheral interrupts
    
    
}

void Input_Capture_init()//Function to initialize Input Capture
{
    TRISIO2=1;//CCP1 pin as output
    CCP1CON=0b00000111;//Input capture configuration interrupt every 16 rising edge
    CCP1IE=0; //Input capture interrupt enable off
    T1CON=0b00100001;//Timer 1 register configuration for input capture
    TMR1GE=0;//Activate T1
  
}

void Input_Capture_conversion()//Funtion to calculate freq
{
        tiempo1=th1;//High part of the time of the first rising edge 
        tiempo1<<=8;//Shift left 8 times to make space for the low part of the time
        tiempo1+=tl1;//Low part of the time added to the variable
        tiempo2=th2;//High part of the second time of the second rising edge 
        tiempo2<<=8;//Shift left 8 times to make space for the low part of the second time
        tiempo2+=tl2;//Low part of the second time added to the variable
        
        if(tiempo2>tiempo1){//With this conditional we ensure that the time 2 has to be greater than time 1
            
        tiempo_final=tiempo2-tiempo1;//Substract T1 from T2 to calculate de period
        

       frecuencia=500000/tiempo_final;//Calculate freq, timepo_final are counts of 2 us 
   
        }
        
        
}

void Input_Capture_On()//Function to turn on the IC module
{   
    ccp_count=0;//Reset a counter of # of times the IC go to interrupt 
    
  CCP1CON=0b00000111;//Every 16 rising edge
    CCP1IE=1;//Enable IC interrupt
   
}

void Tim2_config()//Timer 2 configuration
{   TMR2IE=1;//Enable T2 interrupts
    T2CON=0b00000011;//Timer 2 configuration register
}

void Tim2_on()//Funtion to turn on T2
{   
    TMR2ON=1;//Bit to turn on the T2 module
}
void Tim2_off()//Funtion to turn off T2
{
    TMR2ON=0;//Bit to turn off the T2 module
    
}
void Leds_off()//Function to turn off all the colors of the led
{ 
            red=off;
            green=off;
            blue=off;
            
            if(rojo==1){//Every time the red led flash, the pwm will turn on and off at the same time the led flashes
                  pwm=off;
             }
}
void Leds_on()//Funtion to turn on one color of the led depending on wich color flag is turn on
{
            if(rojo==1)
            {   
                red=on;
                pwm=on;
            }
            else if(verde==1)
            {green=on;
           
            }
            else if(azul==1)
            {blue=on;
           
            }
    
}

void no_sleep()//Function of the mode no sleep
{
    if(modo_sleep>=1)//Funtion to reset all variables if the user had turn off the sleep mode
    {
        green=0;
        pwm=off;
        Tim2_off();
        T2=0;
        flag=0;//Bandera para volver a iniciar la conversion de frecuencias
    }
    
    modo_sleep=0;
    
    if(ccp_count==4)//At the 4 time of the IC interrupt, the code will enter here, in this way we ensure to make the conversion when all the values are set.
        {
            Input_Capture_conversion();//Start freq conversion
            if((frecuencia>3300)&&(frecuencia<3800))//Emergency alarms
            {
                contador1++;//Add 1 to a counter to have the record of how many times the freq is detected
               
            }
            else if((frecuencia>1900)&&(frecuencia<2500))//Alarm clock
            {
                contador2++;
              
            }
            else if((frecuencia>500)&&(frecuencia<700))//Doorbell
            {
                contador3++;
                
            }
            
            
            if(flag==0){//Flag to ensure to enter here only when the led and the motor had stoped
                
                
            //Conditionals to turn on the led and motor depending on the freq.    
            if(contador1>=70)
            {
                red=1;
                pwm=on;
                rojo=1;
                    contador1=0;
                    flag=1;
                    Tim2_on();
            }
            else if(contador2>=30)
            {
                green=1;
                pwm=on;
                verde=1;
                contador2=0;
                flag=1;
                Tim2_on();
            }
            else if(contador3>=15)
            {
                 blue=1;
                 pwm=on;
                 azul=1;
                contador3=0;
                flag=1;
                Tim2_on();
            }
             Input_Capture_On();//Start again IC module
            }
         
           
        }
    
}
void sleep()//Function of the sleep mode it is the same of the no sleep but it wont turn on the led.
{
    
     if(ccp_count==4)
        {
            Input_Capture_conversion();
            if((frecuencia>3300)&&(frecuencia<3800))
            {
                contador1++;
               
            }
            else if((frecuencia>1900)&&(frecuencia<2500)) 
            {
                contador2++;
              
            }
            else if((frecuencia>500)&&(frecuencia<700))
            {
                contador3++;
                
            }
            
            
            if(flag==0){
                
            if(contador1>=70)
            {
                pwm=on;
                contador1=0;
                flag=1;
                Tim2_on();
                modo_sleep=1;
            }
            else if(contador2>=30)
            {
                pwm=on;
                contador2=0;
                flag=1;
                Tim2_on();
                modo_sleep=1;
            }
            else if(contador3>=15)
            { 
                 pwm=on;
                contador3=0;
                flag=1;
                Tim2_on();
                modo_sleep=1;
            }
             Input_Capture_On();
            }
         
           
        }
    
}

void interrupt Interrupts() 
{

    /***************************************************************************/
    
    if((TMR2IE==1)&&(TMR2IF==1))
    {
        TMR2IF=0;
        //Conditionals to turn on and off leds and pwm depending on the color of the led.
        if(modo_sleep==0){
        if((T2>300)&&(T2<600))
        { 
            
            
            Leds_off();
        
        }
        
        else if((T2>600)&&(T2<900))
        { 
            
            
            Leds_on();
            if(verde==1)
            {
                pwm=0;
            }
        }
        
        else if((T2>900)&&(T2<1200))
        {   
            Leds_off();
            
            if(azul==1){
                pwm=0;
            }
        }
        
        else if((T2>1200)&&(T2<1500))
        {      
                Leds_on();
                
                if(verde==1){
                pwm=1;
                      }
        }
        
        else if((T2>1500)&&(T2<1800))
        {
            Leds_off();          
        }
         else if((T2>1800)&&(T2<2100))
        {   
        
            Leds_on(); 
            
            if(verde==1)
            {
                pwm=0;
            }
            else if(azul==1)
            {
                pwm=on;
            }
        }
        
        else if((T2>2100)&&(T2<2400))
        {
            Leds_off();
        }
        else if((T2>2400)&&(T2<2700))
        {   
            Leds_on(); 
            
            if(verde==1){
                pwm=1;
            }
        }
        else if((T2>2700)&&(T2<3000))
        {   
            Leds_off();
            
             if(azul==1){
                pwm=0;
            }
        }
        else if((T2>3000)&&(T2<3300))
        {   
            Leds_on();  
            if(verde==1)
            {
                pwm=0;
            }
        }
        else if((T2>3300)&&(T2<3600))
        {
            Leds_off();   
        }
        else if((T2>3600)&&(T2<3900))
        {   
            
            Leds_on();   
            
            if(verde==1){
                pwm=1;
            }
            else if(azul==1)
            {
                pwm=on;
            }
        }
        else if((T2>3900)&&(T2<4200))
        {
            Leds_off();   
        }
        else if((T2>4200)&&(T2<4500))
        {   
            Leds_on();
            
            if(verde==1)
            {
                pwm=0;
            }
        }
        
        if(++T2>=4500)
        {   
            Leds_off();
            T2=0;
            Tim2_off();
            flag=0;//Bandera para volver a iniciar la conversion de frecuencias
            pwm=0;
            //Banderas para saber que led estaba prendido
            rojo=0;
            verde=0;
            azul=0;
            }
         }
        else
        {
            if(++T2>=5000)
        {   
            
            if(++modo_sleep==2)
            {
                T2=0;
                pwm=off;
            }
            else if(modo_sleep==3)
            {
                T2=0;
                pwm=on;
            }
            else if(modo_sleep==4)
            {
                T2=0;
                pwm=off;
            }
            else if(modo_sleep==5)
            {
                T2=0;
                pwm=on;
            }
            else{
            T2=0;
            Tim2_off();
            flag=0;//Bandera para volver a iniciar la conversion de frecuencias
            pwm=0;
            }
           }
        }
        
    }
    /***************************************************************************/
    
     if((CCP1IF)&&(CCP1IE))
    {   
         
         CCP1IF=0;
    
    if(++ccp_count==1)
    {    
        CCP1CON=0;//Turn off the IC module to change the configuration to every rising edge
        CCP1CON=0x05;//Every rising edge
        
    }
    else if(ccp_count==2)
    {
        TMR1H=0;//Clear the high part of the timer1
        TMR1L=0; //Clear the low part of the timer1
        
    }
    else if(ccp_count==3)
    {
        tl1=CCPR1L;//Read the low part of the Timer 1
        th1=CCPR1H;//Read the high part of the Timer 1
    }
     else if(ccp_count==4)
    {  
         
        CCP1IE=0;//Turn off the IC interrupts to ensure the freq conversion
        tl2=CCPR1L;//Read the low part of the Timer 1 off the second rising edge
        th2=CCPR1H;//Read the high part of the Timer 1 of the second rising edge
        
    }
     else ccp_count=0;//If the IC module makes another interrupt this flag will be set to 0;
    
    
    }
    
    
    
}

void main(void) 
{
    Config_init();//Call the PIC12 configuration function
    Input_Capture_init(); //Call the IC config function
    
    Input_Capture_On(); //Turn on the IC module
    Tim2_config();//Call the timer2 configuration funtion
    
    //Configure all the pins as outputs
    TRISIO0=0;
    TRISIO5=0;
    TRISIO4=0;
    TRISIO1=0;
    //Clear all the GPIOS
    red=0;
    green=0;
    blue=0;
    pwm=0;
    
   
    
    while(1)
    { 
       
        TRISIO4=1;//Make GP4 as input to read the sleep mode switch
        
        if(GP4==1)//If the swith is on
        {
            sleep();//Enter sleep mode
            
        }
        else 
        {
            TRISIO4=0;//Make GP4 as ouput
            no_sleep();//Enter no sleep mode
        }

    }
    
    
}
