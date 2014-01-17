/*
    diecat
    A2L/HEX file reader.

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
#include "comboboxitemdelegate.hpp"
#include "labelinfodialog.hpp"

#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QString>
#include <QVector>
#include <QSharedPointer>
#include <QColor>
#include <QComboBox>
#include <QItemDelegate>
#include <QTableWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_labelInfoDialog(new LabelInfoDialog(this)),
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
    m_scalars.clear();

    readA2LInfo(a2lFileName);
    showLabels();

    ui->groupBox_Labels->setTitle("Labels (" + QString::number(m_scalars.size()) + ")");
}

void MainWindow::on_action_SaveChangesInHex_triggered() {

    //
}

void MainWindow::on_action_JumpToSearchLine_triggered() {

    ui->lineEdit_QuickSearch->setFocus();
}

void MainWindow::on_action_Select_triggered() {

    ui->listWidget_Labels->currentItem()->setBackgroundColor(QColor(Qt::red));
    addParameterToTable();
    ui->listWidget_Labels->setCurrentRow(ui->listWidget_Labels->currentRow() + 1);
}

void MainWindow::on_action_Unselect_triggered() {

    ui->listWidget_Labels->currentItem()->setBackgroundColor(QColor(Qt::white));
    deleteParameterFromTable();
    ui->listWidget_Labels->setCurrentRow(ui->listWidget_Labels->currentRow() + 1);
}

void MainWindow::on_action_SelectAll_triggered() {

    //
}

void MainWindow::on_action_UnselectAll_triggered() {

    //
}

void MainWindow::on_action_LabelInfo_triggered() {

    QTableWidget *tableWidget_Description =
            m_labelInfoDialog->findChild<QTableWidget *>("tableWidget_Description");
    ptrdiff_t currItemInd = ui->listWidget_Labels->currentRow();

    tableWidget_Description->item(0, 1)->setText(m_scalars[currItemInd]->name());
    tableWidget_Description->item(1, 1)->setText(m_scalars[currItemInd]->shortDescription());
    tableWidget_Description->item(2, 1)->setText(m_scalars[currItemInd]->address());
    tableWidget_Description->item(3, 1)->setText(m_scalars[currItemInd]->numType());

    if ( m_scalars[currItemInd]->type() == VARTYPE_SCALAR_NUM ) {
        tableWidget_Description->item(4, 1)->setText("Numeric");
    }
    else if ( m_scalars[currItemInd]->type() == VARTYPE_SCALAR_VTAB ) {
        tableWidget_Description->item(4, 1)->setText("VTable");
    }

    tableWidget_Description->item(5, 1)->setText(QString::number(m_scalars[currItemInd]->minValueSoft(), 'f', m_scalars[currItemInd]->precision()));
    tableWidget_Description->item(6, 1)->setText(QString::number(m_scalars[currItemInd]->maxValueSoft(), 'f', m_scalars[currItemInd]->precision()));
    tableWidget_Description->item(7, 1)->setText(QString::number(m_scalars[currItemInd]->minValueHard(), 'f', m_scalars[currItemInd]->precision()));
    tableWidget_Description->item(8, 1)->setText(QString::number(m_scalars[currItemInd]->maxValueHard(), 'f', m_scalars[currItemInd]->precision()));

    if ( m_scalars[currItemInd]->isReadOnly() ) {
        tableWidget_Description->item(9, 1)->setText("true");
    }
    else {
        tableWidget_Description->item(9, 1)->setText("false");
    }

    tableWidget_Description->item(10, 1)->setText(m_scalars[currItemInd]->dimension());

    tableWidget_Description->resizeColumnsToContents();

    //

    m_labelInfoDialog->show();
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
            + "A2L/HEX file reader.<br><br>"
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

void MainWindow::addParameterToTable() {

    ptrdiff_t currInd = ui->listWidget_Labels->currentRow();

    for ( ptrdiff_t i=0; i<ui->tableWidget_Values->rowCount(); i++ ) {

        if ( ui->tableWidget_Values->item(i, 0)->text().toInt() == currInd ) {
            return;
        }
    }

    ptrdiff_t varType = m_scalars[currInd]->type();

    ptrdiff_t tblRow = ui->tableWidget_Values->rowCount();
    ui->tableWidget_Values->setRowCount(tblRow + 1);

    if ( varType == VARTYPE_SCALAR_NUM ) {

        //ui->tableWidget_Values->setItemDelegate(new QItemDelegate());

        ui->tableWidget_Values->setColumnCount(4);

        ui->tableWidget_Values->setItem(tblRow, 0, new QTableWidgetItem(QString::number(currInd)));
        ui->tableWidget_Values->item(tblRow, 0)->
                setFlags(ui->tableWidget_Values->item(tblRow, 0)->flags() ^ Qt::ItemIsEditable);

        ui->tableWidget_Values->setItem(tblRow, 1, new QTableWidgetItem(m_scalars[currInd]->name()));
        ui->tableWidget_Values->item(tblRow, 1)->
                setFlags(ui->tableWidget_Values->item(tblRow, 1)->flags() ^ Qt::ItemIsEditable);

        ui->tableWidget_Values->setItem(tblRow, 2, new QTableWidgetItem(m_scalars[currInd]->value()));
        ui->tableWidget_Values->item(tblRow, 2)->setTextColor(QColor(Qt::blue));

        ui->tableWidget_Values->setItem(tblRow, 3, new QTableWidgetItem(m_scalars[currInd]->dimension()));
        ui->tableWidget_Values->item(tblRow, 3)->
                setFlags(ui->tableWidget_Values->item(tblRow, 3)->flags() ^ Qt::ItemIsEditable);
    }
    else if ( varType == VARTYPE_SCALAR_VTAB ) {

        //ui->tableWidget_Values->setItemDelegate(new ComboBoxItemDelegate(ui->tableWidget_Values));

        ui->tableWidget_Values->setColumnCount(4);

        ui->tableWidget_Values->setItem(tblRow, 0, new QTableWidgetItem(QString::number(currInd)));
        ui->tableWidget_Values->item(tblRow, 0)->
                setFlags(ui->tableWidget_Values->item(tblRow, 0)->flags() ^ Qt::ItemIsEditable);

        ui->tableWidget_Values->setItem(tblRow, 1, new QTableWidgetItem(m_scalars[currInd]->name()));
        ui->tableWidget_Values->item(tblRow, 1)->
                setFlags(ui->tableWidget_Values->item(tblRow, 1)->flags() ^ Qt::ItemIsEditable);

        m_comboBox_vTable = new QComboBox(ui->tableWidget_Values);
        m_comboBox_vTable->setMinimumWidth(230);
        ui->tableWidget_Values->setCellWidget(tblRow, 2, m_comboBox_vTable);

        m_comboBox_vTable->clear();
        m_comboBox_vTable->addItems(m_scalars[currInd]->vTable());
        m_comboBox_vTable->setCurrentIndex(m_scalars[currInd]->value().toInt());

        ui->tableWidget_Values->setItem(tblRow, 3, new QTableWidgetItem(m_scalars[currInd]->dimension()));
        ui->tableWidget_Values->item(tblRow, 3)->
                setFlags(ui->tableWidget_Values->item(tblRow, 3)->flags() ^ Qt::ItemIsEditable);
    }

    ui->tableWidget_Values->resizeRowsToContents();
    ui->tableWidget_Values->resizeColumnsToContents();
}

void MainWindow::deleteParameterFromTable() {

    ptrdiff_t currInd = ui->listWidget_Labels->currentRow();

    for ( ptrdiff_t i=0; i<ui->tableWidget_Values->rowCount(); i++ ) {

        if ( ui->tableWidget_Values->item(i, 0)->text().toInt() == currInd ) {
            ui->tableWidget_Values->removeRow(i);
        }
    }
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
