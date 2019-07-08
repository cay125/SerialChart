#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QLineSeries>
#include <QChartView>
#include <QLegendMarker>
#include <QLineSeries>
#include <QXYLegendMarker>
#include <QTcpSocket>
#include <QChart>
#include <QTimer>
#include "serialport.h"
#include "status.h"

QT_CHARTS_USE_NAMESPACE
namespace Ui {class MainWindow;}
#define XRANGE 200
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void connectMarkers();
    Ui::MainWindow *ui;
    QPalette p;
    QChartView *chartView;
    QLineSeries *series;
    QChart *chart;
    QTimer *timer;
    QVector<QPointF> data;
    QVector<QPointF> data_twocircle;
    QTcpSocket *tcpClient;
    SerialPort *uart;
    Status *status;
    quint16 count;
    int PData[12]={0};
    QLineSeries *mSeries[12];
    QString SeriesName[12]={"Xaccel","Yaccel","Zaccel","Xgyro","Ygyro","Zgyro","XaccelT","YaccelT","ZaccelT","XgyroT","YgyroT","ZgyroT"};

private slots:
    void timerSlot();
    void ReadData();
    void ReadError(QAbstractSocket::SocketError);
    void on_btnOpenGL_clicked();
    void on_btnConnect_clicked();
    void on_receive_data(QByteArray);
    void uart_connected();
    void on_btnStart_clicked();
    void handleMarkerClicked();

signals:
    void closed();
};

#endif // MAINWINDOW_H
