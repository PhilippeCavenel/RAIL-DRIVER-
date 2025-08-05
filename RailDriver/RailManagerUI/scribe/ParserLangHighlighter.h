// ParserLangHighlighter.hpp
#ifndef PARSERLANGHIGHLIGHTER_H
#define PARSERLANGHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QTextDocument>
#include <highlighter.h>

class ParserLangHighlighter : public Highlighter
{
    Q_OBJECT

public:
    explicit ParserLangHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightRule> rules;
    QTextCharFormat keywordFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat commentFormat;
};


#endif // PARSERLANGHIGHLIGHTER_H
