/**
 * @author Alexander Breuer (alex.breuer AT uni-jena.de)
 *
 * @section DESCRIPTION
 * IO-routines for writing a snapshot as Comma Separated Values (CSV).
 **/
#include "Csv.h"
#include <sstream>
#include "../solvers/Fwave.h"

tsunami_lab::t_real eigenvalueRoe_1 = 0, eigenvalueRoe_2 = 0;

void tsunami_lab::io::Csv::write(t_real i_dxy,
                                 t_idx i_nx,
                                 t_idx i_ny,
                                 t_idx i_stride,
                                 t_real const *i_h,
                                 t_real const *i_hu,
                                 t_real const *i_hv,
                                 std::ostream &io_stream)
{
  // write the CSV header
  io_stream << "x,y";
  if (i_h != nullptr)
    io_stream << ",height";
  if (i_hu != nullptr)
    io_stream << ",momentum_x";
  if (i_hv != nullptr)
    io_stream << ",momentum_y";
  io_stream << "\n";

  // iterate over all cells
  for (t_idx l_iy = 0; l_iy < i_ny; l_iy++)
  {
    for (t_idx l_ix = 0; l_ix < i_nx; l_ix++)
    {
      // derive coordinates of cell center
      t_real l_posX = (l_ix + 0.5) * i_dxy;
      t_real l_posY = (l_iy + 0.5) * i_dxy;

      t_idx l_id = l_iy * i_stride + l_ix;

      //compute Eigenvalues for wavespeed
      void tsunami_lab::solvers::Fwave::computeEigenvalues( t_real i_hL,
                                                            t_real i_hR,
                                                            t_real i_uL,
                                                            t_real i_uR,
                                                            t_real &eigenvalueRoe_1,
                                                            t_real &eigenvalueRoe_2)
                        

      // write data
      io_stream << l_posX << "," << l_posY;
      if (i_h != nullptr)
        io_stream << "," << i_h[l_id];
      if (i_hu != nullptr)
        io_stream << "," << i_hu[l_id];
      if (i_hv != nullptr)
        io_stream << "," << i_hv[l_id];
      io_stream << "\n";
    }
  }
  io_stream << std::flush;
}

std::vector<std::string> tsunami_lab::io::Csv::splitLine(std::stringstream line,
                                                         char separator)
{
  std::vector<std::string> result;
  std::string word;
  while (getline(line, word, separator))
    result.push_back(word);
  return result;
}
