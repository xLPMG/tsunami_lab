/**
 * @author Luca Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 * implementation of station
 **/
#ifndef TSUNAMI_LAB_IO_STATION
#define TSUNAMI_LAB_IO_STATION

#include "../constants.h"
#include "../patches/WavePropagation.h"

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

  //! stride
  t_real m_stride = 0;

  //! name of station
  std::string m_name;

  //! wave propagation
  tsunami_lab::patches::WavePropagation *m_waveProp = nullptr;

  //! filepath
  std::string m_filepath = "stations";

  //! station data
  std::vector<std::vector<tsunami_lab::t_real>> *m_data = nullptr;

public:
  /**
   * Constructor
   *
   * @param i_x position in x dierection
   * @param i_y position in y-direction
   * @param i_name name the station
   * @param i_waveProp active wave propagation patch
   **/
  Station(t_real i_x,
          t_real i_y,
          std::string i_name,
          tsunami_lab::patches::WavePropagation *i_waveProp);

  ~Station();

  void capture(t_real i_time);

  void write();
};
#endif