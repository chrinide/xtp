/*
 * Copyright 2009-2018 The VOTCA Development Team (http://www.votca.org)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#define BOOST_TEST_MAIN

#define BOOST_TEST_MODULE cpmd_test
#include <boost/test/unit_test.hpp>
#include "../libxtp/qmpackages/cpmd.h"
#include <votca/xtp/qmpackagefactory.h>
#include <fstream>
#include <cstdlib>


using namespace votca;
using namespace votca::xtp;

void check_hash(std::string fname, std::string true_hash){
    std::string _command;   
    _command = "md5sum " + fname + " > cpmd_test_md5.txt";
    std::system(_command.c_str());
    std::ifstream hash_file( "cpmd_test_md5.txt" );
    std::string test_hash;
    std::getline(hash_file,test_hash);
    test_hash = test_hash.substr(0, 32);
    BOOST_CHECK_MESSAGE( (test_hash == true_hash), fname +" contents doesn't match expected");
}


BOOST_AUTO_TEST_SUITE(cpmd_test)

BOOST_AUTO_TEST_CASE(input_and_parse_test) {
    //setup logger
    Logger      _log;
    _log.setReportLevel( logDEBUG );
    _log.setMultithreading( true );
    _log.setPreface(logINFO,    "\n... ...");
    _log.setPreface(logERROR,   "\n... ...");
    _log.setPreface(logWARNING, "\n... ...");
    _log.setPreface(logDEBUG,   "\n... ..."); 
    
    //set up xml file with options
    std::ofstream xml_inp( "cpmd_test.xml" );
    xml_inp << "<package>"<< std::endl;
    xml_inp << "  <name>cpmd</name>" << std::endl;
    xml_inp << "  <executable>cpmd_mpi.x</executable>" << std::endl;
    xml_inp << "  <options>none</options>" << std::endl;
    xml_inp << "  <charge>0</charge>" << std::endl;
    xml_inp << "  <spin>0</spin>" << std::endl;
    xml_inp << "  <threads>1</threads>" << std::endl;
    xml_inp << "  <scratch></scratch>" << std::endl;
    xml_inp << "  <cleanup>inp,basis</cleanup>" << std::endl;
    xml_inp << "  <optimizewf>1.0e-6</optimizewf>" << std::endl;
    xml_inp << "  <projectwf>1</projectwf>" << std::endl;
    xml_inp << "  <pwcutoff>90.0</pwcutoff>" << std::endl;
    xml_inp << "  <basisset>ubecp</basisset>" << std::endl;
    xml_inp << "  <ecp>corelevels.xml</ecp>" << std::endl;
    xml_inp << "  <symmetry>0</symmetry>" << std::endl;
    xml_inp << "  <cell>10   1.0   1.0  0.0  0.0  0.0</cell>" << std::endl;
    xml_inp << "  <customCPMDcontrolls>" << std::endl
            << "  STORE WAVEFUNCTIONS" << std::endl
            << "    10 SC=10" << std::endl
            << "  </customCPMDcontrolls>" << std::endl;
    xml_inp << "  <functional>" << std::endl;
    xml_inp << "    <name>BLYP</name>" << std::endl;
    xml_inp << "    <pseudopotentials>" << std::endl;
    xml_inp << "      <O>O_MT_BLYP RAGGIO=1.0</O>" << std::endl;
    xml_inp << "      <H>H_GIA_BLYP RAGGIO=1.0</H>" << std::endl;
    xml_inp << "    </pseudopotentials>" << std::endl;
    xml_inp << "    <l>" << std::endl;
    xml_inp << "      <O>LMAX=P LOC=P</O>" << std::endl;
    xml_inp << "      <H>LMAX=S LOC=S</H>" << std::endl;
    xml_inp << "    </l>" << std::endl;
    xml_inp << "  </functional>" << std::endl;
    xml_inp << "</package>" << std::endl;
    xml_inp.close();
    
    //parse options
    tools::Property    _package_options; 
    load_property_from_xml(_package_options, "cpmd_test.xml");
    
    // get the Cpmd object from the QMPackageFactory
    QMPackageFactory::RegisterAll();
    QMPackage *qmpackage =  QMPackages().Create( "cpmd" );
    qmpackage->setLog( &_log );       
    qmpackage->Initialize( _package_options );
    qmpackage->setRunDir(".");
    
    //set up the system
    Orbitals orbitals;
    std::ofstream xyz_inp( "cpmd_test.xyz" );
    xyz_inp << "         3"<< std::endl;
    xyz_inp << std::endl;
    xyz_inp << "O     2.64589     2.64589     2.763066" << std::endl;
    xyz_inp << "H     1.88894     2.64589     2.177186" << std::endl;
    xyz_inp << "H     3.40284     2.64589     2.177186" << std::endl;
    xyz_inp.close();
    orbitals.QMAtoms().LoadFromXYZ("cpmd_test.xyz");
    qmpackage->WriteInputFile(orbitals);
    
    
    //check that outputs match expected ones
    check_hash("system.inp", "084ec5c118b552f2478e33169e199a52");
    check_hash("system_wfOpt.inp", "37e2e2cba36eeb8cdef493b8d01c642b");
    check_hash("O_ubecp.basis", "7815593830fc7fb4d888dc87275d3564");
    check_hash("H_ubecp.basis", "c479f4d3a194f1ee06d448f90738491f");
    
    //check parsing of output
    qmpackage->ParseLogFile(orbitals);
    BOOST_CHECK_EQUAL(orbitals.getLumo(), 4); //number of states
    Eigen::MatrixXd MO = orbitals.MOCoefficients();
    std::ofstream MO_out( "cpmd_test_MO.dat" );
    MO_out << MO;
    MO_out.close();
    
    
    //cleanup
    std::remove("cpmd_test_md5.txt");
    std::remove("system.inp");
    std::remove("system_wfOpt.inp");
    std::remove("O_ubecp.basis");
    std::remove("H_ubecp.basis");
    
}

BOOST_AUTO_TEST_CASE(parse_output_test) {
  
}



BOOST_AUTO_TEST_SUITE_END()
