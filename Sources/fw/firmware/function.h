/*
 * function.h
 *
 * 
 *  Author: Valek
 */ 
#include <avr/io.h>

//инициализаци€ переферии
void init(void);

//динамическа€ индикаци€
void dinamic(uint8_t pos);

//вывод данных в регистр
//void reg(uint8_t b);

//запуск преобразовани€ ацп
void startADC(uint8_t channel);

/* прибавл€ет к 32 битному числу ,16 битное
  состо€щее из двух 8 битных,
 используетс€ дл€ получени€ суммы выборок ј÷ѕ
*/

//преобразование двоичного числ в дес массив
//point- отображать точку в каком разр€де 1-3 LH-сотни тыс€чи в последнем разр€де 
void hexdec(uint16_t hex,uint8_t point); 


//опрос кнопки, переключение меню 
void button();

//прибавл€ет или отымает значение, используетс€ дл€ калибровки
uint16_t calibr(uint16_t adc,uint8_t val);


//‘ункци€ чтени€ из EEPROM
// задаем адресс €чейки в условном фотмате, получаем значение в этой €чейке.
uint8_t ee_read (uint8_t addr);

//‘ункци€ записи в €чейку с проверкой содержимого в €чейке на предмет равенства
// того что находитс€ в €чейке,если значение одинаково-тогда не производим запись
void ee_write (uint8_t addr,uint8_t data);

//считывает из пам€ти системные константы(калибровка, другие значени€)
void boot_constanta(void);

//балансировка батарей
void balansir(void); 

//отправл€ет системные данные в буфер передатчика
void send_system(void);

//отсылает системные констранты в буфер передатчика
void send_const();

//записывает калибровочные константы
void write_calibr(void);

//выполн€ет все функции бесперебойника
void ups(uint8_t m,uint8_t s,uint16_t *sys, uint8_t *mode);

void clear_RX();
//выполн€ет подсчет емкости батареи и фиксацию параметров системы
//void cell_capacity(void);

//отправл€ет данный
void send_log();

//функци€ вычислени€ crc16(полином как в modbus),crc возвращает в буфер передачи
void crc16(uint8_t *buf,uint8_t sizeBuf);

//устанвливает на дисплее определенные символы
void znak(uint8_t x);

//ставит точку там где нужно
//uint8_t led_point();

void disp_inf(uint8_t mode);

//ассемблерные функции
// прибавл€ет к summa значени€
 inline static void asm_add(uint8_t ah,uint8_t al);
//деление на 64
 inline static uint16_t asm_div();

//вычисл€ет значение тока или напр€жени€
// C=(adc*koef)/128
uint16_t asm_ui(uint16_t adc,uint16_t koef);

void asm_addA(uint16_t abb);

uint8_t asm_divB();
