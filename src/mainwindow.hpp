/*
    diecat
    A2L/HEX file reader.

    File: mainwindow.hpp

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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QString>
#include <QSettings>
#include <QDir>
#include <QComboBox>

#include "ecuscalar.hpp"
#include "labelinfodialog.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_action_OpenProject_triggered();
    void on_action_OpenA2L_triggered();
    void on_action_SearchLine_triggered();
    void on_action_Select_triggered();
    void on_action_Unselect_triggered();
    void on_action_LabelInfo_triggered();
    void on_action_About_triggered();

    void searchTemplChanged(QString);

private:
    Ui::MainWindow *ui;
    LabelInfoDialog *m_labelInfoDialog;

    QComboBox *m_comboBox_vTable;

    QString m_lastA2LPath = QDir::currentPath();
    QString m_lastHEXPath = QDir::currentPath();
    QSettings m_progSettings;
    QVector< QSharedPointer<ECUScalar> > m_scalars;
    QVector<bool> m_scalarsInTable;

    void writeProgramSettings();
    void readProgramSettings();

    void addParameterToTable(ptrdiff_t);
    void deleteParameterFromTable(ptrdiff_t);

    void readA2LInfo(const QString &);
    void readHEXData(const QString &);
    void showLabels();

    void blockGUI();
    void unblockGUI();

};

#endif // MAINWINDOW_HPP
