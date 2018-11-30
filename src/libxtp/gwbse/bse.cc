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


#include <votca/xtp/bse.h>
#include <votca/tools/linalg.h>
#include <votca/xtp/aomatrix.h>
#include "votca/xtp/qmstate.h"
using boost::format;
using std::flush;

namespace votca {
  namespace xtp {

    void BSE::Solve_triplets() {
      MatrixXfd H = MatrixXfd::Zero(_bse_size,_bse_size);
      Add_Hd<real_gwbse>(H);
      Add_Hqp<real_gwbse>(H);
      XTP_LOG(logDEBUG, *_log)
        << TimeStamp() << " Setup TDA triplet hamiltonian " << flush;
      XTP_LOG(logDEBUG, *_log)
        << TimeStamp() << " Solving for first "<<_bse_nmax<<" eigenvectors"<< flush;
      tools::linalg_eigenvalues(H , _bse_triplet_energies, _bse_triplet_coefficients ,_bse_nmax );
      return;
    }

    void BSE::Solve_singlets() {
      MatrixXfd H = MatrixXfd::Zero(_bse_size,_bse_size);
      Add_Hd<real_gwbse>(H);
      Add_Hqp<real_gwbse>(H);
      Add_Hx<real_gwbse>(H,2.0);
      XTP_LOG(logDEBUG, *_log)
        << TimeStamp() << " Setup TDA singlet hamiltonian " << flush;
      XTP_LOG(logDEBUG, *_log)
        << TimeStamp() << " Solving for first "<<_bse_nmax<<" eigenvectors"<< flush;
      tools::linalg_eigenvalues(H, _bse_singlet_energies, _bse_singlet_coefficients , _bse_nmax );
      return;
    }
    
     void BSE::SetupHs(){
      _eh_s = MatrixXfd::Zero(_bse_size,_bse_size);
      Add_Hd<real_gwbse>(_eh_s);
      Add_Hqp<real_gwbse>(_eh_s);
      Add_Hx<real_gwbse>(_eh_s,2.0);
     }
  
  void BSE::SetupHt(){
      _eh_t = MatrixXfd::Zero(_bse_size,_bse_size);
      Add_Hd<real_gwbse>(_eh_t);
      Add_Hqp<real_gwbse>(_eh_t);
  }
    

