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
#include <cstdlib>
#include <string>

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
  std::string m_name;

  //! height
  t_real m_h = 0;

  //! bathymetry
  t_real m_b = 0;

  //! vector for all instantiated stations
  std::vector<Station> station_vector;

  //! output frequency
  //t_real m_frequency = 0;

  //! filepath
  std::string m_filepath = "";

public:
  /**
   * Constructor
   *
   * @param i_x position in x dierection
   * @param i_y position in y-direction
   * @param i_name name of station
   * @param i_h heigth at postion xy
   * @param i_b bathymetry at postion xy
   **/
  Station(t_real i_x,
          t_real i_y,
          std::string i_name,
          t_real i_h,
          t_real i_b);

  ~Station();

  /**
   * Gets the water height at a given point.
   *
   * @return height of a station.
   **/
  t_real getHeight()const;

  /**
   * Gets the bathymetry
   *
   * @return bathymetry of a station.
   **/
  t_real getBathymetry()const;

  /**
   * sets height for a station
   */
  void setHeight(t_real i_h);

  /**
   * sets bathymetry for a station
   */
  void setBathymetry(t_real i_b);
};
#endif