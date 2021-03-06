// File: byteconverterchar.cpp
/*
    The Pep/9 suite of applications (Pep9, Pep9CPU, Pep9Micro) are
    simulators for the Pep/9 virtual machine, and allow users to
    create, simulate, and debug across various levels of abstraction.

    Copyright (C) 2010  J. Stanley Warford, Pepperdine University

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "byteconverterchar.h"
#include "ui_byteconverterchar.h"

ByteConverterChar::ByteConverterChar(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::ByteConverterChar)
{
    m_ui->setupUi(this);
    // Regular expression to validate a single character
    QRegExp charRx("(.){0,1}");
    charValidator = new QRegExpValidator(charRx, this);
    m_ui->lineEdit->setValidator(charValidator);
    // Forward the textEdited() signal from m_ui->lineEdit up to the main window
    QObject::connect(m_ui->lineEdit, &QLineEdit::textEdited, this,
                     &ByteConverterChar::textEdited);
}

ByteConverterChar::~ByteConverterChar()
{
    delete m_ui;
}

void ByteConverterChar::setValue(int value)
{
    m_ui->lineEdit->setText(QString(QChar(value)));
}
