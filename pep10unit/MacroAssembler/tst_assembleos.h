#ifndef TST_ASSEMBLEOS_H
#define TST_ASSEMBLEOS_H

#include <QTest>
// add necessary includes here
class MacroRegistry;
class AssembleOS : public QObject
{
    Q_OBJECT

public:
    AssembleOS();
    ~AssembleOS();

private slots:
    void initTestCase();
    void assembleOS();
private:
    QString osText;
    QSharedPointer<MacroRegistry> registry;
};
#endif // TST_ASSEMBLEOS_H
