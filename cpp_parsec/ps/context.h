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


template <typename Single>
ParserResult<Single> parseSingle(
        ParserRuntime& runtime,
        const std::function<bool(char)>& validator,
        const std::function<Single(char)>& converter,
        const std::string& name
        )
{
    std::string_view s = runtime.get_view();
    std::string failedMsg = std::string("Failed to parse ") + name;

    if (s.empty())
    {
        return { ParserFailed {failedMsg + ": end of imput."} };
    }
    else if (!validator(s.at(0)))
    {
        return { ParserFailed {failedMsg + ": not a " + name + "."} };
    }

    ParserSucceeded<Single> r;
    r.parsed = converter(s.at(0));
    return { r };
}

} // namespace ps

#endif // PS_CONTEXT_H
