#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    transcriber->setWhisperPath(this->ui->le_whisperPath->text().toStdString());
    transcriber->setModel(this->ui->le_currentModel->text().toStdString());
    transcriber->setAudioFile(this->ui->le_currentFile->text().toStdString());

    this->timer = new QTimer(this);
    // Conecta la señal timeout() del temporizador a tu función
    connect(timer, &QTimer::timeout, this, &MainWindow::updateProgressBar);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pb_selectWhisperPath_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open main whispher executable"),
                                                    ".",
                                                    tr("Whispher main"));
    this->ui->le_whisperPath->setText(fileName);
    transcriber->setWhisperPath(fileName.toStdString());
}

void MainWindow::on_pb_selectModel_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Model"),
                                                    "models",
                                                    tr("Model file (*.bin)"));
    this->ui->le_currentModel->setText(fileName);
    transcriber->setModel(fileName.toStdString());
}

void MainWindow::on_pb_selectFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open audio file"),
                                                    "samples",
                                                    tr("Audio file (*.wav)"));
    this->ui->le_currentFile->setText(fileName);
    transcriber->setAudioFile(fileName.toStdString());
}

void MainWindow::on_pb_transcribe_clicked()
{

    double fileDuration = transcriber->getWavDuration(transcriber->getAudioFile());
    this->ui->progressBar->setRange(0,fileDuration);

    // Inicia el temporizador. El intervalo se establece en milisegundos (1000 ms = 1 segundo).
    timer->start(1000);

    transcriber->start();
}


void MainWindow::on_pb_close_clicked()
{
    timer->stop();
    transcriber->killProcess();
    transcriber->join();
    close();
}

void MainWindow::updateProgressBar()
{
    this->ui->progressBar->setValue(transcriber->getProgress());
}


void MainWindow::on_le_whisperPath_editingFinished()
{
    transcriber->setWhisperPath(this->ui->le_whisperPath->text().toStdString());
}


void MainWindow::on_le_currentModel_editingFinished()
{
    transcriber->setModel(this->ui->le_currentModel->text().toStdString());
}


void MainWindow::on_le_currentFile_editingFinished()
{
    transcriber->setAudioFile(this->ui->le_currentFile->text().toStdString());
}

