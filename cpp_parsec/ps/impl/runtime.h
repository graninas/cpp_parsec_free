#ifndef PS_IMPL_RUNTIME_H
#define PS_IMPL_RUNTIME_H

#include <chrono>
#include <thread>
#include <iostream>
#include <random>

#include "../types.h"
#include "../church/parserl.h"
#include "../context.h"

namespace ps
{

//template <typename A>
//using RunnerFunc = std::function<RunResult<A>(AtomicRuntime&)>;

//template <typename A>
//A runParser(/*Context& context,*/
//         /*const RunnerFunc<A>& runner*/
//            const ParserL<A>& p,
//            const std::string& s)
//{
//    while (true)
//    {
//        auto snapshot = context.takeSnapshot();
//        AtomicRuntime runtime {context, ustamp, snapshot};
//        RunResult<A> runResult = runner(runtime);
//        if (runResult.retry)
//        {
//            auto t = backoffIntervalDice();
////            std::cout << "Retry after: " << t * 10 << " milliseconds." << std::endl;
//            std::this_thread::sleep_for(std::chrono::milliseconds(t * 10));
//            continue;
//        }

//        bool success = context.tryCommit(ustamp, runtime.getStagedTVars());
//        if (success)
//        {
//            return runResult.result.value();
//        }
//    }
//}

} // namespace ps

#endif // PS_IMPL_RUNTIME_H
