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

#ifndef _VOTCA_XTP_EXCITONCOUPLINGH_H
#define _VOTCA_XTP_EXCITONCOUPLINGH_H

#include <votca/ctp/logger.h>
#include <votca/ctp/qmtool.h>
#include <votca/ctp/xinteractor.h>

#include <stdio.h>
#include <votca/tools/constants.h>

#include <votca/tools/constants.h>
#include <votca/xtp/bsecoupling.h>

#include <votca/xtp/qmpackagefactory.h>

namespace votca {
namespace xtp {
using namespace std;

class ExcitonCoupling : public ctp::QMTool {
 public:
  ExcitonCoupling(){};
  ~ExcitonCoupling(){};

  string Identify() { return "excitoncoupling"; }

  void Initialize(tools::Property* options);
  bool Evaluate();

 private:
  string _orbA, _orbB, _orbAB;
  // int         _trimA, _trimB;

  tools::Property _coupling_options;

  string _output_file;
  bool _classical;
  // bool        _doSinglets;
  // bool        _doTriplets;
  string _mpsA;
  string _mpsB;
  ctp::Logger _log;
};

void ExcitonCoupling::Initialize(tools::Property* options) {
  // _doSinglets=false;
  // _doTriplets=false;
  // update options with the VOTCASHARE defaults
  UpdateWithDefaults(options, "xtp");
  std::string key = "options." + Identify();
  _classical = false;
  if (options->exists(key + ".classical")) {
    _classical = options->get(key + ".classical").as<bool>();

  } else {
    _classical = false;
  }

  if (!_classical) {

    string _coupling_xml =
        options->get(key + ".bsecoupling_options").as<string>();
    load_property_from_xml(_coupling_options, _coupling_xml.c_str());

    _orbA = options->get(key + ".orbitalsA").as<string>();
    _orbB = options->get(key + ".orbitalsB").as<string>();
    _orbAB = options->get(key + ".orbitalsAB").as<string>();

  } else {
    _mpsA = options->get(key + ".mpsA").as<string>();
    _mpsB = options->get(key + ".mpsB").as<string>();
  }
  _output_file = options->get(key + ".output").as<string>();

  // get the path to the shared folders with xml files
  char* votca_share = getenv("VOTCASHARE");
  if (votca_share == NULL)
    throw std::runtime_error("VOTCASHARE not set, cannot open help files.");
}

bool ExcitonCoupling::Evaluate() {

  _log.setReportLevel(ctp::logDEBUG);
  _log.setMultithreading(true);

  _log.setPreface(ctp::logINFO, "\n... ...");
  _log.setPreface(ctp::logERROR, "\n... ...");
  _log.setPreface(ctp::logWARNING, "\n... ...");
  _log.setPreface(ctp::logDEBUG, "\n... ...");
  tools::Property summary;
  tools::Property& job_output = summary.add("output", "");
  // get the corresponding object from the QMPackageFactory
  if (!_classical) {
    Orbitals orbitalsA, orbitalsB, orbitalsAB;
    // load the QM data from serialized orbitals objects

    CTP_LOG(ctp::logDEBUG, _log)
        << " Loading QM data for molecule A from " << _orbA << flush;
    orbitalsA.ReadFromCpt(_orbA);

    CTP_LOG(ctp::logDEBUG, _log)
        << " Loading QM data for molecule B from " << _orbB << flush;
    orbitalsB.ReadFromCpt(_orbB);

    CTP_LOG(ctp::logDEBUG, _log)
        << " Loading QM data for dimer AB from " << _orbAB << flush;
    orbitalsAB.ReadFromCpt(_orbAB);

    BSECoupling bsecoupling;
    bsecoupling.setLogger(&_log);
    bsecoupling.Initialize(_coupling_options);

    bsecoupling.CalculateCouplings(orbitalsA, orbitalsB, orbitalsAB);
    std::cout << _log;

    tools::Property& pair_summary = job_output.add("pair", "");
    tools::Property& type_summary = pair_summary.add("type", "");
    bsecoupling.Addoutput(type_summary, orbitalsA, orbitalsB);

  }

  else if (_classical) {
    CTP_LOG(ctp::logDEBUG, _log)
        << "Calculating electronic coupling using classical transition charges."
        << _orbB << flush;
    std::vector<ctp::APolarSite*> seg1 = ctp::APS_FROM_MPS(_mpsA, 0);
    std::vector<ctp::APolarSite*> seg2 = ctp::APS_FROM_MPS(_mpsB, 0);

    ctp::PolarSeg Seg1 = ctp::PolarSeg(1, seg1);
    ctp::PolarSeg Seg2 = ctp::PolarSeg(2, seg2);
    ctp::XInteractor actor;
    actor.ResetEnergy();
    tools::vec s = tools::vec(0, 0, 0);
    double E = 0.0;
    for (ctp::APolarSite* site1 : Seg1) {
      for (ctp::APolarSite* site2 : Seg2) {
        actor.BiasIndu(*site1, *site2, s);
        site1->Depolarize();
        site2->Depolarize();
        E += actor.E_f(*site1, *site2);
      }
    }

    double J = E * tools::conv::int2eV;

    tools::Property& pair_summary = job_output.add("pair", "");
    pair_summary.setAttribute("idA", 1);
    pair_summary.setAttribute("idB", 2);
    pair_summary.setAttribute("typeA", _mpsA);
    pair_summary.setAttribute("typeB", _mpsB);
    tools::Property& coupling_summary = pair_summary.add("Coupling", "");
    coupling_summary.setAttribute("jABstatic", J);
  }

  tools::PropertyIOManipulator iomXML(tools::PropertyIOManipulator::XML, 1, "");

  std::ofstream ofs(_output_file.c_str(), std::ofstream::out);
  ofs << job_output;
  ofs.close();
  return true;
}

}  // namespace xtp
}  // namespace votca

#endif
