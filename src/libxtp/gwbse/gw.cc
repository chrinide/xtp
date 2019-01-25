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



#include <votca/xtp/gw.h>
#include "votca/xtp/rpa.h"
#include "votca/xtp/sigma_ppm.h"


namespace votca {
  namespace xtp {

   void GW::configure(const options& opt){
     _opt=opt;
     _qptotal=_opt.qpmax-_opt.qpmin+1;
     if(_opt.sigma_integration=="ppm"){
         _sigma=std::unique_ptr<Sigma_base>(new Sigma_PPM(_Mmn));
     }
     _sigma->configure(_opt.homo,_opt.qpmin,_opt.qpmax);
     _Sigma_x=Eigen::MatrixXd::Zero(_qptotal,_qptotal);
     _Sigma_c=Eigen::MatrixXd::Zero(_qptotal,_qptotal);
  }

double GW::CalcHomoLumoShift()const{
  double DFTgap = _dft_energies(_opt.homo + 1) - _dft_energies(_opt.homo);
  double QPgap = _gwa_energies(_opt.homo + 1-_opt.qpmin) - _gwa_energies(_opt.homo-_opt.qpmin);
  return QPgap - DFTgap;
}

Eigen::VectorXd GW::CalcDiagonalEnergies()const{
return _Sigma_x.diagonal()+_Sigma_c.diagonal()-_vxc.diagonal()+_dft_energies.segment(_opt.qpmin,_qptotal);
}

Eigen::MatrixXd GW::getHQP()const{
    return _Sigma_x+_Sigma_c-_vxc+ Eigen::MatrixXd(_dft_energies.segment(_opt.qpmin,_qptotal).asDiagonal());
}

Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> GW::DiagonalizeQPHamiltonian()const{
        Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> qpdiag(getHQP()) ;
        PrintQP_Energies(qpdiag.eigenvalues());
        return qpdiag;
    }

Eigen::MatrixXd GW::getGWAResults()const{
    Eigen::MatrixXd qp_energies_store=Eigen::MatrixXd::Zero(_qptotal, 5);
    qp_energies_store.col(0)=_dft_energies.segment(_opt.qpmin,_qptotal);
    qp_energies_store.col(1)=_Sigma_x.diagonal();
    qp_energies_store.col(2)=_Sigma_c.diagonal();
    qp_energies_store.col(3)=_vxc.diagonal();
    qp_energies_store.col(4)=_gwa_energies;
    return qp_energies_store;
}
  void GW::PrintGWA_Energies()const{

 double shift=CalcHomoLumoShift();

  XTP_LOG(logINFO, _log)
          << (boost::format(
          "  ====== Perturbative quasiparticle energies (Hartree) ====== "))
          .str()
          << std::flush;
  XTP_LOG(logINFO, _log)
          << (boost::format("   DeltaHLGap = %1$+1.6f Hartree") % shift).str() << std::flush;

  for (int i = 0; i < _qptotal; i++) {
      std::string level="  Level";
    if ((i + _opt.qpmin) == _opt.homo) {level="  HOMO ";}
    else if ((i + _opt.qpmin) == _opt.homo + 1) {level="  LUMO ";}
     
    XTP_LOG(logINFO, _log)
            <<level<<(boost::format(" = %1$4d DFT = %2$+1.4f VXC = %3$+1.4f S-X = "
            "%4$+1.4f S-C = %5$+1.4f GWA = %6$+1.4f") %
            (i + _opt.qpmin + 1) % _dft_energies(i + _opt.qpmin) % _vxc(i, i) %
            _Sigma_x(i,i) % _Sigma_c(i,i) % _gwa_energies(i))
            .str()
            << std::flush;
    }
  return;
}


  void GW::PrintQP_Energies(const Eigen::VectorXd& qp_diag_energies)const{
  XTP_LOG(logDEBUG, _log)
          << TimeStamp() << " Full quasiparticle Hamiltonian  " << std::flush;
  XTP_LOG(logINFO, _log)
          << (boost::format("  ====== Diagonalized quasiparticle energies (Hartree) "
          "====== ")).str()<< std::flush;
  for (int i = 0; i < _qptotal; i++) {
    if ((i +_opt.qpmin) == _opt.homo) {
      XTP_LOG(logINFO, _log)
              << (boost::format("  HOMO  = %1$4d PQP = %2$+1.4f DQP = %3$+1.4f ") %
              (i + _opt.qpmin + 1) % _gwa_energies(i) %
              qp_diag_energies(i)).str()<< std::flush;
    } else if ((i + _opt.qpmin) == _opt.homo + 1) {
      XTP_LOG(logINFO, _log)
              << (boost::format("  LUMO  = %1$4d PQP = %2$+1.4f DQP = %3$+1.4f ") %
              (i + _opt.qpmin + 1) % _gwa_energies(i) %
              qp_diag_energies(i)).str()<< std::flush;
    } else {
      XTP_LOG(logINFO, _log)
              << (boost::format("  Level = %1$4d PQP = %2$+1.4f DQP = %3$+1.4f ") %
              (i + _opt.qpmin + 1) % _gwa_energies(i) %
              qp_diag_energies(i)).str()<< std::flush;
    }
  }
  return;
}

