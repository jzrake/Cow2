#include <ctime>
#include <iomanip>
#include <sstream>
#include "Timer.hpp"

using namespace Cow;




// ============================================================================
Timer::Timer() : timeInstantiated (std::clock())
{

}

double Timer::age() const
{
    return double (std::clock() - timeInstantiated) / CLOCKS_PER_SEC;
}

std::string Timer::ageInSeconds() const
{
    auto stream = std::ostringstream();
    stream << std::setprecision(2) << age() << " seconds";
    return stream.str();
}
