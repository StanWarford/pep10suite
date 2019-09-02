#ifndef ERRORMESSAGE_H
#define ERRORMESSAGE_H

#include <QString>
#include <QSharedPointer>
#include <QMap>

class AsmCode;

enum class Severity
{
    DEBUG,
    WARNING,
    ERROR
};
enum class Stage
{
    FRONTEND,
    BACKEND
};

/*
 * An error message indicates some exception condition in code that is being assembled.
 *
 * Error messages may be raised by the frontend (Preprocessor, tokenizer, assembler),
 * or the backend (linker, stack annotator). Frontend messages may only "insert" their
 * error messages in the source pane, as the concept of a listing doesn't yet exist.
 * Backened messages may insert themselves in either the source, the listing, or both.
 *
 * The instanceIndex allows the error message to be mapped to the failing module, and
 * then with the help of the assembly graph, mapped to the source line in the root module
 * that raised the issue.
 */
class AErrorMessage
{
public:
    AErrorMessage(quint32 instanceIndex, Severity severity, Stage stage, QString message);
    AErrorMessage(const AErrorMessage& other);
    AErrorMessage(AErrorMessage&& other);
    virtual ~AErrorMessage() = 0;

    quint16 getPrototypeIndex() const;
    quint32 getInstanceIndex() const;
    Severity getSeverity() const;
    Stage getStage() const;
    QString getErrorMessage() const;
    virtual int getSourceLineNumber() const = 0;
    virtual bool presentInListing() const = 0;
    virtual int getListingLineNumber() const = 0;
    friend void swap(AErrorMessage& first, AErrorMessage& second)
    {
        using std::swap;
        swap(first.instanceIndex, second.instanceIndex);
        swap(first.severity, second.severity);
        swap(first.stage, second.stage);
        swap(first.errorMessage, second.errorMessage);
    }
private:
    quint32 instanceIndex;
    Severity severity;
    Stage stage;
    QString errorMessage;
};

/*
 * An error message representing a syntatical error in a line of code.
 *
 * It may be generated by the preprocessor, the tokenizer, or the assembler.
 *
 * All of these pieces of the frontend are responsible for generating the IR
 * of the program under assembly, so any fatal error prevents the IR from being
 * generated. Therefore, frontend errors may only be mapped to the source line of
 * code that caused the error; they may not be mapped to a listing line since
 * the listing does not yet exist.
 *
 * In the current assembly model of Pep10, all errors reported by the front end are fatal.
 * However, this class is designed to be flexible and allow any kind of error.
 */
class FrontEndError : public AErrorMessage
{
public:
    FrontEndError(quint32 instanceIndex, Severity severity, QString message, int sourceLine);
    FrontEndError(const FrontEndError& other);
    FrontEndError(FrontEndError&& other);
    FrontEndError& operator=(FrontEndError rhs);
    ~FrontEndError() override;

    // AErrorMessage interface
    int getSourceLineNumber() const override;
    bool presentInListing() const override;
    int getListingLineNumber() const override;
    friend void swap(FrontEndError& first, FrontEndError& second)
    {
        using std::swap;
        swap(static_cast<AErrorMessage&>(first), static_cast<AErrorMessage&>(second));
        swap(first.sourceLine, second.sourceLine);
    }
private:
    int sourceLine;
};

/*
 * An error message generated after the IR has been created, therefore there
 * is in the listing, and part of the source of some module.
 *
 * However, if the line of code is contained in a module that is not the root, then
 * the error message may not be displayed in the source pane. Instead, it will only be
 * displayed in the listing pane. If the line of code is a part of the root module,
 * then the error may appear in both the source pane and the listing.
 *
 * For errors not in the root module, it is up to the programmer to manually propogate them
 * up to the root level.
 */
class BackEndError : public AErrorMessage
{
public:
    BackEndError(quint32 instanceIndex, Severity severity, QString message, QSharedPointer<AsmCode> line);
    BackEndError(const BackEndError& other);
    BackEndError(BackEndError&& other);
    BackEndError& operator=(BackEndError rhs);
    ~BackEndError() override;

    // AErrorMessage interface
    int getSourceLineNumber() const override;
    bool presentInListing() const override;
    int getListingLineNumber() const override;
    friend void swap(BackEndError& first, BackEndError& second)
    {
        using std::swap;
        swap(static_cast<AErrorMessage&>(first), static_cast<AErrorMessage&>(second));
        swap(first.line, second.line);
    }
private:
    QSharedPointer<AsmCode> line;
};

class ErrorDictionary
{
public:
    using ErrorList =  QList<QSharedPointer<AErrorMessage>>;
    using SourceLine = int;
    using ModuleErrors = QMap<SourceLine, ErrorList>;
    using ProjectErrors = QMap<quint32, ModuleErrors>;

    ProjectErrors errors;
    ModuleErrors sourceMapped;

    void addError(QSharedPointer<AErrorMessage> error);
    ModuleErrors getModuleErrors(quint32 instanceIndex);

};

#endif // ERRORMESSAGE_H