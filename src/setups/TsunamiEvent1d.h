/**
 * @author Luca Grombacher, Richard Hofmann
 *
 * @section DESCRIPTION
 * Implementation of One-dimensional Tsunami event
 **/

#ifndef TSUNAMI_LAB_SETUPS_TSUNAMI_EVENT_1D_H
#define TSUNAMI_LAB_SETUPS_TSUNAMI_EVENT_1D_H

#include "Setup.h"
#include <fstream>
#include <sstream> 

namespace tsunami_lab
{
  namespace setups
  {
    class TsunamiEvent1d;
  }
}

class tsunami_lab::setups::TsunamiEvent1d : public Setup
{
private:

public:
    /**
     * 
     * 
     **/
    tsunami_lab::setups::TsunamiEvent1d::TsunamiEvent1d(std::ifstream &i_file){

    }

};
#endif