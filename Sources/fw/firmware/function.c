/*
 * function.c
 *
 * 
 *  Author: Valek
 */ 

#define  F_CPU 8000000UL
#include <util/delay.h>
#include <avr/io.h>
#include <avr/pgmspace.h> // ��� ���� ��� �� ����� ��� � flash ������ 

//�������������� ��� ����������
uint8_t const zgen[21] PROGMEM={
	
	0b00101000, //0
	0b11101110, //1
	0b00110100, //2
	0b10100100, //3
	0b11100010, //4
	0b10100001, //5
	0b00100001, //6
	0b11101100, //7
	0b00100000, //8
	0b10100000, //9
	0b11111111, //blank  10
	0b11110111, //-  11
	0b01100000, //A 12
	0b00100011, //b 13
	0b00111001, //� 14
	0b00100110, //d 15
	0b00110001, //e 16
	0b01110001, //f 17
	0b00111011, //L 18
	0b01111001, //� 19
	0b01100010 //?  20
};


extern uint16_t system[];
extern uint8_t TX_tmp,TX_tmp2;
extern uint8_t TX_pointer;
extern uint8_t TX_buff[];


//������������� ���������
void init(void)
{
	/*
	PD2-SHCP,PD3-STCP,PD4 - DS ���������� ��������� 74hc595
	PD5-AN3,PD6-AN2,PD7-AN1
	*/
    DDRD=0b11111100;
	DDRB=0b00000110; //PB1 PB2 outputs PWM
	PORTB=0b00000001; //PB0 ���������� �� + ����� ��������
	//���������� ���
	DDRC=0b00011100;
	
	//�������������  TC0 ��� ������������ ���������
	//8 000 000 /256/256=122/3 =40HZ
	TCCR0B=0b00000100; //clkI/O/256 (from prescaler)
	TIMSK0=1; //enable interrupt
	
   //���������� �������� ������
   DIDR0=0b11100011; //ADC 7,6,5,1,0  
 
   //�������������� 16bit TC1
   TCCR1A=0b10100010; //Clear OC1A/OC1B on compare match  FAST PWM
   TCCR1B=0b00011001;// clc/1
   //TOP -ICR1
   ICR1=0xffff;
  // OCR1A=0;
   //OCR1B=0;
  
   //�������������� ������,������������ ��� ���� RTC
	//������� �������� �� 128
	TCCR2B=0b00000101;
	//���������� ������ � ����������� ����� AS2=1
	ASSR=0b00100000;
	//��� ���������� ������������� �������
	//Wait for TCN2UB, OCR2xUB, and TCR2xUB.
	TCNT2=1;
	while((ASSR&0b00011111)!=0);
    TIMSK2=0b00000001; //�������� ����������
   
      //������������� USART
      //#define FOSC 8000000 // Clock Speed
      //#define BAUD 9600   //�������� ��������
      //#define MYUBRR (FOSC/16*BAUD)-1
     //Set baud rate 51
     UBRR0H =0;// (uint8_t)(MYUBRR>>8);
     UBRR0L =0x33;// (uint8_t)MYUBRR;
     //�������� �����,�������� � ����������
     UCSR0B=0b10011000;//RXCIE0,TXCIE0=0,RXEN0,TXEN0;
     /* Set frame format: 8data, 2stop bit */
     //����������� �����,��� �������� 1����,8���
     UCSR0C =0b00000110;
 }

//����� ������ � �������
void reg(uint8_t b)
{
	for(int i=0;i<8;i++)
	{
		//��������� ���� �� DS // PD2-SHCP,PD3-STCP,PD2-SHCP,PD3-STCP,
		if ((b&0b10000000)==0b10000000)
		{
			PORTD|=0b00010000;//PD4 - DS
		}
		else
		{
			PORTD&=0b11101111;//PD4 - DS
		}
		//��������� PD2-SHCP
		PORTD|=0b00000100;
		PORTD&=0b11111011;
		b=b<<1;
	}
	//��������� �� �������
	//PD3-STCP
	PORTD|=0b00001000;
	PORTD&=0b11110111;
	
}

