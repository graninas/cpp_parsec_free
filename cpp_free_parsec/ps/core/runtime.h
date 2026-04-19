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

    int _manyCombinatorThreshold;   // Safety check to prevent infinite loops in the ParseMany combinator, in case of a bug in the raw parser or something like that. In a real implementation we might want to handle this differently, maybe by throwing an exception or something like that.

    std::vector<std::string> _messages;

public:
    ParserRuntime(const std::string& source, const State& state, int many_combinator_threshold = 1000)
        : _source(source), _state(state), _manyCombinatorThreshold(many_combinator_threshold)
    {
    };

    std::string_view getView() const;

    State getState() const;
    void putState(const State& state);

    void pushMessage(const std::string& message);
    const std::vector<std::string>& getMessages() const;
    void clearMessages();

    int getManyCombinatorThreshold() const
    {
        return _manyCombinatorThreshold;
    }
};

} // namespace core
} // namespace ps

#endif // PS_CORE_RUNTIME_H
