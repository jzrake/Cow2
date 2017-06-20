#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include "DebugHelper.hpp"
#include "CowBuildConfig.hpp"

#define GUARD_STRING std::string (80, '-')




#ifdef COW_HAVE_LIBUNWIND
#include <libunwind.h>
#include <cxxabi.h>
void Cow::backtrace()
{
    std::cout << GUARD_STRING << std::endl;
    std::cout << "Backtrace:\n";
    std::cout << GUARD_STRING << std::endl;

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
#else
void Cow::backtrace()
{
    std::cout << GUARD_STRING << std::endl;
    std::cout << "No backtrace available (recompile with COW_HAVE_LIBUNWIND)\n";
    std::cout << GUARD_STRING << std::endl;
}
#endif

void Cow::terminateWithBacktrace()
{
    try
    {
        auto e = std::current_exception();

        if (e)
            std::rethrow_exception (e);
    }
    catch (std::exception& e)
    {
        std::cout << GUARD_STRING << std::endl;
        std::cout << "Uncaught exception: "<< e.what() << std::endl;
    }

    Cow::backtrace();
    exit (1);
}

void Cow::terminateWithPrintException()
{
    try
    {
        auto e = std::current_exception();

        if (e)
            std::rethrow_exception (e);
    }
    catch (std::exception& e)
    {
        std::cout << GUARD_STRING << std::endl;
        std::cout << "Uncaught exception: "<< e.what() << std::endl;
    }
    exit (1);
}
