#include <QTest>
#include "tst_free_parsec.h"
#include "tst_samples.h"

int main(int argc, char **argv)
{
    int status = 0;
    ParsecTest tp;
    status |= QTest::qExec(&tp, argc, argv);
    SamplesTest st;
    status |= QTest::qExec(&st, argc, argv);
    return status;
}
