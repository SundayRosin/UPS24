/*
 * function.h
 *
 * 
 *  Author: Valek
 */ 
#include <avr/io.h>

//������������� ���������
void init(void);

//������������ ���������
void dinamic(uint8_t pos);

//����� ������ � �������
//void reg(uint8_t b);

//������ �������������� ���
void startADC(uint8_t channel);

/* ���������� � 32 ������� ����� ,16 ������
  ��������� �� ���� 8 ������,
 ������������ ��� ��������� ����� ������� ���
*/

//�������������� ��������� ���� � ��� ������
//point- ���������� ����� � ����� ������� 1-3 LH-����� ������ � ��������� ������� 
void hexdec(uint16_t hex,uint8_t point); 


//����� ������, ������������ ���� 
void button();

//���������� ��� ������� ��������, ������������ ��� ����������
uint16_t calibr(uint16_t adc,uint8_t val);


//������� ������ �� EEPROM
// ������ ������ ������ � �������� �������, �������� �������� � ���� ������.
uint8_t ee_read (uint8_t addr);

//������� ������ � ������ � ��������� ����������� � ������ �� ������� ���������
// ���� ��� ��������� � ������,���� �������� ���������-����� �� ���������� ������
void ee_write (uint8_t addr,uint8_t data);

//��������� �� ������ ��������� ���������(����������, ������ ��������)
void boot_constanta(void);

//������������ �������
void balansir(void); 

//���������� ��������� ������ � ����� �����������
void send_system(void);

//�������� ��������� ���������� � ����� �����������
void send_const();

//���������� ������������� ���������
void write_calibr(void);

//��������� ��� ������� ��������������
void ups(uint8_t m,uint8_t s,uint16_t *sys, uint8_t *mode);

void clear_RX();
//��������� ������� ������� ������� � �������� ���������� �������
//void cell_capacity(void);

//���������� ������
void send_log();

//������� ���������� crc16(������� ��� � modbus),crc ���������� � ����� ��������
void crc16(uint8_t *buf,uint8_t sizeBuf);

//������������ �� ������� ������������ �������
void znak(uint8_t x);

//������ ����� ��� ��� �����
//uint8_t led_point();

void disp_inf(uint8_t mode);

//������������ �������
// ���������� � summa ��������
 inline static void asm_add(uint8_t ah,uint8_t al);
//������� �� 64
 inline static uint16_t asm_div();

//��������� �������� ���� ��� ����������
// C=(adc*koef)/128
uint16_t asm_ui(uint16_t adc,uint16_t koef);

void asm_addA(uint16_t abb);

uint8_t asm_divB();