//������������ ��������� PD5-AN3,PD6-AN2,PD7-AN1
void dinamic(uint8_t pos)
{
	extern uint8_t led[3];
	uint8_t point,tmp;
	 //��������� ����� 
	  point=led[0];
	  point&=0b01100000; 
	  led[0]=led[0]&0b00011111; //��������� ��� ����	
	
	PORTD&=0b00011111; //�������� ���
	//if (led[pos]<19) //������ �� ������������ ��������
    //{tmp=pgm_read_byte(&zgen[led[pos]]);}// ����� ��� �� ���������������
	//else
	//{tmp=pgm_read_byte(&zgen[10]);} //��������
	tmp=pgm_read_byte(&zgen[led[pos]]);// ����� ��� �� ���������������	
    point>>=5;//��������� � ���������� ��� 1 2 3
	if((pos+1)==point)
	{tmp&=0b11011111;} //������� �����
				
	reg(tmp); //����� ������ � �������
//������� ������
switch(pos)
		{
			
			case 0:  PORTD|=0b10000000; break;	//1
			case 1:  PORTD|=0b01000000; break;	//2
			case 2:  PORTD|=0b00100000; break;	//3
						
		}
	
  }


//������ �������������� ���
void startADC(uint8_t channel)
{
    ADMUX=0;	
	//VCC ref voltage ADLAR=0
	ADMUX=0b01000000+channel;
	ADCSRA=0b11000110;  //64 125��� �������� �������
	
}

//�������������� ��������� ���� � ��� 
void hexdec(uint16_t hex,uint8_t point)
{
	extern uint8_t led[];
	uint8_t dec[3]={0,0,0};
		 
		 while (hex>=1000)
	     	{
		      dec[0]++;
		      hex-=1000;	
		    }
	     	while (hex>=100)
	   	   {
		   	dec[1]++;
		   	hex-=100;
	   	   }
    	   while (hex>=10)
	   	   {
	    	dec[2]++;
		   	hex-=10;
     	   	}	
	   
	   
		led[0]=dec[0]+point; //�������� �����
		led[1]=dec[1];
		led[2]=dec[2];
	}
	
	//������� ������ �� EEPROM
	// ������ ������ ������ � �������� �������, �������� �������� � ���� ������.
	uint8_t ee_read (uint8_t addr)
	{
		asm volatile("cli");
		/* Wait for completion of previous write */
		while(EECR & (1<<EEPE));
		/* Set up address register */
		EEARL = addr;
		/* Start eeprom read by writing EERE */
		EECR |= (1<<EERE);
		/* Return data from data register */
		asm volatile("sei");
		return EEDR;
	};


	//������� ������ � ������ � ��������� ����������� � ������ �� ������� ��������� ���� ��� �������� ���� ���.
	void ee_write (uint8_t addr,uint8_t data)
	{
		asm volatile("cli");
		unsigned char old_data=0;
		/* Wait for completion of previous write */
		while(EECR & (1<<EEPE));
		/* Set up address  */
		old_data=ee_read(addr);


		if (old_data != data)
		{
			EEARL = addr;
			EEDR = data;
			/* Write logical one to EEMPE */
			//EECR |= (1<<EEMPE);
			asm volatile ("sbi 0x1F,2  ");  // ����� ��� �� �������� �� 4 ����� ��������� �������!!!!!! ��� ���������� �����������
			/* Start eeprom write by setting EEPE */
			//EECR |= (1<<EEPE);
			asm volatile ("sbi 0x1F,1");
			asm volatile("sei");

		}
	}
	
	
	
