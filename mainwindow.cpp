#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "stimclicker.h"
#include "./liblsl/include/lsl_cpp.h"

#include <QDebug>
#include <QMap>
#include <QDateTime>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QLegendMarker>

// Globals:
qreal plot_time_s = 5.0;
StimClicker *clicker = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , seriesLSL( nullptr )
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Connect and start timer:
    timerPull = new  QTimer(this);
    connect(timerPull,SIGNAL(timeout()),this,SLOT(pull_and_process()));
    timerPull->start(18); // Run pull_and_process every 18 ms (slightly faster than control signal, so that samples don't accummulate in buffer)

   /* timerCtrl = new QTimer(this);
    connect(timerCtrl,SIGNAL(timeout()),this,SLOT(pull_and_process()));
    timerCtrl->start(20); // Run pull_and_process every 20 ms */



    //chartView->setRenderHint(QPainter::Antialiasing);
    QChart* chart = ui->signalsWidget->chart();
    ui->signalsWidget->setRenderHint(QPainter::Antialiasing);

    seriesLSL = new QLineSeries();
    seriesLSL->setOpacity(0.3);
    seriesIno = new QLineSeries();
    seriesIno->setColor(Qt::red);
    seriesIno->setOpacity(0.3);

    // Init Chart:
    chart->addSeries(seriesLSL);
    chart->addSeries(seriesIno);
    chart->createDefaultAxes();
    chart->axes(Qt::Vertical).constFirst()->setRange(-0.1f,1.2f);
    chart->setTitle("Control Outputs");

    chart->legend()->markers(seriesLSL)[0]->setLabel("Incoming LSL Stream");
    chart->legend()->markers(seriesIno)[0]->setLabel("Stimulator State");

    clicker = new StimClicker();
    isCtrlON = false;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete seriesLSL;
    delete seriesIno;

    delete clicker;
}


void MainWindow::pull_and_process()
{
    // If no stream, don't do anything.
    if (!selected_stream) return;

    auto capture_time = selected_stream->pull_sample(sample,0.0);
    // if no sample was available:
//    qDebug() << capture_time;
//    qDebug() << sample;

    if (capture_time==0.0)return;

    qreal t_rel = ((qreal)QDateTime::currentMSecsSinceEpoch() / (qreal)1000.0f) - stream_start;

    seriesLSL->append(QPointF(t_rel, sample[0]-0.1));

    auto points = seriesLSL->points();
//    points.append(QPointF(t_rel, sample[0]-0.1));

    while (points.at(0).x()<(t_rel-plot_time_s)){
        points.pop_front();
    }

    seriesLSL->replace(points);

    ui->signalsWidget->chart()->axes(Qt::Horizontal).takeAt(0)->setRange(points.at(0).x(), (float)t_rel);

    if (isCtrlON) {
        // Ctrl Inomed:
        float stimChanged;
        bool ctrlSig = sample[0]>0.5;
        bool isStimON = clicker->isStimON();
        stimChanged = ((float)isStimON + 0.2 )/1.5f;
        if (ctrlSig != isStimON) {clicker->sendClicks(); stimChanged = ((float)ctrlSig + 0.2 )/1.5f;}

        seriesIno->append(t_rel, stimChanged);
        points = seriesIno->points();

        while (points.at(0).x()<(t_rel-plot_time_s)){
            points.pop_front();
        }
        seriesIno->replace(points);
    }

}


void MainWindow::on_stopStreamBtn_clicked()
{
    selected_stream = nullptr;
}


void MainWindow::on_startStreamBtn_clicked()
{
    seriesLSL->clear();
    seriesIno->clear();

    auto item = ui->streamList->currentItem();
    if (!item) return;
    QString selected_name = item->text();
    selected_stream = new lsl::stream_inlet(lslStreams[selected_name], 2048);
    selected_stream->open_stream();

    stream_start = (qreal)QDateTime::currentMSecsSinceEpoch() / (qreal)1000.0f;

}


void MainWindow::on_resolveStreamsBtn_clicked()
{
    std::vector<lsl::stream_info> results = lsl::resolve_streams();

    ui->streamList->clear();
    lslStreams.clear();
    for (lsl::stream_info& info : results) {
        QString name = QString(info.name().c_str());
        lslStreams.insert(name,info);
        ui->streamList->addItem(QString(info.name().c_str()));
    }
}


void MainWindow::on_startCtrBtn_clicked()
{
    isCtrlON = true;
}


void MainWindow::on_stopCtrlBtn_clicked()
{
    isCtrlON = false;

    if (clicker->isStimON()){clicker->sendClicks();}
}

void MainWindow::on_changeStimBtn_clicked()
{

    isCtrlON = clicker->isStimON();

    clicker->sendClicks();

    isCtrlON = !isCtrlON;

}

