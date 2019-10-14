#ifndef TST_ASSEMBLEPROGRAMS_H
#define TST_ASSEMBLEPROGRAMS_H

#include <QtTest>
class AsmProgram;
class MacroRegistry;
class AssemblePrograms : public QObject
{
    Q_OBJECT

public:
    AssemblePrograms();
    ~AssemblePrograms() override;

private slots:
    void initTestCase();
    void assemblePrograms_data();
    void assemblePrograms();

private:
    QString osText;
    QSharedPointer<MacroRegistry> registry;
    QSharedPointer<const AsmProgram> operatingSystem;

};

#endif // TST_ASSEMBLEPROGRAMS_H
