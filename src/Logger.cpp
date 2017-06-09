#include <cassert>
#include <iostream>
#include <fstream>
#include "Logger.hpp"




// ============================================================================
Logger::Logger() : nullStream (&nullBuffer)
{
    mode = ToStdout;
}

std::ostream& Logger::log (std::string caller)
{
    switch (mode)
    {
        case ToStdout:
        {
            if (! caller.empty())
            {
                std::cout << "[" << caller << "] ";
            }
            return std::cout;
        }
        case ToFile:
        {
            if (! caller.empty())
            {
                stream << "[" << caller << "] ";
            }
            return stream;
        }
        case ToNullDevice: return nullStream;
        default: assert (false);
    }
}

void Logger::setLogToFile (std::string filenameToUse)
{
    mode = ToFile;
    filename = filenameToUse;
}

void Logger::setLogToNull()
{
    mode = ToNullDevice;
}

void Logger::setLogToStdout()
{
    mode = ToStdout;
}

void Logger::flush()
{
    switch (mode)
    {
        case ToStdout: std::cout.flush(); break;
        case ToFile: std::ofstream (filename, std::ios_base::app) << stream.str(); break;
        case ToNullDevice: break;
    }
    clear();
}

void Logger::clear()
{
    stream.clear();
}
