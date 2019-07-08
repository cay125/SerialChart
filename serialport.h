#ifndef SERIALPORT_H
#define SERIALPORT_H
#include <QObject>
#include <QSerialPort>
#include <QString>
#include <QByteArray>
#include <QObject>
#include <QDebug>
#include <QObject>
#include <QThread>
#include <QMainWindow>
#include <QString>


class SerialPort : public QObject
{
  Q_OBJECT
public:
  explicit SerialPort(QObject *parent = NULL);
  ~SerialPort();

  void init_port();  //初始化串口
  void start_port(QString,int);

public slots:
  void handle_data();  //处理接收到的数据
  void write_data();     //发送数据
  void stop_port();

signals:
  //接收数据
  void receive_data(QByteArray);
  void connected();

private:
  QByteArray pointData;
  QVector<int> PData;
  QThread *my_thread;
  QSerialPort *port;
  int currentTarget=0;
};



#endif // SERIALPORT_H
