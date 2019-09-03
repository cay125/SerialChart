#include "serialport.h"
#include "status.h"
SerialPort::SerialPort(QObject *parent) : QObject(parent)
{
    my_thread = new QThread();
    port = new QSerialPort();
//    init_port();
    this->moveToThread(my_thread);
    port->moveToThread(my_thread);
    my_thread->start();  //启动线程
    buildTableCRC16();
}

SerialPort::~SerialPort()
{
    port->close();
    port->deleteLater();
    my_thread->quit();
    my_thread->wait();
    my_thread->deleteLater();
}

void SerialPort::init_port()
{
//    port->setPortName("/dev/ttyUSB0");                 //串口名 windows下写作COM1
//    port->setBaudRate(115200);                         //波特率
    port->setDataBits(QSerialPort::Data8);             //数据位
    port->setStopBits(QSerialPort::OneStop);           //停止位
    port->setParity(QSerialPort::NoParity);            //奇偶校验
    port->setFlowControl(QSerialPort::NoFlowControl);  //流控制
//    if (port->open(QIODevice::ReadWrite))
//    {
//        qDebug() << "Port have been opened";
//        emit connected();
//    }
//    else
//    {
//        qDebug() << "open it failed";
//    }
//    connect(port, SIGNAL(readyRead()), this, SLOT(handle_data())); //Qt::DirectConnection
}
void SerialPort::start_port(QString portname, int baudrate)
{
//    my_thread = new QThread();
//    port = new QSerialPort();
    init_port();
    port->setPortName(portname);
    port->setBaudRate(baudrate);
    if (port->open(QIODevice::ReadWrite))
    {
        qDebug() << "Port have been opened";
        connect(port, SIGNAL(readyRead()), this, SLOT(handle_data())); //Qt::DirectConnection
//        this->moveToThread(my_thread);
//        port->moveToThread(my_thread);
//        my_thread->start();  //启动线程
        emit connected();
    }
    else
    {
        qDebug() << "open it failed";
    }
}
void SerialPort::stop_port()
{
    port->close();
//    port->deleteLater();
//    my_thread->quit();
//    my_thread->wait();
//    my_thread->deleteLater();
//    delete port;
    qDebug() << "port have been closed";
}
uint32_t SerialPort::crc_check(uint8_t* data, uint32_t length)
{
    unsigned short crc_reg = 0xFFFF;
    while (length--)
        crc_reg = (crc_reg >> 8) ^ crc16_table[(crc_reg ^ *data++) & 0xff];
    return (uint32_t)(~crc_reg) & 0x0000FFFF;
}
void SerialPort::buildTableCRC16()
{
    uint16_t i16 , j16;
    uint16_t data16;
    uint16_t accum16;
    for(i16=0;i16<256;i16++)
    {
      data16 = (uint16_t)(i16<<8);
        accum16 = 0;
        for(j16=0;j16<8;j16++)
        {
          if((data16^accum16)&0x8000)
            {
              accum16 = (accum16<<1)^0x1021;
            }
            else
            {
              accum16<<=1;
            }
            data16 <<= 1;
        }
        tableCRC16[i16] = accum16;
    }
}
uint16_t SerialPort::calcCRC16()
{
    uint16_t crc16 = 0;
    for(int i=0;i<35;i++)
    {
        if(i==0)
            crc16 = (crc16<<8)^tableCRC16[(crc16>>8)^(0x23)];
        else if(i==1)
            crc16 = (crc16<<8)^tableCRC16[(crc16>>8)^(0x01)];
        else
            crc16 = (crc16<<8)^tableCRC16[(crc16>>8)^((uint8_t)pointData.at(i-2))];
    }
    return crc16;
}

void SerialPort::handle_data()
{
    static int state=0;
    QByteArray data = port->readAll();
    for(int i=0;i<data.size();i++)
    {
        if( state==0 && static_cast<unsigned char>(data.at(i))==0xeb )
        {
            pointData.clear();
            state++;
        }
        else if(state==1)
        {
            if(static_cast<unsigned char>(data.at(i))==0x90)
                state++;
            else
                state=0;
        }
        else if(state==2)
        {
            if(static_cast<unsigned char>(data.at(i))==0x23)
                state++;
            else
                state=0;
        }
        else if(state==3)
        {
            if(static_cast<unsigned char>(data.at(i))==0x01)
                state++;
            else
                state=0;
        }
        else if(state>=4)
        {
            pointData.append(data.at(i));
            state++;
            if(state>=39)
            {
                state=0;
                uint16_t crc_res = calcCRC16();
                if((uint8_t)(pointData.at(33))==(uint8_t)(crc_res&0xff) && (uint8_t)(pointData.at(34))==(uint8_t)((crc_res>>8)&0xff))
                    emit receive_data(pointData);
            }
        }
    }
    //qDebug() << QStringLiteral("data received(收到的数据):") << data.toHex();
//    qDebug() << "handing thread is:" << QThread::currentThreadId();
//    qDebug() << "state is: "<< state;
    //emit receive_data(data);
}

void SerialPort::write_data()
{
    qDebug() << "write_id is:" << QThread::currentThreadId();
    port->write("data", 4);   //发送“data”字符
}
