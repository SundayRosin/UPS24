using System;
using System.IO.Ports; //! нужно для Serial Read
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Graph
{
    public partial class Form1 : Form
    {
        byte[] data=new byte[18];//буфер приема данных 
        int k = 0; //счетчик 
        UInt16[] systems=new UInt16[9]; //хранит 16 битные значения чисел и crc
        Double[] systems2 = new Double[7]; //для отображения
        UInt16 t = 0;//подсчет времени 
        int mode = 0;
        int error=0; //Количество ошибок по приему

        public Form1()
        {
            InitializeComponent();
        }


        //подключить
        private void button1_Click(object sender, EventArgs e)
        {

            switch (button1.Text)
            {
                case "Подключить": button1.Text = "Отключить";
                                     //номер ком порта
                                     serialPort1.PortName = comboBox1.SelectedItem.ToString();
                                      serialPort1.Open(); //открыли порт
                                      serialPort1.Write("c"); //послали команду        
                break;

                case "Перезапуск": mode = 0;
                                   serialPort1.Write("c"); //послали команду
                                   button1.Text = "Отключить";
                break;

                case "Отключить": timer1.Enabled = false; //стоп таймер
                                 serialPort1.Close();
                                  button1.Text = "Подключить";
                break;
            }
                  
 
        }

              

        //событие по приходу пакета
        private void serialPort1_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            if (serialPort1.BytesToRead == 18) //если в буфере достаточно чисел
            {
                serialPort1.Read(data, 0, 18); //читаем данные
            }                          
           BeginInvoke(new Action(delegate() { timer1.Enabled = true; }));// запуская таймер 
         }

      

        //из двух байт собирает 16битное число
       void merger()
       {
           
           for (int i=0;i<8;i++)
           {
               systems[i] = 0;
               //младший байт
               systems[i] = Convert.ToUInt16(data[(i*2)+1]);
               systems[i]<<=8;
               systems[i]+=Convert.ToUInt16(data[i*2]);
           }

           //собираю контрольную сумму
           systems[8] = 0;
           systems[8] = Convert.ToUInt16(data[17]); //Нcrc
           systems[8] <<= 8;
           systems[8] += Convert.ToUInt16(data[16]);//Lcrc
          /*
           for (int i = 0; i < 16; i++)
           {
               data[i] = 0;
           }
          */ 
       }

       //добавляет точки в нужных местах, делит на 100 или 1000 
       //сохраняет в другой массив,который используется для отображения в форме
       void point_digit()
       {
           //для тока
           for (int i = 0; i < 2; i++)
           {
               if (systems[i] > 0)
               {
                   systems2[i] = Convert.ToDouble(systems[i]) / 1000;
               }
               else { systems2[i] = 0; }
           }
           //для напряжений
           for (int i = 2; i < 6; i++)
           {
               if (systems[i] > 0)
               {
                   systems2[i] = Convert.ToDouble(systems[i]) / 100;
               }
               else { systems2[i] = 0;}
          }  

       }

        //расчет контрольной суммы
       Int32 crc16()
       {
           merger(); //cобираю двухбайтное число в пакете
           Int32 crc;
           //вычисление контрольной суммы
           crc = 0xffff;
           int a, b;
           for (a = 0; a < 16; a++)
           {
               crc = crc ^ data[a];
               for (b = 0; b < 8; b++)
               {
                   if ((crc & 0x0001) != 0x0000) crc = (crc >> 1) ^ 0xA001;
                   else crc >>= 1;
               }
           }

           return crc;
       }

        //проверяет пакет на совпадение переданой и вычисленной контрольной суммы
        //возвращает 0-все хорошо, 1-ошибка.
       int error_test()
       {
           int x = 0;
           if (crc16() != systems[8])
           {
               if (error < 32765)
               {
                   error++;
                   label26.Text = error.ToString(); //вывожу количество ошибок
               }
               else
               {
                   label26.Text = "DEAD<|_|>";
               }
               x = 1;
               
           }
           return x;
       }


       //строит графики
       void do_graph()
       {
           point_digit(); //cтавит запятую
           label1.Text = systems2[0].ToString() + "A";
           label2.Text = systems2[1].ToString() + "A";
           label3.Text = systems2[2].ToString() + "V";
           label4.Text = systems2[3].ToString() + "V";
           label5.Text = systems2[4].ToString() + "V";
           label6.Text = systems2[5].ToString() + "V";
           label7.Text = systems[6].ToString(); //ocr1a
           label8.Text = systems[7].ToString(); //ocr1b
           
           //x,y coord
           //напряжения
           chart1.Series[0].Points.AddXY(t, systems2[2]);
           chart1.Series[1].Points.AddXY(t, systems2[3]);
           chart1.Series[2].Points.AddXY(t, systems2[4]);
           chart1.Series[3].Points.AddXY(t, systems2[5]);
           
           //токи
           chart2.Series[0].Points.AddXY(t, systems2[0]);
           chart2.Series[1].Points.AddXY(t, systems2[1]);

           //PWM
           //токи
           chart3.Series[0].Points.AddXY(t, systems[6]);
           chart3.Series[1].Points.AddXY(t, systems[7]);
          
       }

   //отображает калибровочные констаныт
       void view_c()
       {
           //калибровочные константы
           label9.Text = data[0].ToString();
           label10.Text = data[1].ToString();
           label11.Text = data[2].ToString();
           label12.Text = data[3].ToString();
           label13.Text = data[4].ToString();

           //параметры напряжений включения,откл,балансировки батарей
           label19.Text = Convert.ToString(((UInt16)data[5] << 8) + data[6]);
           label20.Text = Convert.ToString(((UInt16)data[7] << 8) + data[8]);
           label21.Text = Convert.ToString(((UInt16)data[9] << 8) + data[10]);

           //присваиваем значения текстовым полям
           //что бы при случайном нажатии "Записать" не случилась беда
           textBox6.Text = label19.Text;
           textBox7.Text = label20.Text;
           textBox8.Text = label21.Text;
           //емкость батареи
           label23.Text = Convert.ToString(Convert.ToDouble(data[11])/10)+"Aч";
           //системное время
           label24.Text =data[14].ToString()+":"+data[13].ToString()+":"+data[12].ToString();
           //перезапись реальных значений калибровочных констрант(+ - сколько) в текстовые поля ввода
           int tmp;
           for (int i = 0; i < 5; i++)
           {
               tmp = Convert.ToInt16(data[i]);
               if (tmp > 127) { tmp = 127 - tmp; } //привожу к человеческому виду
               switch (i)
               {
                   case 0: textBox5.Text = tmp.ToString(); break;
                   case 1: textBox4.Text = tmp.ToString(); break;
                   case 2: textBox3.Text = tmp.ToString(); break;
                   case 3: textBox2.Text = tmp.ToString(); break;
                   case 4: textBox1.Text = tmp.ToString(); break;

               }
           }


       
          

       }
   

 //*********************************************************************************      
       //обработчик таймера
       private void timer1_Tick(object sender, EventArgs e)
       {
           int rx_error = error_test();
                 //режим построения графика
                 if (mode == 0)
                 {
                     if (rx_error == 0) { do_graph(); } //cтрою графики
                     serialPort1.Write("c"); //послали команду  
                 }
                 else //режим чтения калибровочных констант
                 {
                     if (rx_error == 0) { view_c();}       
                     button1.Text = "Перезапуск";
                 }
           t++; //отсчет времени
           timer1.Enabled = false; //жду прихода пакета         
           
       }
//**************************************************************
       private void Form1_Load(object sender, EventArgs e)
       {
         //Графики напряжений
           //возможности масштабировать график
          // chart1.ChartAreas[0].AxisX.ScaleView.Zoom(0,360000);
           chart1.ChartAreas[0].CursorX.IsUserEnabled = true;
           //включение возможности выбора интервала для масштабирования
           chart1.ChartAreas[0].CursorX.IsUserSelectionEnabled = true;
           //включаем масштабирование по оси х
           chart1.ChartAreas[0].AxisX.ScaleView.Zoomable = true;
           //добавляю полосу прокрутки
           chart1.ChartAreas[0].AxisX.ScrollBar.IsPositionedInside = true;

           //Масштабирование по Y
           //возможности масштабировать график
           chart1.ChartAreas[0].AxisY.ScaleView.Zoom(0,30);
           chart1.ChartAreas[0].CursorY.IsUserEnabled = true;
           //включение возможности выбора интервала для масштабирования
           chart1.ChartAreas[0].CursorY.IsUserSelectionEnabled = true;
           //включаем масштабирование по оси х
           chart1.ChartAreas[0].AxisY.ScaleView.Zoomable = true;
           //добавляю полосу прокрутки
           chart1.ChartAreas[0].AxisY.ScrollBar.IsPositionedInside = true;
          
           //графики токов
           //возможности масштабировать график
          // chart2.ChartAreas[0].AxisX.ScaleView.Zoom(0, 360000);
           chart2.ChartAreas[0].CursorX.IsUserEnabled = true;
           //включение возможности выбора интервала для масштабирования
           chart2.ChartAreas[0].CursorX.IsUserSelectionEnabled = true;
           //включаем масштабирование по оси х
           chart2.ChartAreas[0].AxisX.ScaleView.Zoomable = true;
           //добавляю полосу прокрутки
           chart2.ChartAreas[0].AxisX.ScrollBar.IsPositionedInside = true;

           //Масштабирование по Y
           //возможности масштабировать график
           chart2.ChartAreas[0].AxisY.ScaleView.Zoom(0, 3);
           chart2.ChartAreas[0].CursorY.IsUserEnabled = true;
           //включение возможности выбора интервала для масштабирования
           chart2.ChartAreas[0].CursorY.IsUserSelectionEnabled = true;
           //включаем масштабирование по оси х
           chart2.ChartAreas[0].AxisY.ScaleView.Zoomable = true;
           //добавляю полосу прокрутки
           chart2.ChartAreas[0].AxisY.ScrollBar.IsPositionedInside = true;



           //макс и мин значения для графиков напряжений
           chart1.ChartAreas[0].AxisX.Minimum = 0;
           //chart1.ChartAreas[0].AxisX.Maximum = 30.00;
           chart1.ChartAreas[0].AxisY.Minimum = 0;
           chart1.ChartAreas[0].AxisY.Maximum = 30.00;

           //макс и мин значения для графиков токов
           chart2.ChartAreas[0].AxisX.Minimum = 0;
           //chart1.ChartAreas[0].AxisX.Maximum = 30.00;
           chart2.ChartAreas[0].AxisY.Minimum = 0;
           chart2.ChartAreas[0].AxisY.Maximum = 3.000;

           chart3.ChartAreas[0].AxisX.Minimum = 0;
       }


       private void label4_Click(object sender, EventArgs e)
       {

       }

       //читать калибровочные констранты
       private void button2_Click(object sender, EventArgs e)
       {
           mode = 1;
           serialPort1.Write("L"); //послали команду

       }

       //запись калибровочных констант
       private void button3_Click(object sender, EventArgs e)
       {
           this.Invoke(new Action(delegate() { timer1.Enabled = false; }));

           int []info=new int[8];
           byte[]send=new byte[14];//для передачи
           //считывание констант
           info[0] = Convert.ToInt16(textBox5.Text);
           info[1] = Convert.ToInt16(textBox4.Text);
           info[2] = Convert.ToInt16(textBox3.Text);
           info[3] = Convert.ToInt16(textBox2.Text);
           info[4] = Convert.ToInt16(textBox1.Text);
           //cчитывание напряжений
           info[5] = Convert.ToInt16(textBox6.Text);
           info[6] = Convert.ToInt16(textBox7.Text);
           info[7] = Convert.ToInt16(textBox8.Text);

           //готовим данные в нужном формате
           for (int i = 0; i < 5;i++)
           {
               if (info[i] < 0) //отрицательные числа
               {
                   send[i+1] = Convert.ToByte(Math.Abs(info[i]) + 127);
               }
               else //положительные числа
               {
                   send[i+1] = Convert.ToByte(info[i]);
               }
               
           }
           int k = 6;
           //разбивка на байты напряжений
           for (int i=5;i<8;i++)
           {                
                //старший
                send[k] = Convert.ToByte(info[i] >> 8);
                send[k+1] = Convert.ToByte(0x00ff & info[i]);
                k+=2;
           }

               mode = 1; //Жду посылки
         
           send[0] =Convert.ToByte('K');
           //отсылаю данные
           serialPort1.Write(send,0,12);
             

       }

       private void serialPort1_ErrorReceived(object sender, SerialErrorReceivedEventArgs e)
       {
           
       }

     

        
    }
}
