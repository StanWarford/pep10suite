#include <QTest>
#include "tst_prepreocessorfail.h"
#include "tst_tokenizerfail.h"
#include "tst_assembleos.h"
#include "tst_assembleprograms.h"
#include "pep.h"
int main(int argc, char *argv[])
{
    // Initialize global state maps.
    Pep::initEnumMnemonMaps();
    Pep::initMnemonicMaps();
    Pep::initAddrModesMap();
    Pep::initDecoderTables();

    int ret = 0;
    // Test the preprocessor first.
    PreprocessorFailure preFail;
    ret += QTest::qExec(&preFail, argc, argv);

    // Then test the tokenizer.
    TokenizerFailure tokenFail;
    ret += QTest::qExec(&tokenFail, argc, argv);
    // Then test that the assembler works.
    // Followed by the linker.
    // Then try out the stack annotator.
    // Now attempt to assemble the operating system.
    AssembleOS os;
    ret += QTest::qExec(&os, argc, argv);
    // Lastly, try assembling all sample programs.
    AssemblePrograms progs;
    ret += QTest::qExec(&progs, argc, argv);
    return ret;
}
