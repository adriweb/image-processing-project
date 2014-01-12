#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    hasOpenedFile = false;

    ui->setupUi(this);
    this->setWindowTitle("Projet Traitement d'images - Adrien Bertrand");

    connect(ui->actionA_propos, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->actionOuvrir_image_source, SIGNAL(triggered()), this, SLOT(openSourceImageFromFileChooser()));
    connect(ui->actionR_initialiser_l_image_modifi_e, SIGNAL(triggered()), this, SLOT(resetDest()));
    connect(ui->actionQuitter, SIGNAL(triggered()), this, SLOT(close()));

    connect(ui->actionHistogramme, SIGNAL(triggered()), this, SLOT(outilChosen()));
    connect(ui->actionNiveau_de_gris, SIGNAL(triggered()), this, SLOT(outilChosen()));
    connect(ui->actionN_gatif, SIGNAL(triggered()), this, SLOT(outilChosen()));

    connect(ui->scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->scrollArea_2->verticalScrollBar(), SLOT(setValue(int)));
    connect(ui->scrollArea_2->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->scrollArea->verticalScrollBar(), SLOT(setValue(int)));
    connect(ui->scrollArea->horizontalScrollBar(), SIGNAL(valueChanged(int)), ui->scrollArea_2->horizontalScrollBar(), SLOT(setValue(int)));
    connect(ui->scrollArea_2->horizontalScrollBar(), SIGNAL(valueChanged(int)), ui->scrollArea->horizontalScrollBar(), SLOT(setValue(int)));

    connect(ui->action3x3, SIGNAL(triggered()), this, SLOT(actionChosen()));
    connect(ui->action5x5, SIGNAL(triggered()), this, SLOT(actionChosen()));
    connect(ui->actionPersonnalis, SIGNAL(triggered()), this, SLOT(actionChosen()));
    connect(ui->actionClosing, SIGNAL(triggered()), this, SLOT(actionChosen()));
    connect(ui->actionOpening, SIGNAL(triggered()), this, SLOT(actionChosen()));
    connect(ui->actionDilatation, SIGNAL(triggered()), this, SLOT(actionChosen()));
    connect(ui->actionErosion, SIGNAL(triggered()), this, SLOT(actionChosen()));
    connect(ui->actionLaplacien_4, SIGNAL(triggered()), this, SLOT(actionChosen()));
    connect(ui->actionLaplacien_8, SIGNAL(triggered()), this, SLOT(actionChosen()));
    connect(ui->actionMoyenneur, SIGNAL(triggered()), this, SLOT(actionChosen()));
    connect(ui->actionM_dian, SIGNAL(triggered()), this, SLOT(actionChosen()));
    connect(ui->actionPrewitt, SIGNAL(triggered()), this, SLOT(actionChosen()));
    connect(ui->actionSobel, SIGNAL(triggered()), this, SLOT(actionChosen()));
    connect(ui->actionRoberts, SIGNAL(triggered()), this, SLOT(actionChosen()));
    connect(ui->actionSeuil_manuel, SIGNAL(triggered()), this, SLOT(actionChosen()));
    connect(ui->actionSeuil_automatique, SIGNAL(triggered()), this, SLOT(actionChosen()));

    connect(ui->actionRectangle_englobant, SIGNAL(triggered()), this, SLOT(actionChosen()));
    //connect(ui->actionR_duction_de_la_palette, SIGNAL(triggered()), this, SLOT(actionChosen()));
    connect(ui->actionHistogramme_cumul, SIGNAL(triggered()), this, SLOT(outilChosen()));
    connect(ui->actionLancer_toutes_les_actions, SIGNAL(triggered()), this, SLOT(actionChosen()));
    connect(ui->actionEgalisation_d_histogramme, SIGNAL(triggered()), this, SLOT(actionChosen()));
    connect(ui->actionWatershed, SIGNAL(triggered()), this, SLOT(actionChosen()));

    setAcceptDrops(true);

    wrapper = new WrapperCPP();
}

bool hasEnding (std::string const &fullString, std::string const &ending)
{
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    QString fileName = urls.first().toLocalFile();
    if (fileName.isEmpty())
        return;

    std::string ext(".bmp");
    if (hasEnding(fileName.toStdString(), ext))
        openSourceImage(fileName);
}

void MainWindow::outilChosen()
{
    if (!hasOpenedFile) {
        QMessageBox::about(this, tr("Erreur"), tr("<p>Veuillez ouvrir une image avant !</p>"));
        return;
    }

    QObject* menuItem = sender();

    previewWindow = new PreviewWindow(this);

    if (menuItem == ui->actionHistogramme) {
        wrapper->choix(4);
        previewWindow->setWindowTitle("Histogramme");
    } else if (menuItem == ui->actionHistogramme_cumul) {
        wrapper->choix(72);
        previewWindow->setWindowTitle("Histogramme Cumulé");
    } else if (menuItem == ui->actionNiveau_de_gris) {
        wrapper->choix(2);
        previewWindow->setWindowTitle("Niveau de gris");
    } else if (menuItem == ui->actionN_gatif) {
        wrapper->choix(1);
        previewWindow->setWindowTitle("Négatif");
    } else {
        std::cerr << "Erreur de signal/slot sur outil" << std::endl;
    }

    QImage img(QString(wrapper->getLatestNewImage()));
    previewWindow->imgLabel->setPixmap(QPixmap::fromImage(img));
    previewWindow->imgLabel->resize(img.size());

    previewWindow->show();
}

