#ifndef PS_CONTEXT_H
#define PS_CONTEXT_H

#include <string_view>

#include "types.h"

namespace ps
{

// UTF-8 is not supported

class ParserRuntime
{
private:
    const std::string& _source;
    State _state;

    std::vector<std::string> _messages;

public:
    ParserRuntime(const std::string& source, const State& state);

    std::string_view get_view() const;
    // void advance(size_t count);  // This is not needed, since the parsers will return the new position to advance to, and the runtime will update the state accordingly.

    State get_state() const;
    void put_state(const State& state);

    void push_message(const std::string& message);
    const std::vector<std::string>& get_messages() const;
};

} // namespace ps

#endif // PS_CONTEXT_H
