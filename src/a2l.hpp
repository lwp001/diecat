/*
    diecat
    A2L/HEX file reader.

    File: a2l.hpp

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

#ifndef A2L_HPP
#define A2L_HPP

#include <QString>
#include <QVector>
#include <QStringList>
#include <QSharedPointer>

#include "ecuscalar.hpp"

class A2L {

public:
    A2L(const QString &); // takes a2l file path
    bool readFile();
    void fillScalarsInfo(QVector< QSharedPointer<ECUScalar> > &) const;
    void clear();

private:
    QString m_a2lpath;
    QVector<QStringList> m_scalarsInfo;
    QVector<QStringList> m_compumethodsInfo;
    QVector<QStringList> m_compuvtabsInfo;

    ptrdiff_t findCompuMethod(const QString &) const;
    ptrdiff_t findCompuVTab(const QString &) const;
    QVector<double> getCoeff(ptrdiff_t) const;
    QVector<double> getHardLimints(const QString &) const;
    bool isReadOnly(ptrdiff_t) const;
    QStringList getVTab(ptrdiff_t) const;
    const QString delQuotes(const QString &) const;

};

#endif // A2L_HPP
