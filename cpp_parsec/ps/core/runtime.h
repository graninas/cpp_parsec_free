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
    const std::string _source;      // TODO: investigate if we can avoid copying the source string, maybe by using a shared_ptr or something similar. For now we just keep it simple with a copy.
    State _state;

    int _many_combinator_threshold;   // Safety check to prevent infinite loops in the ParseMany combinator, in case of a bug in the raw parser or something like that. In a real implementation we might want to handle this differently, maybe by throwing an exception or something like that.

    std::vector<std::string> _messages;

public:
    ParserRuntime(const std::string& source, const State& state, int many_combinator_threshold = 1000)
        : _source(source), _state(state), _many_combinator_threshold(many_combinator_threshold)
    {
    };

    std::string_view get_view() const;

    State get_state() const;
    void put_state(const State& state);

    void push_message(const std::string& message);
    const std::vector<std::string>& get_messages() const;

    int get_many_combinator_threshold() const
    {
        return _many_combinator_threshold;
    }
};

} // namespace core
} // namespace ps

#endif // PS_CORE_RUNTIME_H
