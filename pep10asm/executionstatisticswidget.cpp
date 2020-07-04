#include "executionstatisticswidget.h"
#include "ui_executionstatisticswidget.h"
#include "pep.h"
ExecutionStatisticsWidget::ExecutionStatisticsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ExecutionStatisticsWidget), model(new QStandardItemModel(this))
{
    ui->setupUi(this);
    ui->treeView->setModel(model);
    model->setHorizontalHeaderLabels({"Instruction", "Frequency"});
}

void ExecutionStatisticsWidget::init(QSharedPointer<InterfaceISACPU> cpu, bool showCycles)
{
    this->cpu = cpu;
    if(!showCycles) {
        ui->label->hide();
        ui->lineEdit_Cycles->hide();
    }
}

ExecutionStatisticsWidget::~ExecutionStatisticsWidget()
{
    delete ui;
}

void ExecutionStatisticsWidget::onClear()
{
    ui->lineEdit_Cycles->clear();
    ui->lineEdit_Instructions->clear();
    model->removeRows(0, model->rowCount());
    // Sort by a non-existent column to prevent the "sorting arrow"
    // from appearing over unsorted data.
    ui->treeView->sortByColumn(-1, Qt::SortOrder::AscendingOrder);
}

void ExecutionStatisticsWidget::onSimulationStarted()
{
    // Make sure no statistics are displayed at the start of a run.
    onClear();
}

void ExecutionStatisticsWidget::onSimulationFinished()
{
    // Use locale so that strings have commas in them.
    ui->lineEdit_Cycles->setText(QLocale::system().toString(cpu->getCycleCount()));
    ui->lineEdit_Instructions->setText(QLocale::system().toString(cpu->getInstructionCount()));
    fillModel(cpu->getInstructionHistogram());
}

// POD class to help aggregate statistics.
struct lookup {
    // How many times an instruction was referenced.
    quint32 tally;
    // At what operand specifier does the instruction start.
    quint8 start;
    // How many addressing modes does this instruction have?
    // Options are 0 (unary), 2(non-unary a field), and 8 (non-unary aaa field).
    quint8 addrModes;
};

void ExecutionStatisticsWidget::fillModel(const QVector<quint32> histogram)
{
    // Make sure the model has no existing items.
    // Model will make sure to delete any extra items
    model->removeRows(0, model->rowCount());

    Enu::EMnemonic mnemon;
    QMap<Enu::EMnemonic, lookup> mnemonicMap;

    // Verify that the histogram contains enough entries to be read.
    if(histogram.length() < 256) {
        qWarning() << "Histogram is not long enough";
        return;
    }
    // For every opcode from 00..FF
    for(int it = 0; it < 256; it++) {
        // Convert the int to an actual instruction
        mnemon = Pep::decodeMnemonic[it];
        // If the instruction exists, update the data in place
        if(mnemonicMap.contains(mnemon)) {
            mnemonicMap[mnemon].tally += histogram[it];
            // If the item already exists, then it must be a non-unary instruction.
            // Therefore, the first loop iteration was an addressing mode,
            // and we must adjust the address mode counter to compensate.
            if(mnemonicMap[mnemon].addrModes == 0) mnemonicMap[mnemon].addrModes += 1;
            // Each time we match a mnemonic, we have another addressing mode for the same instruction.
            mnemonicMap[mnemon].addrModes += 1;
        }
        // Otherwsie create a new data object.
        else {
            lookup entry;
            entry.start = it;
            entry.tally = histogram[it];
            // Assume a mnemonic is unary until otherwise proven.
            entry.addrModes = 0;
            mnemonicMap[mnemon] = entry;
        }
    }
    // Metaobjects to help convert enums to QStrings.
    static QMetaEnum mnemonicMetaenum = Enu::staticMetaObject.enumerator(Enu::staticMetaObject.indexOfEnumerator("EMnemonic"));
    static QMetaEnum addrMetaenum = Enu::staticMetaObject.enumerator(Enu::staticMetaObject.indexOfEnumerator("EAddrMode"));

    // Iteratre through the key, value pairs in the map.
    for(auto kvPair = mnemonicMap.keyValueBegin(); kvPair != mnemonicMap.keyValueEnd(); ++kvPair) {
        Enu::EMnemonic mnemon = (*kvPair).first;
        auto tuple = (*kvPair).second;

        // If the instruction was not used, do not insert its entry.
        if(tuple.tally == 0 ) continue;

        // Create entries for the mnemonic name
        QStandardItem* instrName = new QStandardItem(QString(mnemonicMetaenum.valueToKey((int)mnemon)).toLower());
        QStandardItem* instrCount = new QStandardItem();
        // Make a variant from an int type to ensure that sorting works correctly.
        instrCount->setData(QVariant(tuple.tally), Qt::DisplayRole);
        model->insertRow(model->rowCount(), {instrName, instrCount});

        for(int offset = 0;  offset < tuple.addrModes; offset++) {
            // If the addressing mode was not used, do not insert the entry.
            if(histogram[tuple.start + offset] == 0) continue;
            // Otherwise, for every opcode between the start and number of addressing modes,
            // figure out the addressing mode associated with the instruction.
            Enu::EAddrMode addr = Pep::decodeAddrMode[tuple.start + offset];
            QStandardItem* addrName = new QStandardItem(QString(addrMetaenum.valueToKey((int) addr)).toLower());
            QStandardItem* addrCount = new QStandardItem();
            addrCount->setData(QVariant(histogram[tuple.start + offset]), Qt::DisplayRole);

            instrName->appendRow({addrName, addrCount});
        }
    }

}
