/**
 * @author Alexander Breuer (alex.breuer AT uni-jena.de)
 *
 * @section DESCRIPTION
 * One-dimensional wave propagation patch.
 **/
#include "WavePropagation2d.h"
#include "../solvers/Roe.h"
#include "../solvers/Fwave.h"
#include <string>

tsunami_lab::patches::WavePropagation2d::WavePropagation2d(t_idx i_nCellsX,
                                                           t_idx i_nCellsY,
                                                           const std::string &i_solver,
                                                           bool i_hasBoundaryL,
                                                           bool i_hasBoundaryR,
                                                           bool l_hasBoundaryU,
                                                           bool l_hasBoundaryD)
{
  m_nCellsX = i_nCellsX;
  m_nCellsY = i_nCellsY;
  m_solver = i_solver;
  m_hasBoundaryL = i_hasBoundaryL;
  m_hasBoundaryR = i_hasBoundaryR;
  m_hasBoundaryL = i_hasBoundaryL;
  m_hasBoundaryR = i_hasBoundaryR;

  // allocate memory including a single ghost cell on each side
  for (unsigned short l_st = 0; l_st < 2; l_st++)
  {
    m_h[l_st] = new t_real[(m_nCellsX + 2) * (m_nCellsY * 2)];
    m_huX[l_st] = new t_real[(m_nCellsX + 2) * (m_nCellsY * 2)];
    m_huY[l_st] = new t_real[(m_nCellsX + 2) * (m_nCellsY * 2)];
  }
  m_b = new t_real[(m_nCellsX + 2) * (m_nCellsY * 2)];

  // init to zero
  for (unsigned short l_st = 0; l_st < 2; l_st++)
  {
    for (t_idx l_ce = 0; l_ce < m_nCellsX + 2; l_ce++)
    {
      for(t_idx l_de = 0; l_de < m_nCellsY + 2; l_de++){
        m_h[l_st][l_ce + l_de ] = 0;
        m_huX[l_st][l_ce + l_de] = 0;
        m_huY[l_st][l_ce + l_de] = 0;
        m_b[l_ce + l_de] = 0;
     }
    }
  }
}

tsunami_lab::patches::WavePropagation2d::~WavePropagation2d()
{
  for (unsigned short l_st = 0; l_st < 2; l_st++)
  {
    delete[] m_h[l_st];
    delete[] m_huX[l_st];
    delete[] m_huY[l_st];
  }
  delete[] m_b;
}

void tsunami_lab::patches::WavePropagation2d::timeStep(t_real i_scaling)
{
  // pointers to old and new data
  t_real *l_hOld = m_h[m_step];
  t_real *l_huOldX = m_huX[m_step];
  t_real *l_huOldY = m_huY[m_step];

  m_step = (m_step + 1) % 2;
  t_real *l_hNew = m_h[m_step];
  t_real *l_huNewX = m_huX[m_step];
  t_real *l_huNewY = m_huY[m_step];

  // init new cell quantities
  for (t_idx l_ce = 1; l_ce < m_nCellsX +1 ; l_ce++)
  {
    for(t_idx l_de = 1; l_de < m_nCellsY +1 ; l_de++){
        l_hNew[l_ce] = l_hOld[l_ce + l_de];
        l_huNewX[l_ce] = l_huOldX[l_ce + l_de];
        l_huNewY[l_ce] = l_huOldY[l_ce + l_de];
    }
  }

  // iterate over edges and update with Riemann solutions
  //first loop for x sweep
  for (t_idx l_ec = 1; l_ec < m_nCellsX; l_ec++)
  {
    for(t_idx l_ed = 0; l_ed < m_nCellsY + 1; l_ed++)
    {
    // determine left and right cell-id
    t_idx l_ceL = l_ec +l_ed;
    t_idx l_ceR = l_ec + 1 +l_ed;

    t_real l_hL = l_hOld[l_ceL];
    t_real l_hR = l_hOld[l_ceR];
    t_real l_huL = l_huOldX[l_ceL];
    t_real l_huR = l_huOldX[l_ceR];
    t_real l_bL = m_b[l_ceL];
    t_real l_bR = m_b[l_ceR];

    // handle reflections
    handleReflections(l_hOld,
                      l_huOldX,
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
    
   
    solvers::Fwave::netUpdates(l_hL,
                                l_hR,
                                l_huL,
                                l_huR,
                                l_bL,
                                l_bR,
                                l_netUpdates[0],
                                l_netUpdates[1]);
    
    if (l_hOld[l_ceL] > 0)
    {
      l_hNew[l_ceL] -= i_scaling * l_netUpdates[0][0];
      l_huNewX[l_ceL] -= i_scaling * l_netUpdates[0][1];
    }
    else
    {
      l_hNew[l_ceL] = 0;
      l_huNewX[l_ceL] = 0;
    }

    if (l_hOld[l_ceR] > 0)
    {
      l_hNew[l_ceR] -= i_scaling * l_netUpdates[1][0];
      l_huNewX[l_ceR] -= i_scaling * l_netUpdates[1][1];
    }
    else
    {
      l_hNew[l_ceR] = 0;
      l_huNewX[l_ceR] = 0;
    }
  }
  }
  //second loop for y sweep
  for (t_idx l_ec = 1; l_ec < m_nCellsY; l_ec++)
  {
    for(t_idx l_ed = 1; l_ed < m_nCellsX ; l_ed++)
    {
    // determine left and right cell-id
    t_idx l_ceL = l_ec +l_ed;
    t_idx l_ceR = l_ec + 1 +l_ed;

    t_real l_hL = l_hOld[l_ceL];
    t_real l_hR = l_hOld[l_ceR];
    t_real l_huL = l_huOldY[l_ceL];
    t_real l_huR = l_huOldY[l_ceR];
    t_real l_bL = m_b[l_ceL];
    t_real l_bR = m_b[l_ceR];

    // handle reflections
    handleReflections(l_hOld,
                      l_huOldY,
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
    
   
    solvers::Fwave::netUpdates(l_hL,
                                l_hR,
                                l_huL,
                                l_huR,
                                l_bL,
                                l_bR,
                                l_netUpdates[0],
                                l_netUpdates[1]);
    
    if (l_hOld[l_ceL] > 0)
    {
      l_hNew[l_ceL] -= i_scaling * l_netUpdates[0][0];
      l_huNewX[l_ceL] -= i_scaling * l_netUpdates[0][1];
    }
    else
    {
      l_hNew[l_ceL] = 0;
      l_huNewX[l_ceL] = 0;
    }

    if (l_hOld[l_ceR] > 0)
    {
      l_hNew[l_ceR] -= i_scaling * l_netUpdates[1][0];
      l_huNewX[l_ceR] -= i_scaling * l_netUpdates[1][1];
    }
    else
    {
      l_hNew[l_ceR] = 0;
      l_huNewX[l_ceR] = 0;
    }
  }
  }
}

void tsunami_lab::patches::WavePropagation1d::setGhostOutflow()
{
  t_real *l_h = m_h[m_step];
  t_real *l_hu = m_hu[m_step];
  t_real *l_b = m_b;

  // left boundary
  if (m_hasBoundaryL)
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
  if (m_hasBoundaryR)
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