    void BSE::Solve_singlets_BTDA() {

      // For details of the method, see EPL,78(2007)12001,
      // Nuclear Physics A146(1970)449, Nuclear Physics A163(1971)257.
      // setup resonant (A) and RARC blocks (B)
       //corresponds to 
      // _ApB = (_eh_d +_eh_qp + _eh_d2 + 4.0 * _eh_x);
      // _AmB = (_eh_d +_eh_qp - _eh_d2);
        Eigen::MatrixXd ApB=Eigen::MatrixXd::Zero(_bse_size,_bse_size);
        Add_Hd<double>(ApB);
        Add_Hqp<double>(ApB);
        
        Eigen::MatrixXd AmB=ApB;
        Add_Hd2<double>(AmB,-1.0);
 
        Add_Hx<double>(ApB,4.0);
        Add_Hd2<double>(ApB,1.0);
        XTP_LOG(logDEBUG, *_log)
        << TimeStamp() << " Setup singlet hamiltonian " << flush;
     
      // calculate Cholesky decomposition of A-B = LL^T. It throws an error if not positive definite
      //(A-B) is not needed any longer and can be overwritten
      XTP_LOG(logDEBUG, *_log) << TimeStamp() << " Trying Cholesky decomposition of KAA-KAB" << flush;
      Eigen::LLT< Eigen::Ref<Eigen::MatrixXd> > L(AmB);
      
       for (int i=0;i<AmB.rows();++i){
          for (int j=i+1;j<AmB.cols();++j){
          AmB(i,j)=0;
          }
        }

      if(L.info()!=0){
        XTP_LOG(logDEBUG, *_log) << TimeStamp() <<" Cholesky decomposition of KAA-KAB was unsucessful. Try a smaller basisset. This can indicate a triplet instability."<<flush;
        throw std::runtime_error("Cholesky decompostion failed");
      }else{
        XTP_LOG(logDEBUG, *_log) << TimeStamp() <<" Cholesky decomposition of KAA-KAB was successful"<<flush;
      }
      Eigen::MatrixXd temp= ApB*AmB;
      ApB.noalias() =AmB.transpose()*temp;
      temp.resize(0,0);
      XTP_LOG(logDEBUG, *_log) << TimeStamp() << " Calculated H = L^T(A+B)L " << flush;
      Eigen::VectorXd eigenvalues;
      Eigen::MatrixXd eigenvectors;     
      XTP_LOG(logDEBUG, *_log)
        << TimeStamp() << " Solving for first "<<_bse_nmax<<" eigenvectors"<< flush;
      bool success_diag=tools::linalg_eigenvalues(ApB, eigenvalues, eigenvectors ,_bse_nmax);
      if(!success_diag){
        XTP_LOG(logDEBUG, *_log) << TimeStamp() << " Could not solve problem" << flush;
      }else{
        XTP_LOG(logDEBUG, *_log) << TimeStamp() << " Solved HR_l = eps_l^2 R_l " << flush;
      }
      ApB.resize(0,0);
      eigenvalues=eigenvalues.cwiseSqrt();
     
#if (GWBSE_DOUBLE)
      _bse_singlet_energies =eigenvalues;
#else
      _bse_singlet_energies = eigenvalues.cast<float>(); 
#endif
      // reconstruct real eigenvectors X_l = 1/2 [sqrt(eps_l) (L^T)^-1 + 1/sqrt(eps_l)L ] R_l
      //                               Y_l = 1/2 [sqrt(eps_l) (L^T)^-1 - 1/sqrt(eps_l)L ] R_l
      // determine inverse of L^T
     Eigen::MatrixXd LmT = AmB.inverse().transpose();
      int dim = LmT.rows();
      _bse_singlet_energies.resize(_bse_nmax);
      _bse_singlet_coefficients.resize(dim, _bse_nmax); // resonant part (_X_evec)
      _bse_singlet_coefficients_AR.resize(dim, _bse_nmax); // anti-resonant part (_Y_evec)
      for (int level = 0; level < _bse_nmax; level++) {
        //real_gwbse sqrt_eval = sqrt(_eigenvalues(_i));
        double sqrt_eval = sqrt(_bse_singlet_energies(level));
        // get l-th reduced EV
#if (GWBSE_DOUBLE)
        _bse_singlet_coefficients.col(level) = (0.5 / sqrt_eval * (_bse_singlet_energies(level) * LmT + AmB) * eigenvectors.col(level));
        _bse_singlet_coefficients_AR.col(level) = (0.5 / sqrt_eval * (_bse_singlet_energies(level) * LmT - AmB) * eigenvectors.col(level));
#else
        _bse_singlet_coefficients.col(level) = (0.5 / sqrt_eval * (_bse_singlet_energies(level) * LmT + AmB) * eigenvectors.col(level)).cast<float>();
        _bse_singlet_coefficients_AR.col(level) = (0.5 / sqrt_eval * (_bse_singlet_energies(level) * LmT - AmB) * eigenvectors.col(level)).cast<float>();
#endif

      }

      return;
    }

   
template <typename T>
    void BSE::Add_Hqp(Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& H) {
    
    const Eigen::MatrixXd& Hqp=*_Hqp;
#pragma omp parallel for
      for (int v1 = 0; v1 < _bse_vtotal; v1++) {
        for (int c1 = 0; c1 < _bse_ctotal; c1++) {
          int index_vc = _bse_ctotal * v1 + c1;
          // diagonal
          H(index_vc, index_vc) += Hqp(c1 + _bse_vtotal, c1 + _bse_vtotal) -Hqp(v1, v1);
          // v->c
          for (int c2 = 0; c2 < _bse_ctotal; c2++) {
            int index_vc2 = _bse_ctotal * v1 + c2;
            if (c1 != c2) {
              H(index_vc, index_vc2) += Hqp(c1 + _bse_vtotal, c2 + _bse_vtotal);
            }
          }
          // c-> v
          for (int v2 = 0; v2 < _bse_vtotal; v2++) {
            int index_vc2 = _bse_ctotal * v2 + c1;
            if (v1 != v2) {
              H(index_vc, index_vc2) -= Hqp(v1, v2);
            }
          }
        }
      }
      return;
    }

template <typename T>
    void BSE::Add_Hd(Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& H) {
      // gwbasis size
      int auxsize = _Mmn->getAuxDimension();
      int vxv_size=_bse_vtotal * _bse_vtotal;
      int cxc_size=_bse_ctotal * _bse_ctotal;
      
      // messy procedure, first get two matrices for occ and empty subbparts
      // store occs directly transposed
      MatrixXfd storage_v = MatrixXfd::Zero(auxsize, vxv_size);
#pragma omp parallel for
      for (int v1 = 0; v1 < _bse_vtotal; v1++) {
        const MatrixXfd& Mmn = (*_Mmn)[v1 + _bse_vmin ];
        for (int i_gw = 0; i_gw < auxsize; i_gw++) {
          for (int v2 = 0; v2 < _bse_vtotal; v2++) {
            int index_vv = _bse_vtotal * v1 + v2;         
              storage_v(i_gw,index_vv) = Mmn( v2 + _bse_vmin,i_gw);
            }
        }
      }

      MatrixXfd storage_c = MatrixXfd::Zero(auxsize,cxc_size);
#pragma omp parallel for
      for (int c1 = 0; c1 < _bse_ctotal; c1++) {
        const MatrixXfd& Mmn = (*_Mmn)[c1 + _bse_cmin];
        for (int i_gw = 0; i_gw < auxsize; i_gw++) {
          for (int c2 = 0; c2 < _bse_ctotal; c2++) {
            int index_cc = _bse_ctotal * c1 + c2;
            storage_c(i_gw, index_cc) = Mmn( c2 + _bse_cmin,i_gw);
          }
        }
      }

      // store elements in a vtotal^2 x ctotal^2 matrix
      MatrixXfd storage_prod = storage_v.transpose() *storage_c;

      // now patch up _storage for screened interaction
#pragma omp parallel for
      for (int i_gw = 0; i_gw < auxsize; i_gw++) {
        if (_ppm->getPpm_weight()(i_gw) < 1.e-9) {
          for (int v = 0; v < vxv_size; v++) {
            storage_v(i_gw,v ) = 0;
          }
          for (int c = 0; c < cxc_size; c++) {
            storage_c(i_gw, c) = 0;
          }
        } else {
          double ppm_factor = sqrt(_ppm->getPpm_weight()(i_gw));
          for (int v = 0; v < vxv_size; v++) {
            storage_v(i_gw,v ) *= ppm_factor;
          }
          for (int c = 0; c < cxc_size; c++) {
            storage_c(i_gw, c) *= ppm_factor;
          }
        }
      }

      // multiply and subtract from _storage_prod
      storage_prod -= storage_v.transpose()*storage_c;
      // free storage_v and storage_c
      storage_c.resize(0, 0);
      storage_v.resize(0, 0);
      // finally resort into _eh_d
      
#pragma omp parallel for
      for (int v1 = 0; v1 < _bse_vtotal; v1++) {
        for (int v2 = 0; v2 < _bse_vtotal; v2++) {
          int index_vv = _bse_vtotal * v1 + v2;
          for (int c1 = 0; c1 < _bse_ctotal; c1++) {
            int index_vc1 = _bse_ctotal * v1 + c1;
            for (int c2 = 0; c2 < _bse_ctotal; c2++) {
              int index_vc2 = _bse_ctotal * v2 + c2;
              int index_cc = _bse_ctotal * c1 + c2;
              H(index_vc1, index_vc2) -= storage_prod(index_vv, index_cc);
            }
          }
        }
      }

      return;
    }
template <typename T>
    void BSE::Add_Hd2(Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& H, double factor) {
      // gwbasis size
      int auxsize = _Mmn->getAuxDimension();
      int bse_vxc_total=_bse_vtotal * _bse_ctotal;
      // messy procedure, first get two matrices for occ and empty subbparts
      // store occs directly transposed
      MatrixXfd storage_cv = MatrixXfd::Zero(auxsize,bse_vxc_total);
#pragma omp parallel for
      for (int c1 = 0; c1 < _bse_ctotal; c1++) {
        const MatrixXfd& Mmn = (*_Mmn)[c1 + _bse_cmin ];
        for (int i_gw = 0; i_gw < auxsize; i_gw++) {
          for (int v2 = 0; v2 < _bse_vtotal; v2++) {
            int index_cv = _bse_vtotal * c1 + v2;
            storage_cv(i_gw,index_cv ) = Mmn( v2 + _bse_vmin,i_gw);
          }
        }
      }

      MatrixXfd storage_vc = MatrixXfd::Zero(auxsize, bse_vxc_total);
#pragma omp parallel for
      for (int v1 = 0; v1 < _bse_vtotal; v1++) {
        const MatrixXfd& Mmn = (*_Mmn)[v1 + _bse_vmin];
        for (int i_gw = 0; i_gw < auxsize; i_gw++) {
          for (int c2 = 0; c2 < _bse_ctotal; c2++) {
            int index_vc = _bse_ctotal * v1 + c2;
            storage_vc(i_gw, index_vc) = Mmn(c2 + _bse_cmin,i_gw);
          }
        }
      }

      // store elements in a vtotal^2 x ctotal^2 matrix
      MatrixXfd storage_prod = storage_cv.transpose()* storage_vc;

      // now patch up _storage for screened interaction
#pragma omp parallel for
      for (int i_gw = 0; i_gw < auxsize; i_gw++) {
        if (_ppm->getPpm_weight()(i_gw) < 1.e-9) {
          for (int v = 0; v < bse_vxc_total; v++) {
            storage_vc(i_gw, v) = 0;
          }
          for (int c = 0; c < bse_vxc_total; c++) {
            storage_cv(i_gw,c ) = 0;
          }
        } else {
          double ppm_factor = sqrt(_ppm->getPpm_weight()(i_gw));
          for (int v = 0; v < bse_vxc_total; v++) {
            storage_vc(i_gw, v) *= ppm_factor;
          }
          for (int c = 0; c < bse_vxc_total; c++) {
            storage_cv(i_gw,c ) *= ppm_factor;
          }
        }
      }

      // multiply and subtract from _storage_prod
      storage_prod -= storage_cv.transpose() * storage_vc;
      // free storage_v and storage_c
      storage_cv.resize(0, 0);
      storage_vc.resize(0, 0);
  
#pragma omp parallel for
      for (int v1 = 0; v1 < _bse_vtotal; v1++) {
        for (int v2 = 0; v2 < _bse_vtotal; v2++) {
          for (int c1 = 0; c1 < _bse_ctotal; c1++) {
            int index_v1c1 = _bse_ctotal * v1 + c1;
            int index_c1v2 = _bse_vtotal * c1 + v2;
            for (int c2 = 0; c2 < _bse_ctotal; c2++) {
              int index_v2c2 = _bse_ctotal * v2 + c2;
              int index_v1c2 = _bse_ctotal * v1 + c2;
              H(index_v1c1, index_v2c2) -= factor*storage_prod(index_c1v2, index_v1c2);
            }
          }
        }
      }
      return;
    }

template <typename T>
    void BSE::Add_Hx(Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& H, double factor) { 
      // gwbasis size
      int auxsize = _Mmn->getAuxDimension();
      // get a different storage for 3-center integrals we need
      Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> storage = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Zero(auxsize, _bse_size);
      // occupied levels
#pragma omp parallel for
      for (int v = 0; v < _bse_vtotal; v++) {
        const MatrixXfd& Mmn = (*_Mmn)[v + _bse_vmin];
        // empty levels
        for (int i_gw = 0; i_gw < auxsize; i_gw++) {
          for (int c = 0; c < _bse_ctotal; c++) {
            int index_vc = _bse_ctotal * v + c;
            storage(i_gw, index_vc) = Mmn(c + _bse_cmin,i_gw);
          }
        }
      }
      // with this storage, _eh_x is obtained by matrix multiplication
      H += factor*storage.transpose() * storage;
      return;
    }

