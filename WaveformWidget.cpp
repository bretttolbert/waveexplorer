#include "WaveformWidget.h"
#include <cmath>
#include <QPainter>
#include <QApplication>
#include <QtCore/qmath.h>

WaveformWidget::WaveformWidget(QWidget *parent)
    : QWidget(parent),
      wavFile(0),
      channel(0),
      zoomLevel(150),
      scroll(0)
{
    setFixedSize(1000, 400);
}

void WaveformWidget::setWavFile(WavFile *wavFile, int channel)
{
    this->wavFile = wavFile;
    this->channel = channel;
    repaint();
}

void WaveformWidget::paintEvent(QPaintEvent *event)
{
    if (wavFile && wavFile->good())
    {
        QPainter painter;
        painter.begin(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::black);
        QBrush background = QBrush(QColor(0xbb, 0xbb, 0xbb));
        QFont font("Monospace");
        font.setStyleHint(QFont::TypeWriter);
        font.setPixelSize(16);
        painter.setFont(font);
        painter.fillRect(QRect(0,0,width(),height()), background);

        QBrush greenBrush = QBrush(QColor(0x0, 0xff, 0x0));
        int steps = 1000;
        int stepWidthPx = width() / steps;
        INT16 maxSampleVal = 0;
        int x = scroll * 1000;
        for (int i=0; i<steps; i++)
        {
            //qDebug("numSamples=%d x=%d", wavFile->getNumSamples(), x);
            if (x > wavFile->getNumSamples())
            {
                break;
            }
            if (wavFile->getBitsPerSample() == 16)
            {
                INT16 sample;
                wavFile->getSample(channel, x, (char*)&sample);
                if (sample > maxSampleVal)
                {
                    maxSampleVal = sample;
                }
                //qDebug("sample=%d", sample);
                qreal scaling = (height()/2) / qPow(2,wavFile->getBitsPerSample()-3);
                int sampleHeightPx = static_cast<int>(floor(sample * scaling + 0.5));
                //qDebug("sampleHeightPx is %d", sampleHeightPx);

                painter.fillRect(QRect(i*stepWidthPx, height()/2-sampleHeightPx, stepWidthPx, sampleHeightPx), greenBrush);
            }
            else if (wavFile->getBitsPerSample() == 8)
            {
                char sample;
                wavFile->getSample(channel, x, (char*)&sample);
                int sampleHeightPx = sample;
                painter.fillRect(QRect(i*stepWidthPx, height()/2-sampleHeightPx, stepWidthPx, sampleHeightPx), greenBrush);
            }
            x += zoomLevel;
        }
        //qDebug("maxSampleVal=%d",maxSampleVal);

        //draw center line
        QBrush centerLineBrush = QBrush(QColor(0xff, 0, 0));
        painter.fillRect(QRect(0, height()/2, width(), 1), centerLineBrush);

        painter.end();
    }
}

void WaveformWidget::zoom(int amount)
{
    zoomLevel += amount;
    if (zoomLevel <= 0)
    {
        zoomLevel = 1;
    }
    qDebug("zoomLevel=%d", zoomLevel);
}

void WaveformWidget::setScroll(int amount)
{
    scroll = amount;
}
