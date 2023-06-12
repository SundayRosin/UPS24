using System;
using System.IO.Ports;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using System.IO;


namespace Reader
{
    public partial class Form1 : Form
    {
        byte [] data=new byte[18];//буфер для чтения данных
        int t = 0; //номер элемента в буфере
        UInt16[] info_buff = new UInt16[180];
        UInt16[] d = new UInt16[7];
        Int32 crc; //хранит принятую контрольную сумму вычесления контрольной суммы
        int error = 0; //счетчик ошибок
        //StreamWriter myfile = new StreamWriter("data.log", true);

       
       
        
        public Form1()
        {
            InitializeComponent();
        }

        //кнопка Подключить
        private void button1_Click(object sender, EventArgs e)
        {
            if (button1.Text == "Подключить")
            {
                button1.Text = "Отключить";
                serialPort1.PortName = comboBox1.SelectedItem.ToString(); //номер выбранного порта
                serialPort1.Open();//открыл порт
            }
            else
            {
                serialPort1.Close();//закрыл порт
                button1.Text = "Подключить";
            }
        }

        //Кнопка Читать.
        private void button2_Click(object sender, EventArgs e)
        {          
            serialPort1.Write("R");
            //настройка графиков
            t = 0;

        }

        //расчет контрольной суммы
        Int32 crc16()
        {          
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
            if (crc16()!=d[6])
            {
                if (error < 32765)
                {
                    error++;
                    label3.Text = error.ToString(); //вывожу количество ошибок
                }
                else
                {
                    label3.Text = "DEAD<|_|>";
                }
                x = 1;

            }
            return x;
        }

        //собирает из пакета массив 16битных чисел(6шт) и дописывает их в основной буфер
        void converter()
        {   //Преобразование и Перезапись полученных чисел в буфер
            int k = 1;
            for (int i = 0; i < 6; i++)
            {
                //преобразование 8бит в 16
                d[i] = 0;
                d[i] = Convert.ToUInt16(data[k + 1]); //H
                d[i] <<= 8;
                d[i] += Convert.ToUInt16(data[k]);//L
                k += 2;
                //перезапись
                info_buff[t] = d[i];
                t++;
            }
             //собираем принятую контрольную сумму
            d[6] = 0;
            d[6] = Convert.ToUInt16(data[17]); //H
            d[6] <<= 8;
            d[6] += Convert.ToUInt16(data[16]);//L
           
        }



//***********************************************************************************************
        //обработчик события по приходу пакета
        private void serialPort1_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {

            if(serialPort1.BytesToRead==18) //если в буфере достаточно чисел
           {
               serialPort1.Read(data,0,18);//читаю данные 
                //отображаю номер массива 
               BeginInvoke(new Action(delegate() { button2.Text =data[0].ToString(); }));
               string s;
               converter(); //собирает числа в 16битные
    
                //Отображает ошибки если не верна crc
                BeginInvoke(new Action(delegate() { error_test(); }));
                                       
                //создание строки для записи в файл 
                s = data[0].ToString() + " ";
                 for (int i = 0; i < 6;i++ )
                 {
                     s += " " + d[i].ToString();
                 }

                //запись строки в файл            
                   using (StreamWriter myfile = new StreamWriter("data.log", true))
                   {

                       myfile.WriteLine(s);
                       myfile.Close();
                   }

                //если не все считано,отсылка устройству запроса на новое сообщение 
                   if (data[0] < 174)
                   {
                       serialPort1.Write("R");
                   }
                   else
                   {
                       button2.Invoke(new Action(delegate() { button2.Text ="Читать"; }));
                   }
           }
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            StreamWriter myfile = new StreamWriter("data.log", true);
            DateTime data = DateTime.Now; //системное время
            myfile.WriteLine("Program run at: "+data.ToString("g"));
            myfile.Close();
            //настройки графиков
            //макс и мин значения для графиков напряжений
            chart1.ChartAreas[0].AxisX.Minimum = 0;
            //chart1.ChartAreas[0].AxisX.Maximum = 30.00;
            chart1.ChartAreas[0].AxisY.Minimum = 0;
            chart1.ChartAreas[0].AxisY.Maximum = 30.00;

            //макс и мин значения для графиков токов
            chart2.ChartAreas[0].AxisX.Minimum = 0;
            chart2.ChartAreas[0].AxisY.Minimum = 0;
            chart2.ChartAreas[0].AxisY.Maximum = 3.000;

            // *******************************Графики напряжений
            //возможности масштабировать график
              chart1.ChartAreas[0].AxisX.ScaleView.Zoom(0,60);
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

            // *******************************График тока
            //возможности масштабировать график
            chart2.ChartAreas[0].AxisX.ScaleView.Zoom(0,60);
            chart2.ChartAreas[0].CursorX.IsUserEnabled = true;
            //включение возможности выбора интервала для масштабирования
            chart2.ChartAreas[0].CursorX.IsUserSelectionEnabled = true;
            //включаем масштабирование по оси х
            chart2.ChartAreas[0].AxisX.ScaleView.Zoomable = true;
            //добавляю полосу прокрутки
            chart2.ChartAreas[0].AxisX.ScrollBar.IsPositionedInside = true;

            //Масштабирование по Y
            //возможности масштабировать график
            chart2.ChartAreas[0].AxisY.ScaleView.Zoom(0,3);
            chart2.ChartAreas[0].CursorY.IsUserEnabled = true;
            //включение возможности выбора интервала для масштабирования
            chart2.ChartAreas[0].CursorY.IsUserSelectionEnabled = true;
            //включаем масштабирование по оси х
            chart2.ChartAreas[0].AxisY.ScaleView.Zoomable = true;
            //добавляю полосу прокрутки
            chart2.ChartAreas[0].AxisY.ScrollBar.IsPositionedInside = true;
          
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            StreamWriter myfile = new StreamWriter("data.log", true);
            DateTime data = DateTime.Now; //системное время
            myfile.WriteLine("Program close at: " + data.ToString("g"));
            myfile.Close();

        }

        //Строит графики на основе полученных данных
        private void button3_Click(object sender, EventArgs e)
        {
            int c=0;
            for (t = 0; t < 180; t++)
            {

                switch (c)
                {
                    case 0: chart1.Series[0].Points.AddXY(t/3, Convert.ToDouble(info_buff[t])/100); c++; break; //u1
                    case 1: chart1.Series[1].Points.AddXY(t / 3, Convert.ToDouble(info_buff[t]) / 100); c++; break;
                    case 2: chart2.Series[0].Points.AddXY(t / 3, Convert.ToDouble(info_buff[t]) / 1000); c++; break;

                }

                if (c == 3) { c = 0; }
            }



        }
 

        
    }
}
