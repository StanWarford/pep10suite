#ifndef TST_MACROASSEMBLERFAILURE_H
#define TST_MACROASSEMBLERFAILURE_H

#include <QTest>
// add necessary includes here

class TokenizerFailure : public QObject
{
    Q_OBJECT

public:
    TokenizerFailure();
    ~TokenizerFailure();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_case1();

};
#endif // TST_MACROASSEMBLERFAILURE_H
