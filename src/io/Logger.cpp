#include "Logger.h"

tsunami_lab::io::Logger::Logger(){
    m_data = "Logger started.";
}

void tsunami_lab::io::Logger::log(std::string i_line){
    m_data.append("\n"+i_line);
    m_lastLine = i_line;
}

void tsunami_lab::io::Logger::logError(std::string i_line){
    m_data.append("\n[ERROR] "+i_line);
    m_lastLine = i_line;
}

void tsunami_lab::io::Logger::logWarning(std::string i_line){
    m_data.append("\n[WARNING] "+i_line);
    m_lastLine = i_line;
}

void tsunami_lab::io::Logger::clear(){
    m_data.clear();
    m_lastLine.clear();
}