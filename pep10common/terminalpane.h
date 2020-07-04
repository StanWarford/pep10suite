// File: terminalpane.h
/*
    The Pep/9 suite of applications (Pep9, Pep9CPU, Pep9Micro) are
    simulators for the Pep/9 virtual machine, and allow users to
    create, simulate, and debug across various levels of abstraction.
    
    Copyright (C) 2009  J. Stanley Warford, Pepperdine University

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

#ifndef TERMINALPANE_H
#define TERMINALPANE_H

#include <QKeyEvent>
#include <QWidget>

namespace Ui {
    class TerminalPane;
}

class TerminalPane : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(TerminalPane)
public:
    explicit TerminalPane(QWidget *parent = nullptr);
    ~TerminalPane() override;

    void cancelWaiting();
    // Post: if the terminal was waiting for input, cancel the wait

    void appendOutput(QString str);
    // Post: str is appended to the text edit

    void waitingForInput();
    // Post: Sets the writability of the text edit to true, and prevents previously entered text from being modified

    void clearTerminal();
    // Post: Clears the terminal

    void highlightOnFocus();
    // Post: Highlights the label based on the label window color saved in the UI file

    bool hasFocus();
    // Post: returns if the pane has focus

    bool isUndoable() const;
    bool isRedoable() const;

public slots:
    void onFontChanged(QFont font);

    void copy() const;
    // Post: selected text in the text edit is copied to the clipboard
    void cut();
    void paste();
    void undo();
    void redo();

private:
    Ui::TerminalPane *ui;

    bool waiting;

    QString strokeString;
    QString retString;

    void displayTerminal();
    
    bool eventFilter(QObject *, QEvent *event) override;

private slots:
    void mouseReleaseEvent(QMouseEvent *) override;

signals:
    void undoAvailable(bool);
    void redoAvailable(bool);
    void inputReceived();
    void inputReady(const QString&);

};

#endif // TERMINALPANE_H
