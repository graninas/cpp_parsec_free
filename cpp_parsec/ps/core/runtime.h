#ifndef PS_CORE_RUNTIME_H
#define PS_CORE_RUNTIME_H

#include <string_view>

#include "types.h"


namespace ps
{
namespace core
{

class ParserRuntime
{
private:
    const std::string& _source;
    State _state;

    std::vector<std::string> _messages;

public:
    ParserRuntime(const std::string& source, const State& state);

    std::string_view get_view() const;

    State get_state() const;
    void put_state(const State& state);

    void push_message(const std::string& message);
    const std::vector<std::string>& get_messages() const;
};

} // namespace core
} // namespace ps

#endif // PS_CORE_RUNTIME_H
