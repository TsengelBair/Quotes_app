#include <QDebug>
#include <QFile>

#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->treeWidget->setHeaderLabel("Countries");
    ui->horizontalLayout_2->setStretch(0, 2);
    ui->horizontalLayout_2->setStretch(1, 3);

    ui->pb_show_hide->setIcon(QIcon(":/icons/arrow_top.png"));
    ui->pb_add_country->setIcon(QIcon(":/icons/globe.png"));

    loadStyles();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadStyles()
{
    QFile f(":/styles/style.qss");
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Не открылся style.qss:" << f.errorString();
        return;
    }

    const QString qss = QString::fromUtf8(f.readAll());
    setStyleSheet(qss);
}