    void BSE::printFragInfo(const Population& pop, int i){
      XTP_LOG(logINFO, *_log) << format("           Fragment A -- hole: %1$5.1f%%  electron: %2$5.1f%%  dQ: %3$+5.2f  Qeff: %4$+5.2f")
              % (100.0 * pop.popH[i](0)) % (100.0 * pop.popE[i](0)) % (pop.Crgs[i](0)) % (pop.Crgs[i](0) + pop.popGs(0)) << flush;
      XTP_LOG(logINFO, *_log) << format("           Fragment B -- hole: %1$5.1f%%  electron: %2$5.1f%%  dQ: %3$+5.2f  Qeff: %4$+5.2f")
              % (100.0 * pop.popH[i](1)) % (100.0 * pop.popE[i](1)) % (pop.Crgs[i](1)) % (pop.Crgs[i](1) + pop.popGs(1)) << flush;
      return;
    }

    void BSE::printWeights(int i_bse, double weight){
      if (weight > _min_print_weight) {
        XTP_LOG(logINFO, *_log) << format("           HOMO-%1$-3d -> LUMO+%2$-3d  : %3$3.1f%%")
                % (_homo - _index2v[i_bse]) % (_index2c[i_bse] - _homo - 1) % (100.0 * weight) << flush;
      }
      return;
    }

