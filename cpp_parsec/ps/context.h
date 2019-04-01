#ifndef PS_CONTEXT_H
#define PS_CONTEXT_H

#include <optional>

#include "types.h"

namespace ps
{

//using UStamp = Id;

//struct TVarHandle
//{
//    UStamp ustamp;
//    std::any data;
//    bool modified;
//};

//using TVars = std::map<TVarId, TVarHandle>;

//class Context
//{
//private:

//    std::atomic<Id> _id;
//    TVars _tvars;

//    std::mutex _lock;

//public:
//    Context();

//    bool tryCommit(const UStamp& ustamp, const TVars& stagedTvars);

//    Id newId();
//    TVars takeSnapshot();
//};

// UTF-8 is not supported

class ParserRuntime
{
private:
    std::string& _source;
    size_t _current;

public:
    ParserRuntime(std::string& source, size_t current);

    bool has_more(size_t count) const;
};

template <typename A>
struct RunResult
{
    ps::Either<ParseError, A> result;
};

} // namespace ps

#endif // PS_CONTEXT_H
