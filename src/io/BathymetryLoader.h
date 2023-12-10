/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 * Loads bathymetry data from files
 **/
#ifndef TSUNAMI_LAB_BATHYMETRY_LOADER
#define TSUNAMI_LAB_BATHYMETRY_LOADER

#include "../constants.h"
#include "../patches/WavePropagation.h"
#include <string>

namespace tsunami_lab
{
  namespace io
  {
    class BathymetryLoader;
  }
}

class tsunami_lab::io::BathymetryLoader
{
private:
  //! bathymetry
  t_real *m_b = nullptr;
  //! domain size in metres in x-direction
  t_real m_sizeX = 0;
  //! domain size in metres in y-direction
  t_real m_sizeY = 0;

public:
  /**
   * Destructor
   *
   **/
  ~BathymetryLoader();

  /**
   * Load the bathymetry data from a file
   *
   * @param i_file bathymetry file path
   *
   **/
  void loadBathymetry(const std::string &i_file);

  /**
   * Gets the bathymetry at a given point.
   *
   * @param i_x point coordinate in x-direction
   * @param i_y point coordinate in y-direction
   * @return bathymetry at given point
   *
   **/
  t_real getBathymetry(t_real i_x,
                       t_real i_y)
  {
    t_idx l_coordinate = i_x + i_y * m_sizeX;
    if (l_coordinate > (m_sizeX * m_sizeY) - 1)
      return 0;
    return m_b[t_idx(i_x + i_y * m_sizeX)];
  }
};

#endif