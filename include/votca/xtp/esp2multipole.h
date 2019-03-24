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

#ifndef VOTCA_XTP_ESP2MULTIPOLE_H
#define VOTCA_XTP_ESP2MULTIPOLE_H

#include <boost/filesystem.hpp>
#include <stdio.h>
#include <votca/tools/property.h>
#include <votca/xtp/espfit.h>
#include <votca/xtp/logger.h>
#include <votca/xtp/orbitals.h>
namespace votca {
namespace xtp {

class Esp2multipole {
 public:
  Esp2multipole(Logger* log) {
    _log = log;
    _pairconstraint.resize(0);
    _regionconstraint.resize(0);
  }

  std::string Identify() { return "esp2multipole"; }

  void Initialize(tools::Property& options);

  void Extractingcharges(Orbitals& orbitals);

  void WritetoFile(std::string output_file, const Orbitals& orbitals);
  std::string GetStateString() const { return _state.ToString(); }

 private:
  void PrintDipoles(Orbitals& orbitals);

  QMState _state;
  int _openmp_threads;
  std::string _method;
  std::string _integrationmethod;
  std::string _gridsize;
  bool _use_mulliken;
  bool _use_lowdin;
  bool _use_CHELPG;
  bool _do_svd;
  double _conditionnumber;

  Logger* _log;
  std::vector<std::pair<int, int> > _pairconstraint;  //  pairconstraint[i] is
                                                      //  all the atomindices
                                                      //  which have the same
                                                      //  charge
  std::vector<Espfit::ConstraintRegion> _regionconstraint;
};

}  // namespace xtp
}  // namespace votca

#endif  // VOTCA_XTP_ESP2MULTIPOLE_H
