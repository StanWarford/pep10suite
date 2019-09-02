#ifndef TST_PREPREOCESSORFAIL_H
#define TST_PREPREOCESSORFAIL_H


#include <QtTest>
#include "macropreprocessor.h"
#include "macroregistry.h"
// add necessary includes here

/*
 * Various unit tests that should make the assembler fail
 */
class PreprocessorFailure : public QObject
{
    Q_OBJECT
public:
    PreprocessorFailure();
    ~PreprocessorFailure();

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Test cases for macros that do not exist.
    void case_noSuchMacro_data();
    void case_noSuchMacro();

    // Test cases for macros that might or might not exist,
    // but their name has invalid characters in it.
    void case_macroRequiresIdentifier_data();
    void case_macroRequiresIdentifier();

    // Test cases for macros supplied the wrong number of arguments.
    void case_badArgCount_data();
    void case_badArgCount();

    // Test cases for macros given a $1, $2 as a parameter.
    void case_noDollarInArgument_data();
    void case_noDollarInArgument();

    // Test cases for macros that participate in circular inclusion.
    void case_circularInclude_data();
    void case_circularInclude();

    // Test cases for macros that reference themselves.
    void case_selfReference_data();
    void case_selfReference();
private:
    QScopedPointer<MacroRegistry> registry;
    QScopedPointer<MacroPreprocessor> preprocessor;

};

#endif // TST_PREPREOCESSORFAIL_H
