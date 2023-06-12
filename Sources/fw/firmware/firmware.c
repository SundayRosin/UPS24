/*
 * firmware.c
 *
 * 
 *  Author: Valek
  */ 
#define  F_CPU 8000000UL
#include "function.h"
#include <avr/io.h>
#include <avr/interrupt.h>
//#include <util/delay.h>
#include <avr/wdt.h>


uint8_t razrad=0; //����� ������� ������� ����� ����������
uint8_t led[3]={0,0,0};//����� ������ � �������
uint8_t  a=0;
uint8_t v=0; //������� �������  
volatile uint32_t summa=0; //c���� ������� ���
uint8_t kanal=0;//����� ������ ��� �������� �������������� ���
uint16_t signals[]={0,0,0,0,0}; //����� ��� �������� ������ ���
uint8_t tmp=0,tmp1=0; //��������� ����������.
uint8_t b_timer=0; //�������� ������� ������
uint8_t b_timer2=0; //������� �������� ������� ������
uint8_t menu=0; //���� ����������� ���������� �� �������
uint16_t tmp16=0;//��������� ����������
uint16_t system[]={0,0,0,0,0,0,0,0}; //���� ��,��� ��������, ��� ������, ��� ���., ��� 1
uint8_t calibr_cells[]={0,0,0,0,0}; //������������� �������� ��� ������� �� �������
uint16_t u_param[]={0,0,0} ;// ��������� ���������� 	
uint16_t BT1=0,BT2=0;//����� �������� 16������� ��� ���������� ������� 	 
uint8_t RX_buff[14]; //����� ������ ������ USART
uint8_t RX_=0; //������� ������
uint8_t TX_tmp,TX_tmp2; 
uint8_t TX_pointer=0;
uint8_t TX_buff[18];//16 �������, ��� CRC16
uint8_t RTC_s=0; //������� ������
uint8_t RTC_m=0; //������� �����
uint8_t RTC_h=0; //������� �����
uint8_t ups_mode=0;//����� ������ ��������������
uint32_t C60=0;//������� ������� �� 60 ����� 
uint8_t minut_flag=0; //���� ��� ������� ������ �������
uint8_t C=0; //�������� ������� ������� ����������
uint16_t L[180]; //������ ��� ���������� ����������
uint8_t log_point=0;//����� ������� ������ �������
uint8_t send_pointer=0; //������������ ��� �������� ������



 
//RTC. ���������� �������� ������ �������
ISR(TIMER2_OVF_vect)
{
	//���� ����� � ����� ������� ������, ������� ��� �������(RTC_s=0;) � button()
	if((RTC_s==10)&(menu>=5)) //��� 10���
	{
		if (menu==6){menu=0; ups_mode=5;} //������ ������ ������ ������� �������
			else {menu=0;ups_mode=0;} //�������  � ������� �����.
			 
	}
  //���� ��������� �������
  if(RTC_s==59) //������ ������
  {
	 minut_flag+=128; //������ ��� ������ ������!	  
	  if(RTC_m==59)
	  {
		  if (RTC_h<0xff)
		  {
			  RTC_h++;
		  }
		  else {RTC_h=0;}
		  
		  RTC_m=0;
	  }
	  else
	  {
		  RTC_m++;
	  }
	  RTC_s=0;  
  } 
  else
  {
	 
	  RTC_s++;
  }

}
	
	
//���������� ��� ������������ ��������� 122��
ISR(TIMER0_OVF_vect)
{
   //������� led �� �������
   dinamic(razrad);
   if(razrad!=2)razrad++;
	   else razrad=0;
   button();
    
}

//���������� ���������� �� ������ USART
ISR(USART_RX_vect)
{
	if(RX_<13)
	{ 
	  RX_buff[RX_] =UDR0; //������� ������
	  //RX_buff[0]=UDR0; //������� ������
	 RX_++;
	}
	else
	{
		tmp=UDR0; //������ ������ ����� ���������� ����� �����������
	}
}

