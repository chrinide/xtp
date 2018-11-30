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



#include <votca/xtp/rpa.h>
#include <votca/xtp/aomatrix.h>
#include "votca/xtp/threecenter.h"



namespace votca {
  namespace xtp {

 void RPA::calculate_epsilon(const Eigen::VectorXd& qp_energies,const TCMatrix_gwbse& Mmn_full) {
const int size = Mmn_full.getAuxDimension(); // size of gwbasis
            for (auto& matrix : _epsilon_r) {
                matrix = Eigen::MatrixXd::Identity(size,size);
            }
            for (auto& matrix : _epsilon_i) {
                matrix = Eigen::MatrixXd::Identity(size,size);
            }

            int lumo=_homo+1;
            int n_occ=lumo-_rpamin;
            int n_unocc=_rpamax-_homo;
            
#pragma omp parallel for 
            for (int m_level = 0; m_level < n_occ; m_level++) {
                const double qp_energy_m = qp_energies(m_level + _rpamin);
#if (GWBSE_DOUBLE)
                const Eigen::MatrixXd Mmn_RPA = Mmn_full[ m_level ].block(n_occ, 0,n_unocc, size );
#else
                const Eigen::MatrixXd Mmn_RPA = Mmn_full[ m_level ].block(n_occ,0,  n_unocc, size).cast<double>();       
#endif
                Eigen::MatrixXd tempresult=Eigen::MatrixXd::Zero(size,size);
                Eigen::MatrixXd denom_x_Mmn_RPA=Eigen::MatrixXd::Zero(n_unocc,size);
                for (int i = 0; i < _screen_freq_i.size(); ++i) {   
                    // a temporary matrix, that will get filled in empty levels loop
                    const double screen_freq2 = _screen_freq_i(i) * _screen_freq_i(i);
                    for (int n_level = 0; n_level < n_unocc; n_level++) {
                        const double deltaE = qp_energies(n_level + lumo) - qp_energy_m;
                        const double denom=4.0 * deltaE / (deltaE * deltaE + screen_freq2);  
                        denom_x_Mmn_RPA.row(n_level)=Mmn_RPA.row(n_level)*denom; //hartree    
                    }
                    tempresult.noalias() = Mmn_RPA.transpose() * denom_x_Mmn_RPA;

#pragma omp critical
                    {
                        _epsilon_i[i] += tempresult;
                    }
                }

                //real parts
                for (int i = 0; i < _screen_freq_r.size(); ++i) {
                    for (int n_level = 0;  n_level < n_unocc; n_level++) {
                        const double deltaE = qp_energies(n_level + lumo) - qp_energy_m;
                        const double denom=2.0 * (1.0 / (deltaE - _screen_freq_r(i)) + 1.0 / (deltaE + _screen_freq_r(i)));
                        denom_x_Mmn_RPA.row(n_level)=Mmn_RPA.row(n_level)*denom; //hartree    
                    }
                    tempresult.noalias() = Mmn_RPA.transpose() * denom_x_Mmn_RPA;

#pragma omp critical
                    {
                        _epsilon_r[i] += tempresult;
                    }
                }

            } // occupied levels


            return;
        }

   
  }
};
