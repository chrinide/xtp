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

#define BOOST_TEST_MODULE orbitals_test
#include <boost/test/unit_test.hpp>
#include <votca/xtp/convergenceacc.h>
#include <votca/xtp/orbitals.h>
#include <fstream>
#include "votca/xtp/aomatrix.h"

using namespace votca::xtp;
using std::endl;
BOOST_AUTO_TEST_SUITE(orbitals_test)
BOOST_AUTO_TEST_CASE(readxyztest){
  std::ofstream xyzfile("molecule.xyz");
  xyzfile << " C 0.0 3 1" << std::endl;
  xyzfile << " methane" << std::endl;
  xyzfile << " C            .000000     .000000     .000000" << std::endl;
  xyzfile << " H            .629118     .629118     .629118" << std::endl;
  xyzfile << " H           -.629118    -.629118     .629118" << std::endl;
  xyzfile << " H            .629118    -.629118    -.629118" << std::endl;
  xyzfile << " H           -.629118     .629118    -.629118" << std::endl;
  xyzfile.close();
  
  bool errorhappen=false;
  Orbitals orb;
  try{
    orb.QMAtoms().LoadFromXYZ("molecule.xyz");
  }catch (const std::runtime_error& error)
{
    std::cout<<error.what()<<std::endl;
    errorhappen=true;
  }
  
   BOOST_CHECK_EQUAL(errorhappen, true);
}

BOOST_AUTO_TEST_CASE(sortEnergies){
  
  Orbitals orb;
  Eigen::VectorXd Energies=Eigen::VectorXd::LinSpaced(10,-5,5);
  Eigen::VectorXd switched=Energies;
  switched(3)=Energies(5);
  switched(5)=Energies(3);
  orb.MOEnergies()=switched;
  orb.MOCoefficients()=Eigen::MatrixXd::Zero(10,10);
  orb.OrderMOsbyEnergy();
  bool issorted=Energies.isApprox(orb.MOEnergies(),0.001);
  if(!issorted){
    std::cout<<"before"<<std::endl;
    std::cout<<Energies<<std::endl;
    std::cout<<"after"<<std::endl;
    std::cout<<orb.MOEnergies()<<std::endl;
  }
  BOOST_CHECK_EQUAL(issorted, true);
}


