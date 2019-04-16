/*
 *            Copyright 2009-2019 The VOTCA Development Team
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

#ifndef _VOTCA_XTP_BSE_H
#define _VOTCA_XTP_BSE_H

#include <votca/xtp/orbitals.h>
#include <votca/xtp/qmstate.h>
#include <votca/xtp/rpa.h>
#include <votca/xtp/threecenter.h>

#include <votca/xtp/bse_operator.h>

namespace votca {
namespace xtp {

template <int cqp, int cx, int cd, int cd2>
class BSE_OPERATOR;
typedef BSE_OPERATOR<1, 2, 1, 0> SingletOperator_TDA;
typedef BSE_OPERATOR<1, 0, 1, 0> TripletOperator_TDA;

class BSE {

 public:
  BSE(Orbitals& orbitals, ctp::Logger& log, TCMatrix_gwbse& Mmn,
      const Eigen::MatrixXd& Hqp)
      : _log(log),
        _orbitals(orbitals),
        _bse_singlet_energies(orbitals.BSESingletEnergies()),
        _bse_singlet_coefficients(orbitals.BSESingletCoefficients()),
        _bse_singlet_coefficients_AR(orbitals.BSESingletCoefficientsAR()),
        _bse_triplet_energies(orbitals.BSETripletEnergies()),
        _bse_triplet_coefficients(orbitals.BSETripletCoefficients()),
        _bse_triplet_coefficients_AR(orbitals.BSETripletCoefficientsAR()),
        _Mmn(Mmn),
        _Hqp(Hqp){};

  struct options {
    bool useTDA = true;
    int homo;
    int rpamin;
    int rpamax;
    int qpmin;
    int vmin;
    int cmax;
    int nmax;             // number of eigenvectors to calculate
    bool davidson = 1;    // use davidson to diagonalize the matrix
    bool matrixfree = 0;  // use matrix free method
    std::string davidson_correction = "DPR";
    std::string davidson_ortho = "GS";
    std::string davidson_tolerance = "normal";
    std::string davidson_update = "safe";
    int davidson_maxiter = 50;
    double min_print_weight =
        0.5;  // minimium contribution for state to print it
  };

  void configure(const options& opt) {
    _opt = opt;
    _bse_vmax = _opt.homo;
    _bse_cmin = _opt.homo + 1;
    _bse_vtotal = _bse_vmax - _opt.vmin + 1;
    _bse_ctotal = _opt.cmax - _bse_cmin + 1;
    _bse_size = _bse_vtotal * _bse_ctotal;
    SetupDirectInteractionOperator();
  }

  void Solve_singlets();
  void Solve_triplets();

  SingletOperator_TDA getSingletOperator_TDA();
  TripletOperator_TDA getTripletOperator_TDA();

  void Analyze_triplets(const AOBasis& dftbasis);
  void Analyze_singlets(const AOBasis& dftbasis);

  void FreeTriplets() {
    _bse_triplet_coefficients.resize(0, 0);
    _bse_triplet_coefficients_AR.resize(0, 0);
  }

  void FreeSinglets() {
    _bse_singlet_coefficients.resize(0, 0);
    _bse_singlet_coefficients_AR.resize(0, 0);
  }

 private:
  options _opt;

  struct Interaction {
    Eigen::VectorXd exchange_contrib;
    Eigen::VectorXd direct_contrib;
    Eigen::VectorXd qp_contrib;
  };

  struct Population {
    std::vector<Eigen::VectorXd> popH;
    std::vector<Eigen::VectorXd> popE;
    std::vector<Eigen::VectorXd> Crgs;
    Eigen::VectorXd popGs;
  };

  int _bse_vmax;
  int _bse_cmin;
  int _bse_size;
  int _bse_vtotal;
  int _bse_ctotal;

  ctp::Logger& _log;
  Orbitals& _orbitals;
  Eigen::VectorXd _epsilon_0_inv;

  // references are stored in orbitals object
  Eigen::VectorXd& _bse_singlet_energies;
  Eigen::MatrixXd& _bse_singlet_coefficients;
  Eigen::MatrixXd& _bse_singlet_coefficients_AR;
  Eigen::VectorXd& _bse_triplet_energies;
  Eigen::MatrixXd& _bse_triplet_coefficients;
  Eigen::MatrixXd& _bse_triplet_coefficients_AR;
  TCMatrix_gwbse& _Mmn;
  const Eigen::MatrixXd& _Hqp;

  void Solve_singlets_TDA();
  void Solve_singlets_BTDA();

  void Solve_triplets_TDA();
  void Solve_triplets_BTDA();

  void PrintWeight(int i, int i_bse, QMStateType type);

  template <typename BSE_OPERATOR>
  void configureBSEOperator(BSE_OPERATOR& H);

  template <typename BSE_OPERATOR>
  void solve_hermitian(BSE_OPERATOR& H, Eigen::VectorXd& eigenvalues,
                       Eigen::MatrixXd& coefficients);

  template <typename BSE_OPERATOR_ApB, typename BSE_OPERATOR_AmB>
  void Solve_antihermitian(BSE_OPERATOR_ApB& apb, BSE_OPERATOR_AmB& amb,
                           Eigen::VectorXd& energies,
                           Eigen::MatrixXd& coefficients,
                           Eigen::MatrixXd& coefficients_AR);

  void printFragInfo(const Population& pop, int i);
  void printWeights(int i_bse, double weight);
  void SetupDirectInteractionOperator();

  Interaction Analyze_eh_interaction(const QMStateType& type);

  template <typename BSE_OPERATOR>
  Eigen::VectorXd Analyze_IndividualContribution(const QMStateType& type,
                                                 const BSE_OPERATOR& H);

  Population FragmentPopulations(const QMStateType& type,
                                 const AOBasis& dftbasis);

  std::vector<Eigen::MatrixXd> CalcFreeTransition_Dipoles(
      const AOBasis& dftbasis);

  std::vector<tools::vec> CalcCoupledTransition_Dipoles(
      const AOBasis& dftbasis);
};
}  // namespace xtp
}  // namespace votca

#endif /* _VOTCA_XTP_BSE_H */
