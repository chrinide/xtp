/*
 *            Copyright 2009-2018 The VOTCA Development Team
 *                       (http://www.votca.org)
 *
 *      Licensed under the Apache License, Version 2.0 (the "License")
 *
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef VOTCA_XTP_GWBSE_H
#define VOTCA_XTP_GWBSE_H
#include <votca/xtp/logger.h>
#include <votca/tools/property.h>
#include <fstream>
#include <votca/xtp/eigen.h>


namespace votca {
namespace xtp {
    class Orbitals;
    class Sigma;
    class AOBasis;
/**
* \brief Electronic excitations from GW-BSE
*
* Evaluates electronic excitations in molecular systems based on
* many-body Green's functions theory within the GW approximation and
* the Bethe-Salpeter equation. Requires molecular orbitals 
*
*  B. Baumeier, Y. Ma, D. Andrienko, M. Rohlfing
*  J. Chem. Theory Comput. 8, 997-1002 (2012)
*
*  B. Baumeier, D. Andrienko, M. Rohlfing
*  J. Chem. Theory Comput. 8, 2790-2795 (2012)
*
*/

class GWBSE {
 public:
  GWBSE(Orbitals& orbitals)
      : _orbitals(orbitals){};

  void Initialize(tools::Property& options);

  std::string Identify() { return "gwbse"; }

  void setLogger(Logger* pLog) { _pLog = pLog; }

  bool Evaluate();
    
  void addoutput(tools::Property& summary);

 private:
     
 void PrintQP_Energies(const Eigen::VectorXd& gwa_energies, const Eigen::VectorXd& qp_diag_energies);
 void PrintGWA_Energies(const Eigen::MatrixXd& vxc,const Sigma& sigma, const Eigen::VectorXd& dft_energies);    
 
 Eigen::MatrixXd CalculateVXC(const AOBasis& dftbasis);
 Logger* _pLog;
 Orbitals& _orbitals;
  
  // program tasks
  bool _do_qp_diag;
  bool _do_bse_diag;
  bool _do_bse_singlets;
  bool _do_bse_triplets;

  // storage tasks
  bool _store_qp_pert;
  bool _store_qp_diag;
  bool _store_bse_singlets;
  bool _store_bse_triplets;
  bool _store_eh_interaction;

  // iterate G and W and not only G
  bool _iterate_gw;

  // options for own Vxc calculation
  bool _doVxc;
  std::string _functional;
  std::string _grid;

  int _openmp_threads;

  // fragment definitions
  int _fragA;

  // BSE variant
  bool _do_full_BSE;

  // basis sets
  std::string _auxbasis_name;
  std::string _dftbasis_name;
  int _reset_3c; //how often the 3c integrals in iterate shoudl be rebuild
  double _shift;  // pre-shift of DFT energies
  int _homo;   // HOMO index
  int _rpamin;
  int _rpamax;
  int _qpmin;
  int _qpmax;
  int _qptotal;
  double _g_sc_limit;  // convergence criteria for g iteration [Hartree]]
  int _g_sc_max_iterations;
  int _gw_sc_max_iterations;
  double _gw_sc_limit;  // convergence criteria for gw iteration [Hartree]]
  
  int _bse_vmin;
  int _bse_vmax;
  int _bse_cmin;
  int _bse_cmax;
  int _bse_maxeigenvectors;
  double _min_print_weight;

};
}
}

#endif // VOTCA_XTP_GWBSE_H 
