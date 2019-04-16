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

#ifndef __VOTCA_XTP_NWCHEM_H
#define __VOTCA_XTP_NWCHEM_H

#include <votca/ctp/apolarsite.h>
#include <votca/xtp/qmpackage.h>

#include <string>

namespace votca {
namespace xtp {
/**
    \brief Wrapper for the Gaussian program

    The Gaussian class executes the Gaussian package
    and extracts information from its log and io files

*/
class NWChem : public QMPackage {
 public:
  std::string getPackageName() { return "nwchem"; }

  void Initialize(tools::Property& options);

  bool WriteInputFile(Orbitals& orbitals);

  bool Run();

  void CleanUp();

  bool ParseLogFile(Orbitals& orbitals);

  bool ParseOrbitalsFile(Orbitals& orbitals);

  std::string getScratchDir() { return _scratch_dir; }

 private:
  bool CheckLogFile();
  bool WriteShellScript();
  bool WriteGuess(Orbitals& orbitals);

  std::string _shell_file_name;
  std::string _chk_file_name;
  std::string _scratch_dir;
  bool _is_optimization;

  std::string _cleanup;

  void WriteBasisset(std::ofstream& nw_file, std::vector<QMAtom*>& qmatoms);
  void WriteECP(std::ofstream& nw_file, std::vector<QMAtom*>& qmatoms);

  std::string FortranFormat(const double& number);
  int WriteBackgroundCharges(std::ofstream& nw_file);
  void WriteChargeOption();
};

}  // namespace xtp
}  // namespace votca

#endif /* __VOTCA_XTP_NWCHEM_H */
