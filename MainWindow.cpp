#include <QtGui>
#include <QFileDialog>

#include "MainWindow.h"

#define DEBUG 1

MainWindow::MainWindow()
    : wavFile(0)
{
    QWidget *widget = new QWidget();
    setCentralWidget(widget);
    QWidget *topFiller = new QWidget;
    topFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QWidget *bottomFiller = new QWidget;
    bottomFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    openAct = new QAction(tr("&Open"), this);
    openAct->setShortcut(QKeySequence::Open);
    openAct->setStatusTip("Open an existing file");
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAsAct = new QAction(tr("&Save As"), this);
    saveAsAct->setShortcut(QKeySequence::SaveAs);
    saveAsAct->setStatusTip("Save as");
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAsAct);

    waveform0 = new WaveformWidget(this);
    waveform1 = new WaveformWidget(this);

    scrollBar = new QScrollBar(Qt::Horizontal);
    scrollBar->setFocusPolicy(Qt::StrongFocus);
    connect(scrollBar, SIGNAL(valueChanged(int)), this, SLOT(setScroll(int)));

    wavInfo = new QLabel(this);

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addWidget(waveform0);
    vlayout->addWidget(waveform1);
    vlayout->addWidget(scrollBar);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addLayout(vlayout);
    hlayout->addWidget(wavInfo);

    widget->setLayout(hlayout);

    if (DEBUG)
    {
        const char* filename = "C:\\Users\\brett\\Desktop\\audio\\guitar-44.1k-16bit-stereo.wav";
        loadWavFile(filename);
    }
}

MainWindow::~MainWindow()
{
    delete wavFile;
    wavFile = 0;
}

void MainWindow::open()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), "",
        tr("Wav Files (*.wav)"));
    if (filename != "")
    {
        loadWavFile(filename.toLocal8Bit().constData());
    }
}

void MainWindow::saveAs()
{
    if (wavFile)
    {
        QString filename = QFileDialog::getSaveFileName(this, tr("Save as"), "",
                                                        tr("Wav Files (*.wav)"));
        if (filename != "")
        {
            wavFile->convertToMono();
            wavFile->convertSampleRate();
            wavFile->convertBitDepth();
            wavFile->saveAs(filename.toLocal8Bit().constData());
        }
    }
}

void MainWindow::loadWavFile(const char* filename)
{
    if (wavFile)
    {
        delete wavFile;
    }
    wavFile = new WavFile(filename);
    if (wavFile->good())
    {
        waveform0->setWavFile(wavFile, 0);
        if (wavFile->getNumChannels() > 1)
        {
            waveform1->show();
            waveform1->setWavFile(wavFile, 1);
        }
        else
        {
            waveform1->hide();
        }
        setWindowTitle(filename);
        updateWavInfo();
        scrollBar->setMaximum(wavFile->getNumSamples() / 1000);
    }
    else
    {
        QMessageBox::critical(this, tr("Wav Explorer"),
                             tr("Failed to open WAV file"),
                             QMessageBox::Ok);
    }
}

void MainWindow::updateWavInfo()
{
    QString info;
    info += QString("Audio Format: %1\n").arg(wavFile->getAudioFormat());
    info += QString("Num Channels: %1\n").arg(wavFile->getNumChannels());
    info += QString("Sample Rate: %1\n").arg(wavFile->getSampleRate());
    info += QString("Byte Rate: %1\n").arg(wavFile->getByteRate());
    info += QString("Block Align: %1\n").arg(wavFile->getBlockAlign());
    info += QString("Bits Per Sample: %1\n").arg(wavFile->getBitsPerSample());
    info += QString("Num Samples: %1\n").arg(wavFile->getNumSamples());
    info += QString("Length: %1 seconds\n").arg(wavFile->getNumSamples() / (qreal)wavFile->getSampleRate());
    wavInfo->setText(info);
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    int delta = event->delta() / -120;
    waveform0->zoom(delta);
    waveform1->zoom(delta);
    repaint();
}

void MainWindow::setScroll(int amount)
{
    waveform0->setScroll(amount);
    waveform1->setScroll(amount);
    repaint();
}
