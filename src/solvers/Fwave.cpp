/**
 * @author Luca-Philipp Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 * Fwave solver for the one-dimensional shallow water equations.
 **/
#include "Fwave.h"
#include <cmath>
#include <iostream>

void tsunami_lab::solvers::Fwave::computeEigenvalues(t_real i_hL,
                                                     t_real i_hR,
                                                     t_real i_uL,
                                                     t_real i_uR,
                                                     t_real &eigenvalueRoe_1,
                                                     t_real &eigenvalueRoe_2)
{
  // pre-compute square-root ops
  t_real l_hSqrtL = std::sqrt(i_hL);
  t_real l_hSqrtR = std::sqrt(i_hR);

  // compute Roe averages
  t_real l_hRoe = t_real(0.5) * (i_hL + i_hR);
  t_real l_uRoe = l_hSqrtL * i_uL + l_hSqrtR * i_uR;
  l_uRoe /= l_hSqrtL + l_hSqrtR;

  // compute eigenvalues
  t_real l_ghSqrtRoe = m_gSqrt * std::sqrt(l_hRoe);
  eigenvalueRoe_1 = l_uRoe - l_ghSqrtRoe;
  eigenvalueRoe_2 = l_uRoe + l_ghSqrtRoe;
}

void tsunami_lab::solvers::Fwave::computeEigencoefficients(t_real i_hL,
                                                           t_real i_hR,
                                                           t_real i_huL,
                                                           t_real i_huR,
                                                           t_real eigenvalueRoe_1,
                                                           t_real eigenvalueRoe_2,
                                                           t_real &alpha_1,
                                                           t_real &alpha_2)
{
  // compute inverse of right eigenvector-matrix
  t_real l_detInv = 1 / (eigenvalueRoe_2 - eigenvalueRoe_1);

  t_real l_rInv[2][2] = {{0}};
  l_rInv[0][0] = l_detInv * eigenvalueRoe_2;
  l_rInv[0][1] = -l_detInv;
  l_rInv[1][0] = -l_detInv * eigenvalueRoe_1;
  l_rInv[1][1] = l_detInv;

  // compute particle velocities
  t_real l_uL = i_huL / i_hL;
  t_real l_uR = i_huR / i_hR;

  // compute f delta
  t_real f_delta[2] = {0};
  f_delta[0] = i_huR - i_huL;
  f_delta[1] = (i_huR * l_uR + t_real(0.5) * m_g * i_hR * i_hR) - (i_huL * l_uL + t_real(0.5) * m_g * i_hL * i_hL);

  // compute alpha
  alpha_1 = l_rInv[0][0] * f_delta[0] + l_rInv[0][1] * f_delta[1];
  alpha_2 = l_rInv[1][0] * f_delta[0] + l_rInv[1][1] * f_delta[1];
}

void tsunami_lab::solvers::Fwave::netUpdates(t_real i_hL,
                                             t_real i_hR,
                                             t_real i_huL,
                                             t_real i_huR,
                                             t_real i_bl,
                                             t_real i_br,
                                             t_real o_netUpdateL[2],
                                             t_real o_netUpdateR[2])
{

  // compute particle velocities
  t_real l_uL = i_huL / i_hL;
  t_real l_uR = i_huR / i_hR;

  // compute eigenvalues
  t_real eigenvalueRoe_1 = 0;
  t_real eigenvalueRoe_2 = 0;

  computeEigenvalues(i_hL,
                     i_hR,
                     l_uL,
                     l_uR,
                     eigenvalueRoe_1,
                     eigenvalueRoe_2);

  // compute eigenvectors
  t_real eigenvectorRoe_1[2] = {1, eigenvalueRoe_1};
  t_real eigenvectorRoe_2[2] = {1, eigenvalueRoe_2};

  // compute eigencoefficients
  t_real eigencoefficientRoe_1 = 0;
  t_real eigencoefficientRoe_2 = 0;

  computeEigencoefficients(i_hL,
                           i_hR,
                           i_huL,
                           i_huR,
                           eigenvalueRoe_1,
                           eigenvalueRoe_2,
                           eigencoefficientRoe_1,
                           eigencoefficientRoe_2);

  // compute waves Z_p
  t_real z1[2] = {0};
  z1[0] = eigencoefficientRoe_1 * eigenvectorRoe_1[0];
  z1[1] = eigencoefficientRoe_1 * eigenvectorRoe_1[1];

  t_real z2[2] = {0};
  z2[0] = eigencoefficientRoe_2 * eigenvectorRoe_2[0];
  z2[1] = eigencoefficientRoe_2 * eigenvectorRoe_2[1];

  //computing dealtaX_phi
  t_real x_phi[2];
  x_phi[0] = 0;
  x_phi[1] = -m_g * (i_br - i_bl) * ((i_hL + i_hR) / 2);

  // set net-updates depending on wave speeds
  for (unsigned short l_qt = 0; l_qt < 2; l_qt++)
  {
    // init
    o_netUpdateL[l_qt] = 0;
    o_netUpdateR[l_qt] = 0;

    // wave 1
    if (eigenvalueRoe_1 < 0)
      o_netUpdateL[l_qt] += z1[l_qt] - x_phi[l_qt];
      
    else
      o_netUpdateR[l_qt] += z1[l_qt] - x_phi[l_qt];

    // wave 2
    if (eigenvalueRoe_2 < 0){
      o_netUpdateL[l_qt] += z2[l_qt] - x_phi[l_qt];
      std::cout<< o_netUpdateL[l_qt]<<std::endl;}
    else{
      o_netUpdateR[l_qt] += z2[l_qt] - x_phi[l_qt];
      std::cout<< o_netUpdateL[l_qt]<<std::endl;}
  }
}
