#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serialport.h"
#include <QSerialPortInfo>
#include "status.h"
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::MainWindow),series(new QLineSeries),chart(new QChart),timer(new QTimer),status(new Status)
{
    //set white background color
    ui->setupUi(this);
    p.setColor(QPalette::Background,Qt::white);
    setAutoFillBackground(true);
    setPalette(p);

    status->isrunning=false;
    status->isconnected=false;
    ui->btnStart->setEnabled(false);

    // add portname to combobox
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {
        qDebug()<<"serialPortName:"<<info.portName();
        ui->ComBox->addItem(info.portName());
        ui->ComBox->setCurrentIndex(0);
    }

    //add Baudrate to combobox
    ui->BaudBox->addItem("9600");
    ui->BaudBox->addItem("19200");
    ui->BaudBox->addItem("38400");
    ui->BaudBox->addItem("115200");
    ui->BaudBox->setCurrentIndex(3);
    ui->FlashEdit->setText("100");

    uart=new SerialPort();
    connect(uart,SIGNAL(connected()),this,SLOT(uart_connected()),Qt::QueuedConnection);
    connect(uart,SIGNAL(receive_data(QByteArray)),this,SLOT(on_receive_data(QByteArray)), Qt::QueuedConnection);
    connect(this,SIGNAL(closed()),uart,SLOT(stop_port()));

    for(int i=0;i<12;i++)
    {
        mSeries[i]=new QLineSeries();
        mSeries[i]->setName(SeriesName[i]);
        chart->addSeries(mSeries[i]);
    }

//    series2=new QLineSeries();
//    series->setName("line1");
//    chart->addSeries(series);
//    chart->addSeries(series2);
    chart->createDefaultAxes();

    chart->axisX()->setTitleFont(QFont("Microsoft YaHei", 10, QFont::Normal,false));
    chart->axisY()->setTitleFont(QFont("Microsoft YaHei", 10, QFont::Normal,false));
    chart->axisX()->setRange(0,XRANGE);
    chart->axisY()->setRange(-1000,1000);
    chart->axisX()->setTitleText("Time");
    chart->axisY()->setTitleText("Voltage");
    chart->axisX()->setGridLineVisible(true);
    chart->axisY()->setGridLineVisible(true);
    chart->axisX()->setVisible(true);
    chart->axisX()->setTitleVisible(true);

    //chart->legend()->hide();
//    const auto markers=chart->legend()->markers();
//    markers[1]->series()->setVisible(false);
//    markers[1]->setVisible(true);

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    ui->mainLayout->addWidget(chartView);

    connectMarkers();

    for(int i=0;i<200;i++)
        data.append(QPointF(i,4*sin(2*3.14/200*i)));
    //mSeries[0]->replace(data);
    //series2->replace(data);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerSlot()));
    timer->setInterval(20);
    //timer->start();
}
void MainWindow::on_receive_data(QByteArray data)
{
    for(int i=0;i<6;i++)
        PData[i]=((int)((uint8_t)(data.at(i*4))<<24))|((int)((uint8_t)(data.at(i*4+1))<<16))|((int)((uint8_t)(data.at(i*4+2))<<8))|((int)((uint8_t)(data.at(i*4+3))));
    for(int i=0;i<6;i++)
        PData[i+6]=(int)(data[24+i]);
    //data.clear();
    //qDebug() << "main handing thread is:" << QThread::currentThreadId();
    //qDebug() << "main: " << data.toHex();
    //qDebug() << PData[0];
}
void MainWindow::ReadData()
{
    static int cnt=0;
    static int numb=0,pre_size=0;
    QByteArray buffer = tcpClient->readAll();
    if(!buffer.isEmpty())
    {
        int tsum=buffer.size();
        for(int i=0;i<tsum;i++)
            data.append(QPointF(cnt+i,(((unsigned char)buffer[i])/255.0*3.3-2.5)*5*0.92));
        if((5==buffer[tsum-1])&&(5==buffer[tsum-2]))
        {
            data.removeLast();
            data.removeLast();
            double vmax=-100,vmin=100;
            if(numb==1)
            {
                numb=0;
                for(int i=0;i<data.size();i++)
                {
                    if(data[i].y()>vmax)vmax=data[i].y();
                    if(data[i].y()<vmin)vmin=data[i].y();
                    data_twocircle.append(QPointF(data[i].x()+pre_size,data[i].y()));
                }

                chart->axisX()->setRange(0,2*(cnt+tsum-2));
                series->replace(data_twocircle);
                data_twocircle.clear();
                pre_size=0;
            }
            else
            {
                numb++;
                pre_size=data.size();
                data_twocircle.append(data);
            }
//            chart->axisX()->setRange(0,cnt+tsum-2);
//            series->replace(data);
            data.clear();
            cnt=0;
        }
        else
        {
            cnt+=tsum;
        }
//        if(cnt>=200)
//        {
//            series->replace(data);
//            data.clear();
//            cnt=0;
//        }

    }
}
void MainWindow::ReadError(QAbstractSocket::SocketError)
{
    tcpClient->disconnectFromHost();
    ui->btnConnect->setText(tr("Connect"));
    QMessageBox msgBox;
    msgBox.setText(tr("failed to connect server because %1").arg(tcpClient->errorString()));
    msgBox.exec();
}
void MainWindow::timerSlot()
{
//    QVector<QPointF> temp;
//    for(int i=0;i<200;i++)
//        temp.append(QPointF(i,data[i].y()));
//    series->clear();
//    series->replace(temp);

    for(int cnt=0;cnt<12;cnt++)
    {
        QVector<QPointF> oldData = mSeries[cnt]->pointsVector();
        QVector<QPointF> data;
        if (oldData.isEmpty() || oldData.at(oldData.size()-1).x() < XRANGE)
        //if(oldData.size()<XRANGE)
        {
            data = mSeries[cnt]->pointsVector();
        }
        else
        {
            for (int i = 1; i < oldData.size(); i++)
            {
                data.append(QPointF(oldData.at(i-1).x() , oldData.at(i).y()));
            }
        }

        for(int i = 1; i < 2; ++i)
        {
            if(!data.isEmpty())
                data.append(QPointF(i * 1 + data.at(data.size()-1).x(), PData[cnt]));
                //data.append(QPointF(i * 1 + data.at(data.size()-1).x(), 10 * sin(M_PI * count * 4 / 180 + cnt*3.14/20)));
            else
                data.append(QPointF(0,0));
                //data.append(QPointF(0, 10 * sin(M_PI * count * 4 / 180 + cnt*3.14/20)));
        }
        mSeries[cnt]->replace(data);
    }
    count++;
}
void MainWindow::on_btnOpenGL_clicked()
{
    static bool flag=true;
    for(int i=0;i<12;i++)
        mSeries[i]->setUseOpenGL(flag);
    flag=1-flag;
}
void MainWindow::on_btnConnect_clicked()
{
    if(status->isconnected == false)
        uart->start_port(ui->ComBox->currentText(),ui->BaudBox->currentText().toInt());
    else
    {
        ui->btnConnect->setText("Connect");
        ui->btnStart->setEnabled(false);
        timer->stop();
        status->isconnected=false;
        status->isrunning=false;
        emit closed();
    }
}
void MainWindow::uart_connected()
{
    status->isconnected=true;
    ui->btnConnect->setText("Disconnect");
    ui->btnStart->setEnabled(true);
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnStart_clicked()
{
    if(status->isrunning==false)
    {
        ui->btnStart->setText("Stop");
        status->isrunning=true;
        timer->start();
    }
    else
    {
        ui->btnStart->setText("Start");
        status->isrunning=false;
        timer->stop();
    }
}
void MainWindow::connectMarkers()
{
//![1]
    // Connect all markers to handler
    const auto markers = chart->legend()->markers();
    for (QLegendMarker *marker : markers)
    {
        // Disconnect possible existing connection to avoid multiple connections
        QObject::disconnect(marker, &QLegendMarker::clicked, this, &MainWindow::handleMarkerClicked);
        QObject::connect(marker, &QLegendMarker::clicked, this, &MainWindow::handleMarkerClicked);
    }
//![1]
}
void MainWindow::handleMarkerClicked()
{
    QLegendMarker* marker = qobject_cast<QLegendMarker*> (sender());
    Q_ASSERT(marker);
    switch (marker->type())
    {
        case QLegendMarker::LegendMarkerTypeXY:
        {
            // Toggle visibility of series
            marker->series()->setVisible(!marker->series()->isVisible());

            // Turn legend marker back to visible, since hiding series also hides the marker
            // and we don't want it to happen now.
            marker->setVisible(true);

            // Dim the marker, if series is not visible
            qreal alpha = 1.0;

            if (!marker->series()->isVisible())
                alpha = 0.5;

            QColor color;
            QBrush brush = marker->labelBrush();
            color = brush.color();
            color.setAlphaF(alpha);
            brush.setColor(color);
            marker->setLabelBrush(brush);

            brush = marker->brush();
            color = brush.color();
            color.setAlphaF(alpha);
            brush.setColor(color);
            marker->setBrush(brush);

            QPen pen = marker->pen();
            color = pen.color();
            color.setAlphaF(alpha);
            pen.setColor(color);
            marker->setPen(pen);

            break;
        }
        default:
        {
            qDebug() << "Unknown marker type";
            break;
        }
    }
}
