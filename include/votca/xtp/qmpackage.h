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

#ifndef VOTCA_XTP_QM_PACKAGE_H
#define VOTCA_XTP_QM_PACKAGE_H

#include <votca/xtp/logger.h>
#include <votca/xtp/orbitals.h>
#include <votca/tools/property.h>
#include <votca/xtp/mmregion.h>
#include "votca/xtp/aobasis.h"
#include <boost/format.hpp>

namespace votca {
    namespace xtp {


        // ========================================================================== //
        // QMPackage base class for wrappers of ORCA, GAUSSIAN, NWCHEM etc              //
        // ========================================================================== //

        class QMPackage {
        public:
   
            virtual ~QMPackage() {};

            virtual std::string getPackageName()const = 0;

            virtual void Initialize(tools::Property &options) = 0;

            /// writes a coordinate file WITHOUT taking into account PBCs
            virtual bool WriteInputFile(const Orbitals& orbitals) = 0;

            virtual bool Run() = 0;

            virtual bool ParseLogFile(Orbitals& orbitals) = 0;

            virtual bool ParseOrbitalsFile(Orbitals& orbitals) = 0;

            virtual void CleanUp() = 0;
            
            void setMultipoleBackground(const std::shared_ptr<MMRegion>& PolarSegments);

            void setRunDir(const std::string& run_dir) {
                _run_dir = run_dir;
            }

            void setInputFileName(const std::string& input_file_name) {
                _input_file_name = input_file_name;
            }

            void setLogFileName(const std::string& log_file_name) {
                _log_file_name = log_file_name;
            }

            void setOrbitalsFileName(const std::string& orb_file) {
                _orb_file_name = orb_file;
            }

            void setLog(Logger* pLog) {
                _pLog = pLog;
            }

            bool GuessRequested() {
                return _write_guess;
            }

            bool ECPRequested() {
                return _write_pseudopotentials;
            }

            bool VXCRequested() {
                return _output_Vxc;
            }

            void setCharge(const int charge) {
                _charge = charge;
            }

            void setSpin(const int spin) {
                _spin = spin;
            }

            void setThreads(const int threads) {
                _threads = threads;
            }

            void doGetCharges(bool do_get_charges) {
                _get_charges = do_get_charges;
            }

            const std::string& getBasisSetName() {
                return _basisset_name;
            }

            const std::string& getExecutable() {
                return _executable;
            };

            void setWithPolarization(bool polar){
                _with_polarization=polar;
                return;
            }

            void setDipoleSpacing(double spacing){
                _dpl_spacing=spacing;
                return;
            }

        protected:

            struct MinimalMMCharge{
                MinimalMMCharge(const Eigen::Vector3d& pos,double q):_pos(pos),_q(q){};
                Eigen::Vector3d _pos;
                double _q;
            };

            virtual void WriteChargeOption() =0;
            std::vector<MinimalMMCharge > SplitMultipoles(const PolarSite& site);
            void ReorderOutput(Orbitals& orbitals);
            Eigen::MatrixXd ReorderMOsBack(const Orbitals& orbitals)const;
            bool isLinker( std::string name, std::vector< std::string> linker_names );
            
            
            std::vector<std::string> GetLineAndSplit(std::ifstream& input_file,const std::string separators );
            
            int _charge;
            int _spin; // 2S+1
            int _threads;
            std::string _memory;
            std::string _options;

            std::string _executable;
            std::string _input_file_name;
            std::string _log_file_name;
            std::string _orb_file_name;

            std::string _run_dir;

            std::string _basisset_name;
            std::string _auxbasisset_name;
            std::string _ecp_name;

            std::list< std::string > _cleanup_list;

            bool _get_orbitals;
            bool _get_overlap;
            bool _get_charges;
            

            bool _write_guess;
            bool _write_charges;
            bool _write_basis_set;
            bool _write_pseudopotentials;

            bool _output_Vxc;

            Logger* _pLog;

            
            std::shared_ptr<MMRegion>  _PolarSegments;
            double _dpl_spacing;
            bool _with_polarization;
            
        };
        
       

    }
}

#endif // VOTCA_XTP_QM_PACKAGE_H

