/*
    diecat
    Diesel engines calibration tool.

    File: mainwindow.cpp

    Copyright (C) 2013-2014 Artem Petrov <pa2311@gmail.com>

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
#include "intelhex.hpp"

#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QString>
#include <QVector>
#include <QSharedPointer>
#include <QColor>

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
    prepareInfoTable();

    //

    connect(ui->listWidget_Labels, SIGNAL(currentRowChanged(int)), this, SLOT(itemChanged(int)));
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

    const QFileInfo a2lFileInfo(a2lFileName);
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

    const QFileInfo hexFileInfo(hexFileName);
    m_lastHEXPath = hexFileInfo.absolutePath();

    //

    ui->lineEdit_QuickSearch->clear();
    ui->listWidget_Labels->clear();
    clearTables();
    m_scalars.clear();

    readA2LInfo(a2lFileName);
    readHEXData(hexFileName);
    showLabels();

    ui->groupBox_Labels->setTitle("Labels (" + QString::number(m_scalars.size()) + ")");
}

void MainWindow::on_action_OpenA2L_triggered() {

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

    const QFileInfo a2lFileInfo(a2lFileName);
    m_lastA2LPath = a2lFileInfo.absolutePath();

    //

    ui->lineEdit_QuickSearch->clear();
    ui->listWidget_Labels->clear();
    clearTables();
    m_scalars.clear();

    readA2LInfo(a2lFileName);
    showLabels();

    ui->groupBox_Labels->setTitle("Labels (" + QString::number(m_scalars.size()) + ")");
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
            "Copyright (C) 2013-2014 Artem Petrov "
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

void MainWindow::prepareInfoTable() {

    ui->tableWidget_Description->setRowCount(12);

    ui->tableWidget_Description->setItem(0, 0, new QTableWidgetItem("Name"));
    ui->tableWidget_Description->setItem(0, 1, new QTableWidgetItem(""));
    ui->tableWidget_Description->setItem(1, 0, new QTableWidgetItem("Description"));
    ui->tableWidget_Description->setItem(1, 1, new QTableWidgetItem(""));
    ui->tableWidget_Description->setItem(2, 0, new QTableWidgetItem("Address"));
    ui->tableWidget_Description->setItem(2, 1, new QTableWidgetItem(""));
    ui->tableWidget_Description->setItem(3, 0, new QTableWidgetItem("Length, bytes"));
    ui->tableWidget_Description->setItem(3, 1, new QTableWidgetItem(""));
    ui->tableWidget_Description->setItem(4, 0, new QTableWidgetItem("Type"));
    ui->tableWidget_Description->setItem(4, 1, new QTableWidgetItem(""));
    ui->tableWidget_Description->setItem(5, 0, new QTableWidgetItem("Min value (soft)"));
    ui->tableWidget_Description->setItem(5, 1, new QTableWidgetItem(""));
    ui->tableWidget_Description->setItem(6, 0, new QTableWidgetItem("Max value (soft)"));
    ui->tableWidget_Description->setItem(6, 1, new QTableWidgetItem(""));
    ui->tableWidget_Description->setItem(7, 0, new QTableWidgetItem("Min value (hard)"));
    ui->tableWidget_Description->setItem(7, 1, new QTableWidgetItem(""));
    ui->tableWidget_Description->setItem(8, 0, new QTableWidgetItem("Max value (hard)"));
    ui->tableWidget_Description->setItem(8, 1, new QTableWidgetItem(""));
    ui->tableWidget_Description->setItem(9, 0, new QTableWidgetItem("Read only"));
    ui->tableWidget_Description->setItem(9, 1, new QTableWidgetItem(""));
    ui->tableWidget_Description->setItem(10, 0, new QTableWidgetItem("Signed"));
    ui->tableWidget_Description->setItem(10, 1, new QTableWidgetItem(""));
    ui->tableWidget_Description->setItem(11, 0, new QTableWidgetItem("Dimension"));
    ui->tableWidget_Description->setItem(11, 1, new QTableWidgetItem(""));

    for ( ptrdiff_t i=0; i<ui->tableWidget_Description->rowCount(); i++ ) {

        for ( ptrdiff_t j=0; j<ui->tableWidget_Description->columnCount(); j++ ) {

            ui->tableWidget_Description->item(i, j)->
                    setFlags(ui->tableWidget_Description->item(i, j)->flags() ^ Qt::ItemIsEditable);
        }
    }

    ui->tableWidget_Description->resizeRowsToContents();
    ui->tableWidget_Description->resizeColumnsToContents();
}

void MainWindow::prepareValuesTable(ptrdiff_t vartype) {

    if ( vartype == VARTYPE_SCALAR_NUM ) {

        ui->tableWidget_Values->setRowCount(1);
        ui->tableWidget_Values->setColumnCount(3);

        ui->tableWidget_Values->setItem(0, 0, new QTableWidgetItem(""));
        ui->tableWidget_Values->item(0, 0)->
                setFlags(ui->tableWidget_Values->item(0, 0)->flags() ^ Qt::ItemIsEditable);

        ui->tableWidget_Values->setItem(0, 1, new QTableWidgetItem(""));
        ui->tableWidget_Values->item(0, 1)->setTextColor(QColor(Qt::blue));

        ui->tableWidget_Values->setItem(0, 2, new QTableWidgetItem(""));
        ui->tableWidget_Values->item(0, 2)->
                setFlags(ui->tableWidget_Values->item(0, 2)->flags() ^ Qt::ItemIsEditable);
    }
    else if ( vartype == VARTYPE_SCALAR_VTAB ) {

        //
    }

    ui->tableWidget_Values->resizeRowsToContents();
}

void MainWindow::clearTables() {

    for ( ptrdiff_t i=0; i<ui->tableWidget_Description->rowCount(); i++ ) {
        ui->tableWidget_Description->item(i, 1)->setText("");
    }

    ui->tableWidget_Description->resizeColumnsToContents();

    //

    ui->tableWidget_Values->setRowCount(0);
    ui->tableWidget_Values->setColumnCount(0);
}

void MainWindow::readA2LInfo(const QString &filepath) {

    QSharedPointer<A2L> a2l(new A2L(filepath));

    if ( !a2l->readFile() ) {
        QMessageBox::critical(this, QString(PROGNAME) + ": error", "Error occured during a2l file reading!");
        a2l.clear();
        return;
    }

    a2l->fillScalarsInfo(m_scalars);
    a2l->clear();
}

void MainWindow::readHEXData(const QString &filepath) {

    QSharedPointer<IntelHEX> ihex(new IntelHEX(filepath));

    if ( !ihex->readValues(m_scalars) ) {
        QMessageBox::critical(this, QString(PROGNAME) + ": error", "Error occured during hex file reading!");
    }

    ihex.clear();
}

void MainWindow::showLabels() {

    for ( ptrdiff_t i=0; i<m_scalars.size(); i++ ) {
        ui->listWidget_Labels->addItem(m_scalars[i]->name());
    }
}

void MainWindow::showA2LInfo(int currItemInd) {

    ui->tableWidget_Description->item(0, 1)->setText(m_scalars[currItemInd]->name());
    ui->tableWidget_Description->item(1, 1)->setText(m_scalars[currItemInd]->shortDescription());
    ui->tableWidget_Description->item(2, 1)->setText(m_scalars[currItemInd]->address());
    ui->tableWidget_Description->item(3, 1)->setText(QString::number(m_scalars[currItemInd]->length()));

    if ( m_scalars[currItemInd]->type() == VARTYPE_SCALAR_NUM ) {
        ui->tableWidget_Description->item(4, 1)->setText("Numeric");
    }
    else if ( m_scalars[currItemInd]->type() == VARTYPE_SCALAR_VTAB ) {
        ui->tableWidget_Description->item(4, 1)->setText("VTable");
    }

    ui->tableWidget_Description->item(5, 1)->setText(QString::number(m_scalars[currItemInd]->minValueSoft(), 'f', m_scalars[currItemInd]->precision()));
    ui->tableWidget_Description->item(6, 1)->setText(QString::number(m_scalars[currItemInd]->maxValueSoft(), 'f', m_scalars[currItemInd]->precision()));
    ui->tableWidget_Description->item(7, 1)->setText(QString::number(m_scalars[currItemInd]->minValueHard(), 'f', m_scalars[currItemInd]->precision()));
    ui->tableWidget_Description->item(8, 1)->setText(QString::number(m_scalars[currItemInd]->maxValueHard(), 'f', m_scalars[currItemInd]->precision()));

    if ( m_scalars[currItemInd]->isReadOnly() ) {
        ui->tableWidget_Description->item(9, 1)->setText("true");
    }
    else {
        ui->tableWidget_Description->item(9, 1)->setText("false");
    }

    if ( m_scalars[currItemInd]->isSigned() ) {
        ui->tableWidget_Description->item(10, 1)->setText("true");
    }
    else {
        ui->tableWidget_Description->item(10, 1)->setText("false");
    }

    ui->tableWidget_Description->item(11, 1)->setText(m_scalars[currItemInd]->dimension());

    ui->tableWidget_Description->resizeColumnsToContents();
}

void MainWindow::showHEXValue(int currItemInd, ptrdiff_t vartype) {

    ui->tableWidget_Values->item(0, 0)->setText(m_scalars[currItemInd]->name());

    if ( vartype == VARTYPE_SCALAR_NUM ) {
        ui->tableWidget_Values->item(0, 1)->setText(m_scalars[currItemInd]->value());
    }
    else if ( vartype == VARTYPE_SCALAR_VTAB ) {

        //
    }

    ui->tableWidget_Values->item(0, 2)->setText(m_scalars[currItemInd]->dimension());

    ui->tableWidget_Values->resizeColumnsToContents();
}

void MainWindow::itemChanged(int currItemInd) {

    if ( ui->listWidget_Labels->item(currItemInd) == nullptr ) {
        return;
    }

    showA2LInfo(currItemInd);

    prepareValuesTable(VARTYPE_SCALAR_NUM);
    showHEXValue(currItemInd, VARTYPE_SCALAR_NUM);
}
