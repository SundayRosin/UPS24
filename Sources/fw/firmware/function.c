/*
 * function.c
 *
 * 
 *  Author: Valek
 */ 

#define  F_CPU 8000000UL
#include <util/delay.h>
#include <avr/io.h>
#include <avr/pgmspace.h> // для того что бы масив был в flash памяти 

//знакогенератор для индикатора
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
	0b00111001, //С 14
	0b00100110, //d 15
	0b00110001, //e 16
	0b01110001, //f 17
	0b00111011, //L 18
	0b01111001, //Г 19
	0b01100010 //?  20
};


extern uint16_t system[];
extern uint8_t TX_tmp,TX_tmp2;
extern uint8_t TX_pointer;
extern uint8_t TX_buff[];


//инициализация переферии
void init(void)
{
	/*
	PD2-SHCP,PD3-STCP,PD4 - DS управление регистром 74hc595
	PD5-AN3,PD6-AN2,PD7-AN1
	*/
    DDRD=0b11111100;
	DDRB=0b00000110; //PB1 PB2 outputs PWM
	PORTB=0b00000001; //PB0 подтягиваю на + через резистор
	//управление ИБП
	DDRC=0b00011100;
	
	//инициализация  TC0 для динамической индикации
	//8 000 000 /256/256=122/3 =40HZ
	TCCR0B=0b00000100; //clkI/O/256 (from prescaler)
	TIMSK0=1; //enable interrupt
	
   //отключение цифровых входов
   DIDR0=0b11100011; //ADC 7,6,5,1,0  
 
   //инициализируем 16bit TC1
   TCCR1A=0b10100010; //Clear OC1A/OC1B on compare match  FAST PWM
   TCCR1B=0b00011001;// clc/1
   //TOP -ICR1
   ICR1=0xffff;
  // OCR1A=0;
   //OCR1B=0;
  
   //Инициализируем таймер,используемый как часы RTC
	//включаю делитель на 128
	TCCR2B=0b00000101;
	//переключаю таймер в асинхронный режим AS2=1
	ASSR=0b00100000;
	//жду завершения инициализации таймера
	//Wait for TCN2UB, OCR2xUB, and TCR2xUB.
	TCNT2=1;
	while((ASSR&0b00011111)!=0);
    TIMSK2=0b00000001; //разрешаю прерывание
   
      //инициализация USART
      //#define FOSC 8000000 // Clock Speed
      //#define BAUD 9600   //скорость передачи
      //#define MYUBRR (FOSC/16*BAUD)-1
     //Set baud rate 51
     UBRR0H =0;// (uint8_t)(MYUBRR>>8);
     UBRR0L =0x33;// (uint8_t)MYUBRR;
     //Включаем прием,передачу и прерывания
     UCSR0B=0b10011000;//RXCIE0,TXCIE0=0,RXEN0,TXEN0;
     /* Set frame format: 8data, 2stop bit */
     //асинхронный режим,без четности 1стоп,8бит
     UCSR0C =0b00000110;
 }

//вывод данных в регистр
void reg(uint8_t b)
{
	for(int i=0;i<8;i++)
	{
		//установка бита на DS // PD2-SHCP,PD3-STCP,PD2-SHCP,PD3-STCP,
		if ((b&0b10000000)==0b10000000)
		{
			PORTD|=0b00010000;//PD4 - DS
		}
		else
		{
			PORTD&=0b11101111;//PD4 - DS
		}
		//стробирую PD2-SHCP
		PORTD|=0b00000100;
		PORTD&=0b11111011;
		b=b<<1;
	}
	//установка на выводах
	//PD3-STCP
	PORTD|=0b00001000;
	PORTD&=0b11110111;
	
}

//динамическая индикация PD5-AN3,PD6-AN2,PD7-AN1
void dinamic(uint8_t pos)
{
	extern uint8_t led[3];
	uint8_t point,tmp;
	 //извлекаем точку 
	  point=led[0];
	  point&=0b01100000; 
	  led[0]=led[0]&0b00011111; //возвращаю как было	
	
	PORTD&=0b00011111; //потушить все
	//if (led[pos]<19) //защита от недопустимых символов
    //{tmp=pgm_read_byte(&zgen[led[pos]]);}// читаю код из знакогенератора
	//else
	//{tmp=pgm_read_byte(&zgen[10]);} //потушить
	tmp=pgm_read_byte(&zgen[led[pos]]);// читаю код из знакогенератора	
    point>>=5;//привращаю в нормальный вид 1 2 3
	if((pos+1)==point)
	{tmp&=0b11011111;} //включаю точку
				
	reg(tmp); //вывод данных в регистр
//включаю разряд
switch(pos)
		{
			
			case 0:  PORTD|=0b10000000; break;	//1
			case 1:  PORTD|=0b01000000; break;	//2
			case 2:  PORTD|=0b00100000; break;	//3
						
		}
	
  }