    void BSE::Analyze_singlets(const AOBasis& dftbasis) {

      Interaction act;
      Population pop;
      QMStateType singlet=QMStateType(QMStateType::Singlet);
      std::vector< Eigen::Vector3d > transition_dipoles=CalcCoupledTransition_Dipoles(dftbasis);
      _orbitals.TransitionDipoles()=transition_dipoles;
      std::vector<double> oscs = _orbitals.Oscillatorstrengths();
      
      if(tools::globals::verbose){
        act = Analyze_eh_interaction(singlet);
      }
      if(dftbasis.getAOBasisFragA() > 0 && dftbasis.getAOBasisFragB()>0){
        pop=FragmentPopulations(singlet,dftbasis);
        _orbitals.setFragmentChargesSingEXC(pop.Crgs);
        _orbitals.setFragment_E_localisation_singlet(pop.popE);
        _orbitals.setFragment_H_localisation_singlet(pop.popH);
        _orbitals.setFragmentChargesGS(pop.popGs);
      }
      
      double hrt2ev = tools::conv::hrt2ev;
      XTP_LOG(logINFO, *_log) << "  ====== singlet energies (eV) ====== "<< flush;
      int maxoutput=(_bse_nmax>200) ? 200:_bse_nmax;
      for (int i = 0; i < maxoutput; ++i) {     
        const Eigen::Vector3d& trdip = transition_dipoles[i];
        double osc = oscs[i];
        if (tools::globals::verbose) {
          XTP_LOG(logINFO, *_log) << format("  S = %1$4d Omega = %2$+1.12f eV  lamdba = %3$+3.2f nm <FT> = %4$+1.4f <K_x> = %5$+1.4f <K_d> = %6$+1.4f")
                  % (i + 1) % (hrt2ev * _bse_singlet_energies(i)) % (1240.0 / (hrt2ev * _bse_singlet_energies(i)))
                  % (hrt2ev * act.qp_contrib(i)) % (hrt2ev * act.exchange_contrib(i)) % (hrt2ev * act.direct_contrib(i)) << flush;
        } else {
          XTP_LOG(logINFO, *_log) << format("  S = %1$4d Omega = %2$+1.12f eV  lamdba = %3$+3.2f nm")
                  % (i + 1) % (hrt2ev * _bse_singlet_energies(i)) % (1240.0 / (hrt2ev * _bse_singlet_energies(i))) << flush;
        }
        XTP_LOG(logINFO, *_log) << format("           TrDipole length gauge[e*bohr]  dx = %1$+1.4f dy = %2$+1.4f dz = %3$+1.4f |d|^2 = %4$+1.4f f = %5$+1.4f")
                % trdip[0] % trdip[1] % trdip[2] % (trdip.squaredNorm()) % osc << flush;
        for (int i_bse = 0; i_bse < _bse_size; ++i_bse) {
          // if contribution is larger than 0.2, print
          double weight = std::pow(_bse_singlet_coefficients(i_bse, i), 2);
          if (_bse_singlet_coefficients_AR.rows()>0){
               weight-= std::pow(_bse_singlet_coefficients_AR(i_bse, i), 2);
          }
          printWeights(i_bse, weight);
        }
        // results of fragment population analysis 
        if (dftbasis.getAOBasisFragA() > 0 && dftbasis.getAOBasisFragB()>0) {
          printFragInfo(pop, i);
        }

        XTP_LOG(logINFO, *_log) << flush;
      }
      return;
    }
    
    
    
    
    void BSE::Analyze_triplets(const AOBasis& dftbasis) {

      Interaction act;
      Population pop;
      QMStateType triplet=QMStateType(QMStateType::Triplet);
      if(tools::globals::verbose){
        act = Analyze_eh_interaction(triplet);
      }
      if(dftbasis.getAOBasisFragA()>0){
        pop=FragmentPopulations(triplet,dftbasis);
        _orbitals.setFragmentChargesTripEXC(pop.Crgs);
        _orbitals.setFragment_E_localisation_triplet(pop.popE);
        _orbitals.setFragment_H_localisation_triplet(pop.popH);
        _orbitals.setFragmentChargesGS(pop.popGs);
      }
      XTP_LOG(logINFO, *_log) << "  ====== triplet energies (eV) ====== " << flush;
      int maxoutput=(_bse_nmax>200) ? 200:_bse_nmax;
      for (int i = 0; i < maxoutput; ++i) {    
        if (tools::globals::verbose) {
          XTP_LOG(logINFO, *_log) << format("  T = %1$4d Omega = %2$+1.12f eV  lamdba = %3$+3.2f nm <FT> = %4$+1.4f <K_d> = %5$+1.4f")
                  % (i + 1) % (tools::conv::hrt2ev * _bse_triplet_energies(i)) % (1240.0 / (tools::conv::hrt2ev * _bse_triplet_energies(i)))
                  % (tools::conv::hrt2ev * act.qp_contrib(i)) % (tools::conv::hrt2ev *act.direct_contrib(i)) << flush;
        } else {
          XTP_LOG(logINFO, *_log) << format("  T = %1$4d Omega = %2$+1.12f eV  lamdba = %3$+3.2f nm")
                  % (i + 1) % (tools::conv::hrt2ev * _bse_triplet_energies(i)) % (1240.0 / (tools::conv::hrt2ev * _bse_triplet_energies(i))) << flush;
        }
        for (int i_bse = 0; i_bse < _bse_size; ++i_bse) {
          // if contribution is larger than 0.2, print
          double weight = pow(_bse_triplet_coefficients(i_bse, i), 2);
          printWeights(i_bse, weight);
        }
        // results of fragment population analysis 
        if (dftbasis.getAOBasisFragA() > 0) {
          printFragInfo(pop, i);
        }
        XTP_LOG(logINFO, *_log) << format("   ") << flush;
      }
      // storage to orbitals object

      return;
    }

