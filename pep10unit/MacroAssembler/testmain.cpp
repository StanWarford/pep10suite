#include <QTest>
#include "tst_prepreocessorfail.h"
#include "tst_tokenizer.h"
#include "tst_tokenbuffer.h"
#include "tst_assembler.h"
#include "tst_linker.h"
#include "tst_assembleos.h"
#include "tst_assembleprograms.h"
#include "tst_userosintegration.h"
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
    TokenizerTest tokenizerTest;
    ret += QTest::qExec(&tokenizerTest, argc, argv);

    // Test that streams of tokens may be parsed.
    TokenBufferTest tokenBuffer;
    ret += QTest::qExec(&tokenBuffer, argc, argv);

    // Then test that the assembler works.
    AssemblerTest assemblerTest;
    ret += QTest::qExec(&assemblerTest, argc, argv);

    // Test the operation of the linker.
    LinkerTest linkerTest;
    ret += QTest::qExec(&linkerTest, argc, argv);

    // Check that information exported from the OS reaches user programs
    UserOSIntegrationTest userOSTest;
    ret += QTest::qExec(&userOSTest, argc, argv);

    // Then try out the stack annotator.
    // Now attempt to assemble the operating system.
    AssembleOS os;
    ret += QTest::qExec(&os, argc, argv);

    // Lastly, try assembling all sample programs.
    AssemblePrograms progs;
    ret += QTest::qExec(&progs, argc, argv);
    return ret;
}