//����� ������, ������������ ����	
void button()
{
	extern uint8_t b_timer,b_timer2,menu;
	extern uint8_t RTC_s;
	
		 //����� ������
	 if((PINB&0b00000001)==0) //������ ������
	 {
		 //������ ������������ 122/24=5hz. 1/5=0.2c
		 if(b_timer==24)
		 {
			 //������ ���� ������
			 
			 //������� �����:  ������ ����,���� ������ ���������
			 if((menu&128)==0)
			 { 				  	   
				     //������������ ���� �� �����
				     if (menu<4) //5 ������� ����
				     {menu+=128+1;}
				     else 
				     {
				        if (menu<5){menu=128;} //c���� ������ ���� 
					     else //������������ �������(�������� ��� ������� �������)
					    {
							RTC_s=0; //����� �������� ������
						   //������������ �������(5,6) �� �����
						   if (menu<6) {menu+=128+1;}
							else {menu=5+128;}
				     	}  
				     }
				 
			 } //++���� � ��������� ����(128) ��� ���� �������
				 
			    //�������� ��������(2.12c) ������� �� ������
			     if(b_timer2==255)
			      {
				     //���� ������� �������
					   menu=5; //����� � �������
					  //menu&=0b01111111; 
					    b_timer=0; b_timer2=0; menu+=128;
			      }
			     else
			      { b_timer2++; }
		   
		 }
		 else
		 { b_timer++;}
	 }
	 else {b_timer=0;b_timer2=0; menu&=0b01111111; } //������ ���������
}	


//��������� �� ������ ��������� ���������(����������, ������ ��������)
void boot_constanta()//uint8_t *sys_const,uint16_t *u_param)
{	
	extern uint8_t calibr_cells[];
	extern uint16_t u_param[];
	//����� �� EEPROM ������������� ���������
	for (uint8_t i=0;i<5;i++)   //��������� ��� ���������� �������
	{   		
		calibr_cells[i]=ee_read(i);
		//���� ������ ������
	    if (calibr_cells[i]==0xFF) {calibr_cells[i]=0;}
	}
     uint8_t tmpH,tmpL,k=0;
	//����� �� ������ �������� ���������� 5H6L,7H8L,9H10L
	//������ ���������� � ���� ����
	for(uint8_t i=5;i<11;i+=2)
	{
		tmpH=ee_read(i);
		tmpL=ee_read(i+1);
		//�������� int16_t �� ���� int8_t. "�" ����� ����� ������ ��� ��������
		asm volatile(   "clr %A[RES]"	"\n\t"
		                "clr %B[RES]"	"\n\t" 
		                "mov %A[RES],%[Low]" "\n\t"
						"mov %B[RES],%[Hi]" "\n\t"
		               :[RES]"=&d"(u_param[k]):[Hi]"r"(tmpH),[Low]"r"(tmpL)  
		);
        k++;	
	}
	//�� ������ ������ �����
	for (uint8_t i=0;i<3;i++)
	{
		if (u_param[i]>3000)
		{
			switch(i)
			{
				case 0: u_param[i]=1400; break; //���������� ��� ����������
				case 1: u_param[i]=1300; break; //���������� ���� ���
				case 2: u_param[i]=1080; break; //������ ���� �� ��������
				default: break;
			}
		}
	}
 		
}



//���������� ��� ������� ��������, ������������ ��� ����������
uint16_t calibr(uint16_t adc,uint8_t val)
{
	if (val <=127) //���������
	{
		adc+=val;		
	}
	else //�������
	{
	   val-=127; //-10
	   if (adc>val){adc-=val;} 	
	}
   return adc;	
}


//������������ �������
void balansir(void)
{
	extern uint8_t ups_mode;
	extern uint16_t system[];
	extern uint16_t BT1,BT2;
	extern uint16_t u_param[];
	uint16_t raznost;
	
	//system[5] ���������� �� ������� �������
	//system[3] ���������� �� ������ �������
	//���������� �� ������ ������� ������ ��� �� �������
	if (system[3]>system[5])
	{
		raznost=system[3]-system[5]; //�������� ������
		
		//���� ������ ������ ������  
		if((raznost>15)&(BT2<0xffff))
		{
		  BT2++;	
		}
         //������ ���� ������ 
		if (raznost<5)
		{
			BT2=0;
		}
		  		  	
	}
	else
	{
		BT2=0;
	}
  //------------------------		
		
	//���������� �� ������� ������� ������ ��� �� ������
	if (system[5]>system[3])
	{
		raznost=system[5]-system[3];
		
	    //���� ������ ������ ������  
		if((raznost>15)&(BT1<0xffff))
		{
			BT1++;
		}
		
		if (raznost<5)
		{
			BT1=0;
		}		
	}
	else
	{
		BT1=0;
	}

	
	OCR1A=BT1;
	system[6]=BT1; //������� �������
	OCR1B=BT2;
	system[7]=BT2; //������ �������
	
}