    Eigen::VectorXd BSE::Analyze_IndividualContribution(const QMStateType& type,const MatrixXfd& H){
        Eigen::VectorXd contrib=Eigen::VectorXd::Zero(_bse_nmax);
        if (type == QMStateType::Singlet) {
            for (int i_exc = 0; i_exc < _bse_nmax; i_exc++) {
                MatrixXfd _slice_R = _bse_singlet_coefficients.block(0, i_exc, _bse_size, 1);
                contrib(i_exc) =  (_slice_R.transpose()*H * _slice_R).value();
                if (_bse_singlet_coefficients_AR.cols() > 0) {
                    MatrixXfd _slice_AR = _bse_singlet_coefficients_AR.block(0, i_exc, _bse_size, 1);
                    // get anti-resonant contribution from direct Keh 
                    contrib(i_exc)-= (_slice_AR.transpose()*H * _slice_AR).value();           
                }
            }
        } else if (type == QMStateType::Triplet) {
            for (int i_exc = 0; i_exc < _bse_nmax; i_exc++) {
                MatrixXfd _slice_R = _bse_triplet_coefficients.block(0, i_exc, _bse_size, 1);
                contrib(i_exc) =  (_slice_R.transpose()*H * _slice_R).value();
            }
        } else {
            throw std::runtime_error("BSE::Analyze_eh_interaction:Spin not known!");
        }
        return contrib;
    }

