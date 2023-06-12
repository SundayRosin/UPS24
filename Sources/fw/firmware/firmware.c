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


uint8_t razrad=0; //номер разряда который нужно отображать
uint8_t led[3]={0,0,0};//буфер вывода в дисплей
uint8_t  a=0;
uint8_t v=0; //счетчик выборок  
volatile uint32_t summa=0; //cумма замеров АЦП
uint8_t kanal=0;//номер канала для текущего преобразования АЦП
uint16_t signals[]={0,0,0,0,0}; //буфер для хранения данных ацп
uint8_t tmp=0,tmp1=0; //временные переменные.
uint8_t b_timer=0; //интервал нажатия кнопки
uint8_t b_timer2=0; //счетчик длинного нажатия кнопки
uint8_t menu=0; //меню отображения информации на дисплее
uint16_t tmp16=0;//временная переменная
uint16_t system[]={0,0,0,0,0,0,0,0}; //напр БП,ток нагрузки, ток заряда, бат общ., бат 1
uint8_t calibr_cells[]={0,0,0,0,0}; //калибровочные значения для каждого из каналов
uint16_t u_param[]={0,0,0} ;// параметры напряжений 	
uint16_t BT1=0,BT2=0;//буфер значений 16битного ШИМ балансиров батареи 	 
uint8_t RX_buff[14]; //буфер приема данных USART
uint8_t RX_=0; //счетчик буфера
uint8_t TX_tmp,TX_tmp2; 
uint8_t TX_pointer=0;
uint8_t TX_buff[18];//16 посылка, два CRC16
uint8_t RTC_s=0; //счетчик секунд
uint8_t RTC_m=0; //счетчик минут
uint8_t RTC_h=0; //счетчик часов
uint8_t ups_mode=0;//режим работы бесперебойника
uint32_t C60=0;//емкость батареи за 60 минут 
uint8_t minut_flag=0; //флаг для функции замера емкости
uint8_t C=0; //реальная емкость которая получилась
uint16_t L[180]; //массив для сохранения параметров
uint8_t log_point=0;//номер текущей ячейки массива
uint8_t send_pointer=0; //используется для отправки данных



 
//RTC. Прерывание приходит каждую секунду
ISR(TIMER2_OVF_vect)
{
	//если вошли в режим запуска замера, счетчик был сброшен(RTC_s=0;) в button()
	if((RTC_s==10)&(menu>=5)) //жду 10сек
	{
		if (menu==6){menu=0; ups_mode=5;} //запуск режима замера емкости батареи
			else {menu=0;ups_mode=0;} //перевод  в обычный режим.
			 
	}
  //Часы реального времени
  if(RTC_s==59) //прошла минута
  {
	 minut_flag+=128; //говорю что прошла минута!	  
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
	
	
//Прерывание для динамической индикации 122гц
ISR(TIMER0_OVF_vect)
{
   //выводит led на дисплей
   dinamic(razrad);
   if(razrad!=2)razrad++;
	   else razrad=0;
   button();
    
}

//Обработчик прерыванию по приему USART
ISR(USART_RX_vect)
{
	if(RX_<13)
	{ 
	  RX_buff[RX_] =UDR0; //считали данные
	  //RX_buff[0]=UDR0; //считали данные
	 RX_++;
	}
	else
	{
		tmp=UDR0; //чтение буфера иначе прерывание будет бесконечным
	}
}

//прерывание по опустошению буфера
ISR(USART_UDRE_vect)
{	
	//посылка данных
	if (TX_pointer<18) //8*2byte
		{
			UDR0=TX_buff[TX_pointer]; //отправляем в передатчик
			TX_pointer++;
		}
		else //все данные переданы
		{
			TX_pointer=0;
			//запрещение прерывания по опустош буфера
			UCSR0B&=0b11011111; //UDRIE0=0;
		}
		
}

int main(void)
{ 
	wdt_reset(); //сброс watchdog таймера,фуз WDTON=0,стоит
	wdt_enable(WDTO_2S);//  2секунды если не сбросил вачдог,ресет
	//настройка перефирии
	init();
	asm("sei");
	boot_constanta(); //загрузка калибровочных констант
		 
	startADC(kanal);
  loop:
	 wdt_reset(); //сброс watchdog таймера
	  
	  //преобразование завершено
  	if((ADCSRA&0b01000000)==0)
	   {	   
		    tmp=ADCL;
			tmp1=ADCH; 		  		  
		   if(v<64)
		   {			    
			   //прибавляю к summa значения АЦП
			   asm_add(tmp1,tmp);
			   v++;
			  
		   }
		   else
		   {
			 // исправляю разрыв, каналы  7 6 5 1 0
			 //      signals[]            4 3 2 1 0
		     if (kanal>1)kanal-=3;			 
			 tmp16=asm_div(summa);// делю на 64
			 //сохранение значений с учетом калибровки
			 signals[kanal]=calibr(tmp16,calibr_cells[kanal]);
			 summa=0;
			 v=0;
			 if (kanal<4) {kanal++;}
				 else //все замеры выполнены
				 {
					 kanal=0;
					 //расчеты измерений
					 system[0]=asm_ui(signals[0],391);// ток нагрузки
					 system[1]=asm_ui(signals[1],391);// ток заряда батарей
					 system[2]=asm_ui(signals[2],375);// напряжение общее на батареях
					 system[3]=asm_ui(signals[3],375);// напряжение на батареи 1(нижней)
					 system[4]=asm_ui(signals[4],375);// напряжение БП
					 if (system[2]>system[3])
					 {system[5]=system[2]-system[3];} //напряжение верхней батареи
			         else {system[5]=0;} 
					//сам упс
					ups(RTC_m,RTC_s,system,&ups_mode);
					
				 }  //напр БП,ток нагрузки, ток заряда, бат общ., бат 1
			 // исправляю разрыв
			 if (kanal>1) {kanal+=3;} 
			 			   
		   }		
		  startADC(kanal);      
       }
	    	
	     
		  
		//работа с командами усарт
		switch(RX_buff[0])
		  {
		       case 'c':
			          send_system();
					//разрешение прерывания по опустошению буфера
			       clear_RX();
				    UCSR0B|=0b00100000;///UDRIE0=1;
			       
			   break;
			 
			  case 'L':
			        send_const();
			        //разрешение прерывания по опустошению буфера
			        clear_RX();
					UCSR0B|=0b00100000;///UDRIE0=1;
		      
			  case 'K':   if (RX_==12) //если весь пакет принят
			               {
							   write_calibr(); //запись данных в eeprom		   
							   clear_RX(); 
			               }
			    break;
			   //считывание результатов
			   case 'R':{
				          //отправляет данный
						  if(send_pointer<175){send_log();} 
							  else {send_pointer=0;send_log();}
						  clear_RX(); 
					      UCSR0B|=0b00100000;///UDRIE0=1;  
			            }  break;
						
			  default: clear_RX(); 	break;
		  }
				   
		//вывод на дисплей
		
		if (menu>=128) //преобразование пунктов меню в нормальный вид
		{tmp=menu&0b01111111; } else {tmp=menu;}
			
		if(tmp<5){
			disp_inf(tmp);
			
		} //обычный режим отображения
		else { //вошли в подменю включения/откл замера
			
			   //если tmp=5 отображает на дисплее 0С если tmp=6  1С
			  znak(tmp);
			  //если вместо znak(tmp); тут поставить то что она содержит увидим бегающие цифры
			  //странный глюк
			 }
				
	goto loop;
}