#ifndef WAVEFORM_WIDGET_H
#define WAVEFORM_WIDGET_H

#include <QWidget>
#include <QBrush>
#include <QFont>
#include <QPen>
#include "WavFile.h"

class QPainter;

class WaveformWidget : public QWidget
{
        Q_OBJECT
public:
    WaveformWidget(QWidget *parent=0);
    void setWavFile(WavFile* wavFile, int channel);
    void zoom(int amount);
    void setScroll(int amount);
protected:
    void paintEvent(QPaintEvent *event);
    WavFile *wavFile;
    int channel;
    int zoomLevel;
    int scroll;
};

#endif // WAVEFORM_WIDGET_H