  Eigen::VectorXd GW::ScissorShift_DFTlevel(const Eigen::VectorXd& dft_energies)const{
       XTP_LOG(logDEBUG, _log) << TimeStamp()
                                 << " Scissor shifting DFT energies by: "<< _opt.shift<<" Hrt"<< std::flush;
      Eigen::VectorXd RPAenergies=dft_energies;
      RPAenergies.segment(_opt.homo+1,dft_energies.size()-_opt.homo-1).array()+=_opt.shift;
      return RPAenergies;
  }
  
bool GW::Converged(const Eigen::VectorXd& e1, const Eigen::VectorXd& e2, double epsilon)const {
    int state = 0;
    bool energies_converged = true;
    double diff_max = (e1 - e2).cwiseAbs().maxCoeff(&state);
    if (diff_max > epsilon) {
        energies_converged = false;
    }
    if (tools::globals::verbose) {
        XTP_LOG(logDEBUG, _log) << TimeStamp() << " E_diff max=" << diff_max << " StateNo:" << state << std::flush;
    }
    return energies_converged;
}

Eigen::VectorXd GW::CalculateExcitationFreq(const Eigen::VectorXd& rpa_energies, Eigen::VectorXd frequencies){
    for (int i_freq = 0; i_freq < _opt.g_sc_max_iterations; ++i_freq) {

        _Sigma_c.diagonal()= _sigma->CalcCorrelationDiag(frequencies, rpa_energies);
        _gwa_energies = CalcDiagonalEnergies();

        if(tools::globals::verbose){
            XTP_LOG(logDEBUG, _log) << TimeStamp() <<" G_Iteration:" <<i_freq << " Shift[Hrt]:" << CalcHomoLumoShift() << std::flush;
        }
        if (Converged(_gwa_energies, frequencies, _opt.g_sc_limit)) {
            XTP_LOG(logDEBUG, _log) << TimeStamp() << " Converged after " <<i_freq + 1 << " G iterations." << std::flush;
            break;
        } else if (i_freq == _opt.g_sc_max_iterations - 1 &&  _opt.g_sc_max_iterations>1) {
            XTP_LOG(logDEBUG, _log) << TimeStamp() << " G-self-consistency cycle not converged after "
                    << _opt.g_sc_max_iterations << " iterations." << std::flush;
            break;
        } else {
            double alpha = 0.0;
            frequencies = (1 - alpha) * _gwa_energies + alpha*frequencies;
        }

    }
    return frequencies;
}

void GW::CalculateGWPerturbation() {
    Eigen::VectorXd rpa_energies = ScissorShift_DFTlevel(_dft_energies);
    _Sigma_x = (1-_opt.ScaHFX)*_sigma->CalcExchange();
         XTP_LOG(logDEBUG, _log) << TimeStamp()
                                 << " Calculated Hartree exchange contribution  " << std::flush;

    Eigen::VectorXd frequencies = rpa_energies.segment(_opt.qpmin, _qptotal);

    RPA rpa(rpa_energies, _Mmn);
    rpa.configure(_opt.homo, _opt.rpamin, _opt.rpamax);
     XTP_LOG(logDEBUG, _log) << TimeStamp()
                                 << " Prepared RPA  " << std::flush;
    for (int i_gw=0; i_gw < _opt.gw_sc_max_iterations; ++i_gw) {

         if(i_gw%_opt.reset_3c==0 && i_gw!=0){
             _Mmn.Rebuild();
             XTP_LOG(logDEBUG, _log) << TimeStamp() <<" Rebuilding 3c integrals" << std::flush;
         }
        _sigma->PrepareScreening(rpa);
        XTP_LOG(logDEBUG, _log) << TimeStamp()
                                   << " Calculated screening via RPA  " << std::flush;
        frequencies=CalculateExcitationFreq(rpa_energies, frequencies);
        XTP_LOG(logDEBUG, _log)
        << TimeStamp() << " Calculated diagonal part of Sigma  " << std::flush;
        Eigen::VectorXd rpa_energies_old=rpa_energies;
        rpa_energies=rpa.UpdateRPAInput(_dft_energies,frequencies,_opt.qpmin,_opt.homo);
        
        XTP_LOG(logDEBUG, _log) << TimeStamp() <<" GW_Iteration:" <<i_gw << " Shift[Hrt]:" << CalcHomoLumoShift() << std::flush;
        if(Converged(rpa_energies, rpa_energies_old, _opt.gw_sc_limit)){
             XTP_LOG(logDEBUG, _log) << TimeStamp() << " Converged after " <<i_gw + 1 << " GW iterations." << std::flush;
                break;
        } else if (i_gw == _opt.gw_sc_max_iterations - 1 &&  _opt.gw_sc_max_iterations>1) {
                XTP_LOG(logDEBUG, _log) << TimeStamp() << " WARNING! GW-self-consistency cycle not converged after "
                        << _opt.gw_sc_max_iterations << " iterations." << std::flush;
             XTP_LOG(logDEBUG, _log)<< TimeStamp()
            << "      Run continues. Inspect results carefully!" << std::flush;
                break;
            } else {
                double alpha = 0.0;
                rpa_energies = (1 - alpha) * rpa_energies + alpha*rpa_energies_old;
            }
    }

    PrintGWA_Energies();
}

   void GW::CalculateHQP(){
       Eigen::VectorXd rpa_energies=RPA::UpdateRPAInput(_dft_energies,_gwa_energies,_opt.qpmin,_opt.homo);
       Eigen::VectorXd diag_backup=_Sigma_c.diagonal();
       _Sigma_c=_sigma->CalcCorrelationOffDiag(_gwa_energies,rpa_energies);
       _Sigma_c.diagonal()=diag_backup;
       
   }
 

  }
};
