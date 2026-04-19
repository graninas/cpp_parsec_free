#ifndef TST_SAMPLES_H
#define TST_SAMPLES_H

#include <QObject>

class SamplesTest : public QObject
{
    Q_OBJECT
public:
    explicit SamplesTest(QObject *parent = nullptr);

private slots:
    void personInfoParserTest();
};

#endif // TST_SAMPLES_H
