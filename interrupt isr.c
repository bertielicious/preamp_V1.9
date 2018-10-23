#include <xc.h>
#include <stdio.h>
#include <stdbool.h>
#include "config.h"
#include "stateMachine.h"
#include "bar_graph.h"
#include "fsm.h"
#include "split_digits.h"
#include "get_digit.h"
#include "write_character.h"
#include "update_PGA2311.h"
#include "clear_oled.h"
#include "state_of_input.h"
#include "setup_vol_scrn.h"
#include "interrupt isr.h"   

void interrupt isr(void) 
{
   
    if(PIR1bits.TMR1IF == 1)
    {  
        //RC0 = ~RC0;
        
        TMR1L = 0x00;
        TMR1H = 0x00;  
        PIR1bits.TMR1IF = 0;
    }
    if(INTCONbits.TMR0IF == 1)
    { 
        
        
        fsm(&kurrent_state); // FSM for rotary encoder detecting de-bounced CW and ACW rotations to provide volume control functionality
        
        if(CW_Flag && current_button_state != MUTE)       // clockwise movement of the knob
        { 
            if (count >=0 && count <123)
            {
                count++;
            }
            bar_graph();
            update_PGA2311(count * 2);
            split_digits();
        if(prev_count != count)
        {
        if(count >=0 && count <95)
                {
                    write_character(&minus[0], 0x0f,0xb3);
                }
                else
                {
                    write_character(&space[0],0x0f,0xb3);
                }   
            get_digit(tens, 0x1f);
            get_digit(units, 0x2f);
        }
            CW_Flag == LO;   
        }
        else if (ACW_Flag && current_button_state != MUTE) // anti-clockwise movement of the knob
        {
            if(count > 0 && count <= 124)
            {
                count--;
            }
            bar_graph();
            update_PGA2311(count * 2);
            split_digits();
        if(prev_count != count)
        {
        if(count >=0 && count <95)
                {
                    write_character(&minus[0], 0x0f,0xb3);
                }
                else
                {
                    write_character(&space[0],0x0f,0xb3);
                }   
            get_digit(tens, 0x1f);
            get_digit(units, 0x2f);
        }
            ACW_Flag == LO;     
        }
        
        
        
        /* detect if an input button has been pressed*/
        ADCON0bits.GO_nDONE = 1;    // start ADC conversion
        if(adc_result > 185 && adc_result < 240)
        {
            current_state = STREAM;
            CD_RELAY = 0;
            TUNER_RELAY = 0;
            PHONO_RELAY = 0;
            STREAM_RELAY = 1;
        }
        else if(adc_result >150 && adc_result < 185)
        {   
            current_state = CD;
            STREAM_RELAY = 0;
            TUNER_RELAY = 0;
            PHONO_RELAY = 0;
            CD_RELAY = 1;
        }
        else if(adc_result >110 && adc_result < 150 )
        {   
            current_state = TUNER;
            STREAM_RELAY = 0;
            CD_RELAY = 0;
            PHONO_RELAY = 0;
            TUNER_RELAY = 1;
        }
        else if(adc_result >70 && adc_result < 110)
        {   
            current_state = PHONO;
            STREAM_RELAY = 0;
            CD_RELAY = 0;
            TUNER_RELAY = 0;
            PHONO_RELAY = 1;
        }
     /*   else if(adc_result >20 && adc_result < 70)
        {   
            current_state = DAC;
        }*/
        
        /*****************MUTE on SWIP***********************/
        if(SWIP == LO && shit == 0)
        {
            while(SWIP != HI);
            old_count = count;
            
            while(count != 0)
                {
                    count--;
                    CW_Flag == LO;
                    split_digits();
                    if(count <95)
                    {
                        write_character(&minus[0], 0x0f,0xb3);
                    }
        
                    get_digit(tens, 0x1f);
                    get_digit(units, 0x2f);
                    bar_graph();
                    update_PGA2311(count * 2);
                }  
            write_character(&space[0],0x0f,0xb3);
            write_character(&m[0],0x1f,0xb3);  //do this
            write_character(&u[0],0x2f, 0xb3); 
            write_character(&t[0],0x3f, 0xb3);
            write_character(&e[0],0x4f,0xb3);
            current_button_state = MUTE;
            shit = 1;
        }
        else if (SWIP == LO && shit == 1)
        {
            while(SWIP != HI);
            current_button_state = UNMUTE;
            //count = old_count;
            write_character(&space[0],0x0f,0xb3);
            write_character(&space[0],0x1f,0xb3);
            write_character(&space[0],0x2f,0xb3);
            write_character(&space[0],0x3f,0xb3);
            write_character(&space[0],0x4f,0xb3);
            
            write_character(&d[0],0x3f,0xb3);   // write dB
            write_character(&B[0],0x4f,0xb3);
            
            while(count != old_count)
                {
                    count++;
                    ACW_Flag == LO;
                    split_digits();
                    
                    if(count <95)
                    {
                        write_character(&minus[0], 0x0f,0xb3);
                    }
                    else
                    {
                       write_character(&space[0],0x0f,0xb3); 
                    }
                    get_digit(tens, 0x1f);
                    get_digit(units, 0x2f);
                    bar_graph();
                    update_PGA2311(count * 2);
                }  
            
          
            
            
            shit = 0;
        }
        
        
       // if(prev_count != count) 
       // {
            //printf("volume position = %d\n", count);  // display latest value of count via serial monitor
       // }
        
        prev_count = count;
        TMR0 = 0xE0;
        INTCONbits.TMR0IF = 0;
    
    }

    
    if (PIR1bits.ADIF == 1)
    {
        adc_result = ADRESH<<6 | ADRESL;       // combine two 8 bit results into 10-bit ADC value
      //  printf("adc_result = %d\n", adc_result);
        
        ADRESH = 0x00;
        ADRESL = 0x00;
        PIR1bits.ADIF = 0;
    }
  
    
    if(PIR1bits.TMR2IF == 1)
    {
        if(current_state != previous_state)
        {
            state_of_input(&current_state);
            previous_state = current_state;
        }
        
        TMR2 = 0x00;
        PIR1bits.TMR2IF = 0; 
    }
     
     if(IOCBFbits.IOCBF5 == 1)
     {
         if(magic_num%2 != 0)   // first rising edge of pulse detected
         {
             if(countRC5 == 0)                 // path A
             {
                T1CONbits.TMR1ON = 1;       // start TMR1
                magic_num++;               // make an even number so %evennum = 0
                countRC5 = 1;
                IOCBPbits.IOCBP5 = 1;       // enable interrupt on rising edge
                IOCBFbits.IOCBF5 = 0;      // clear relevant interrupt flags
                INTCONbits.IOCIF = 0;  
             }
             else if(countRC5 !=0)             // path C
             {
                 
                 temp3 = TMR1H<<8 | TMR1L;    // store TMR1 at start of next pulse 
                 space_width = temp3 - pulse_width;
                 
                 if(space_width >= 1776 && space_width <= 5332)
                 {
                     pulse_type = SS;
                 }
                 else if(space_width >= 5333 && space_width <= 8888)
                 {
                     pulse_type = LS;
                 }
                 stateMachine();
                 TMR1H = 0x00;
                 TMR1L = 0x00;
                 //temp1 = 0;
                 magic_num++;
                 IOCBFbits.IOCBF5 = 0;      // clear relevant interrupt flags
                 INTCONbits.IOCIF = 0;    
             }
         }
         else           // path B
         {
            
             temp2 = TMR1H<<8 | TMR1L;    // store TMR1 at start of pulse
             pulse_width = temp2; //- temp1;
             magic_num = 1;
             
             if(pulse_width >= 1776 && pulse_width <= 5332)
                 {
                     pulse_type = SP;
                 }
             else if(pulse_width >= 5333 && pulse_width <= 8888)
                 {
                     pulse_type = LP;
                 }
             stateMachine(); 
             IOCBFbits.IOCBF5 = 0;      // clear relevant interrupt flags
             INTCONbits.IOCIF = 0;
         }   
     }
}
