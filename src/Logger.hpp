#ifndef Logger_hpp
#define Logger_hpp

#include <ostream>
#include <sstream>




class Logger
{
public:
    class NullBuffer : public std::streambuf
    {
    public:
        int overflow (int c) { return c; }
    };

    Logger();
    std::ostream& log (std::string caller="");
    void setLogToFile (std::string filenameToUse);
    void setLogToNull();
    void setLogToStdout();
    void flush();
    void clear();
private:
    enum LoggingMode { ToNullDevice, ToStdout, ToFile };
    LoggingMode mode;
    NullBuffer nullBuffer;
    std::ostream nullStream;
    std::string filename;
    std::ostringstream stream;
};

#endif