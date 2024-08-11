#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "audiotranscriber.h"
#include <QMainWindow>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pb_selectModel_clicked();

    void on_pb_transcribe_clicked();

    void on_pb_close_clicked();

    void updateProgressBar();

    void on_pb_selectFile_clicked();

    void on_pb_selectWhisperPath_clicked();

    void on_le_whisperPath_editingFinished();

    void on_le_currentModel_editingFinished();

    void on_le_currentFile_editingFinished();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    AudioTranscriber *transcriber = new AudioTranscriber();
};
#endif // MAINWINDOW_H