//������� ���������� crc16(��� � modbus)
//���������: ��������� �� �����,
//           ������ ������,
//
//����������:������������ �������� �RC16, ����� ��� �  ����� ����
void crc16(uint8_t *buf,uint8_t sizeBuf)
{
	extern uint8_t TX_buff[];
	uint8_t i, j;
	uint16_t crc=0xffff;
	for(i=0;i<sizeBuf;i++)
	{ crc=crc^buf[i];
		for (j=0;j<8;j++)
		{ if ((crc&0x0001)!=0x0000) crc=(crc>>1)^0xA001;
			else crc>>=1;
		}
	}
	
	//�������� ����������� ����� � �����
	//������� ���� ����� uint16_t
	asm volatile (
	"mov %[RES],%A[D16]"	"\n\t"
	:[RES]"=&d"(TX_buff[16]):[D16]"r"(crc)
	);
	
	//�������
	asm volatile (
	"mov %[RES],%B[D16]"	"\n\t"
	:[RES]"=&d"(TX_buff[17]):[D16]"r"(crc)
	);
	
	
}


//������������ ��� ������� ��������� ������(6 ���������)
void send_system()
{		
	uint8_t k;
	for(uint8_t i=0;i<16;i++) //���������� ��� ��� ���������
	{
		k=i/2;
		//TX_tmp2=TX_pointer/2;
		//������� ���� �����
		if((i%2)==0)
		{
			asm volatile (
			"mov %[RES],%A[D16]"	"\n\t"
			:[RES]"=&d"(TX_buff[i]):[D16]"r"(system[k])
			);
		}
		else //�������
		{
			asm volatile (
			"mov %[RES],%B[D16]"	"\n\t"
			:[RES]"=&d"(TX_buff[i]):[D16]"r"(system[k])
			);
		}
			
	}
	
	//������ ����������� �����
	crc16(TX_buff,16);	
}

//�������� ������������� ���������� � ����� �����������
void send_const()
{
	extern uint8_t calibr_cells[]; //5����
	extern uint16_t u_param[];
	extern uint8_t RTC_s;
	extern uint8_t RTC_m;
	extern uint8_t RTC_h;
	//������������� ��������� ������� ���������
	for (uint8_t i=0;i<5;i++)
	{
	   TX_buff[i]=calibr_cells[i];
	}
	
	uint8_t k=5; 
	//��������� ���������� 5:6 7:8 9:10
	for (uint8_t i=0;i<3;i++)
	{
		
		//�������� uint16_t �� ��� uint8_t 
		asm volatile(  "mov %[Low],%A[X]" "\n\t"
		               "mov %[Hi],%B[X]" "\n\t"
		:[Hi]"=&d"(TX_buff[k]),[Low]"=&d"(TX_buff[k+1]):[X]"r"(u_param[i])
		);		
		k+=2;
	}				
				 
	TX_buff[11]=ee_read(11); //������� �������
	TX_buff[12]=RTC_s; //���
	TX_buff[13]=RTC_m; //���
	TX_buff[14]=RTC_h; //����
	TX_buff[15]=0;

	//������ ����������� �����
	crc16(TX_buff,16);		
}

