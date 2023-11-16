/**
 * @author Luca Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 * implementation of station
 **/
#ifndef TSUNAMI_LAB_IO_STATION
#define TSUNAMI_LAB_IO_STATION

#include "../constants.h"

#include <vector>

namespace tsunami_lab
{
  namespace io
  {
    class Station;
  }
}

class tsunami_lab::io::Station
{
private:
  //! x position
  t_real m_x = 0;

  //! y position
  t_real m_y = 0;

  //! name of station
  std::string name = "";

  //! height
  t_real m_height = 0;

  //! bathymetry
  t_real m_bythymetry = 0;

  //! vector for all instantiated stations 
  std::vector<Station> station_vector;

public:
  /**
   * Constructor
   *
   * @param i_x position in x dierection
   * @param i_y position in y-direction
   * @param name name of station
   * @param i_h heigth at postion xy
   * @param i_b bathymetry at postion xy
   **/
  tsunami_lab::io::Station::Station(t_real i_x,
                                    t_real i_y,
                                    std::string name,
                                    t_real i_h,
                                    t_real i_b);
};
#endif