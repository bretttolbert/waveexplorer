#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>

#include "WaveformWidget.h"
#include "WavFile.h"

class QMenu;
class QAction;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();
    void wheelEvent(QWheelEvent *);
private slots:
    void open();
    void saveAs();
    void setScroll(int amount);
private:
    void loadWavFile(const char* filename);
    void updateWavInfo();
    QMenu *fileMenu;
    QAction *openAct;
    QAction *saveAsAct;
    WavFile *wavFile;
    QWidget *centralWidget;
    QWidget *topFiller;
    QWidget *bottomFiller;
    WaveformWidget *waveform0;
    WaveformWidget *waveform1;
    QScrollBar *scrollBar;
    QLabel *wavInfo;
};

#endif // MAINWINDOW_H
