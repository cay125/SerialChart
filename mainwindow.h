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
#include <QLineEdit>
#include <QLabel>
#include "qcustomplot.h"
#include "axistag.h"
#include "serialport.h"
#include "status.h"

QT_CHARTS_USE_NAMESPACE
namespace Ui {class MainWindow;}
#define XRANGE 10000
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void connectMarkers();
    double onlineVariance(double,int);
    Ui::MainWindow *ui;
    QPalette p;
    QChartView *chartView[6];
    QCustomPlot *customplot[6];
    AxisTag *mTags[6];
    AxisTag *mAveTags[6];
    QLineEdit *dataEdit[42];
    QLabel *dataLabel[42];
    QLabel *varianceLabel[21];
    QLineSeries *series;
    QChart *chart[6];
    QTimer *timer;
    QVector<QPointF> data;
    QVector<QPointF> data_twocircle;
    QTcpSocket *tcpClient;
    SerialPort *uart;
    Status *status;
    quint16 count;
    int PData[21]={0};
    long long PDataBuffer[21]={0};
    QVector<int> chartLine[6];
//    int SeriesIndex[21]={0,0,0,1,1,1,2,2,2,2,2,2,1,1,1,3,3,3,3,3,3};
    int SeriesIndex[21]={0,1,2,3,4,5,2,2,2,2,2,2,1,1,1,3,3,3,3,3,3};
    QLineSeries *mSeries[21];
    QCPGraph * mGraphs[21];
    QString SerialUnit[6]={"(m/s^2)","(m/s^2)","(m/s^2)","(deg/h)","(deg/h)","(deg/h)"};
    QString SeriesName[21]={"Acc_X","Acc_Y","Acc_Z","Gyro_X","Gyro_Y","Gyro_Z","Xspeed","Yspeed","Zspeed","Xdist","Ydist","Zdist","Angle_X","Angle_Y","Angle_Z","Acc_Temp_X","Acc_Temp_Y","Acc_Temp_Z","Gyro_Temp_X","Gyro_Temp_Y","Gyro_Temp_Z"};
//    QString SeriesName[21]={"Acc_X","Acc_Y","Acc_Z","Gyro_X","Gyro_Y","Gyro_Z","Acc_Temp_X","Acc_Temp_Y","Acc_Temp_Z","Gyro_Temp_X","Gyro_Temp_Y","Gyro_Temp_Z","Yspeed","Zspeed","Xdist","Ydist","Zdist","Xangle","Yangle","Zangle","XaccelT"};
    int maxValue[4]={-0xfffffff,-0xfffffff,-0xfffffff,-0xfffffff},minValue[4]={0xfffffff,0xfffffff,0xfffffff,0xfffffff};
    int mainGraph[6]={0};
    int plotSelect=0,dataCnt=0;
    double dataTotalSum[21]={0},dataTotalVariance[21]={0};
    double data_calib[21]={0};
    QVector<double> PDataVec[21];
    int dx=0;double dx_len=2;
    int flashRate=1;
    int receive_data_cnt=0;
    QTimer *timer_data;
    double gra_accel=9.8;
    double angle_xyz[3]={0};
private slots:
    void timerSlot_data();
    void timerSlot();
    void timerSlot_customplot();
    void ReadData();
    void ReadError(QAbstractSocket::SocketError);
    void on_btnOpenGL_clicked();
    void on_btnConnect_clicked();
    void on_receive_data(QByteArray);
    void uart_connected();
    void on_btnStart_clicked();
    void handleMarkerClicked();
    void contextMenuRequest(QPoint);
    void legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*);
    void moveLegend();
    void removeSelectedGraph();
    void removeAllGraphs();
    void selectionChanged();
    void applyMainGraph();
    void setLineVisible();

    void on_btnFlash_clicked();

signals:
    void port_started(QString,int);
    void port_closed();
};

#endif // MAINWINDOW_H
