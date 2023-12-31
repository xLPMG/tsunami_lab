/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 * One-dimensional wave propagation patch.
 **/
#ifndef TSUNAMI_LAB_PATCHES_WAVE_PROPAGATION_1D
#define TSUNAMI_LAB_PATCHES_WAVE_PROPAGATION_1D

#include "WavePropagation.h"
#include <string>

namespace tsunami_lab
{
  namespace patches
  {
    class WavePropagation1d;
  }
}

class tsunami_lab::patches::WavePropagation1d : public WavePropagation
{
private:
  //! current step which indicates the active values in the arrays below
  unsigned short m_step = 0;

  //! number of cells discretizing the computational domain
  t_idx m_nCells = 0;

  //! water heights for the current and next time step for all cells
  t_real *m_h[2] = {nullptr, nullptr};

  //! momenta for the current and next time step for all cells
  t_real *m_hu[2] = {nullptr, nullptr};

  //! bathymetry 
  t_real *m_b = nullptr;

  //! selected solver (roe or fwave)
  std::string m_solver = "";

  //! boundary condition on the left side
  Boundary m_boundaryL = OUTFLOW;

  //! boundary condition on the right side
  Boundary m_boundaryR = OUTFLOW;

 /**
  * Compute the reflection effect
  * 
  * @param i_h water height
  * @param i_hu water momentum
  * @param i_ceL left cell
  * @param i_ceR right cell
  * @param o_hL water height on the left side
  * @param o_hR water height on the right side
  * @param o_huL water momentum on the left side
  * @param o_huR water momentum on the right side
  * @param o_bL bathymetry on the left side 
  * @param o_bR bathymetry on the right side
  */
  void handleReflections(t_real *i_h,
                         t_real *i_hu,
                         t_idx i_ceL,
                         t_idx i_ceR,
                         t_real &o_hL,
                         t_real &o_hR,
                         t_real &o_huL,
                         t_real &o_huR,
                         t_real &o_bL,
                         t_real &o_bR);

public:
  /**
   * Constructs the 1d wave propagation solver.
   *
   * @param i_nCells number of cells.
   * @param i_solver selected solver.
   * @param i_boundaryL boundary condition on the left side
   * @param i_boundaryR boundary condition on the right side
   **/
  WavePropagation1d(t_idx i_nCells,
                    const std::string &i_solver,
                    Boundary i_boundaryL,
                    Boundary i_boundaryR);

  /**
   * Destructor which frees all allocated memory.
   **/
  ~WavePropagation1d();

  /**
   * Performs a time step.
   *
   * @param i_scaling scaling of the time step (dt / dx).
   **/
  void timeStep(t_real i_scaling,
                t_real);

  /**
   * Sets the values of the ghost cells according to outflow boundary conditions.
   **/
  void setGhostOutflow();

  /**
   * Gets the stride in y-direction. x-direction is stride-1.
   *
   * @return stride in y-direction.
   **/
  t_idx getStride()
  {
    return m_nCells + 2;
  }

  /**
   * Gets cells' water heights.
   *
   * @return water heights.
   */
  t_real const *getHeight()
  {
    // add 1 to account for first ghost cell
    return m_h[m_step] + 1;
  }

  /**
   * Gets the cells' momenta in x-direction.
   *
   * @return momenta in x-direction.
   **/
  t_real const *getMomentumX()
  {
    // add 1 to account for first ghost cell
    return m_hu[m_step] + 1;
  }

  /**
   * Dummy function which returns a nullptr.
   **/
  t_real const *getMomentumY()
  {
    return nullptr;
  }

  /**
   * Gets cells bathymetry
   *
   * @return bathymetry.
   */
  t_real const *getBathymetry()
  {
    // add 1 to account for first ghost cell
    return m_b + 1;
  }

  /**
   * Sets the height of the cell to the given value.
   *
   * @param i_ix id of the cell in x-direction.
   * @param i_h water height.
   **/
  void setHeight(t_idx i_ix,
                 t_idx,
                 t_real i_h)
  {
    m_h[m_step][i_ix + 1] = i_h;
  }

  /**
   * Sets the momentum in x-direction to the given value.
   *
   * @param i_ix id of the cell in x-direction.
   * @param i_hu momentum in x-direction.
   **/
  void setMomentumX(t_idx i_ix,
                    t_idx,
                    t_real i_hu)
  {
    m_hu[m_step][i_ix + 1] = i_hu;
  }

  /**
   * Dummy function since there is no y-momentum in the 1d solver.
   **/
  void setMomentumY(t_idx,
                    t_idx,
                    t_real){};

  /**
   * Sets the bathymetry of the cell to the given value.
   *
   * @param i_ix id of the cell in x-direction.
   * @param i_b bathymetry.
   **/
  void setBathymetry(t_idx i_ix,
                     t_idx,
                     t_real i_b)
  {
    m_b[i_ix + 1] = i_b;
  }

  /**
   * Corrects the water height based on the bathymetry
   *
   **/
  void adjustWaterHeight()
  {
    for (t_idx i = 1; i < m_nCells + 1; i++)
    {
      m_h[m_step][i] -= m_b[i];
      if (m_h[m_step][i] < 0)
        m_h[m_step][i] = 0;
    }
  }
};

#endif