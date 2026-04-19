#ifndef TST_PARSECH_H
#define TST_PARSECH_H

#include <QObject>

class ParsecTest : public QObject
{
    Q_OBJECT
public:
  explicit ParsecTest(QObject *parent = nullptr);

private slots:
    void initTestCase();
    void cleanupTestCase();

    void singleDigitParserTest();
    void onlyOneDigitTest();
    void singleDigitFromManyTest();
    void singleDigitFromMiddleTest();
    void onlyOneDigitFromMiddleTest();
    void singleDigitFailureTest();
    void litParserTest();

    void digitCastTest();

    void manyTest();
    void manyDigitsCastedTest();
    void manyParserCastedTest();

    void bindPureTest();

    void bindLeftIdentityTest();
    void bindRightIdentityTest();
    void bindAssociativityTest();
    void nestedBindSequenceTest();

    void parserRuntimeTest();

    void seqTest();
    void leftRightTest();
    void many1Test();
    void sepBy1Test();
    void betweenTest();
    void countTest();
    void discardTest();
    void bothTest();
};

#endif // TST_PARSECH_H
