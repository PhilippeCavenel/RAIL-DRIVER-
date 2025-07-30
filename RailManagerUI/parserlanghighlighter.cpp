// ParserLangHighlighter.cpp
#include "parserlanghighlighter.h"
#include <QTextDocument>

ParserLangHighlighter::ParserLangHighlighter(QTextDocument *parent)
    : Highlighter(parent)
{
    // Format for keywords
    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);

    QStringList keywords = {
        "PROG", "COM", "DCC", "STOP", "RUNALL", "ANA", "LPO", "GPIO",
        "TRACK", "IN", "ACT", "STA", "OUT", "ONTRACK", "OFFTRACK",
        "DEL", "FORW", "BACK", "GSTAT", "LSTAT", "TSTAT", "BSTAT",
        "AUTLIST", "AUT", "BOARD", "ERR", "AUTFULL", "NOAUT", "TIMEOUT",
        "NODCC", "NOANA", "GPIOIN", "RUN", "RESET", "TIMER", "INERTIA",
        "SPEED", "AUTON", "AUTOFF", "DUMP", "KNOB0", "KNOB1", "MANUAL",
        "MANUAL0", "AUTOMATIC", "CALIB"
    };

    for (const QString &keyword : keywords) {
        HighlightRule rule;
        rule.pattern = QRegularExpression("\\b" + QRegularExpression::escape(keyword) + "\\b");
        rule.format = keywordFormat;
        rules.append(rule);
    }

    // Format for numbers
    numberFormat.setForeground(Qt::darkMagenta);
    HighlightRule numberRule;
    numberRule.pattern = QRegularExpression("\\b\\d+\\b");
    numberRule.format = numberFormat;
    rules.append(numberRule);

    // Format for comments (lines starting with //)
    commentFormat.setForeground(Qt::darkGreen);
    HighlightRule commentRule;
    commentRule.pattern = QRegularExpression("//.*");
    commentRule.format = commentFormat;
    rules.append(commentRule);
}

void ParserLangHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightRule &rule : std::as_const(rules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