    BSE::Interaction BSE::Analyze_eh_interaction(const QMStateType& type) {

      Interaction analysis;
      MatrixXfd H = MatrixXfd::Zero(_bse_size, _bse_size);
      Add_Hqp(H); 
      analysis.qp_contrib=Analyze_IndividualContribution(type,H);
      
      H = MatrixXfd::Zero(_bse_size, _bse_size);
      Add_Hd(H);
      analysis.direct_contrib=Analyze_IndividualContribution(type,H);
      if (type == QMStateType::Singlet) {
          H = MatrixXfd::Zero(_bse_size, _bse_size);
          Add_Hx(H,2.0);
          analysis.exchange_contrib=Analyze_IndividualContribution(type,H);
      }else{
            analysis.exchange_contrib=Eigen::VectorXd::Zero(0);
      }
      
      return analysis;
    }

    BSE::Population BSE::FragmentPopulations(const QMStateType& type, const AOBasis& dftbasis) {
      Population pop;
      // Mulliken fragment population analysis
        AOOverlap dftoverlap;
        dftoverlap.Fill(dftbasis);
        XTP_LOG(logDEBUG, *_log) << TimeStamp() << " Filled DFT Overlap matrix of dimension: " << dftoverlap.Matrix().rows() << flush;
        // ground state populations
        Eigen::MatrixXd DMAT = _orbitals.DensityMatrixGroundState();
        Eigen::VectorXd nuccharges = _orbitals.FragmentNuclearCharges(dftbasis.getAOBasisFragA());
        Eigen::VectorXd pops = _orbitals.LoewdinPopulation(DMAT, dftoverlap.Matrix(), dftbasis.getAOBasisFragA());
        pop.popGs=nuccharges - pops;
        // population to electron charges and add nuclear charges         
        for (int i_state = 0; i_state < _bse_nmax; i_state++) {
          QMState state=QMState(type,i_state,false);
          // checking Density Matrices
          std::vector< Eigen::MatrixXd > DMAT = _orbitals.DensityMatrixExcitedState(state);
          // hole part
          Eigen::VectorXd popsH = _orbitals.LoewdinPopulation(DMAT[0], dftoverlap.Matrix(), dftbasis.getAOBasisFragA());
          pop.popH.push_back(popsH);
          // electron part
          Eigen::VectorXd popsE = _orbitals.LoewdinPopulation(DMAT[1], dftoverlap.Matrix(), dftbasis.getAOBasisFragA());
          pop.popE.push_back(popsE);
          // update effective charges
          Eigen::VectorXd diff = popsH - popsE;
          pop.Crgs.push_back(diff);
        }
        XTP_LOG(logDEBUG, *_log) << TimeStamp() << " Ran Excitation fragment population analysis " << flush;
     
      return pop;
    }

