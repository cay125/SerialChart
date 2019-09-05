#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serialport.h"
#include <QSerialPortInfo>
#include "status.h"
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::MainWindow),series(new QLineSeries),timer(new QTimer),status(new Status),timer_data(new QTimer),linePalette(new stylePalette),fftwin(new fftWindow(parent)),fftloader(new fftLoader())
{
    //showMaximized();
    //set white background color
    ui->setupUi(this);
    p.setColor(QPalette::Background,Qt::white);
    setAutoFillBackground(true);
    setPalette(p);
    qDebug()<< "mainwindow work on thread id = " << QThread::currentThreadId();
    fft_thread=new QThread();
    fftloader->moveToThread(fft_thread);
    fft_thread->start();
    for(int i=0;i<6;i++)
        onlineVar[i] = new onlineVarian();
    for(int i=0;i<21;i++)
    {
        dataEdit[i]=new QLineEdit();
        dataEdit[i]->setMaximumWidth(70);
        dataEdit[i]->setReadOnly(true);
        dataEdit[i]->setText("NULL");

        dataLabel[i]=new QLabel();
        dataLabel[i]->setMinimumHeight(20);
        if(i<6)
            dataLabel[i]->setText(SeriesName[i]);
        else if(i>=6 && i<9)
            dataLabel[i]->setText(SeriesName[i-6].left(4) + "STD_" + SeriesName[i-6].right(1));
        else if(i>=9 && i<12)
            dataLabel[i]->setText(SeriesName[i-6].left(5) + "STD_" + SeriesName[i-6].right(1));
        else if(i>=12 && i<18)
            dataLabel[i]->setText(SeriesName[i-12+15]);
        else
            dataLabel[i]->setText(SeriesName[i-18+12]);
        dataLabel[i]->setFont(QFont("Microsoft YaHei", 9, QFont::Normal,false));

        ui->lineLayout->addWidget(dataEdit[i]);
        ui->labelLayout->addWidget(dataLabel[i]);
    }
    ui->lineLayout->addStretch();
    ui->labelLayout->addStretch();

    ui->btnStart->setEnabled(false);

    // add portname to combobox
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {
        qDebug()<<"serialPortName:"<<info.portName();
        ui->ComBox->addItem(info.portName());
        ui->ComBox->setCurrentIndex(0);
    }
    ui->ComBox->setFont(QFont("Microsoft YaHei", 9, QFont::Normal,false));

    //add Baudrate to combobox
    ui->BaudBox->addItem("9600");
    ui->BaudBox->addItem("19200");
    ui->BaudBox->addItem("38400");
    ui->BaudBox->addItem("115200");
    ui->BaudBox->addItem("230400");
    ui->BaudBox->setCurrentIndex(4);
    ui->BaudBox->setFont(QFont("Microsoft YaHei", 9, QFont::Normal,false));
    ui->FlashEdit->setText("1");
    ui->FlashEdit->setValidator(new QIntValidator(0,1000));
    ui->FlashEdit->setFont(QFont("Microsoft YaHei", 9, QFont::Normal,false));
    ui->GraEdit->setText("9.8");
    ui->GraEdit->setValidator(new QDoubleValidator());
    ui->GraEdit->setFont(QFont("Microsoft YaHei", 9, QFont::Normal,false));
    ui->speedSlider->setRange(2,50);
    ui->speedSlider->setValue(2);
    dx_len=ui->speedSlider->value();

    uart=new SerialPort();
    connect(uart,SIGNAL(connected()),this,SLOT(uart_connected()),Qt::QueuedConnection);
//    connect(uart,SIGNAL(receive_data(QByteArray)),this,SLOT(on_receive_data(QByteArray)), Qt::QueuedConnection);
    connect(this,SIGNAL(port_closed()),uart,SLOT(stop_port()));
    connect(this,SIGNAL(port_started(QString,int)),uart,SLOT(start_port(QString,int)));

//    series->setName("line1");
//    chart->addSeries(series);

//    for(int i=0;i<21;i++)
//        chartLine[SeriesIndex[i]].push_back(i);
    for(int i=0;i<6;i++)
    {
        customplot[i]=new QCustomPlot(this);
//        chart[i]=new QChart();
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
//    for(int i=0;i<9;i++)
//        pen[i].setWidth(2);
    int color[6]={0};
    for(int i=0;i<6;i++)
    {
//        mSeries[i]=new QLineSeries();
//        mSeries[i]->setName(SeriesName[i]);
//        chart[SeriesIndex[i]]->addSeries(mSeries[i]);

        customplot[SeriesIndex[i]]->addGraph();
        mGraphs[i]=customplot[SeriesIndex[i]]->graph();
        mGraphs[i]->setName(SeriesName[i]+SerialUnit[i]);
        mGraphs[i]->setPen(pen[color[SeriesIndex[i]]++]);
        QCPSelectionDecorator *decorator=new QCPSelectionDecorator();
        QPen tpen;
        tpen.setColor(mGraphs[i]->pen().color());
        tpen.setWidth(2);
        decorator->setPen(tpen);
        mGraphs[i]->setSelectionDecorator(decorator);
    }
    for(int i=0;i<6;i++)
    {
        customplot[i]->axisRect()->axis(QCPAxis::atRight, 0)->setPadding(80); // add some padding to have space for tags
        mTags[i] = new AxisTag(mGraphs[i]->valueAxis());
        mTags[i]->setPen(mGraphs[i]->pen());
        mTags[i]->setText("0");
        mAveTags[i] = new AxisTag(mGraphs[i]->valueAxis());
        mAveTags[i]->setPen(QPen(QColor(0,0,255)));
        mAveTags[i]->setText("0");
//        chart[i]->createDefaultAxes();
//        chart[i]->setTitle("Chart");
//        chart[i]->axisX()->setTitleFont(QFont("Microsoft YaHei", 10, QFont::Normal,false));
//        chart[i]->axisY()->setTitleFont(QFont("Microsoft YaHei", 10, QFont::Normal,false));
//        chart[i]->axisX()->setRange(0,XRANGE);
//        chart[i]->axisY()->setRange(-10,10);
//        chart[i]->axisX()->setTitleText("Time");
//        chart[i]->axisY()->setTitleText("data");
//        chart[i]->axisX()->setGridLineVisible(true);
//        chart[i]->axisY()->setGridLineVisible(true);
//        chart[i]->axisX()->setVisible(false);
//        chart[i]->axisY()->setVisible(true);
//        chart[i]->axisX()->setTitleVisible(true);
//        chart[i]->axisY()->setTitleVisible(true);
//        chart[i]->legend()->setAlignment(Qt::AlignBottom);
//        chartView[i] = new QChartView(chart[i]);
//        chartView[i]->setRenderHint(QPainter::Antialiasing);
//        ui->mainLayout->addWidget(chartView[i],i/2,i%2);

        customplot[i]->setInteractions(QCP::iSelectLegend | QCP::iSelectPlottables);
        customplot[i]->axisRect()->setupFullAxesBox();
        customplot[i]->legend->setSelectedFont(QFont("Microsoft YaHei", 9, QFont::Normal,false));
        customplot[i]->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items
        // connect some interaction slots:
        connect(customplot[i], SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
//        connect(customplot[i], SIGNAL(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)), this, SLOT(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*)));
        customplot[i]->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(customplot[i], SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));

        customplot[i]->legend->setVisible(true);
        customplot[i]->xAxis2->setVisible(true);
        customplot[i]->xAxis2->setTickLabels(false);
        customplot[i]->yAxis2->setVisible(true);
        customplot[i]->yAxis2->setTickLabels(false);
        customplot[i]->xAxis->setRange(0,XRANGE);
        customplot[i]->yAxis->setRange(-10,10);
        customplot[i]->xAxis->setTickLabels(false);
        customplot[i]->legend->setVisible(true);
        customplot[i]->legend->setFont(QFont("Microsoft YaHei", 9, QFont::Normal,false));
        customplot[i]->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignRight);
        ui->mainLayout->addWidget(customplot[i],i,0);
    }

//    connectMarkers();

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
    connect(timer_data, SIGNAL(timeout()), this, SLOT(timerSlot_data()));
    connect(linePalette,SIGNAL(signal_changeBackColor(QColor)),this,SLOT(paletteColorSlot(QColor)));
    connect(this, SIGNAL(FFTstart_signal(QVariant,QString)), fftloader,SLOT(FFTstart_slot(QVariant,QString)));
    connect(fftloader,SIGNAL(FFTfinished_signal(QVariant,QString)),fftwin,SLOT(FFTfinished_slot(QVariant,QString)));
    connect(fftwin,SIGNAL(fftNum_signal(int)),fftloader,SLOT(fftNum_slot(int)));
    timer_data->setInterval((int)(1000.0/flashRate));
    timer_data->start();
    timer->setInterval(30);
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
void MainWindow::selectionChanged()
{
    QCustomPlot* custom_chart = qobject_cast<QCustomPlot*> (sender());
    // synchronize selection of graphs with selection of corresponding legend items:
    for (int i=0; i<custom_chart->graphCount(); i++)
    {
        QCPGraph *graph = custom_chart->graph(i);
        QCPPlottableLegendItem *item = custom_chart->legend->itemWithPlottable(graph);
        if (item->selected() || graph->selected())
        {
            item->setSelected(true);
            graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
        }
    }
}
void MainWindow::legendDoubleClick(QCPLegend* legend,QCPAbstractLegendItem* item)
{
    // Rename a graph by double clicking on its legend item
    Q_UNUSED(legend)
    if (item) // only react if item was clicked (user could have clicked on border padding of legend where there is no item, then item is 0)
    {
//        QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
//        plItem->plottable()->setVisible(!plItem->plottable()->visible());
//        plItem->setTextColor(QColor(0,0,0,plItem->plottable()->visible()?255:100));
//        plItem->setSelected(false);
        item->setSelected(false);
    }
}
void MainWindow::removeSelectedGraph()
{
    if(customplot[plotSelect]->selectedGraphs().size()>0)
    {
        customplot[plotSelect]->selectedGraphs().first()->setVisible(false);
    }
}
void MainWindow::removeAllGraphs()
{
    for(int i=0;i<chartLine[plotSelect].size();i++)
    {
        if(mainGraph[plotSelect]!=i)
        {
            mGraphs[chartLine[plotSelect][i]]->setVisible(false);
            QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(customplot[plotSelect]->legend->item(i));
            plItem->setTextColor(QColor(180,180,180));
        }
    }
}
void MainWindow::applyMainGraph()
{
    if (QAction* contextAction = qobject_cast<QAction*>(sender()))
    {
        mainGraph[plotSelect] = contextAction->data().toInt();
        mGraphs[chartLine[plotSelect][mainGraph[plotSelect]]]->setVisible(true);
        QCPPlottableLegendItem *item = customplot[plotSelect]->legend->itemWithPlottable(mGraphs[chartLine[plotSelect][mainGraph[plotSelect]]]);
        item->setTextColor(QColor(0,0,0));
        mTags[plotSelect]->setPen(mGraphs[chartLine[plotSelect][mainGraph[plotSelect]]]->pen());
    }
}
void MainWindow::setLineVisible()
{
    if (QAction* contextAction = qobject_cast<QAction*>(sender()))
    {
        int index = contextAction->data().toInt();
        mGraphs[chartLine[plotSelect][index]]->setVisible(!mGraphs[chartLine[plotSelect][index]]->visible());
        int colorRGB=mGraphs[chartLine[plotSelect][index]]->visible()?0:180;
        customplot[plotSelect]->legend->item(index)->setTextColor(QColor(colorRGB,colorRGB,colorRGB));
//        if(!mGraphs[chartLine[plotSelect][index]]->visible())
//        {
//            customplot[plotSelect]->legend->item(index)->setSelected(false);
//            customplot[plotSelect]->legend->item(index)->setSelectable(false);
//        }
    }
}
void MainWindow::contextMenuRequest(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QCustomPlot* custom_chart = qobject_cast<QCustomPlot*> (sender());
    for(int i=0;i<6;i++)
    {
        if(custom_chart==customplot[i])
        {
            plotSelect = i;
            break;
        }
    }
    if (custom_chart->legend->selectTest(pos, false) >= 0) // context menu on legend requested
    {
        for(int i=0;i<chartLine[plotSelect].size();i++)
        {
            if(custom_chart->legend->item(i)->selectTest(pos,false)>=0)
            {
                if(mainGraph[plotSelect]!=i)
                {
                    menu->addAction("Apply item as main chart", this, SLOT(applyMainGraph()))->setData(i);
                    QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(custom_chart->legend->item(i));
                    QString setVisibleItem=plItem->plottable()->visible()?"Remove line from chart":"Add line to chart";
                    menu->addAction(setVisibleItem,this,SLOT(setLineVisible()))->setData(i);
                }
                break;
            }
        }
        menu->addAction("Change chart color",linePalette,SLOT(slot_OpenColorPad()));
        menu->addAction((isfftTransfer[plotSelect] && fftwin->isVisible())? "Hide signal FFT" : "Show signal FFT",this,SLOT(addFFTplotSlot()));
        menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignLeft));
        menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignHCenter));
        menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignRight));
        menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignRight));
        menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignLeft));
    }
    else  // general context menu on graphs requested
    {
//        if (custom_chart->selectedGraphs().size() > 0)
//          menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
        if (custom_chart->graphCount() > 0)
          menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
    }
    menu->popup(custom_chart->mapToGlobal(pos));
}
void MainWindow::moveLegend()
{
    if (QAction* contextAction = qobject_cast<QAction*>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
    {
        bool ok;
        int dataInt = contextAction->data().toInt(&ok);
        if (ok)
        {
            customplot[plotSelect]->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
            if(!status->isrunning)
                customplot[plotSelect]->replot();
        }
    }
}
void MainWindow::on_receive_data(QByteArray data)
{
    //static int receive_data_cnt=0;
    for(int i=0;i<6;i++)
        PData[i]=((int)((uint8_t)(data.at(i*4+3))<<24))|((int)((uint8_t)(data.at(i*4+2))<<16))|((int)((uint8_t)(data.at(i*4+1))<<8))|((int)((uint8_t)(data.at(i*4+0))));
    for(int i=0;i<6;i++)
        PData[i+15]=(int)(data[24+i]);
    for(int i=0;i<3;i++)
    {
        PData[i+6]+=PData[i];
//        PData[i+9]+=PData[i+6];
        PData[i+12]+=PData[i+3];
    }
    for(int i=0;i<21;i++)
        PDataBuffer[i]+=PData[i];
//    for(int i=0;i<21;i++)
//    {
//        if(i<3)
//            data_calib[i]=PData[i]*0.0000001*9.8;
//        else if(i>=3 && i<6)
//            data_calib[i]=PData[i]*0.000001*3600;
//        if(dataCnt!=0)
//            dataTotalVariance[i]=onlineVariance(data_calib[i],i);
//        dataTotalSum[i]+=data_calib[i];
//        PDataBuffer[i]+=PData[i];
//    }
//    dataCnt++;
    receive_data_cnt++;
//    if(receive_data_cnt>=flashRate)
//    {
//        for(int i=0;i<21;i++)
//        {
//            if(i<3)
//                PDataVec[i].append(1.0*PDataBuffer[i]/receive_data_cnt*0.0000001*9.8);
//            else if(i>=3 && i<6)
//                PDataVec[i].append(1.0*PDataBuffer[i]/receive_data_cnt*0.000001*3600);
//            else
//                PDataVec[i].append(1.0*PDataBuffer[i]/receive_data_cnt);
//            PDataBuffer[i]=0;
//        }
//        receive_data_cnt=0;
//    }
    //dataCnt++;
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
void MainWindow::timerSlot_data()
{
    if(receive_data_cnt>0)
    {
        for(int i=0;i<21;i++)
        {
           if(i<3)
           {
               PDataVec[i].append(1.0*PDataBuffer[i]/receive_data_cnt*0.0000001*gra_accel);
           }
           else if(i>=3 && i<6)
           {
               PDataVec[i].append(1.0*PDataBuffer[i]/receive_data_cnt*0.000001*3600);
           }
           else if(i>=12 && i<15)
           {
               angle_xyz[i-12]+=1.0*PDataVec[i-12+3][PDataVec[i-12+3].size()-1]/3600.0/flashRate;
               if(PDataVec[i].size()==0)
                   PDataVec[i].append(1.0*PDataVec[i-12+3][PDataVec[i-12+3].size()-1]/3600.0/flashRate);
               else
                   PDataVec[i].append(PDataVec[i][PDataVec[i].size()-1]+1.0*PDataVec[i-12+3][PDataVec[i-12+3].size()-1]/3600.0/flashRate);
           }
           else
           {
               PDataVec[i].append(1.0*PDataBuffer[i]/receive_data_cnt);
           }
           PDataBuffer[i]=0;
        }
        for(int i=0;i<21;i++)
        {
            if(dataCnt!=0)
                dataTotalVariance[i]=onlineVariance(PDataVec[i][PDataVec[i].size()-1],i);
            dataTotalSum[i]+=PDataVec[i][PDataVec[i].size()-1];
        }
        dataCnt++;
        receive_data_cnt=0;
    }
}
void MainWindow::timerSlot_customplot()
{
    static int dataTextUpdateCnt=0;
    dataTextUpdateCnt++;
    int index=mGraphs[0]->dataCount()-1;
    double lastX=index>=0?mGraphs[0]->dataMainKey(index):0;
    for(int cnt=0;cnt<6;cnt++)
    {
//        if(dataTextUpdateCnt>=3)
//        {
//            dataEdit[cnt*2]->setText(QString::number(PData[cnt]));
//            dataEdit[cnt*2+1]->setText(QString::number(dataTotalVariance[cnt],'f',2));
//        }
        // calculate and add a new data point to each graph:
        //mGraphs[cnt]->addData(dx, PData[cnt]);
        QVector<double> xPos;
        int len=PDataVec[cnt].size();
        if(len!=0)
        {
            for(int j=1;j<=len;j++)
            {
//                mGraphs[cnt]->addData(j*dx_len/len+lastX,PDataVec[cnt][j-1]);
//                mGraphs[cnt]->addData(dx_len+lastX,PData[cnt]);
                xPos.append(j*dx_len/len+lastX);
                fftData[cnt].append(PDataVec[cnt][j-1]);
                if(fftData[cnt].size()>=fftloader->N)
                {
                    if(isfftTransfer[cnt])
                    {
                        QVariant dataTransfer;
                        dataTransfer.setValue(fftData[cnt]);
                        emit FFTstart_signal(dataTransfer,mGraphs[cnt]->name());
                    }
                    fftData[cnt].clear();
                }
            }
            mGraphs[cnt]->addData(xPos,PDataVec[cnt]);
            onlineVar[cnt]->addData(PDataVec[cnt]);
        }
        else
        {
            //mGraphs[cnt]->addData(lastX+dx_len,index>=0?mGraphs[cnt]->dataMainValue(index):0);
        }
//        PDataVec[cnt].clear();
        if((lastX+dx_len) > XRANGE && len!=0)
        {
            QVector<double> removeX;
            int j=0;
            while(mGraphs[cnt]->dataMainKey(j)<(lastX+dx_len-XRANGE))
            {
                //mGraphs[cnt]->data()->remove(mGraphs[cnt]->dataMainKey(0));
                removeX.append(mGraphs[cnt]->dataMainValue(j));
                j++;
            }
            onlineVar[cnt]->removeData(removeX);
            mGraphs[cnt]->data()->removeBefore(lastX+dx_len-XRANGE);
        }
//        mGraphs[cnt]->rescaleValueAxis(true, true);
    }
//    for(int cnt=0;cnt<21;cnt++)
//        PDataVec[cnt].clear();
    if(dataTextUpdateCnt>=3 && PDataVec[0].size()!=0)
    {
        for(int cnt=0;cnt<21;cnt++)
        {
            if(cnt<6)
                dataEdit[cnt]->setText(QString::number(PDataVec[cnt][PDataVec[cnt].size()-1],'f',4));
            else if(cnt>=6 && cnt <12)
                //dataEdit[cnt]->setText(QString::number(sqrt(dataTotalVariance[cnt-6]),'f',4));
                dataEdit[cnt]->setText(QString::number(sqrt(onlineVar[cnt-6]->currentVar),'f',4));
            else if(cnt>=12 && cnt<18)
                dataEdit[cnt]->setText(QString::number(PData[cnt-12+15]));
            else
                dataEdit[cnt]->setText(QString::number(angle_xyz[cnt-18],'f',4));
        }
        dataTextUpdateCnt=0;
    }
    if(PDataVec[0].size()!=0)
    {
        for(int i=0;i<6;i++)
        {
            mGraphs[i]->rescaleValueAxis(false,true);
            double graphValue;
            if((lastX+dx_len)>XRANGE)
            {
                customplot[i]->xAxis->rescale();
                customplot[i]->xAxis->setRange(customplot[i]->xAxis->range().upper, XRANGE, Qt::AlignRight);
    //            graphValue = mGraphs[chartLine[i][mainGraph[i]]]->dataMainValue(XRANGE);
            }
    //        else
    //        {
    //            graphValue = mGraphs[chartLine[i][mainGraph[i]]]->dataMainValue(dx);
    //        }
            graphValue=mGraphs[i]->dataMainValue(mGraphs[i]->dataCount()-1);
            graphValue=mGraphs[i]->visible()?graphValue:0;
            mTags[i]->updatePosition(graphValue);
            mTags[i]->setText(QString::number(graphValue,'f'));
            //double tag_ave=dataCnt>0?dataTotalSum[i]/dataCnt:0;
            //mAveTags[i]->updatePosition(tag_ave);
            //mAveTags[i]->setText(QString::number(tag_ave,'f'));
            mAveTags[i]->updatePosition(onlineVar[i]->currentMean);
            mAveTags[i]->setText(QString::number(onlineVar[i]->currentMean,'f'));
            customplot[i]->replot();
        }
    }
    for(int cnt=0;cnt<21;cnt++)
        PDataVec[cnt].clear();
//    qDebug()<<"count"<<mGraphs[0]->dataCount();
//    qDebug()<<"main "<<mGraphs[0]->dataMainKey(0);
//    qDebug()<<"sorted "<<mGraphs[0]->dataSortKey(0);
//    qDebug()<<"main v "<<mGraphs[0]->dataMainValue(0);
//    dx+=dx_len;
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
//    static bool flag=true;
//    for(int i=0;i<21;i++)
//        mSeries[i]->setUseOpenGL(flag);
//    flag=1-flag;
//    for(int i=0;i<6;i++)
//        customplot[i]->setOpenGl(!customplot[i]->openGl());
    gra_accel=ui->GraEdit->text().toDouble();
}
void MainWindow::on_btnConnect_clicked()
{
    if(status->isconnected == false)
    {
//        uart->start_port(ui->ComBox->currentText(),ui->BaudBox->currentText().toInt());
        emit port_started(ui->ComBox->currentText(),ui->BaudBox->currentText().toInt());
    }
    else
    {
        ui->btnConnect->setText("Connect");
        ui->btnStart->setEnabled(false);
        ui->ComBox->setEnabled(true);
        ui->BaudBox->setEnabled(true);
        timer->stop();
        ui->btnStart->setText("Start");
        status->isconnected=false;
        status->isrunning=false;
        emit port_closed();
    }
}
void MainWindow::uart_connected()
{
    status->isconnected=true;
    ui->btnConnect->setText("Disconnect");
    ui->btnStart->setEnabled(true);
    ui->ComBox->setEnabled(false);
    ui->BaudBox->setEnabled(false);
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnStart_clicked()
{
    if(status->isrunning==false)
    {
        for(int i=0;i<21;i++)
        {
            if(i<6)
            {
//                mSeries[i]->clear();
                mGraphs[i]->data()->clear();
            }
            PDataVec[i].clear();
            PData[i]=0;
            PDataBuffer[i]=0;
            dataTotalSum[i]=0;
            dataTotalVariance[i]=0;
            data_calib[i]=0;
        }
        receive_data_cnt=0;
        dataCnt=0;
        for(int i=0;i<3;i++)
            angle_xyz[i]=0;
        for(int i=0;i<6;i++)
        {
            onlineVar[i]->clearData();
            fftData[i].clear();
            customplot[i]->xAxis->setRange(0,XRANGE);
            customplot[i]->yAxis->setRange(-10,10);
            customplot[i]->setInteraction(QCP::iRangeZoom,false);
            customplot[i]->setInteraction(QCP::iRangeDrag,false);
        }
        ui->btnStart->setText("Stop");
        status->isrunning=true;
        connect(uart,SIGNAL(receive_data(QByteArray)),this,SLOT(on_receive_data(QByteArray)), Qt::QueuedConnection);
        timer->start();
    }
    else
    {
        disconnect(uart,SIGNAL(receive_data(QByteArray)),this,SLOT(on_receive_data(QByteArray)));
        ui->btnStart->setText("Start");
        status->isrunning=false;
        timer->stop();
        for(int i=0;i<6;i++)
        {
            customplot[i]->setInteraction(QCP::iRangeZoom,true);
            customplot[i]->setInteraction(QCP::iRangeDrag,true);
        }
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

void MainWindow::on_btnFlash_clicked()
{
    int trate=ui->FlashEdit->text().toInt();
    if(trate>0 && trate<=300)
    {
        flashRate=trate;
        timer_data->setInterval((int)(1000.0/flashRate));
    }
    else
    {
        QMessageBox::information(this,"Warning","Invalid Input(flash rate should be larger than 0 and smaller than 300)");
    }
}
double MainWindow::onlineVariance(double input,int index)
{
    double newAve=1.0*(dataTotalSum[index]+input)/(dataCnt+1);
    double oldAve=1.0*dataTotalSum[index]/dataCnt;
    double t1=dataCnt*(dataTotalVariance[index]+(newAve-oldAve)*(newAve-oldAve));
    double t2=(newAve-input)*(newAve-input);
    return (t1+t2)/(dataCnt+1);
}

void MainWindow::on_speedSlider_valueChanged(int value)
{
    dx_len=value;
}
void MainWindow::paletteColorSlot(QColor color)
{
    QPen pen(color);
    mGraphs[plotSelect]->setPen(pen);
    mTags[plotSelect]->setPen(pen);
    if(isfftTransfer[plotSelect])
        fftwin->changePlotPen(mGraphs[plotSelect]->name(),pen);
    QCPSelectionDecorator *decorator=new QCPSelectionDecorator();
    pen.setWidth(2);
    decorator->setPen(pen);
    mGraphs[plotSelect]->setSelectionDecorator(decorator);
    if(status->isrunning==false)
        customplot[plotSelect]->replot();
}
void MainWindow::addFFTplotSlot()
{
    if(!isfftTransfer[plotSelect])
    {
        fftwin->addPlot(mGraphs[plotSelect]->name(), mGraphs[plotSelect]->pen(), 512);
        isfftTransfer[plotSelect]=true;
        fftwin->show();
        fftwin->activateWindow();
    }
    else
    {
        if(fftwin->isVisible())
        {
            fftwin->removePlot(mGraphs[plotSelect]->name());
            isfftTransfer[plotSelect]=false;
        }
        else
        {
            fftwin->show();
        }
    }
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    fftwin->close();
}