//запуск преобразования ацп
void startADC(uint8_t channel)
{
    ADMUX=0;	
	//VCC ref voltage ADLAR=0
	ADMUX=0b01000000+channel;
	ADCSRA=0b11000110;  //64 125кгц тактовая частота
	
}

//преобразование двоичного числ в дес 
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
	   
	   
		led[0]=dec[0]+point; //передача точки
		led[1]=dec[1];
		led[2]=dec[2];
	}
	
	//Функция чтения из EEPROM
	// задаем адресс ячейки в условном фотмате, получаем значение в этой ячейке.
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


	//Функция записи в ячейку с проверкой содержимого в ячейке на предмет равенства того что записано туда уже.
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
			asm volatile ("sbi 0x1F,2  ");  // Иначе оно не успевает за 4 такта выполнить команду!!!!!! без включенной оптимизации
			/* Start eeprom write by setting EEPE */
			//EECR |= (1<<EEPE);
			asm volatile ("sbi 0x1F,1");
			asm volatile("sei");

		}
	}
	
	
	
//опрос кнопки, переключение меню	
void button()
{
	extern uint8_t b_timer,b_timer2,menu;
	extern uint8_t RTC_s;
	
		 //опрос кнопки
	 if((PINB&0b00000001)==0) //кнопка нажата
	 {
		 //кнопка удерживается 122/24=5hz. 1/5=0.2c
		 if(b_timer==24)
		 {
			 //кнопка была нажата
			 
			 //Обычный режим:  меняем меню,если кнопку отпускали
			 if((menu&128)==0)
			 { 				  	   
				     //переключение меню по кругу
				     if (menu<4) //5 пунктов меню
				     {menu+=128+1;}
				     else 
				     {
				        if (menu<5){menu=128;} //cброс пункта меню 
					     else //переключение подменю(получено при длинном нажатии)
					    {
							RTC_s=0; //сброс счетчика секунд
						   //переключение подменю(5,6) по кругу
						   if (menu<6) {menu+=128+1;}
							else {menu=5+128;}
				     	}  
				     }
				 
			 } //++меню и добавляем флаг(128) что меню сменили
				 
			    //проверка длинного(2.12c) нажатия на кнопку
			     if(b_timer2==255)
			      {
				     //было длинное нажатие
					   menu=5; //зашел в подменю
					  //menu&=0b01111111; 
					    b_timer=0; b_timer2=0; menu+=128;
			      }
			     else
			      { b_timer2++; }
		   
		 }
		 else
		 { b_timer++;}
	 }
	 else {b_timer=0;b_timer2=0; menu&=0b01111111; } //кнопку отпустили
}	


//считывает из памяти системные константы(калибровка, другие значения)
void boot_constanta()//uint8_t *sys_const,uint16_t *u_param)
{	
	extern uint8_t calibr_cells[];
	extern uint16_t u_param[];
	//читаю из EEPROM калибровочные константы
	for (uint8_t i=0;i<5;i++)   //константы для калибровки каналов
	{   		
		calibr_cells[i]=ee_read(i);
		//если память чистая
	    if (calibr_cells[i]==0xFF) {calibr_cells[i]=0;}
	}
     uint8_t tmpH,tmpL,k=0;
	//читаю из еепром значения напряжений 5H6L,7H8L,9H10L
	//пороги балансиров и откл нагр
	for(uint8_t i=5;i<11;i+=2)
	{
		tmpH=ee_read(i);
		tmpL=ee_read(i+1);
		//собираем int16_t из двух int8_t. "С" очень криво делает эту операцию
		asm volatile(   "clr %A[RES]"	"\n\t"
		                "clr %B[RES]"	"\n\t" 
		                "mov %A[RES],%[Low]" "\n\t"
						"mov %B[RES],%[Hi]" "\n\t"
		               :[RES]"=&d"(u_param[k]):[Hi]"r"(tmpH),[Low]"r"(tmpL)  
		);
        k++;	
	}
	//на случай чистых ячеек
	for (uint8_t i=0;i<3;i++)
	{
		if (u_param[i]>3000)
		{
			switch(i)
			{
				case 0: u_param[i]=1400; break; //напряжение вкл балансиров
				case 1: u_param[i]=1300; break; //напряжение откл бал
				case 2: u_param[i]=1080; break; //напряж откл от нагрузки
				default: break;
			}
		}
	}
 		
}



