#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QMap>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>

#include <vector>
#include "./liblsl/include/lsl_cpp.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_stopStreamBtn_clicked();

    void on_startStreamBtn_clicked();

    void on_resolveStreamsBtn_clicked();

    void pull_and_process();

    void on_startCtrBtn_clicked();

    void on_stopCtrlBtn_clicked();

    void on_changeStimBtn_clicked();

private:

    QTimer *timerPull, *timerCtrl;

    QMap<QString,lsl::stream_info> lslStreams;
    lsl::stream_inlet* selected_stream = nullptr;
    std::vector<float> sample{4};

    QLineSeries *seriesLSL, *seriesIno;

    qreal stream_start;
    bool isCtrlON;

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
