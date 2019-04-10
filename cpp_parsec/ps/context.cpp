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

} // namespace ps
