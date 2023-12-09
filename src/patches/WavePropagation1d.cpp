/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 * One-dimensional wave propagation patch.
 **/
#include "WavePropagation1d.h"
#include "../solvers/Roe.h"
#include "../solvers/Fwave.h"
#include <string>

tsunami_lab::patches::WavePropagation1d::WavePropagation1d(t_idx i_nCells,
                                                           const std::string &i_solver,
                                                           Boundary i_boundaryL,
                                                           Boundary i_boundaryR)
{
  m_nCells = i_nCells;
  m_solver = i_solver;
  m_boundaryL = i_boundaryL;
  m_boundaryR = i_boundaryR;

  // allocate memory including a single ghost cell on each side
  for (unsigned short l_st = 0; l_st < 2; l_st++)
  {
    m_h[l_st] = new t_real[m_nCells + 2];
    m_hu[l_st] = new t_real[m_nCells + 2];
  }
  m_b = new t_real[m_nCells + 2];

  // init to zero
  for (unsigned short l_st = 0; l_st < 2; l_st++)
  {
    for (t_idx l_ce = 0; l_ce < m_nCells + 2; l_ce++)
    {
      m_h[l_st][l_ce] = 0;
      m_hu[l_st][l_ce] = 0;
      m_b[l_ce] = 0;
    }
  }
}

tsunami_lab::patches::WavePropagation1d::~WavePropagation1d()
{
  for (unsigned short l_st = 0; l_st < 2; l_st++)
  {
    delete[] m_h[l_st];
    delete[] m_hu[l_st];
  }
  delete[] m_b;
}

void tsunami_lab::patches::WavePropagation1d::timeStep(t_real i_scaling,
                                                       t_real)
{
  // pointers to old and new data
  t_real *l_hOld = m_h[m_step];
  t_real *l_huOld = m_hu[m_step];

  m_step = (m_step + 1) % 2;
  t_real *l_hNew = m_h[m_step];
  t_real *l_huNew = m_hu[m_step];

  // init new cell quantities
  for (t_idx l_ce = 1; l_ce < m_nCells + 1; l_ce++)
  {
    l_hNew[l_ce] = l_hOld[l_ce];
    l_huNew[l_ce] = l_huOld[l_ce];
  }

  // iterate over edges and update with Riemann solutions
  for (t_idx l_ed = 0; l_ed < m_nCells + 1; l_ed++)
  {
    // determine left and right cell-id
    t_idx l_ceL = l_ed;
    t_idx l_ceR = l_ed + 1;

    t_real l_hL = l_hOld[l_ceL];
    t_real l_hR = l_hOld[l_ceR];
    t_real l_huL = l_huOld[l_ceL];
    t_real l_huR = l_huOld[l_ceR];
    t_real l_bL = m_b[l_ceL];
    t_real l_bR = m_b[l_ceR];

    // handle reflections
    handleReflections(l_hOld,
                      l_huOld,
                      l_ceL,
                      l_ceR,
                      l_hL,
                      l_hR,
                      l_huL,
                      l_huR,
                      l_bL,
                      l_bR);

    // compute net-updates
    t_real l_netUpdates[2][2];
    if (m_solver == "roe")
    {
      solvers::Roe::netUpdates(l_hL,
                               l_hR,
                               l_huL,
                               l_huR,
                               l_netUpdates[0],
                               l_netUpdates[1]);
    }
    else if (m_solver == "fwave")
    {
      solvers::Fwave::netUpdates(l_hL,
                                 l_hR,
                                 l_huL,
                                 l_huR,
                                 l_bL,
                                 l_bR,
                                 l_netUpdates[0],
                                 l_netUpdates[1]);
    }
    if (l_hOld[l_ceL] > 0)
    {
      l_hNew[l_ceL] -= i_scaling * l_netUpdates[0][0];
      l_huNew[l_ceL] -= i_scaling * l_netUpdates[0][1];
    }
    else
    {
      l_hNew[l_ceL] = 0;
      l_huNew[l_ceL] = 0;
    }

    if (l_hOld[l_ceR] > 0)
    {
      l_hNew[l_ceR] -= i_scaling * l_netUpdates[1][0];
      l_huNew[l_ceR] -= i_scaling * l_netUpdates[1][1];
    }
    else
    {
      l_hNew[l_ceR] = 0;
      l_huNew[l_ceR] = 0;
    }
  }
}

void tsunami_lab::patches::WavePropagation1d::setGhostOutflow()
{
  t_real *l_h = m_h[m_step];
  t_real *l_hu = m_hu[m_step];
  t_real *l_b = m_b;

  // left boundary
  if (m_boundaryL == WALL)
  {
    l_h[0] = 0;
  }
  else
  {
    l_h[0] = l_h[1];
    l_hu[0] = l_hu[1];
    l_b[0] = l_b[1];
  }
  // right boundary
  if (m_boundaryR == WALL)
  {
    l_h[m_nCells + 1] = 0;
  }
  else
  {
    l_h[m_nCells + 1] = l_h[m_nCells];
    l_hu[m_nCells + 1] = l_hu[m_nCells];
    l_b[m_nCells + 1] = l_b[m_nCells];
  }
}

void tsunami_lab::patches::WavePropagation1d::handleReflections(t_real *i_h,
                                                                t_real *i_hu,
                                                                t_idx i_ceL,
                                                                t_idx i_ceR,
                                                                t_real &o_hL,
                                                                t_real &o_hR,
                                                                t_real &o_huL,
                                                                t_real &o_huR,
                                                                t_real &o_bL,
                                                                t_real &o_bR)
{
  // use margin for comparison in case of rounding errors
  tsunami_lab::t_real margin = 0.00001;
  if (i_h[i_ceR] <= margin)
  {
    // right cell dry
    o_hR = i_h[i_ceL];
    o_bR = m_b[i_ceL];
    o_huR = -i_hu[i_ceL];
  }
  else if (i_h[i_ceL] <= margin)
  {
    // left cell dry
    o_hL = i_h[i_ceR];
    o_bL = m_b[i_ceR];
    o_huL = -i_hu[i_ceR];
  }
}
