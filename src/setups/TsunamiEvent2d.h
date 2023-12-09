/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 * Two-dimensional  tsunami event
 **/
#ifndef TSUNAMI_LAB_SETUPS_TSUNAMI_EVENT_2D
#define TSUNAMI_LAB_SETUPS_TSUNAMI_EVENT_2D

#include "Setup.h"
#include "../io/NetCdf.h"
#include <cmath>

namespace tsunami_lab
{
  namespace setups
  {
    class TsunamiEvent2d;
  }
}

/**
 * 2d dam break setup.
 **/
class tsunami_lab::setups::TsunamiEvent2d : public Setup
{
private:
  // value for delta
  t_real m_delta = 20;

  // stride size
  t_idx m_stride = 0;

  // amount of cells
  t_idx m_nxB = 0, m_nyB = 0;
  // bathymetry data x
  t_real *m_xDataB = nullptr;
  // bathymetry data y
  t_real *m_yDataB = nullptr;
  // bathymetry
  t_real *m_b = nullptr;
  //
  t_idx m_lastNegativeIndexBX = 0;
  t_idx m_lastNegativeIndexBY = 0;

  // displacements
  // amount of cells
  t_idx m_nxD = 0, m_nyD = 0;
  // displacement data x
  t_real *m_xDataD = nullptr;
  // displacement data x
  t_real *m_yDataD = nullptr;
  // displacement
  t_real *m_d = nullptr;
  //
  t_idx m_lastNegativeIndexDX = 0;
  t_idx m_lastNegativeIndexDY = 0;

  /**
   * gets the value for bathymetry at a point
   * @param i_x x position
   * @param i_y y position
   * @return bathymetry value
   */
  t_real getBathymetryFromArray(t_real i_x,
                                t_real i_y) const;

  /**
   * gets the value for displacement at a point
   * @param i_x x position
   * @param i_y y position
   * @return displacment value
   */
  t_real getDisplacementFromArray(t_real i_x,
                                  t_real i_y) const;

public:
  /**
   * Constructor.
   * @param i_bathymetryPath path to file containing bathymetry data
   * @param i_displacementPath path to file containing bathymetry data
   * @param i_netCdf NetCdf instance
   * @param i_stride stride
   **/
  TsunamiEvent2d(const char *i_bathymetryPath,
                 const char *i_displacementPath,
                 tsunami_lab::io::NetCdf *i_netCdf,
                 t_idx i_stride);

  /**
   * Destructor.
   * 
   **/
  ~TsunamiEvent2d();

  /**
   * Gets the water height at a given point.
   * @param i_x x position
   * @param i_y y position
   * @return height at the given point.
   **/
  t_real getHeight(t_real i_x,
                   t_real i_y) const;

  /**
   * Gets the momentum in x-direction.
   *
   * @return momentum in x-direction.
   **/
  t_real getMomentumX(t_real,
                      t_real) const;

  /**
   * Gets the momentum in y-direction.
   *
   * @return momentum in y-direction.
   **/
  t_real getMomentumY(t_real,
                      t_real) const;

  /**
   * Gets the bathymetry
   * @param i_x x position
   * @param i_y y position
   * @return bathymetry.
   **/
  t_real getBathymetry(t_real i_x,
                       t_real i_y) const;
};

#endif