//���������� ������������� ���������
void write_calibr(void)
{
	
	extern uint8_t RX_buff[]; //����� ������ ������ USART
	extern uint8_t RX_; //������� ������
	extern uint8_t calibr_cells[]; //5����
	extern uint16_t u_param[];
	  for(uint8_t i=0;i<5;i++)
		{
			calibr_cells[i]=RX_buff[i+1];
			RX_buff[i+1]=0;
		   	
		}
		
	 //������������ � EEPROM ������������� ���������
	    for (uint8_t i=0;i<5;i++)
	     {
		 ee_write(i,calibr_cells[i]);
	     }
	
		 
	//������ ����������	
	 uint8_t k=6;	
	 for(uint8_t i=0;i<3;i++)
	 {
		 	//�������� int16_t �� ���� int8_t. "�" ����� ����� ������ ��� ��������
		 	asm volatile(   "clr %A[RES]"	"\n\t"
		 	"clr %B[RES]"	"\n\t"
		 	"mov %A[RES],%[Low]" "\n\t"
		 	"mov %B[RES],%[Hi]" "\n\t"
		 	:[RES]"=&d"(u_param[i]):[Hi]"r"(RX_buff[k]),[Low]"r"(RX_buff[k+1])
		 	);
	       k+=2;
	 }	
	 
	 //���������� � ������ ����������
	  for (uint8_t i=5;i<11;i++)
	  {
		  ee_write(i,RX_buff[i+1]);
	  }
	  RX_=0;
		
		
}


//uint8_t *minut_flag,uint8_t *mode,uint32_t *Sbuff,uint8_t *C,uint16_t *sys);
//��������� ������� ������� �������
void cell_capacity(void)
{
	extern uint8_t minut_flag,C;
	extern uint32_t C60;//������� ������� �� 60 ����� 
	extern uint16_t system[];
	extern uint16_t L[];
	extern uint8_t log_point;
	
	//������ ������ � mode=5
	if((minut_flag&0b10000000)==128)//&(*mode==5))
	{
		minut_flag&=0b01111111; //������� ����
	    
		//������ 10 ����� �������� ��������� � ���
		if ((minut_flag%10)==0)  
		{
			//���� ������ �������� �� ����
			if (log_point<180)
			{  
				L[log_point]=system[3];  //�1
				log_point++;
				L[log_point]=system[5]; //�2
				log_point++;
				L[log_point]=system[0]; //I ��������
			    log_point++;
			}
						
		}
	    
		//������� ������� �������
	    if (minut_flag<59)	
		{
		   asm_addA(system[0]); //c�������� ��� �������� �� 60���		   	     
		   minut_flag++;
		}
		else
		{				  
		  	
		   C+=asm_divB(); //��������� ������� �60/6000
		   C60=0;
		   minut_flag=0;
		}
	}
	
	
}

//��������� ���������� ������� �������, � ���������� �� � ������
void write_cap()
{
	extern uint8_t C;
	extern uint32_t C60;
	C+=asm_divB(); //��������� ������� �60/6000
	C60=0;
	
	ee_write(11,C);//��������� �������
	C=0;	
}

//��������� ��������
void clear_balansir()
{
	extern uint16_t system[];
	OCR1A=0;
	system[6]=0; //������� �������
	OCR1B=0;
	system[7]=0; //������ �������
}

