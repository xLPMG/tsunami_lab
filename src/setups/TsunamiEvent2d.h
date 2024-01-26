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
  //! value for delta
  t_real m_delta = 20;
  //! bathymetry file path
  const char *m_bathymetryPath;
  //! displacement file path
  const char *m_displacementPath;
  //! offset of the bathymetry domain in x-direction
  t_real m_bathymetryOffsetX = 0;
  //! offset of the displacement domain in y-direction
  t_real m_bathymetryOffsetY = 0;
  //! offset of the displacement domain in x-direction
  t_real m_displacementOffsetX = 0;
  //! offset of the displacement domain in y-direction
  t_real m_displacementOffsetY = 0;
  //! distance between to bathymetry samples in x-direction
  t_real m_bathymetrySampleDistanceX = 0;
  //! inverse distance between to bathymetry samples in x-direction
  t_real m_bathymetrySampleDistanceXInverse = 0;
  //! distance between to bathymetry samples in y-direction
  t_real m_bathymetrySampleDistanceY = 0;
  //! inverse distance between to bathymetry samples in y-direction
  t_real m_bathymetrySampleDistanceYInverse = 0;
  //! distance between to displacement samples in x-direction
  t_real m_displacementSampleDistanceX = 0;
  //! inverse distance between to displacement samples in x-direction
  t_real m_displacementSampleDistanceXInverse = 0;
  //! distance between to displacement samples in y-direction
  t_real m_displacementSampleDistanceY = 0;
  //! inverse distance between to displacement samples in y-direction
  t_real m_displacementSampleDistanceYInverse = 0;
  //! amount of cells bathymetry
  t_idx m_nxB = 0, m_nyB = 0;
  //! bathymetry data x
  t_real *m_xDataB = nullptr;
  //! bathymetry data y
  t_real *m_yDataB = nullptr;
  //! bathymetry
  t_real *m_b = nullptr;

  //! amount of cells displacement
  t_idx m_nxD = 0, m_nyD = 0;
  //! displacement data x
  t_real *m_xDataD = nullptr;
  //! displacement data x
  t_real *m_yDataD = nullptr;
  //! displacement
  t_real *m_d = nullptr;

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
   * 
   * @param i_bathymetryPath path to file containing bathymetry data
   * @param i_displacementPath path to file containing bathymetry data
   * @param o_domainSizeX size of the domain in x-direction
   * @param o_domainSizeY size of the domain in y-direction
   * @param o_offsetX offset of the domain in x-direction
   * @param o_offsetY offset of the domain in y-direction
   **/
  TsunamiEvent2d(const char *i_bathymetryPath,
                 const char *i_displacementPath,
                 t_real &o_domainSizeX,
                 t_real &o_domainSizeY,
                 t_real &o_offsetX,
                 t_real &o_offsetY);

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