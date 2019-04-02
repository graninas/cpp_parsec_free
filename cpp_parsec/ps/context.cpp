#include "context.h"

namespace ps
{

ParserRuntime::ParserRuntime(const std::string& source, size_t current)
    : _source(source)
    , _current(current)
{
}

std::string_view ParserRuntime::get_view() const
{
    return std::string_view(_source).substr(_current);
}

} // namespace ps
