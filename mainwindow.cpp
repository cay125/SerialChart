#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serialport.h"
#include <QSerialPortInfo>
#include "status.h"
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::MainWindow),series(new QLineSeries),timer(new QTimer),status(new Status)
{
//    showMaximized();
    //set white background color
    ui->setupUi(this);
    p.setColor(QPalette::Background,Qt::white);
    setAutoFillBackground(true);
    setPalette(p);

    for(int i=0;i<21;i++)
    {
        dataEdit[i]=new QLineEdit();
        dataEdit[i]->setMaximumWidth(70);
        dataEdit[i]->setReadOnly(true);
        dataEdit[i]->setText("NULL");
        ui->lineLayout->addWidget(dataEdit[i]);
    }
    ui->lineLayout->addStretch();

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

//    series->setName("line1");
//    chart->addSeries(series);

    for(int i=0;i<4;i++)
    {
        customplot[i]=new QCustomPlot(this);
        chart[i]=new QChart();
    }
    QPen pen[9];
    pen[0].setColor(QColor(250,120,  0));
    pen[1].setColor(QColor(  0,180, 60));
    pen[2].setColor(QColor(0xff,0, 0));
    pen[8].setColor(QColor(  0,0x66, 0xff));
    pen[7].setColor(QColor(  0xff,0x77, 0xff));
//    pen[5].setColor(QColor(  0xd2,0x69, 0x1e));
    pen[5].setColor(QColor(  0xda,0xa5, 0x20));
    pen[6].setColor(QColor(  0xff,0x14, 0x93));
    pen[4].setColor(QColor(  0x80,0x80, 0));
    pen[3].setColor(QColor(  0x22,0x8b, 0x22));
    int color[4]={0};
    for(int i=0;i<21;i++)
    {
        mSeries[i]=new QLineSeries();
        mSeries[i]->setName(SeriesName[i]);
        chart[SeriesIndex[i]]->addSeries(mSeries[i]);

        customplot[SeriesIndex[i]]->addGraph();
        mGraphs[i]=customplot[SeriesIndex[i]]->graph();
        mGraphs[i]->setName(SeriesName[i]);
        mGraphs[i]->setPen(pen[color[SeriesIndex[i]]++]);
    }
    for(int i=0;i<4;i++)
    {
        chart[i]->createDefaultAxes();
        chart[i]->setTitle("Chart");
        chart[i]->axisX()->setTitleFont(QFont("Microsoft YaHei", 10, QFont::Normal,false));
        chart[i]->axisY()->setTitleFont(QFont("Microsoft YaHei", 10, QFont::Normal,false));
        chart[i]->axisX()->setRange(0,XRANGE);
        chart[i]->axisY()->setRange(-10,10);
        chart[i]->axisX()->setTitleText("Time");
        chart[i]->axisY()->setTitleText("data");
        chart[i]->axisX()->setGridLineVisible(true);
        chart[i]->axisY()->setGridLineVisible(true);
        chart[i]->axisX()->setVisible(false);
        chart[i]->axisY()->setVisible(true);
        chart[i]->axisX()->setTitleVisible(true);
        chart[i]->axisY()->setTitleVisible(true);
        chart[i]->legend()->setAlignment(Qt::AlignBottom);
        chartView[i] = new QChartView(chart[i]);
        chartView[i]->setRenderHint(QPainter::Antialiasing);
//        ui->mainLayout->addWidget(chartView[i],i/2,i%2);

        customplot[i]->legend->setVisible(true);
        customplot[i]->xAxis2->setVisible(true);
        customplot[i]->xAxis2->setTickLabels(false);
        customplot[i]->yAxis2->setVisible(true);
        customplot[i]->yAxis2->setTickLabels(false);
        customplot[i]->xAxis->setRange(0,XRANGE);
        customplot[i]->yAxis->setRange(-10,10);
        customplot[i]->xAxis->setTickLabels(false);
        customplot[i]->legend->setVisible(true);
        ui->mainLayout->addWidget(customplot[i],i/2,i%2);
    }

    connectMarkers();

    QVector<double> x,y;
    for(int i=0;i<200;i++)
    {
        x.append(i);
        y.append(4*sin(2*3.14/200*i));
        data.append(QPointF(i,4*sin(2*3.14/200*i)));
    }
    //mSeries[0]->replace(data);
    //series2->replace(data);

//    connect(timer, SIGNAL(timeout()), this, SLOT(timerSlot()));
    connect(timer, SIGNAL(timeout()), this, SLOT(timerSlot_customplot()));
    timer->setInterval(20);
    //timer->start();

//    customplot[0] = new QCustomPlot(this);
//    customplot[0]->addGraph();
//    customplot[0]->graph(0)->setData(x,y);
//    customplot[0]->xAxis->setRange(0,200);
//    customplot[0]->yAxis->setRange(-5,5);
//    customplot[0]->xAxis2->setVisible(true);
//    customplot[0]->xAxis2->setTickLabels(false);
//    customplot[0]->yAxis2->setVisible(true);
//    customplot[0]->yAxis2->setTickLabels(false);
//    customplot[0]->legend->setVisible(false);
//    ui->mainLayout->addWidget(customplot[0]);
}
void MainWindow::on_receive_data(QByteArray data)
{
    for(int i=0;i<6;i++)
        PData[i]=((int)((uint8_t)(data.at(i*4))<<24))|((int)((uint8_t)(data.at(i*4+1))<<16))|((int)((uint8_t)(data.at(i*4+2))<<8))|((int)((uint8_t)(data.at(i*4+3))));
    for(int i=0;i<6;i++)
        PData[i+15]=(int)(data[24+i]);
    for(int i=0;i<3;i++)
    {
        PData[i+6]+=PData[i];
//        PData[i+9]+=PData[i+6];
        PData[i+12]+=PData[i+3];
    }
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

                chart[0]->axisX()->setRange(0,2*(cnt+tsum-2));
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
void MainWindow::timerSlot_customplot()
{
    static int dx=0;
    for(int cnt=0;cnt<21;cnt++)
    {
        // calculate and add a new data point to each graph:
        mGraphs[cnt]->addData(dx, PData[cnt]);
        if(dx > XRANGE)
        {
            mGraphs[cnt]->data()->remove(dx-XRANGE);
        }
        mGraphs[cnt]->rescaleValueAxis(true, true);
    }
    for(int i=0;i<4;i++)
    {
        if(dx>XRANGE)
        {
            customplot[i]->xAxis->rescale();
            customplot[i]->xAxis->setRange(customplot[i]->xAxis->range().upper, XRANGE, Qt::AlignRight);
        }
        customplot[i]->replot();
    }
    dx++;
}
void MainWindow::timerSlot()
{
//    QVector<QPointF> temp;
//    for(int i=0;i<200;i++)
//        temp.append(QPointF(i,data[i].y()));
//    series->clear();
//    series->replace(temp);
    static uint8_t dataTextUpdatecnt=0;
    dataTextUpdatecnt++;
    for(int cnt=0;cnt<21;cnt++)
    {
        if(dataTextUpdatecnt>=4)
            dataEdit[cnt]->setText(QString::number(PData[cnt]));
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
            {
                if(maxValue[SeriesIndex[cnt]]<PData[cnt])
                {
                    maxValue[SeriesIndex[cnt]]=PData[cnt];
                    chart[SeriesIndex[cnt]]->axisY()->setMax(maxValue[SeriesIndex[cnt]]*1.1+10);
                }
                if(minValue[SeriesIndex[cnt]]>PData[cnt])
                {
                    minValue[SeriesIndex[cnt]]=PData[cnt];
                    chart[SeriesIndex[cnt]]->axisY()->setMin(minValue[SeriesIndex[cnt]]*1.1-10);
                }
                //chart->axisY()->setRange(minValue*1.1,maxValue*1.1);
                data.append(QPointF(i * 1 + data.at(data.size()-1).x(), PData[cnt]));
                //data.append(QPointF(i * 1 + data.at(data.size()-1).x(), 10 * sin(M_PI * count * 4 / 180 + cnt*3.14/20)));
            }
            else
            {
                data.append(QPointF(0,0));
                //data.append(QPointF(0, 10 * sin(M_PI * count * 4 / 180 + cnt*3.14/20)));
            }
        }
        mSeries[cnt]->replace(data);

//        double dx=0;
//        if(!oldData.isEmpty())
//        {
//            if(maxValue<PData[cnt])
//            {
//               maxValue=PData[cnt];
//               chart->axisY()->setMax(maxValue*1.1+10);
//            }
//            if(minValue>PData[cnt])
//            {
//               minValue=PData[cnt];
//               chart->axisY()->setMin(minValue*1.1-10);
//            }
//            dx=1 + oldData.at(oldData.size()-1).x();
//            mSeries[cnt]->append(QPointF( dx, PData[cnt]));
//        }
//        else
//        {
//            mSeries[cnt]->append(QPointF(0,0));
//        }
//        if (!oldData.isEmpty() && oldData.at(oldData.size()-1).x() >= XRANGE)
//        {
//            mSeries[cnt]->remove(0);
//            chart->axisX()->setRange(dx-XRANGE,dx);
//        }

    }
    if(dataTextUpdatecnt>=4)
        dataTextUpdatecnt=0;
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
        for(int i=0;i<12;i++)
            mSeries[i]->clear();
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
    for(int i=0;i<4;i++)
    {
        const auto markers = chart[i]->legend()->markers();
        for (QLegendMarker *marker : markers)
        {
            // Disconnect possible existing connection to avoid multiple connections
            QObject::disconnect(marker, &QLegendMarker::clicked, this, &MainWindow::handleMarkerClicked);
            QObject::connect(marker, &QLegendMarker::clicked, this, &MainWindow::handleMarkerClicked);
        }
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
