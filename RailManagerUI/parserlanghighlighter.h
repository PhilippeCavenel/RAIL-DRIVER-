// ParserLangHighlighter.hpp
#ifndef PARSERLANGHIGHLIGHTER_HPP
#define PARSERLANGHIGHLIGHTER_HPP

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QTextDocument>
#include "highlighters/highlighter.h"

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


#endif // PARSERLANGHIGHLIGHTER_HPP
