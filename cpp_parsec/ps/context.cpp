#include "context.h"

namespace ps
{

ParserRuntime::ParserRuntime(const std::string& source, const State& state)
    : _source(source)
    , _state(state)
{
}

std::string_view ParserRuntime::get_view() const
{
    return std::string_view(_source).substr(_state.pos);
}

void ParserRuntime::advance(size_t count)
{
    _state.pos += count;
}

State ParserRuntime::get_state() const
{
    return _state;
}

void ParserRuntime::put_state(const State& state)
{
    _state = state;
}


ParserResult<std::string> parseLit(
        ParserRuntime& runtime,
        const std::string& litS
        )
{
    std::string_view s = runtime.get_view();
    std::string failedMsg = std::string("Failed to parse ") + litS;

    if (s.size() < litS.size())
    {
        return { ParserFailed {failedMsg + ": end of imput."} };
    }
    else if (s.find(litS) != 0)
    {
        return { ParserFailed {failedMsg} };
    }

    return ParserSucceeded<std::string>{ litS };
}

} // namespace ps