    std::vector<Eigen::MatrixXd > BSE::CalcFreeTransition_Dipoles(const AOBasis& dftbasis) {
      const Eigen::MatrixXd& dft_orbitals = _orbitals.MOCoefficients();
      // Testing electric dipole AOMatrix
      AODipole dft_dipole;
      dft_dipole.Fill(dftbasis);

      // now transition dipole elements for free interlevel transitions
      std::vector<Eigen::MatrixXd > interlevel_dipoles;

      Eigen::MatrixXd empty = dft_orbitals.block(0,_bse_cmin,dftbasis.AOBasisSize() , _bse_ctotal);
      Eigen::MatrixXd occ = dft_orbitals.block(0,_bse_vmin, dftbasis.AOBasisSize(), _bse_vtotal);
      for (int i_comp = 0; i_comp < 3; i_comp++) {
        interlevel_dipoles.push_back(occ.transpose() * dft_dipole.Matrix()[i_comp] * empty);
      }
      XTP_LOG(logDEBUG, *_log) << TimeStamp() << " Calculated free interlevel transition dipole moments " << flush;
      return interlevel_dipoles;
    }

    std::vector<Eigen::Vector3d > BSE::CalcCoupledTransition_Dipoles(const AOBasis& dftbasis) {
    std::vector<Eigen::MatrixXd > interlevel_dipoles= CalcFreeTransition_Dipoles(dftbasis);
    std::vector<Eigen::Vector3d > dipols;
    const double sqrt2 = sqrt(2.0);
      for (int i_exc = 0; i_exc < _bse_nmax; i_exc++) {
        Eigen::Vector3d tdipole = Eigen::Vector3d::Zero();
        for (int c = 0; c < _bse_ctotal; c++) {
          for (int v = 0; v < _bse_vtotal; v++) {
            int index_vc = _bse_ctotal * v + c;
            double factor = _bse_singlet_coefficients(index_vc, i_exc);
            if (_bse_singlet_coefficients_AR.rows()>0) {
              factor += _bse_singlet_coefficients_AR(index_vc, i_exc);
            }
            for(int i=0;i<3;i++){
                // The Transition dipole is sqrt2 bigger because of the spin, the excited state is a linear combination of 2 slater determinants, where either alpha or beta spin electron is excited
            tdipole[i] += factor * interlevel_dipoles[i](v, c);
            }

          }
        }
        
        dipols.push_back(-sqrt2 *tdipole);     //- because electrons are negative
      }
      return dipols;
    }
    


  }
};
