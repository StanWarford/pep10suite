#ifndef TST_MACROASSEMBLERFAILURE_H
#define TST_MACROASSEMBLERFAILURE_H

#include <QTest>
// add necessary includes here

class MacroRegistry;
class MacroTokenizer;
struct ModuleAssemblyGraph;
class TokenizerTest : public QObject
{
    Q_OBJECT

public:
    TokenizerTest();
    ~TokenizerTest();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void case_malformedMacroInvoke_data();
    void case_malformedMacroInvoke();

    void case_malformedMacroSubstitution_data();
    void case_malformedMacroSubstitution();

    void case_malformedAddrMode_data();
    void case_malformedAddrMode();

    void case_malformedCharConst_data();
    void case_malformedCharConst();

    // There should be no way to form an invalid comment
    //void case_malformedComment_data();
    //void case_malformedComment();

    void case_malformedDecConst_data();
    void case_malformedDecConst();

    void case_malformedHexConst_data();
    void case_malformedHexConst();

    void case_malformedDot_data();
    void case_malformedDot();

    void case_malformedIdentifier_data();
    void case_malformedIdentifier();

    void case_malformedStringConst_data();
    void case_malformedStringConst();

    void case_syntaxError_data();
    void case_syntaxError();
private:
    void preprocess(ModuleAssemblyGraph& graph);
    void execute();
    QSharedPointer<MacroRegistry> registry;
    QSharedPointer<MacroTokenizer> tokenizer;
};
#endif // TST_MACROASSEMBLERFAILURE_H
