#include <cstdio>
#include <cstdlib>
#include <libunwind.h>
#include <cxxabi.h>
#include "DebugHelper.hpp"




void Cow::backtrace()
{
    unw_cursor_t cursor;
    unw_context_t context;

    // Initialize cursor to current frame for local unwinding.
    unw_getcontext (&context);
    unw_init_local (&cursor, &context);

    // Unwind frames one by one, going up the frame stack.
    while (unw_step (&cursor) > 0)
    {
        unw_word_t offset, pc;
        unw_get_reg (&cursor, UNW_REG_IP, &pc);

        if (pc == 0)
        {
            break;
        }
        std::printf ("0x%llx:", pc);

        char sym[256];

        if (unw_get_proc_name (&cursor, sym, sizeof(sym), &offset) == 0)
        {
            int status;
            char* nameptr = sym;
            char* demangled = abi::__cxa_demangle (sym, nullptr, nullptr, &status);

            if (status == 0)
            {
                nameptr = demangled;
            }
            std::printf (" (%s+0x%llx)\n", nameptr, offset);
            std::free (demangled);
        }
        else
        {
            std::printf (" -- error: unable to obtain symbol name for this frame\n");
        }
    }
}

void Cow::terminateWithBacktrace()
{
    Cow::backtrace();
    exit (1);
}
