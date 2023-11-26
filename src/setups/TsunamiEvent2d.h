/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
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
  t_real m_delta = 20;

  t_idx m_stride = 0;

  // bathymetry
  t_idx m_nxB = 0, m_nyB = 0;
  t_real *m_xDataB = nullptr;
  t_real *m_yDataB = nullptr;
  t_real *m_b = nullptr;

  // displacements
  t_idx m_nxD = 0, m_nyD = 0;
  t_real *m_xDataD = nullptr;
  t_real *m_yDataD = nullptr;
  t_real *m_d = nullptr;

  t_real getBathymetryFromArray(t_real,
                                t_real) const;
  t_real getDisplacementFromArray(t_real,
                                  t_real) const;

public:
  /**
   * Constructor.
   * @param bathymetryPath path to file containing bathymetry data
   * @param displacementPath path to file containing bathymetry data
   * @param i_stride stride
   **/
  TsunamiEvent2d(const char *i_bathymetryPath,
                 const char *i_displacementPath,
                 tsunami_lab::io::NetCdf *i_netCdf,
                 t_idx i_stride);

  /**
   * Gets the water height at a given point.
   *
   * @return height at the given point.
   **/
  t_real getHeight(t_real,
                   t_real) const;

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
   *
   * @return bathymetry.
   **/
  t_real getBathymetry(t_real i_x,
                       t_real i_y) const;
};

#endif