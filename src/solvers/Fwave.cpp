/**
 * @author Luca-Philipp Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 * Fwave solver for the one-dimensional shallow water equations.
 **/
#include "Fwave.h"
#include <cmath>

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
                                                           t_real i_bL,
                                                           t_real i_bR,
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

  // compute f delta
  t_real l_fDelta[2] = {0};
  l_fDelta[0] = i_huR - i_huL;
  l_fDelta[1] = (i_huR * i_huR / i_hR + t_real(0.5) * m_g * i_hR * i_hR) 
              - (i_huL * i_huL / i_hL + t_real(0.5) * m_g * i_hL * i_hL);

  // computing delta x psi
  t_real l_xPsi;
  l_xPsi = -m_g * (i_bR - i_bL) * (t_real(0.5) *(i_hL + i_hR));
  l_fDelta[1] -= l_xPsi;

  // compute alpha
  alpha_1 = l_rInv[0][0] * l_fDelta[0] + l_rInv[0][1] * l_fDelta[1];
  alpha_2 = l_rInv[1][0] * l_fDelta[0] + l_rInv[1][1] * l_fDelta[1];
}

void tsunami_lab::solvers::Fwave::netUpdates(t_real i_hL,
                                             t_real i_hR,
                                             t_real i_huL,
                                             t_real i_huR,
                                             t_real i_bL,
                                             t_real i_bR,
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

  // compute eigencoefficients
  t_real eigencoefficientRoe_1 = 0;
  t_real eigencoefficientRoe_2 = 0;

  computeEigencoefficients(i_hL,
                           i_hR,
                           i_huL,
                           i_huR,
                           i_bL,
                           i_bR,
                           eigenvalueRoe_1,
                           eigenvalueRoe_2,
                           eigencoefficientRoe_1,
                           eigencoefficientRoe_2);

  // compute waves Z_p
  t_real z1[2] = {0};
  z1[0] = eigencoefficientRoe_1;
  z1[1] = eigencoefficientRoe_1 * eigenvalueRoe_1;

  t_real z2[2] = {0};
  z2[0] = eigencoefficientRoe_2;
  z2[1] = eigencoefficientRoe_2 * eigenvalueRoe_2;

  // set net-updates depending on wave speeds
  for (unsigned short l_qt = 0; l_qt < 2; l_qt++)
  {
    // init
    o_netUpdateL[l_qt] = 0;
    o_netUpdateR[l_qt] = 0;

    // wave 1
    if (eigenvalueRoe_1 < 0)
      o_netUpdateL[l_qt] += z1[l_qt];
    else
      o_netUpdateR[l_qt] += z1[l_qt];

    // wave 2
    if (eigenvalueRoe_2 < 0)
      o_netUpdateL[l_qt] += z2[l_qt];
    else
      o_netUpdateR[l_qt] += z2[l_qt];
  }
}
