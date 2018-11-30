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
    orb.LoadFromXYZ("molecule.xyz");
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
  xyzfile << " C" << std::endl;
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
  
  orb.LoadFromXYZ("molecule.xyz");
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

BOOST_AUTO_TEST_SUITE_END()
