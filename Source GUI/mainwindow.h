#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <map>
#include <QMainWindow>
#include "previewwindow.h"
#include "wrappercpp.h"
#include <QScrollBar>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public slots:
    void about();
    void openSourceImageFromFileChooser();
    void actionChosen();
    void outilChosen();
    void resetDest();

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void openSourceImage(QString &fileName);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private:
    Ui::MainWindow *ui;
    WrapperCPP* wrapper;
    PreviewWindow *previewWindow;
    bool hasOpenedFile;

};

#endif // MAINWINDOW_H
