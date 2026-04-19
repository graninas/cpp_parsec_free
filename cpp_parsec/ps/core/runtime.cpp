#include "runtime.h"

namespace ps
{
namespace core
{


std::string_view ParserRuntime::getView() const
{
    return std::string_view(_source);
}

State ParserRuntime::getState() const
{
    return _state;
}

void ParserRuntime::putState(const State& state)
{
    _state = state;
}

void ParserRuntime::pushMessage(const std::string& message)
{
    _messages.push_back(message);
}

const std::vector<std::string>& ParserRuntime::getMessages() const
{
    return _messages;
}

} // namespace core
} // namespace ps
