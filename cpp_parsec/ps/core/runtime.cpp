#include "runtime.h"

namespace ps
{
namespace core
{


std::string_view ParserRuntime::get_view() const
{
    return std::string_view(_source);
}

State ParserRuntime::get_state() const
{
    return _state;
}

void ParserRuntime::put_state(const State& state)
{
    _state = state;
}

void ParserRuntime::push_message(const std::string& message)
{
    _messages.push_back(message);
}

const std::vector<std::string>& ParserRuntime::get_messages() const
{
    return _messages;
}

} // namespace core
} // namespace ps