void MainWindow::actionChosen()
{
    if (!hasOpenedFile) {
        QMessageBox::about(this, tr("Erreur"), tr("<p>Veuillez ouvrir une image avant !</p>"));
        return;
    }

    QObject* menuItem = sender();

    if (menuItem == ui->action3x3)
        wrapper->choix(622);
    else if (menuItem == ui->action5x5)
        wrapper->choix(623);
    else if (menuItem == ui->actionClosing)
        wrapper->choix(614);
    else if (menuItem == ui->actionOpening)
        wrapper->choix(613);
    else if (menuItem == ui->actionDilatation)
        wrapper->choix(611);
    else if (menuItem == ui->actionErosion)
        wrapper->choix(612);
    else if (menuItem == ui->actionLaplacien_4)
        wrapper->choix(64);
    else if (menuItem == ui->actionLaplacien_8)
        wrapper->choix(65);
    else if (menuItem == ui->actionMoyenneur)
        wrapper->choix(621);
    else if (menuItem == ui->actionM_dian)
        wrapper->choix(66);
    else if (menuItem == ui->actionPrewitt)
        wrapper->choix(632);
    else if (menuItem == ui->actionSobel)
        wrapper->choix(633);
    else if (menuItem == ui->actionRoberts)
        wrapper->choix(631);
    else if (menuItem == ui->actionEgalisation_d_histogramme)
        wrapper->choix(73);
    else if (menuItem == ui->actionRectangle_englobant)
        wrapper->choix(74);
    /*else if (menuItem == ui->actionR_duction_de_la_palette) {
        bool ok;
        QString text = QInputDialog::getText(this, tr("Nombre de couleurs "),
                                           tr("Nombre entier positif < 255"), QLineEdit::Normal,
                                           "128", &ok);
        if (ok && !text.isEmpty() && text.toInt() > 0 && text.toInt() < 255) {
            wrapper->doPaletteReduction(text.toInt());
        }
    }*/
    else if (menuItem == ui->actionLancer_toutes_les_actions) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Projet de traitement d'images", "Lancer toutes les actions possibles ? (quelques secondes)",
                                    QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            QTime myTimer;
            myTimer.start();
            wrapper->choix(-1);

            QMessageBox::information(this, tr("Mini-Projet Tdi"), tr("Terminé (%1 s.)").arg((float)(myTimer.elapsed())/1000.0f));
            resetDest();
        }
        return;
    } else if (menuItem == ui->actionWatershed) {
        wrapper->choix(8);
    } else if (menuItem == ui->actionPersonnalis) {
        bool ok;
        QString text = QInputDialog::getText(this, tr("Rayon du filtre gaussien"),
                                           tr("Nombre entier positif impair < 50"), QLineEdit::Normal,
                                           "9", &ok);
        if (ok && !text.isEmpty() && text.toInt() > 0 && text.toInt() < 50 && text.toInt() %2 != 0) {
            wrapper->setTailleGaussien(text.toInt());
            wrapper->choix(624);
        }

    } else if (menuItem == ui->actionSeuil_manuel) {
        bool ok;
        QString text = QInputDialog::getText(this, tr("Valeur du seuil"),
                                           tr("Nombre entier positif [0-255]"), QLineEdit::Normal,
                                           "100", &ok);
        if (ok && !text.isEmpty() && text.toInt() >= 0 && text.toInt() <= 255) {
            wrapper->setManSeuil(text.toInt());
            wrapper->choix(3);
        }
    } else if (menuItem == ui->actionSeuil_automatique) {
        wrapper->choix(5);
    } else {
        std::cerr << "Erreur de signal/slot sur outil" << std::endl;
    }

    QImage img(QString(wrapper->getLatestNewImage()));
    ui->modifiedImage->setPixmap(QPixmap::fromImage(img));
}

void MainWindow::resetDest()
{
    const QPixmap* src = ui->sourceImage->pixmap();
    if (src) ui->modifiedImage->setPixmap(*src);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("Projet TdI"), tr("<p>Ce mini-projet a été fait par Adrien Bertrand.<br/><i>Version : 4.5 (12/01/2014)</i></p>"));
}

void MainWindow::openSourceImage(QString& fileName)
{
    fileName = QDir::toNativeSeparators(fileName);

    QImage image(fileName);
    if (image.isNull()) {
        QMessageBox::information(this, tr("Projet Tdi"), tr("Impossible d'ouvrir %1.").arg(fileName));
        return;
    }

    if (hasOpenedFile) {
        wrapper->freeCData();
        delete wrapper;
        if (previewWindow) {
            previewWindow->close();
            delete previewWindow;
        }
        wrapper = new WrapperCPP();
    }

    wrapper->openFile(fileName.toStdString());
    ui->sourceImage->setPixmap(QPixmap::fromImage(image));
    resetDest();
    hasOpenedFile = true;
}

void MainWindow::openSourceImageFromFileChooser()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Ouvrir une image"), QDir::currentPath());
    if (!fileName.isEmpty())
        openSourceImage(fileName);
}

MainWindow::~MainWindow()
{
    if (hasOpenedFile) wrapper->freeCData();
    delete wrapper;
    if (previewWindow) {
        previewWindow->close();
        delete previewWindow;
    }
    delete ui;
}
