#include <QString>
#include <QtTest>

#include <variant>
#include <iostream>

#include "common.h"
#include "ps/ps.h"

class PSTest : public QObject
{
    Q_OBJECT

public:
    PSTest();

private Q_SLOTS:

    void atomicallyTest();
    void coercingTest();

    void sequenceCombinatorTest();
    void whenCombinatorTest();
    void unlessCombinatorTest();
    void bothCombinatorTest();
    void bothTVarsCombinatorTest();
    void bothVoidedCombinatorTest();
    void modifyTVarTest();
};

PSTest::PSTest()
{
}

using TVarInt = ps::TVar<int>;
using TVarStr = ps::TVar<std::string>;

void PSTest::atomicallyTest()
{
    std::function<ps::PSL<int>(TVarInt)> f =
            [](const TVarInt& tvar)
    {
        return ps::readTVar(tvar);
    };

    auto x = ps::newTVar(10);

    ps::PSL<int> s = ps::bind(x, f);

    ps::Context context;
    auto result = ps::atomically(context, s);
    QVERIFY(result == 10);
}

void PSTest::coercingTest()
{
    ps::PSL<TVarInt>   m1 = ps::newTVar(10);
    ps::PSL<int>       m2 = ps::bind<TVarInt, int>(m1, ps::mReadTVar);
    ps::PSL<ps::Unit> x1 = ps::bind<TVarInt, ps::Unit>(m1, ps::mWriteTVarV(20));
    ps::PSL<ps::Unit> y1 = ps::sequence<TVarInt, ps::Unit>(m1, ps::mRetry);

    ps::Context context;
    int result1 = ps::atomically(context, m2);

    ps::atomically(context, x1);

//  Will run forever because of retry.
//    ps::atomically(context, y1);

    QVERIFY(result1 == 10);
}

void PSTest::sequenceCombinatorTest()
{
    using namespace ps;

    auto m1 = newTVar(10);
    auto m2 = ps::bind<TVarInt, TVarInt>(m1, [](const auto& tvar)
    {
        auto mm1 = writeTVar(tvar, 20);
        return sequence(mm1, pure(tvar));
    });
    auto m3 = ps::bind<TVarInt, int>(m2, mReadTVar);

    Context ctx;
    auto result = atomically(ctx, m3);
    QVERIFY(result == 20);
}

void PSTest::whenCombinatorTest()
{
    using namespace ps;

    auto m1 = when(pure(true),  newTVar(10));
    auto m2 = when(pure(false), newTVar(10));

    Context context1;
    Context context2;

    TVars tvars1_1 = context1.takeSnapshot();
    TVars tvars2_1 = context2.takeSnapshot();

    QVERIFY(tvars1_1.size() == 0);
    QVERIFY(tvars2_1.size() == 0);

    atomically(context1, m1);
    atomically(context2, m2);

    TVars tvars1_2 = context1.takeSnapshot();
    TVars tvars2_2 = context2.takeSnapshot();

    QVERIFY(tvars1_2.size() == 1);
    QVERIFY(tvars2_2.size() == 0);
}

void PSTest::unlessCombinatorTest()
{
    using namespace ps;

    auto m1 = unless(pure(true),  newTVar(10));
    auto m2 = unless(pure(false), newTVar(10));

    Context context1;
    Context context2;

    TVars tvars1_1 = context1.takeSnapshot();
    TVars tvars2_1 = context2.takeSnapshot();

    QVERIFY(tvars1_1.size() == 0);
    QVERIFY(tvars2_1.size() == 0);

    atomically(context1, m1);
    atomically(context2, m2);

    TVars tvars1_2 = context1.takeSnapshot();
    TVars tvars2_2 = context2.takeSnapshot();

    QVERIFY(tvars1_2.size() == 0);
    QVERIFY(tvars2_2.size() == 1);
}

void PSTest::bothCombinatorTest()
{
    using namespace ps;

    auto mResult = both<int, std::string, std::string>
            (pure(10),
             pure(std::string("abc")),
             [](int i, const std::string& s)
    {
        return std::to_string(i) + s;
    });

    Context ctx;
    auto result = atomically(ctx, mResult);
    QVERIFY(result == std::string("10abc"));
}

void PSTest::bothTVarsCombinatorTest()
{
    using namespace ps;

    auto mResult = withTVars<int, std::string, std::string>(
                newTVar(10),
                newTVar(std::string("abc")),
                [](int i, const std::string& s)
                {
                    return std::to_string(i) + s;
                });

    Context ctx;
    auto result = atomically(ctx, mResult);
    QVERIFY(result == std::string("10abc"));
}

void PSTest::bothVoidedCombinatorTest()
{
    using namespace ps;

    auto mResult = bothVoided<int, std::string>
            (pure(10),
             pure(std::string("abc")));

    Context ctx;
    Unit result = atomically(ctx, mResult);
    Q_UNUSED(result);
}

void PSTest::modifyTVarTest()
{
    using namespace ps;

    std::function<int(int)> f = [](int i) { return i + 5; };

    auto m1 = newTVar(10);
    auto mResult = bind<TVar<int>, int>(m1, [=](auto tvar)
    {
        auto m3 = modifyTVar(tvar, f);
        return bind<Unit, int>(m3, [=](const auto&)
        {
            return readTVar(tvar);
        });
    });

    Context ctx;
    auto result = atomically(ctx, mResult);
    QVERIFY(result == 15);
}

QTEST_APPLESS_MAIN(PSTest)

#include "tst_parsec.moc"

