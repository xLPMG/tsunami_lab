/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 *
 **/
#ifndef TSUNAMI_LAB_IO_LOGGER
#define TSUNAMI_LAB_IO_LOGGER

#include <string>

namespace tsunami_lab
{
    namespace io
    {
        class Logger;
    }
}

class tsunami_lab::io::Logger
{
private:
    std::string m_data;
    std::string m_lastLine;
public:
    Logger();

    void log(std::string i_line);
    void logError(std::string i_line);
    void logWarning(std::string i_line);
    void clear();
    std::string getLog(){
        return m_data;
    }
    std::string getLastLine(){
        return m_lastLine;
    }
};
#endif