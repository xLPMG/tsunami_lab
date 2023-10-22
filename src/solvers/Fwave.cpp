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

  // compute wave speeds
  t_real l_ghSqrtRoe = m_gSqrt * std::sqrt( l_hRoe );
  o_waveSpeedL = l_uRoe - l_ghSqrtRoe;
  o_waveSpeedR = l_uRoe + l_ghSqrtRoe;
}

void tsunami_lab::solvers::Fwave::waveStrengths( t_real   i_hL,
                                               t_real   i_hR,
                                               t_real   i_huL,
                                               t_real   i_huR,
                                               t_real   i_waveSpeedL,
                                               t_real   i_waveSpeedR,
                                               t_real & o_strengthL,
                                               t_real & o_strengthR ) {
  // compute inverse of right eigenvector-matrix
  t_real l_detInv = 1 / (i_waveSpeedR - i_waveSpeedL);

  t_real l_rInv[2][2] = {{0}};
  l_rInv[0][0] =  l_detInv * i_waveSpeedR;
  l_rInv[0][1] = -l_detInv;
  l_rInv[1][0] = -l_detInv * i_waveSpeedL;
  l_rInv[1][1] =  l_detInv;

  // compute jump in quantities
  t_real l_hJump  = i_hR  - i_hL;
  t_real l_huJump = i_huR - i_huL;

  // compute wave strengths
  o_strengthL  = l_rInv[0][0] * l_hJump;
  o_strengthL += l_rInv[0][1] * l_huJump;

  o_strengthR  = l_rInv[1][0] * l_hJump;
  o_strengthR += l_rInv[1][1] * l_huJump;
}

void tsunami_lab::solvers::Fwave::netUpdates( t_real i_hL,
                                            t_real i_hR,
                                            t_real i_huL,
                                            t_real i_huR,
                                            t_real o_netUpdateL[2],
                                            t_real o_netUpdateR[2] ) {
  // compute particle velocities
  t_real l_uL = i_huL / i_hL;
  t_real l_uR = i_huR / i_hR;

  // compute wave speeds
  t_real l_sL = 0;
  t_real l_sR = 0;

  waveSpeeds( i_hL,
              i_hR,
              l_uL,
              l_uR,
              l_sL,
              l_sR );

  // compute wave strengths
  t_real l_aL = 0;
  t_real l_aR = 0;

  waveStrengths( i_hL,
                 i_hR,
                 i_huL,
                 i_huR,
                 l_sL,
                 l_sR,
                 l_aL,
                 l_aR );
}