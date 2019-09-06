#ifndef TST_TOKENBUFFER_H
#define TST_TOKENBUFFER_H

#include <QTest>

class MacroRegistry;
class TokenizerBuffer;
struct ModuleAssemblyGraph;

/*
 * Test parsing of entire lines, allowing for better error checking.
 */
class TokenBufferTest : public QObject
{
    Q_OBJECT
public:
    TokenBufferTest();
    ~TokenBufferTest();

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Catch malformed decimal constants that the tokenizer could not find on it own.
    void case_malformedDecConst_data();
    void case_malformedDecConst();

    // Catch malformed hex constants that the tokenizer could not find on it own.
    void case_malformedHexConst_data();
    void case_malformedHexConst();

    // Catch malformed dot commands that the tokenizer could not find on it own.
    void case_malformedDot_data();
    void case_malformedDot();

    // Catch malformed dot commands that the tokenizer could not find on it own.
    void case_malformedIdentifier_data();
    void case_malformedIdentifier();
private:
    void execute();
    QSharedPointer<MacroRegistry> registry;
    QSharedPointer<TokenizerBuffer> buffer;
};

#endif // TST_TOKENBUFFER_H