BOOST_AUTO_TEST_CASE(densmatgs_test) {
  
  Orbitals orb;
  orb.setBasisSetSize(17);
  orb.setNumberOfLevels(4,13);
  
  Eigen::MatrixXd H=Eigen::MatrixXd::Zero(17,17);
  //generated from 3-21G with ecp on methane independent electron guess
  H<<13.2967782,-1.99797328,0,0,0,-0.26575698,0,0,0,-0.0909339466,-0.147466802,-0.0909339466,-0.147466802,-0.0909339466,-0.147466802,-0.0909339466,-0.147466802,
-1.99797328,-4.04412972,0,0,0,-3.49418055,0,0,0,-0.994581408,-1.89398582,-0.994581408,-1.89398582,-0.994581408,-1.89398582,-0.994581408,-1.89398582,
0,0,-3.93848515,0,0,0,-2.25634153,0,0,-0.780335933,-0.599314142,-0.780335933,-0.599314142,0.780335933,0.599314142,0.780335933,0.599314142,
0,0,0,-3.93848515,0,0,0,-2.25634153,0,-0.780335933,-0.599314142,0.780335933,0.599314142,0.780335933,0.599314142,-0.780335933,-0.599314142,
0,0,0,0,-3.93848515,0,0,0,-2.25634153,-0.780335933,-0.599314142,0.780335933,0.599314142,-0.780335933,-0.599314142,0.780335933,0.599314142,
-0.26575698,-3.49418055,0,0,0,-3.88216043,0,0,0,-1.38139383,-2.47288528,-1.38139383,-2.47288528,-1.38139383,-2.47288528,-1.38139383,-2.47288528,
0,0,-2.25634153,0,0,0,-3.02562938,0,0,-1.03641022,-0.99951947,-1.03641022,-0.99951947,1.03641022,0.99951947,1.03641022,0.99951947,
0,0,0,-2.25634153,0,0,0,-3.02562938,0,-1.03641022,-0.99951947,1.03641022,0.99951947,1.03641022,0.99951947,-1.03641022,-0.99951947,
0,0,0,0,-2.25634153,0,0,0,-3.02562938,-1.03641022,-0.99951947,1.03641022,0.99951947,-1.03641022,-0.99951947,1.03641022,0.99951947,
-0.0909339466,-0.994581408,-0.780335933,-0.780335933,-0.780335933,-1.38139383,-1.03641022,-1.03641022,-1.03641022,-3.00123345,-2.29509192,-0.0552940511,-0.512094198,-0.0552940511,-0.512094198,-0.0552940511,-0.512094198,
-0.147466802,-1.89398582,-0.599314142,-0.599314142,-0.599314142,-2.47288528,-0.99951947,-0.99951947,-0.99951947,-2.29509192,-2.99604761,-0.512094198,-1.30279378,-0.512094198,-1.30279378,-0.512094198,-1.30279378,
-0.0909339466,-0.994581408,-0.780335933,0.780335933,0.780335933,-1.38139383,-1.03641022,1.03641022,1.03641022,-0.0552940511,-0.512094198,-3.00123345,-2.29509192,-0.0552940511,-0.512094198,-0.0552940511,-0.512094198,
-0.147466802,-1.89398582,-0.599314142,0.599314142,0.599314142,-2.47288528,-0.99951947,0.99951947,0.99951947,-0.512094198,-1.30279378,-2.29509192,-2.99604761,-0.512094198,-1.30279378,-0.512094198,-1.30279378,
-0.0909339466,-0.994581408,0.780335933,0.780335933,-0.780335933,-1.38139383,1.03641022,1.03641022,-1.03641022,-0.0552940511,-0.512094198,-0.0552940511,-0.512094198,-3.00123345,-2.29509192,-0.0552940511,-0.512094198,
-0.147466802,-1.89398582,0.599314142,0.599314142,-0.599314142,-2.47288528,0.99951947,0.99951947,-0.99951947,-0.512094198,-1.30279378,-0.512094198,-1.30279378,-2.29509192,-2.99604761,-0.512094198,-1.30279378,
-0.0909339466,-0.994581408,0.780335933,-0.780335933,0.780335933,-1.38139383,1.03641022,-1.03641022,1.03641022,-0.0552940511,-0.512094198,-0.0552940511,-0.512094198,-0.0552940511,-0.512094198,-3.00123345,-2.29509192,
-0.147466802,-1.89398582,0.599314142,-0.599314142,0.599314142,-2.47288528,0.99951947,-0.99951947,0.99951947,-0.512094198,-1.30279378,-0.512094198,-1.30279378,-0.512094198,-1.30279378,-2.29509192,-2.99604761;
Eigen::MatrixXd overlap_ref=Eigen::MatrixXd::Zero(17,17);
 
 std::ofstream xyzfile("molecule.xyz");
  xyzfile << " 5" << std::endl;
  xyzfile << " methane" << std::endl;
  xyzfile << " C            .000000     .000000     .000000" << std::endl;
  xyzfile << " H            .629118     .629118     .629118" << std::endl;
  xyzfile << " H           -.629118    -.629118     .629118" << std::endl;
  xyzfile << " H            .629118    -.629118    -.629118" << std::endl;
  xyzfile << " H           -.629118     .629118    -.629118" << std::endl;
  xyzfile.close();

  std::ofstream basisfile("3-21G.xml");
  basisfile <<"<basis name=\"3-21G\">" << endl;
  basisfile << "  <element name=\"H\">" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << endl;
  basisfile << "      <constant decay=\"5.447178e+00\">" << endl;
  basisfile << "        <contractions factor=\"1.562850e-01\" type=\"S\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"8.245470e-01\">" << endl;
  basisfile << "        <contractions factor=\"9.046910e-01\" type=\"S\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << endl;
  basisfile << "      <constant decay=\"1.831920e-01\">" << endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"S\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "  </element>" << endl;
  basisfile << "  <element name=\"C\">" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << endl;
  basisfile << "      <constant decay=\"1.722560e+02\">" << endl;
  basisfile << "        <contractions factor=\"6.176690e-02\" type=\"S\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"2.591090e+01\">" << endl;
  basisfile << "        <contractions factor=\"3.587940e-01\" type=\"S\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"5.533350e+00\">" << endl;
  basisfile << "        <contractions factor=\"7.007130e-01\" type=\"S\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"SP\">" << endl;
  basisfile << "      <constant decay=\"3.664980e+00\">" << endl;
  basisfile << "        <contractions factor=\"-3.958970e-01\" type=\"S\"/>" << endl;
  basisfile << "        <contractions factor=\"2.364600e-01\" type=\"P\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"7.705450e-01\">" << endl;
  basisfile << "        <contractions factor=\"1.215840e+00\" type=\"S\"/>" << endl;
  basisfile << "        <contractions factor=\"8.606190e-01\" type=\"P\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"SP\">" << endl;
  basisfile << "      <constant decay=\"1.958570e-01\">" << endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"S\"/>" << endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"P\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "  </element>" << endl;
  basisfile << "</basis>" << endl;
  basisfile.close();
  
  orb.QMAtoms().LoadFromXYZ("molecule.xyz");
  BasisSet basis;
  basis.LoadBasisSet("3-21G.xml");
  AOBasis aobasis;
  aobasis.AOBasisFill(basis,orb.QMAtoms());
  AOOverlap overlap;
  overlap.Fill(aobasis);
  
  ConvergenceAcc d;
  d.Configure(ConvergenceAcc::closed,false,false,10,false,0,0,0.0,0,8,0);
  d.setOverlap(&overlap,1e-8);
  d.SolveFockmatrix(orb.MOEnergies(),orb.MOCoefficients(),H);

  Eigen::VectorXd MOEnergies_ref=Eigen::VectorXd::Zero(17);
  MOEnergies_ref<<-4.29332753,-3.99146858,-3.99146858,-3.99146858,-2.69172222,-2.69172222,-2.69172222,-2.61521973,-2.19277057,-2.19277057,-2.19277057,-1.75923211,-1.46241535,-1.46241535,-1.46241535,-1.21150295,14.6697624;
  bool check_moenergies=orb.MOEnergies().isApprox(MOEnergies_ref,00001);
  
  BOOST_CHECK_EQUAL(check_moenergies, 1);
  Eigen::MatrixXd dmat=orb.DensityMatrixGroundState();
 
          
   Eigen::MatrixXd dmat_ref=Eigen::MatrixXd::Zero(17,17);       
 dmat_ref<<0.00157507,0.0337454,4.48905e-16,-5.93152e-16,7.87133e-17,0.030876,2.51254e-16,-1.49094e-16,5.77899e-17,0.00415998,-0.00445632,0.00415998,-0.00445632,0.00415998,-0.00445632,0.00415998,-0.00445632,
0.0337454,0.722983,2.66427e-15,-4.44783e-15,3.45846e-16,0.661507,4.39854e-15,-2.02475e-15,1.04832e-15,0.0891262,-0.095475,0.0891262,-0.095475,0.0891262,-0.095475,0.0891262,-0.095475,
4.48905e-16,2.66427e-15,1.52199,2.88658e-15,2.09034e-15,-7.94212e-15,0.215492,2.8727e-15,-1.40513e-15,0.141933,-0.0402359,0.141933,-0.0402359,-0.141933,0.0402359,-0.141933,0.0402359,
-5.93152e-16,-4.44783e-15,2.88658e-15,1.52199,-2.31759e-15,9.21105e-15,-2.22045e-15,0.215492,1.6263e-15,0.141933,-0.0402359,-0.141933,0.0402359,-0.141933,0.0402359,0.141933,-0.0402359,
7.87133e-17,3.45846e-16,2.09034e-15,-2.31759e-15,1.52199,2.98902e-15,-2.04958e-15,4.79738e-15,0.215492,0.141933,-0.0402359,-0.141933,0.0402359,0.141933,-0.0402359,-0.141933,0.0402359,
0.030876,0.661507,-7.94212e-15,9.21105e-15,2.98902e-15,0.605259,2.55488e-15,2.7779e-17,1.33759e-15,0.0815477,-0.0873567,0.0815477,-0.0873567,0.0815477,-0.0873567,0.0815477,-0.0873567,
2.51254e-16,4.39854e-15,0.215492,-2.22045e-15,-2.04958e-15,2.55488e-15,0.0305108,3.29597e-17,-5.29036e-16,0.0200958,-0.00569686,0.0200958,-0.00569686,-0.0200958,0.00569686,-0.0200958,0.00569686,
-1.49094e-16,-2.02475e-15,2.8727e-15,0.215492,4.79738e-15,2.7779e-17,3.29597e-17,0.0305108,9.55941e-16,0.0200958,-0.00569686,-0.0200958,0.00569686,-0.0200958,0.00569686,0.0200958,-0.00569686,
5.77899e-17,1.04832e-15,-1.40513e-15,1.6263e-15,0.215492,1.33759e-15,-5.29036e-16,9.55941e-16,0.0305108,0.0200958,-0.00569686,-0.0200958,0.00569686,0.0200958,-0.00569686,-0.0200958,0.00569686,
0.00415998,0.0891262,0.141933,0.141933,0.141933,0.0815477,0.0200958,0.0200958,0.0200958,0.0506951,-0.0230264,-0.00224894,-0.00801753,-0.00224894,-0.00801753,-0.00224894,-0.00801753,
-0.00445632,-0.095475,-0.0402359,-0.0402359,-0.0402359,-0.0873567,-0.00569686,-0.00569686,-0.00569686,-0.0230264,0.0157992,-0.00801753,0.0115445,-0.00801753,0.0115445,-0.00801753,0.0115445,
0.00415998,0.0891262,0.141933,-0.141933,-0.141933,0.0815477,0.0200958,-0.0200958,-0.0200958,-0.00224894,-0.00801753,0.0506951,-0.0230264,-0.00224894,-0.00801753,-0.00224894,-0.00801753,
-0.00445632,-0.095475,-0.0402359,0.0402359,0.0402359,-0.0873567,-0.00569686,0.00569686,0.00569686,-0.00801753,0.0115445,-0.0230264,0.0157992,-0.00801753,0.0115445,-0.00801753,0.0115445,
0.00415998,0.0891262,-0.141933,-0.141933,0.141933,0.0815477,-0.0200958,-0.0200958,0.0200958,-0.00224894,-0.00801753,-0.00224894,-0.00801753,0.0506951,-0.0230264,-0.00224894,-0.00801753,
-0.00445632,-0.095475,0.0402359,0.0402359,-0.0402359,-0.0873567,0.00569686,0.00569686,-0.00569686,-0.00801753,0.0115445,-0.00801753,0.0115445,-0.0230264,0.0157992,-0.00801753,0.0115445,
0.00415998,0.0891262,-0.141933,0.141933,-0.141933,0.0815477,-0.0200958,0.0200958,-0.0200958,-0.00224894,-0.00801753,-0.00224894,-0.00801753,-0.00224894,-0.00801753,0.0506951,-0.0230264,
-0.00445632,-0.095475,0.0402359,-0.0402359,0.0402359,-0.0873567,0.00569686,-0.00569686,0.00569686,-0.00801753,0.0115445,-0.00801753,0.0115445,-0.00801753,0.0115445,-0.0230264,0.0157992;
 
  bool check_dmat = dmat.isApprox(dmat_ref,0.0001);
BOOST_CHECK_EQUAL(check_dmat, 1);


}


