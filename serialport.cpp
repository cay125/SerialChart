#include "serialport.h"
#include "status.h"
SerialPort::SerialPort(QObject *parent) : QObject(parent)
{
//    my_thread = new QThread();
//    port = new QSerialPort();
//    init_port();
//    this->moveToThread(my_thread);
//    port->moveToThread(my_thread);
//    my_thread->start();  //启动线程
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
    my_thread = new QThread();
    port = new QSerialPort();
    init_port();
    port->setPortName(portname);
    port->setBaudRate(baudrate);
    if (port->open(QIODevice::ReadWrite))
    {
        qDebug() << "Port have been opened";
        emit connected();
    }
    else
    {
        qDebug() << "open it failed";
    }
    connect(port, SIGNAL(readyRead()), this, SLOT(handle_data())); //Qt::DirectConnection
    this->moveToThread(my_thread);
    port->moveToThread(my_thread);
    my_thread->start();  //启动线程
}
void SerialPort::stop_port()
{
    port->close();
    port->deleteLater();
    my_thread->quit();
    my_thread->wait();
    my_thread->deleteLater();
    delete port;
    qDebug() << "port have been closed";
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
//                qDebug() << "emit signal";
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
