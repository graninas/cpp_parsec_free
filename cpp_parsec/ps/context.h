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

public:
    ParserRuntime(const std::string& source, const State& state);

    std::string_view get_view() const;
    void advance(size_t count);

    State get_state() const;
    void put_state(const State& state);
};

} // namespace ps

#endif // PS_CONTEXT_H
