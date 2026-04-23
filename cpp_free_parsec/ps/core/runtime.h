#ifndef PS_CORE_RUNTIME_H
#define PS_CORE_RUNTIME_H

#include <string_view>

#include "types.h"


namespace ps
{
namespace core
{

template <typename Dummy = int>
class ParserRuntimeImpl
{
private:
    const std::string& _source;
    State _state;

    int _manyCombinatorThreshold;   // Safety check to prevent infinite loops in the ParseMany combinator, in case of a bug in the raw parser or something like that. In a real implementation we might want to handle this differently, maybe by throwing an exception or something like that.
    bool _debugPrint;
    std::vector<std::string> _messages;

public:
  ParserRuntimeImpl(const std::string &source,
                    const State &state,
                    int many_combinator_threshold = 1000,
                    bool debugPrint = false)
      : _source(source), _state(state), _manyCombinatorThreshold(many_combinator_threshold), _debugPrint(debugPrint) {
        };

  std::string_view getView() const
  {
    return std::string_view(_source);
  }

    State getState() const
    {
      return _state;
    }

    void putState(const State& state)
    {
      _state = state;
    }

    void pushMessage(const std::string& message)
    {
      if (_debugPrint)
        std::cout << message;

      _messages.push_back(message);
    }

    const std::vector<std::string>& getMessages() const
    {
      return _messages;
    }

    void adoptMessages(const ParserRuntimeImpl&other)
    {
      auto messages = other.getMessages();
      for (const auto &msg : messages)
      {
        pushMessage(msg);
      }
    }

    void clearMessages()
    {
      _messages.clear();
    }

    ParserRuntimeImpl cloneClean() const
    {
      return ParserRuntimeImpl(_source, _state, _manyCombinatorThreshold);
    }

    int getManyCombinatorThreshold() const
    {
        return _manyCombinatorThreshold;
    }
};

using ParserRuntime = ParserRuntimeImpl<int>;


} // namespace core
} // namespace ps

#endif // PS_CORE_RUNTIME_H
