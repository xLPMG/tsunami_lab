/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * Two-dimensional  tsunami event
 **/
#ifndef TSUNAMI_LAB_SETUPS_TSUNAMI_EVENT_2D
#define TSUNAMI_LAB_SETUPS_TSUNAMI_EVENT_2D

#include "Setup.h"
#include  <cmath> 

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

    t_real * m_b = nullptr;

    tsunami_lab::setups::Setup *m_artificial = nullptr;

public:
  /**
   * Constructor.
   *
   **/
  TsunamiEvent2d(t_real * i_b,t_idx i_stride);

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