BOOST_AUTO_TEST_CASE(dipole_test) {

  
  std::ofstream xyzfile("molecule.xyz");
  xyzfile << " 5" << endl;
  xyzfile << " methane" << endl;
  xyzfile << " C            .000000     .000000     .000000" << endl;
  xyzfile << " H            .629118     .629118     .629118" << endl;
  xyzfile << " H           -.629118    -.629118     .629118" << endl;
  xyzfile << " H            .629118    -.629118    -.629118" << endl;
  xyzfile << " H           -.629118     .629118    -.629118" << endl;
  xyzfile.close();

  std::ofstream basisfile("3-21G.xml");
  basisfile <<"<basis name=\"3-21G\">" << endl;
  basisfile << "  <element name=\"H\">" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << endl;
  basisfile << "      <constant decay=\"5.447178e+00\">" << endl;
  basisfile << "        <contractions factor=\"1.562850e-01\" type=\"S\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"8.245470e-01\">" << endl;
  basisfile << "        <contractions factor=\"9.046910e-01\" type=\"S\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << endl;
  basisfile << "      <constant decay=\"1.831920e-01\">" << endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"S\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "  </element>" << endl;
  basisfile << "  <element name=\"C\">" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << endl;
  basisfile << "      <constant decay=\"1.722560e+02\">" << endl;
  basisfile << "        <contractions factor=\"6.176690e-02\" type=\"S\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"2.591090e+01\">" << endl;
  basisfile << "        <contractions factor=\"3.587940e-01\" type=\"S\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"5.533350e+00\">" << endl;
  basisfile << "        <contractions factor=\"7.007130e-01\" type=\"S\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"SP\">" << endl;
  basisfile << "      <constant decay=\"3.664980e+00\">" << endl;
  basisfile << "        <contractions factor=\"-3.958970e-01\" type=\"S\"/>" << endl;
  basisfile << "        <contractions factor=\"2.364600e-01\" type=\"P\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"7.705450e-01\">" << endl;
  basisfile << "        <contractions factor=\"1.215840e+00\" type=\"S\"/>" << endl;
  basisfile << "        <contractions factor=\"8.606190e-01\" type=\"P\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"SP\">" << endl;
  basisfile << "      <constant decay=\"1.958570e-01\">" << endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"S\"/>" << endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"P\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "  </element>" << endl;
  basisfile << "</basis>" << endl;
  basisfile.close();
  
  Orbitals orbitals;
  orbitals.QMAtoms().LoadFromXYZ("molecule.xyz");
  BasisSet basis;
  basis.LoadBasisSet("3-21G.xml");
  orbitals.setDFTbasis("3-21G.xml");
  AOBasis aobasis;
  aobasis.AOBasisFill(basis,orbitals.QMAtoms());

  orbitals.setBasisSetSize(17);
  orbitals.setNumberOfLevels(4,13);
 Eigen::MatrixXd& MOs=orbitals.MOCoefficients();
MOs=Eigen::MatrixXd::Zero(17,17);
MOs<<-0.00761992, -4.69664e-13, 8.35009e-15, -1.15214e-14, -0.0156169, -2.23157e-12, 1.52916e-14, 2.10997e-15, 8.21478e-15, 3.18517e-15, 2.89043e-13, -0.00949189, 1.95787e-12, 1.22168e-14, -2.63092e-15, -0.22227, 1.00844,
0.233602, -3.18103e-12, 4.05093e-14, -4.70943e-14, 0.1578, 4.75897e-11, -1.87447e-13, -1.02418e-14, 6.44484e-14, -2.6602e-14, 6.5906e-12, -0.281033, -6.67755e-12, 2.70339e-14, -9.78783e-14, -1.94373, -0.36629,
-1.63678e-13, -0.22745, -0.054851, 0.30351, 3.78688e-11, -0.201627, -0.158318, -0.233561, -0.0509347, -0.650424, 0.452606, -5.88565e-11, 0.453936, -0.165715, -0.619056, 7.0149e-12, 2.395e-14,
-4.51653e-14, -0.216509, 0.296975, -0.108582, 3.79159e-11, -0.199301, 0.283114, -0.0198557, 0.584622, 0.275311, 0.461431, -5.93732e-11, 0.453057, 0.619523, 0.166374, 7.13235e-12, 2.56811e-14,
-9.0903e-14, -0.21966, -0.235919, -0.207249, 3.75979e-11, -0.199736, -0.122681, 0.255585, -0.534902, 0.362837, 0.461224, -5.91028e-11, 0.453245, -0.453298, 0.453695, 7.01644e-12, 2.60987e-14,
0.480866, 1.8992e-11, -2.56795e-13, 4.14571e-13, 2.2709, 4.78615e-10, -2.39153e-12, -2.53852e-13, -2.15605e-13, -2.80359e-13, 7.00137e-12, 0.145171, -1.96136e-11, -2.24876e-13, -2.57294e-14, 4.04176, 0.193617,
-1.64421e-12, -0.182159, -0.0439288, 0.243073, 1.80753e-10, -0.764779, -0.600505, -0.885907, 0.0862014, 1.10077, -0.765985, 6.65828e-11, -0.579266, 0.211468, 0.789976, -1.41532e-11, -1.29659e-13,
-1.64105e-12, -0.173397, 0.23784, -0.0869607, 1.80537e-10, -0.755957, 1.07386, -0.0753135, -0.989408, -0.465933, -0.78092, 6.72256e-11, -0.578145, -0.790571, -0.212309, -1.42443e-11, -1.31306e-13,
-1.63849e-12, -0.17592, -0.188941, -0.165981, 1.79403e-10, -0.757606, -0.465334, 0.969444, 0.905262, -0.61406, -0.78057, 6.69453e-11, -0.578385, 0.578453, -0.578959, -1.40917e-11, -1.31002e-13,
0.129798, -0.274485, 0.00256652, -0.00509635, -0.0118465, 0.141392, -0.000497905, -0.000510338, -0.000526798, -0.00532572, 0.596595, 0.65313, -0.964582, -0.000361559, -0.000717866, -0.195084, 0.0246232,
0.0541331, -0.255228, 0.00238646, -0.0047388, -0.88576, 1.68364, -0.00592888, -0.00607692, -9.5047e-05, -0.000960887, 0.10764, -0.362701, 1.53456, 0.000575205, 0.00114206, -0.793844, -0.035336,
0.129798, 0.0863299, -0.0479412, 0.25617, -0.0118465, -0.0464689, 0.0750316, 0.110468, -0.0436647, -0.558989, -0.203909, 0.65313, 0.320785, 0.235387, 0.878697, -0.195084, 0.0246232,
0.0541331, 0.0802732, -0.0445777, 0.238198, -0.88576, -0.553335, 0.893449, 1.31541, -0.00787816, -0.100855, -0.0367902, -0.362701, -0.510338, -0.374479, -1.39792, -0.793844, -0.035336,
0.129798, 0.0927742, -0.197727, -0.166347, -0.0118465, -0.0473592, 0.0582544, -0.119815, -0.463559, 0.320126, -0.196433, 0.65313, 0.321765, 0.643254, -0.642737, -0.195084, 0.0246232,
0.0541331, 0.0862654, -0.183855, -0.154677, -0.88576, -0.563936, 0.693672, -1.42672, -0.0836372, 0.0577585, -0.0354411, -0.362701, -0.511897, -1.02335, 1.02253, -0.793844, -0.035336,
0.129798, 0.0953806, 0.243102, -0.0847266, -0.0118465, -0.0475639, -0.132788, 0.00985812, 0.507751, 0.244188, -0.196253, 0.65313, 0.322032, -0.87828, -0.235242, -0.195084, 0.0246232,
0.0541331, 0.088689, 0.226046, -0.0787824, -0.88576, -0.566373, -1.58119, 0.117387, 0.0916104, 0.0440574, -0.0354087, -0.362701, -0.512321, 1.39726, 0.374248, -0.793844, -0.035336;

orbitals.setBSEindices(0,16,1);
orbitals.setTDAApprox(true);

MatrixXfd spsi_ref=MatrixXfd::Zero(60,1);
spsi_ref<<-0.000150849,0.00516987,0.0511522,0.00428958,-0.00966668,-0.000155227,1.02978e-08,5.82225e-05,-0.00216177,0.00907102,6.297e-09,-9.84993e-11,0.00159727,
        0.0039042,0.0481196,0.00495382,-0.0106013,0.00025141,-0.000155626,-0.000382828,-0.00322057,0.0124251,1.32177e-05,6.794e-07,
        -0.0153713,0.0200649,-0.067081,-0.0122678,0.0117612,-0.00358901,0.00605007,0.00404793,0.0108884,-0.0151075,-0.000513827,
        -2.64139e-05,-0.0466653,0.0672016,0.021747,-0.0115096,-0.0124868,-0.0115055,0.0187191,0.0124754,0.0149534,0.0112807,-0.00158977,
        -8.17254e-05,-0.00290157,0.0994541,0.984029,0.017835,-0.0401912,-0.000645537,-7.54896e-08,-5.91055e-05,0.00219348,-0.00920484,1.82832e-08,5.56223e-11;

orbitals.BSESingletCoefficients()=spsi_ref;
QMState state_trans=QMState("n2s1");

Eigen::Vector3d res_trans=orbitals.CalcElDipole(state_trans);
Eigen::Vector3d ref_trans=Eigen::Vector3d::Zero();
ref_trans<<  0.118565, 0.0444239,-0.0505149;

bool check_trans=ref_trans.isApprox(res_trans,0.0001);
if(!check_trans){
  std::cout<<"Result transition dipole"<<std::endl;
  std::cout<<res_trans<<std::endl;
  std::cout<<"Ref transition dipole"<<std::endl;
  std::cout<<ref_trans<<std::endl;
}
BOOST_CHECK_EQUAL(check_trans, 1);

QMState state_s1=QMState("s1");
Eigen::Vector3d res_s1=orbitals.CalcElDipole(state_s1);  
Eigen::Vector3d ref_s1=Eigen::Vector3d::Zero();  
ref_s1<<-0.15153501734,-0.42406579479,0.033954362839;
bool check_s1=ref_s1.isApprox(res_s1,0.0001);
if(!check_s1 ){
  std::cout<<"Result s1 dipole"<<std::endl;
  std::cout<<res_s1<<std::endl;
  std::cout<<"Ref s1 dipole"<<std::endl;
  std::cout<<ref_s1<<std::endl;
}
BOOST_CHECK_EQUAL(check_s1, 1);
  
}

BOOST_AUTO_TEST_SUITE_END()
