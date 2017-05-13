#ifndef DebugHelper_hpp
#define DebugHelper_hpp

namespace Cow
{
    void backtrace();
    void terminateWithBacktrace();
}

#ifndef COW_DEBUG_USE_CASSERT
#undef assert
#define assert(predicate) if (! (predicate)) Cow::terminateWithBacktrace()
#endif

#endif
