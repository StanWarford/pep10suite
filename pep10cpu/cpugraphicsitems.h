// File: cpugraphicsitems.h
/*
    Pep9CPU is a CPU simulator for executing microcode sequences to
    implement instructions in the instruction set of the Pep/9 computer.

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
#ifndef CPUGRAPHICSITEMS_H
#define CPUGRAPHICSITEMS_H

#include <QGraphicsItem>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>

#include "enu.h"
#include "tristatelabel.h"
#include "colors.h"
class CPUDataSection;
class CpuGraphicsItems : public QGraphicsItem
{
public:
    CpuGraphicsItems(Enu::CPUType type, CPUDataSection* dataSection, QWidget *widgetParent, QGraphicsItem *itemParent = nullptr,
                             QGraphicsScene *scene = nullptr);
    ~CpuGraphicsItems() override;

    QRectF boundingRect() const override;

    bool aluHasCorrectOutput();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
    void darkModeChanged(bool darkMode, QString styleSheet);
    void CPUTypeChanged(Enu::CPUType newType);

private:
    // Try to draw as many free-floating strings in one centralized function as possible. Both 1 & 2 byte models.
    void drawDiagramFreeText(QPainter *painter);
    void drawLabels();
    void drawStaticRects(QPainter* painter);
    void drawALUPoly();
    void drawRegisterBank();
    void repaintLoadCk(QPainter *painter);
    void repaintCSelect(QPainter *painter);
    void repaintBSelect(QPainter *painter);
    void repaintASelect(QPainter *painter);
    void repaintMARCk(QPainter *painter);
    void repaintAMuxSelect(QPainter *painter);

    void repaintCMuxSelect(QPainter *painter);

    void repaintSCk(QPainter *painter);
    void repaintCCk(QPainter *painter);
    void repaintVCk(QPainter *painter);
    void repaintZCk(QPainter *painter);
    void repaintNCk(QPainter *painter);
    void repaintMemCommon(QPainter *painter);
    void repaintMemRead(QPainter *painter);
    void repaintMemWrite(QPainter *painter);
    void repaintSBitOut(QPainter *painter);
    void repaintCBitOut(QPainter *painter);
    void repaintVBitOut(QPainter *painter);
    void repaintZBitOut(QPainter *painter);
    void repaintNBitOut(QPainter *painter);

    void repaintCSMuxSelect(QPainter *painter);
    void repaintAndZSelect(QPainter *painter);
    void repaintALUSelect(QPainter *painter);

    // One byte specific repainting methods
    void repaintMDRCk(QPainter *painter);
    void repaintMDRMuxSelect(QPainter *painter);
    void repaintMARCkOneByte(QPainter *painter);
    void repaintALUSelectOneByte(QPainter *painter);
    void repaintMemCommonOneByte(QPainter *painter);
    void repaintMemReadOneByte(QPainter *painter);
    void repaintMemWriteOneByte(QPainter *painter);
    void repaintABusOneByte(QPainter *painter);
    void repaintBBusOneByte(QPainter *painter);
    void repaintCBusOneByte(QPainter *painter);

    // two byte specific repainting methods
    void repaintMARMuxSelect(QPainter *painter);
    void repaintMARCkTwoByte(QPainter *painter);

    void repaintMDROCk(QPainter *painter);
    void repaintMDROSelect(QPainter *painter);
    void repaintMDRECk(QPainter *painter);
    void repaintMDRESelect(QPainter *painter);
    void repaintEOMuxSelect(QPainter *painter);
    void repaintEOMuxOutpusBus(QPainter *painter);

    void repaintALUSelectTwoByte(QPainter *painter);

    void repaintMemCommonTwoByte(QPainter *painter);
    void repaintMemReadTwoByte(QPainter *painter);
    void repaintMemWriteTwoByte(QPainter *painter);

    void repaintMARMUXToMARBuses(QPainter *painter);

    void repaintMDRMuxOutputBuses(QPainter *painter);
    void repaintMDREToEOMuxBus(QPainter *painter);
    void repaintMDROToEOMuxBus(QPainter *painter);
    void repaintABusTwoByte(QPainter *painter);
    void repaintBBusTwoByte(QPainter *painter);
    void repaintCBusTwoByte(QPainter *painter);

private:
    QWidget *parent;
    QGraphicsScene *parentScene;
    CPUDataSection* dataSection;
    bool darkMode = false;
    Enu::CPUType type;

    const PepColors::Colors *colorScheme;

    QImage arrowLeft;
    QImage arrowRight;
    QImage arrowUp;
    QImage arrowDown;

    QImage arrowLeftGray;
    QImage arrowRightGray;
    QImage arrowUpGray;
    QImage arrowDownGray;

public:
    // OUTSIDE REGISTERS
    QCheckBox *loadCk;
    QLabel *cLabel;
    QLineEdit *cLineEdit;
    QLabel *bLabel;
    QLineEdit *bLineEdit;
    QLabel *aLabel;
    QLineEdit *aLineEdit;

    QCheckBox *MARCk;
    QLabel *MARALabel; // data section
    QLabel *MARBLabel; // data section

    QLabel *aMuxLabel;
    QLabel *aMuxerDataLabel; // data section
    QGraphicsRectItem *aMuxerBorder; // data section
    TristateLabel *aMuxTristateLabel;

    QLabel *cMuxLabel;
    TristateLabel *cMuxTristateLabel;
    QLabel *cMuxerLabel; // data section

    QLabel *ALULabel;
    QLineEdit *ALULineEdit;
    QLabel *ALUFunctionLabel; // data section
    QGraphicsPolygonItem *ALUPoly; //data section

    QLabel *CSMuxLabel;
    QLabel *CSMuxerDataLabel;
    TristateLabel *CSMuxTristateLabel;
    QCheckBox *SCkCheckBox;

    QCheckBox *CCkCheckBox;

    QCheckBox *VCkCheckBox;
    QLabel *AndZLabel;
    TristateLabel *AndZTristateLabel;
    QLabel *AndZMuxLabel; // data section

    QCheckBox *ZCkCheckBox;

    QCheckBox *NCkCheckBox;

    TristateLabel *nBitLabel; // data section
    TristateLabel *zBitLabel; // data section
    TristateLabel *vBitLabel; // data section
    TristateLabel *cBitLabel; // data section
    TristateLabel *sBitLabel; // data section

    QLabel *MemReadLabel;
    TristateLabel *MemReadTristateLabel;

    QLabel *MemWriteLabel;
    TristateLabel *MemWriteTristateLabel;

    QVector<QCheckBox*> checkVector;
    QVector<QLineEdit*> framedVector;
    // REGISTER BANK
    QGraphicsRectItem* regBankOutline;
    QGraphicsRectItem* regBank;

    QVector<QLabel*> labelVec;
    QVector<QLineEdit*> editorVector;
    QLineEdit *aRegLineEdit;
    QLineEdit *xRegLineEdit;
    QLineEdit *spRegLineEdit;
    QLineEdit *pcRegLineEdit;
    QLineEdit *irRegLineEdit;
    QLineEdit *t1RegLineEdit;
    QLineEdit *trRegLineEdit;
    QLineEdit *t2RegLineEdit;
    QLineEdit *t3RegLineEdit;
    QLineEdit *t4RegLineEdit;
    QLineEdit *t5RegLineEdit;

    // One byte data bus model features;
    QCheckBox  *MDRCk;
    TristateLabel *MDRMuxTristateLabel;
    QLabel *MDRMuxLabel;
    QLabel *MDRMuxerDataLabel;
    QLabel *MDRLabel;
    // Two byte data bus model features:

    QCheckBox *MDROCk, *MDRECk;
    TristateLabel *MARMuxTristateLabel;
    TristateLabel *MDROMuxTristateLabel, *MDREMuxTristateLabel;
    TristateLabel *EOMuxTristateLabel;
    QLabel *MARMuxLabel;
    QLabel *MDROMuxLabel, *MDREMuxLabel;

    QLabel *MARMuxerDataLabel;
    QLabel *EOMuxLabel;
    QLabel *EOMuxerDataLabel;
    QLabel *MDROMuxerDataLabel, *MDREMuxerDataLabel;
    QLabel *MDROLabel, *MDRELabel;

};


#endif // CPUGRAPHICSITEMS_H