//прибавляет или отымает значение, используется для калибровки
uint16_t calibr(uint16_t adc,uint8_t val)
{
	if (val <=127) //прибавляю
	{
		adc+=val;		
	}
	else //отнимаю
	{
	   val-=127; //-10
	   if (adc>val){adc-=val;} 	
	}
   return adc;	
}


//балансировка батарей
void balansir(void)
{
	extern uint8_t ups_mode;
	extern uint16_t system[];
	extern uint16_t BT1,BT2;
	extern uint16_t u_param[];
	uint16_t raznost;
	
	//system[5] напряжение на верхней батареи
	//system[3] напряжение на нижней батареи
	//напряжение на нижней батареи больше чем на верхней
	if (system[3]>system[5])
	{
		raznost=system[3]-system[5]; //вычисляю ошибку
		
		//если ошибка больше порога  
		if((raznost>15)&(BT2<0xffff))
		{
		  BT2++;	
		}
         //ошибка ниже порога 
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
		
	//напряжение на верхней батареи больше чем на нижней
	if (system[5]>system[3])
	{
		raznost=system[5]-system[3];
		
	    //если ошибка больше порога  
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
	system[6]=BT1; //верхняя батарея
	OCR1B=BT2;
	system[7]=BT2; //нижняя батарея
	
}


//функция вычисления crc16(как в modbus)
//аргументы: указатель на буфер,
//           размер буфера,
//
//возвращает:подсчитанное значение СRC16, ложит его в  буфер УАРТ
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
	
	//добавляю контрольную сумму в пакет
	//младший байт числа uint16_t
	asm volatile (
	"mov %[RES],%A[D16]"	"\n\t"
	:[RES]"=&d"(TX_buff[16]):[D16]"r"(crc)
	);
	
	//старший
	asm volatile (
	"mov %[RES],%B[D16]"	"\n\t"
	:[RES]"=&d"(TX_buff[17]):[D16]"r"(crc)
	);
	
	
}


//используется для посылки системных данных(6 измерений)
void send_system()
{		
	uint8_t k;
	for(uint8_t i=0;i<16;i++) //количество бит для пересылки
	{
		k=i/2;
		//TX_tmp2=TX_pointer/2;
		//младший байт числа
		if((i%2)==0)
		{
			asm volatile (
			"mov %[RES],%A[D16]"	"\n\t"
			:[RES]"=&d"(TX_buff[i]):[D16]"r"(system[k])
			);
		}
		else //старший
		{
			asm volatile (
			"mov %[RES],%B[D16]"	"\n\t"
			:[RES]"=&d"(TX_buff[i]):[D16]"r"(system[k])
			);
		}
			
	}
	
	//расчет контрольной суммы
	crc16(TX_buff,16);	
}

//отсылает калибровочные констранты в буфер передатчика
void send_const()
{
	extern uint8_t calibr_cells[]; //5штук
	extern uint16_t u_param[];
	extern uint8_t RTC_s;
	extern uint8_t RTC_m;
	extern uint8_t RTC_h;
	//калибровочные константы каналов измерений
	for (uint8_t i=0;i<5;i++)
	{
	   TX_buff[i]=calibr_cells[i];
	}
	
	uint8_t k=5; 
	//параметры напряжения 5:6 7:8 9:10
	for (uint8_t i=0;i<3;i++)
	{
		
		//разбиваю uint16_t на два uint8_t 
		asm volatile(  "mov %[Low],%A[X]" "\n\t"
		               "mov %[Hi],%B[X]" "\n\t"
		:[Hi]"=&d"(TX_buff[k]),[Low]"=&d"(TX_buff[k+1]):[X]"r"(u_param[i])
		);		
		k+=2;
	}				
				 
	TX_buff[11]=ee_read(11); //емкость батареи
	TX_buff[12]=RTC_s; //сек
	TX_buff[13]=RTC_m; //мин
	TX_buff[14]=RTC_h; //часы
	TX_buff[15]=0;

	//расчет контрольной суммы
	crc16(TX_buff,16);		
}

//записывает калибровочные константы
void write_calibr(void)
{
	
	extern uint8_t RX_buff[]; //буфер приема данных USART
	extern uint8_t RX_; //счетчик буфера
	extern uint8_t calibr_cells[]; //5штук
	extern uint16_t u_param[];
	  for(uint8_t i=0;i<5;i++)
		{
			calibr_cells[i]=RX_buff[i+1];
			RX_buff[i+1]=0;
		   	
		}
		
	 //переписываем в EEPROM калибровочные константы
	    for (uint8_t i=0;i<5;i++)
	     {
		 ee_write(i,calibr_cells[i]);
	     }
	
		 
	//запись напряжений	
	 uint8_t k=6;	
	 for(uint8_t i=0;i<3;i++)
	 {
		 	//собираем int16_t из двух int8_t. "С" очень криво делает эту операцию
		 	asm volatile(   "clr %A[RES]"	"\n\t"
		 	"clr %B[RES]"	"\n\t"
		 	"mov %A[RES],%[Low]" "\n\t"
		 	"mov %B[RES],%[Hi]" "\n\t"
		 	:[RES]"=&d"(u_param[i]):[Hi]"r"(RX_buff[k]),[Low]"r"(RX_buff[k+1])
		 	);
	       k+=2;
	 }	
	 
	 //перезапись в еепром напряжений
	  for (uint8_t i=5;i<11;i++)
	  {
		  ee_write(i,RX_buff[i+1]);
	  }
	  RX_=0;
		
		
}


//uint8_t *minut_flag,uint8_t *mode,uint32_t *Sbuff,uint8_t *C,uint16_t *sys);
//выполняет подсчет емкости батареи
void cell_capacity(void)
{
	extern uint8_t minut_flag,C;
	extern uint32_t C60;//емкость батареи за 60 минут 
	extern uint16_t system[];
	extern uint16_t L[];
	extern uint8_t log_point;
	
	//прошла минута и mode=5
	if((minut_flag&0b10000000)==128)//&(*mode==5))
	{
		minut_flag&=0b01111111; //удаляем флаг
	    
		//каждые 10 минут сохраняю показания в ОЗУ
		if ((minut_flag%10)==0)  
		{
			//Если массив заполнен не весь
			if (log_point<180)
			{  
				L[log_point]=system[3];  //Б1
				log_point++;
				L[log_point]=system[5]; //Б2
				log_point++;
				L[log_point]=system[0]; //I нагрузки
			    log_point++;
			}
						
		}
	    
		//подсчет емкости батареи
	    if (minut_flag<59)	
		{
		   asm_addA(system[0]); //cуммирует ток нагрузки за 60мин		   	     
		   minut_flag++;
		}
		else
		{				  
		  	
		   C+=asm_divB(); //вычисляет емкость С60/6000
		   C60=0;
		   minut_flag=0;
		}
	}
	
	
}

//вычесляет остаточную емкость батареи, и записывает ее в память
void write_cap()
{
	extern uint8_t C;
	extern uint32_t C60;
	C+=asm_divB(); //вычисляет емкость С60/6000
	C60=0;
	
	ee_write(11,C);//записываю емкость
	C=0;	
}

//выключает балансир
void clear_balansir()
{
	extern uint16_t system[];
	OCR1A=0;
	system[6]=0; //верхняя батарея
	OCR1B=0;
	system[7]=0; //нижняя батарея
}

//выполняет все функции бесперебойника
void ups(uint8_t m,uint8_t s,uint16_t *sys,uint8_t *mode)
{	
	extern uint8_t C;
	extern uint32_t C60;
	extern uint16_t u_param[];
	
	// u_param[0]=1360; break; //верхий порог заряда
	// u_param[1]=1350; break; //нижний порог,вкл заряд
	// u_param[2]=1080; break; //напряж откл от нагрузки
	if (sys[4]>2400) //напряжение есть в сети?
	{ 		
		//тоолько включились?прошло 3 сек? включаем заряд
		if ((*mode==0)&(s>3)){PORTC|=0b000000100; *mode=1;C=0;C60=0;}; //вкл заряда  сброс счетчиков емкости бат
		//прошло 2 минуты подкл нагрузку к БП и вкл полевик	
		if ((*mode==1)&(m>1)){PORTC|=0b00011000; *mode=2;}; //вкл заряда 
		
		if ((*mode>0)&(*mode<3)){balansir();} //балансир батарей
		else {clear_balansir();} //выкл
		//зарядит полностью банки перейдет в режим 3-ждущий буферный.
		if((*mode==2)&((sys[3]>u_param[0])&(sys[5]>u_param[0])))
		{
			*mode=3;
			PORTC&=0b11111011; //отключить заряд
		}
		
		//включить заряд
		if ((*mode==3)&((sys[3]<u_param[1])&(sys[5]<u_param[1])))
		{
			*mode=2; PORTC|=0b00000100; 
		}
		
		//упало ниже нормы опять в режим 2	
		//режим замера емкости батареи
		if(*mode==5) 
		{			
			PORTC&=0b11110011; //отключаю заряд и нагрузку от БП, перехожу на работу от батареи
			cell_capacity();//замер емкости батареи
			//батареи разряжены!!!
		    if ((sys[3]<u_param[2])|(sys[5]<u_param[2]))  
			{
				write_cap(); //пишу емкость батарей
				PORTC|=0b00011100;//вкл заряда и подкл нагр к БП 
			    *mode=1;//возвращаю режим работы
			}
		
		}
	}
	else //нет напр в сети
	{ 
		clear_balansir(); //выкл балансиров
		*mode=0; //позволяет быстро реагировать на появления напр
	   //отключить заряд батарей и нагрузку от БП.
	   PORTC&=0b11110011;	
	   cell_capacity(); //замер емкости батарей
	  //полное отключение устройств при снижение напряжения на одной из бат.
	  if ((sys[3]<u_param[2])|(sys[5]<u_param[2]))  
	  {
		   //нужно для того что бы не писал в еепром при откл питания.
		   //емкость питания мк разряжается пожже чем в БП, успевает записать нуль.
		 for(uint8_t Q=0;Q<10;Q++)
		 { _delay_ms(3000);}
		 
		  write_cap(); //сохраняю емкость батареи в память.
		  PORTC&=0b11100011; //выключаю нагрузку(полевик)
		  //устраняет эффект включения/выключения нагрузки, пока разряжается
		  //конденсатор микроконтроллерной платы    
		   while(sys[4]<2400);//жду электричества,а вдруг мгновенно появится
	  }
	    		
	}
		
	
	
}



//отправляет данный
void send_log()
{
	extern uint8_t send_pointer;	
	extern uint16_t L[];
	//формирую пакет
	TX_buff[0]=send_pointer; //№
	
	for(uint8_t i=1;i<13;i+=2) //количество бит для пересылки
	{
		    //младший байт числа uint16_t
			asm volatile (
			"mov %[RES],%A[D16]"	"\n\t"
			:[RES]"=&d"(TX_buff[i]):[D16]"r"(L[send_pointer])
			);
		
		   //старший
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
	//расчет контрольной суммы
	crc16(TX_buff,16);
	
	
}

void znak(uint8_t x)
{
	extern uint8_t led[3];
	
	if (x==5)  //показываю нуль-замер емкости откл
	{
		
		//разряды вид на экран "0" "1" "2"
		led[0]=0;//режим откл
		led[1]=14;//C  14
		led[2]=10;//путушил все сегм
		
	}
	else //menu=6
	{
		//led[0]=1; //вместо единицы отображает 5. Чюдеса. возможно компелятор переоптимизировал код)
		//tmp=1;
		led[0]=1; //вот так присваиваю 1))
		led[1]=14;//  C
		led[2]=10; //откл все сегменты
	}
	
}

void disp_inf(uint8_t mode )
{
	extern uint16_t system[];
	extern uint8_t ups_mode;
	
	uint8_t led_point; //если ее объявить в начале,имеем при показании тока единицу,вместо запятой.
	if(mode>1) {led_point=0x40;} //точка для напряжений
	else {led_point=0x20;};//точка для тока
	
	if(ups_mode==5){led_point=0x60;}; //точка в самом начале, говорит что работает режим замера емкости бат
	
	if (mode==2){mode=5;}//изменяю формат отображения
	hexdec(system[mode],led_point);
	
}


void clear_RX()
{
	extern uint8_t RX_buff[];
	extern uint8_t RX_;
	RX_buff[0]=0; RX_=0;
	
}

