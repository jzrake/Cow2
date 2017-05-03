#include <ctime>
#include "Timer.hpp"

using namespace Cow;




// ============================================================================
Timer::Timer() : timeInstantiated (std::clock())
{

}

double Timer::age()
{
    return double (std::clock() - timeInstantiated) / CLOCKS_PER_SEC;
}
