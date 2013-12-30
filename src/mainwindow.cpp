/*
    diecat
    Diesel engines calibration tool.

    File: mainwindow.cpp

    Copyright (C) 2013 Artem Petrov <pa2311@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "constants.hpp"
#include "a2l.hpp"
#include "ecuscalar.hpp"

#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QString>
#include <QVector>
#include <QSharedPointer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_progSettings("pa23software", PROGNAME) {

    ui->setupUi(this);

    //

    QCoreApplication::setApplicationName(QString(PROGNAME));
    QCoreApplication::setApplicationVersion(QString(PROGVER));

    this->setWindowTitle(QString(PROGNAME) + " v" + QString(PROGVER));

    //

    readProgramSettings();
}

MainWindow::~MainWindow() {

    writeProgramSettings();

    delete ui;
}

void MainWindow::on_action_OpenProject_triggered() {

    const QString a2lFileName(
                QFileDialog::getOpenFileName(
                    this,
                    tr("Open a2l file..."),
                    m_lastA2LPath,
                    QString::fromLatin1("a2l files (*.a2l);;All files (*)"),
                    0, 0)
                );

    if ( a2lFileName.isEmpty() ) {
        return;
    }

    QFileInfo a2lFileInfo(a2lFileName);
    m_lastA2LPath = a2lFileInfo.absolutePath();

    const QString hexFileName(
                QFileDialog::getOpenFileName(
                    this,
                    tr("Open hex file..."),
                    m_lastHEXPath,
                    QString::fromLatin1("hex files (*.hex);;All files (*)"),
                    0, 0)
                );

    if ( hexFileName.isEmpty() ) {
        return;
    }

    QFileInfo hexFileInfo(hexFileName);
    m_lastHEXPath = hexFileInfo.absolutePath();

    //

    ui->lineEdit_QuickSearch->clear();
    ui->listWidget_Labels->clear();
    ui->tableWidget_ValuesEditor->clear();
    m_scalars.clear();

    readA2LInfo(a2lFileName);
    showData();
}

void MainWindow::on_action_SaveChangesInHex_triggered() {

    //
}

void MainWindow::on_action_Undo_triggered() {

    //
}

void MainWindow::on_action_Redo_triggered() {

    //
}

void MainWindow::on_action_ResetSelections_triggered() {

    //
}

void MainWindow::on_action_ResetAllChanges_triggered() {

    //
}

void MainWindow::on_action_About_triggered() {

    const QString str =
            "<b>" + QString(PROGNAME) + " v" + QString(PROGVER) + "</b> "
            + "(Date of build: " + QString(__DATE__) + ")<br>"
            + "Diesel engines calibration tool.<br><br>"
            "Copyright (C) 2013 Artem Petrov "
            "<a href=\"mailto:pa2311@gmail.com\">pa2311@gmail.com</a><br><br>"
            "Source code hosting: "
            "<a href=\"https://github.com/pa23/diecat\">https://github.com/pa23/diecat</a><br>"
            "Author's blog (RU): "
            "<a href=\"http://pa2311.blogspot.com\">http://pa2311.blogspot.com</a><br><br>"
            "This program is free software: you can redistribute it and/or modify "
            "it under the terms of the GNU General Public License as "
            "published by the Free Software Foundation, either version 3 of the License, or "
            "(at your option) any later version.<br>"
            "This program is distributed in the hope that it will be useful, "
            "but WITHOUT ANY WARRANTY; without even the implied warranty of "
            "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
            "GNU General Public License for more details.<br>"
            "You should have received a copy of the GNU General Public License "
            "along with this program. If not, see <a href=\"http://www.gnu.org/licenses/\">"
            "http://www.gnu.org/licenses/</a>.<br>";

    QMessageBox::about(this, "About " + QString(PROGNAME), str);
}

void MainWindow::writeProgramSettings() {

    m_progSettings.beginGroup("/settings");
    m_progSettings.setValue("/window_geometry", geometry());
    m_progSettings.setValue("/last_a2l_path", m_lastA2LPath);
    m_progSettings.setValue("/last_hex_path", m_lastHEXPath);
    m_progSettings.endGroup();
}

void MainWindow::readProgramSettings() {

    m_progSettings.beginGroup("/settings");
    setGeometry(m_progSettings.value("/window_geometry", QRect(20, 40, 0, 0)).toRect());
    m_lastA2LPath = m_progSettings.value("/last_a2l_path", QDir::currentPath()).toString();
    m_lastHEXPath = m_progSettings.value("/last_hex_path", QDir::currentPath()).toString();
    m_progSettings.endGroup();
}

void MainWindow::readA2LInfo(const QString &filepath) {

    QSharedPointer<A2L> a2l(new A2L(filepath));

    if ( !a2l->readFile() ) {

        QMessageBox::critical(this, QString(PROGNAME) + ": error", "Error occured during a2l file reading!");
        a2l.clear();
        return;
    }

    a2l->fillScalarsInfo(m_scalars);
}

void MainWindow::showData() {

    for ( ptrdiff_t i=0; i<m_scalars.size(); i++ ) {

        ui->listWidget_Labels->addItem(m_scalars[i]->name());
    }
}