//��������� ��� ������� ��������������
void ups(uint8_t m,uint8_t s,uint16_t *sys,uint8_t *mode)
{	
	extern uint8_t C;
	extern uint32_t C60;
	extern uint16_t u_param[];
	
	// u_param[0]=1360; break; //������ ����� ������
	// u_param[1]=1350; break; //������ �����,��� �����
	// u_param[2]=1080; break; //������ ���� �� ��������
	if (sys[4]>2400) //���������� ���� � ����?
	{ 		
		//������� ����������?������ 3 ���? �������� �����
		if ((*mode==0)&(s>3)){PORTC|=0b000000100; *mode=1;C=0;C60=0;}; //��� ������  ����� ��������� ������� ���
		//������ 2 ������ ����� �������� � �� � ��� �������	
		if ((*mode==1)&(m>1)){PORTC|=0b00011000; *mode=2;}; //��� ������ 
		
		if ((*mode>0)&(*mode<3)){balansir();} //�������� �������
		else {clear_balansir();} //����
		//������� ��������� ����� �������� � ����� 3-������ ��������.
		if((*mode==2)&((sys[3]>u_param[0])&(sys[5]>u_param[0])))
		{
			*mode=3;
			PORTC&=0b11111011; //��������� �����
		}
		
		//�������� �����
		if ((*mode==3)&((sys[3]<u_param[1])&(sys[5]<u_param[1])))
		{
			*mode=2; PORTC|=0b00000100; 
		}
		
		//����� ���� ����� ����� � ����� 2	
		//����� ������ ������� �������
		if(*mode==5) 
		{			
			PORTC&=0b11110011; //�������� ����� � �������� �� ��, �������� �� ������ �� �������
			cell_capacity();//����� ������� �������
			//������� ���������!!!
		    if ((sys[3]<u_param[2])|(sys[5]<u_param[2]))  
			{
				write_cap(); //���� ������� �������
				PORTC|=0b00011100;//��� ������ � ����� ���� � �� 
			    *mode=1;//��������� ����� ������
			}
		
		}
	}
	else //��� ���� � ����
	{ 
		clear_balansir(); //���� ����������
		*mode=0; //��������� ������ ����������� �� ��������� ����
	   //��������� ����� ������� � �������� �� ��.
	   PORTC&=0b11110011;	
	   cell_capacity(); //����� ������� �������
	  //������ ���������� ��������� ��� �������� ���������� �� ����� �� ���.
	  if ((sys[3]<u_param[2])|(sys[5]<u_param[2]))  
	  {
		   //����� ��� ���� ��� �� �� ����� � ������ ��� ���� �������.
		   //������� ������� �� ����������� ����� ��� � ��, �������� �������� ����.
		 for(uint8_t Q=0;Q<10;Q++)
		 { _delay_ms(3000);}
		 
		  write_cap(); //�������� ������� ������� � ������.
		  PORTC&=0b11100011; //�������� ��������(�������)
		  //��������� ������ ���������/���������� ��������, ���� �����������
		  //����������� ������������������ �����    
		   while(sys[4]<2400);//��� �������������,� ����� ��������� ��������
	  }
	    		
	}
		
	
	
}



//���������� ������
void send_log()
{
	extern uint8_t send_pointer;	
	extern uint16_t L[];
	//�������� �����
	TX_buff[0]=send_pointer; //�
	
	for(uint8_t i=1;i<13;i+=2) //���������� ��� ��� ���������
	{
		    //������� ���� ����� uint16_t
			asm volatile (
			"mov %[RES],%A[D16]"	"\n\t"
			:[RES]"=&d"(TX_buff[i]):[D16]"r"(L[send_pointer])
			);
		
		   //�������
			asm volatile (
			"mov %[RES],%B[D16]"	"\n\t"
			:[RES]"=&d"(TX_buff[i+1]):[D16]"r"(L[send_pointer])
			);
		send_pointer++;
	}
	for(uint8_t i=13;i<16;i++)
	{
	   TX_buff[i]=0;
	}
	//������ ����������� �����
	crc16(TX_buff,16);
	
	
}

void znak(uint8_t x)
{
	extern uint8_t led[3];
	
	if (x==5)  //��������� ����-����� ������� ����
	{
		
		//������� ��� �� ����� "0" "1" "2"
		led[0]=0;//����� ����
		led[1]=14;//C  14
		led[2]=10;//������� ��� ����
		
	}
	else //menu=6
	{
		//led[0]=1; //������ ������� ���������� 5. ������. �������� ���������� ����������������� ���)
		//tmp=1;
		led[0]=1; //��� ��� ���������� 1))
		led[1]=14;//  C
		led[2]=10; //���� ��� ��������
	}
	
}

void disp_inf(uint8_t mode )
{
	extern uint16_t system[];
	extern uint8_t ups_mode;
	
	uint8_t led_point; //���� �� �������� � ������,����� ��� ��������� ���� �������,������ �������.
	if(mode>1) {led_point=0x40;} //����� ��� ����������
	else {led_point=0x20;};//����� ��� ����
	
	if(ups_mode==5){led_point=0x60;}; //����� � ����� ������, ������� ��� �������� ����� ������ ������� ���
	
	if (mode==2){mode=5;}//������� ������ �����������
	hexdec(system[mode],led_point);
	
}


void clear_RX()
{
	extern uint8_t RX_buff[];
	extern uint8_t RX_;
	RX_buff[0]=0; RX_=0;
	
}