//���������� �� ����������� ������
ISR(USART_UDRE_vect)
{	
	//������� ������
	if (TX_pointer<18) //8*2byte
		{
			UDR0=TX_buff[TX_pointer]; //���������� � ����������
			TX_pointer++;
		}
		else //��� ������ ��������
		{
			TX_pointer=0;
			//���������� ���������� �� ������� ������
			UCSR0B&=0b11011111; //UDRIE0=0;
		}
		
}

int main(void)
{ 
	wdt_reset(); //����� watchdog �������,��� WDTON=0,�����
	wdt_enable(WDTO_2S);//  2������� ���� �� ������� ������,�����
	//��������� ���������
	init();
	asm("sei");
	boot_constanta(); //�������� ������������� ��������
		 
	startADC(kanal);
  loop:
	 wdt_reset(); //����� watchdog �������
	  
	  //�������������� ���������
  	if((ADCSRA&0b01000000)==0)
	   {	   
		    tmp=ADCL;
			tmp1=ADCH; 		  		  
		   if(v<64)
		   {			    
			   //��������� � summa �������� ���
			   asm_add(tmp1,tmp);
			   v++;
			  
		   }
		   else
		   {
			 // ��������� ������, ������  7 6 5 1 0
			 //      signals[]            4 3 2 1 0
		     if (kanal>1)kanal-=3;			 
			 tmp16=asm_div(summa);// ���� �� 64
			 //���������� �������� � ������ ����������
			 signals[kanal]=calibr(tmp16,calibr_cells[kanal]);
			 summa=0;
			 v=0;
			 if (kanal<4) {kanal++;}
				 else //��� ������ ���������
				 {
					 kanal=0;
					 //������� ���������
					 system[0]=asm_ui(signals[0],391);// ��� ��������
					 system[1]=asm_ui(signals[1],391);// ��� ������ �������
					 system[2]=asm_ui(signals[2],375);// ���������� ����� �� ��������
					 system[3]=asm_ui(signals[3],375);// ���������� �� ������� 1(������)
					 system[4]=asm_ui(signals[4],375);// ���������� ��
					 if (system[2]>system[3])
					 {system[5]=system[2]-system[3];} //���������� ������� �������
			         else {system[5]=0;} 
					//��� ���
					ups(RTC_m,RTC_s,system,&ups_mode);
					
				 }  //���� ��,��� ��������, ��� ������, ��� ���., ��� 1
			 // ��������� ������
			 if (kanal>1) {kanal+=3;} 
			 			   
		   }		
		  startADC(kanal);      
       }
	    	
	     
		  
		//������ � ��������� �����
		switch(RX_buff[0])
		  {
		       case 'c':
			          send_system();
					//���������� ���������� �� ����������� ������
			       clear_RX();
				    UCSR0B|=0b00100000;///UDRIE0=1;
			       
			   break;
			 
			  case 'L':
			        send_const();
			        //���������� ���������� �� ����������� ������
			        clear_RX();
					UCSR0B|=0b00100000;///UDRIE0=1;
		      
			  case 'K':   if (RX_==12) //���� ���� ����� ������
			               {
							   write_calibr(); //������ ������ � eeprom		   
							   clear_RX(); 
			               }
			    break;
			   //���������� �����������
			   case 'R':{
				          //���������� ������
						  if(send_pointer<175){send_log();} 
							  else {send_pointer=0;send_log();}
						  clear_RX(); 
					      UCSR0B|=0b00100000;///UDRIE0=1;  
			            }  break;
						
			  default: clear_RX(); 	break;
		  }
				   
		//����� �� �������
		
		if (menu>=128) //�������������� ������� ���� � ���������� ���
		{tmp=menu&0b01111111; } else {tmp=menu;}
			
		if(tmp<5){
			disp_inf(tmp);
			
		} //������� ����� �����������
		else { //����� � ������� ���������/���� ������
			
			   //���� tmp=5 ���������� �� ������� 0� ���� tmp=6  1�
			  znak(tmp);
			  //���� ������ znak(tmp); ��� ��������� �� ��� ��� �������� ������ �������� �����
			  //�������� ����
			 }
				
	goto loop;
}