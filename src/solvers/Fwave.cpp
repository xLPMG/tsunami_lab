/**
 * @author Luca-Philipp Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 * Fwave
 **/
#include "Fwave.h"
#include <cmath>

void tsunami_lab::solvers::Fwave::waveSpeeds( t_real   i_hL,
                                            t_real   i_hR,
                                            t_real   i_uL,
                                            t_real   i_uR,
                                            t_real & o_waveSpeedL,
                                            t_real & o_waveSpeedR ) {
  // pre-compute square-root ops
  t_real l_hSqrtL = std::sqrt( i_hL );
  t_real l_hSqrtR = std::sqrt( i_hR );

  // compute Roe averages
  t_real l_hRoe = 0.5f * ( i_hL + i_hR );
  t_real l_uRoe = l_hSqrtL * i_uL + l_hSqrtR * i_uR;
  l_uRoe /= l_hSqrtL + l_hSqrtR;

  // compute eigen values
  t_real l_ghSqrtRoe = m_gSqrt * std::sqrt( l_hRoe );
  t_real eigenValue_roe_1 = l_uRoe - l_ghSqrtRoe;
  t_real eigenValue_roe_2 = l_uRoe + l_ghSqrtRoe;

  // compute eigen vectors
  t_real eigenVector_roe_1[2] = {1, eigenValue_roe_1};
  t_real eigenVector_roe_2[2] = {1, eigenValue_roe_2};
}