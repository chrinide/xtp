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

#define BOOST_TEST_MODULE eris_test
#include <boost/test/unit_test.hpp>
#include <votca/xtp/ERIs.h>
#include <votca/xtp/orbitals.h>
using namespace votca::xtp;
using namespace std;

BOOST_AUTO_TEST_SUITE(eris_test)

BOOST_AUTO_TEST_CASE(fourcenter_cache) {

  ofstream xyzfile("molecule.xyz");
  xyzfile << " 5" << endl;
  xyzfile << " methane" << endl;
  xyzfile << " C            .000000     .000000     .000000" << endl;
  xyzfile << " H            .629118     .629118     .629118" << endl;
  xyzfile << " H           -.629118    -.629118     .629118" << endl;
  xyzfile << " H            .629118    -.629118    -.629118" << endl;
  xyzfile << " H           -.629118     .629118    -.629118" << endl;
  xyzfile.close();

  ofstream basisfile("3-21G.xml");
  basisfile << "<basis name=\"3-21G\">" << endl;
  basisfile << "  <element name=\"H\">" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << endl;
  basisfile << "      <constant decay=\"5.447178e+00\">" << endl;
  basisfile << "        <contractions factor=\"1.562850e-01\" type=\"S\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"8.245470e-01\">" << endl;
  basisfile << "        <contractions factor=\"9.046910e-01\" type=\"S\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << endl;
  basisfile << "      <constant decay=\"1.831920e-01\">" << endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"S\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "  </element>" << endl;
  basisfile << "  <element name=\"C\">" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << endl;
  basisfile << "      <constant decay=\"1.722560e+02\">" << endl;
  basisfile << "        <contractions factor=\"6.176690e-02\" type=\"S\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"2.591090e+01\">" << endl;
  basisfile << "        <contractions factor=\"3.587940e-01\" type=\"S\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"5.533350e+00\">" << endl;
  basisfile << "        <contractions factor=\"7.007130e-01\" type=\"S\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"SP\">" << endl;
  basisfile << "      <constant decay=\"3.664980e+00\">" << endl;
  basisfile << "        <contractions factor=\"-3.958970e-01\" type=\"S\"/>"
            << endl;
  basisfile << "        <contractions factor=\"2.364600e-01\" type=\"P\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"7.705450e-01\">" << endl;
  basisfile << "        <contractions factor=\"1.215840e+00\" type=\"S\"/>"
            << endl;
  basisfile << "        <contractions factor=\"8.606190e-01\" type=\"P\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"SP\">" << endl;
  basisfile << "      <constant decay=\"1.958570e-01\">" << endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"S\"/>"
            << endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"P\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "  </element>" << endl;
  basisfile << "</basis>" << endl;
  basisfile.close();

  Orbitals orbitals;
  orbitals.QMAtoms().LoadFromXYZ("molecule.xyz");
  BasisSet basis;
  basis.LoadBasisSet("3-21G.xml");

  AOBasis aobasis;
  aobasis.AOBasisFill(basis, orbitals.QMAtoms());

  Eigen::MatrixXd dmat = Eigen::MatrixXd::Zero(17, 17);
  dmat << 0.00157507, 0.0337454, 4.48905e-16, -5.93152e-16, 7.87133e-17,
      0.030876, 2.51254e-16, -1.49094e-16, 5.77899e-17, 0.00415998, -0.00445632,
      0.00415998, -0.00445632, 0.00415998, -0.00445632, 0.00415998, -0.00445632,
      0.0337454, 0.722983, 2.66427e-15, -4.44783e-15, 3.45846e-16, 0.661507,
      4.39854e-15, -2.02475e-15, 1.04832e-15, 0.0891262, -0.095475, 0.0891262,
      -0.095475, 0.0891262, -0.095475, 0.0891262, -0.095475, 4.48905e-16,
      2.66427e-15, 1.52199, 2.88658e-15, 2.09034e-15, -7.94212e-15, 0.215492,
      2.8727e-15, -1.40513e-15, 0.141933, -0.0402359, 0.141933, -0.0402359,
      -0.141933, 0.0402359, -0.141933, 0.0402359, -5.93152e-16, -4.44783e-15,
      2.88658e-15, 1.52199, -2.31759e-15, 9.21105e-15, -2.22045e-15, 0.215492,
      1.6263e-15, 0.141933, -0.0402359, -0.141933, 0.0402359, -0.141933,
      0.0402359, 0.141933, -0.0402359, 7.87133e-17, 3.45846e-16, 2.09034e-15,
      -2.31759e-15, 1.52199, 2.98902e-15, -2.04958e-15, 4.79738e-15, 0.215492,
      0.141933, -0.0402359, -0.141933, 0.0402359, 0.141933, -0.0402359,
      -0.141933, 0.0402359, 0.030876, 0.661507, -7.94212e-15, 9.21105e-15,
      2.98902e-15, 0.605259, 2.55488e-15, 2.7779e-17, 1.33759e-15, 0.0815477,
      -0.0873567, 0.0815477, -0.0873567, 0.0815477, -0.0873567, 0.0815477,
      -0.0873567, 2.51254e-16, 4.39854e-15, 0.215492, -2.22045e-15,
      -2.04958e-15, 2.55488e-15, 0.0305108, 3.29597e-17, -5.29036e-16,
      0.0200958, -0.00569686, 0.0200958, -0.00569686, -0.0200958, 0.00569686,
      -0.0200958, 0.00569686, -1.49094e-16, -2.02475e-15, 2.8727e-15, 0.215492,
      4.79738e-15, 2.7779e-17, 3.29597e-17, 0.0305108, 9.55941e-16, 0.0200958,
      -0.00569686, -0.0200958, 0.00569686, -0.0200958, 0.00569686, 0.0200958,
      -0.00569686, 5.77899e-17, 1.04832e-15, -1.40513e-15, 1.6263e-15, 0.215492,
      1.33759e-15, -5.29036e-16, 9.55941e-16, 0.0305108, 0.0200958, -0.00569686,
      -0.0200958, 0.00569686, 0.0200958, -0.00569686, -0.0200958, 0.00569686,
      0.00415998, 0.0891262, 0.141933, 0.141933, 0.141933, 0.0815477, 0.0200958,
      0.0200958, 0.0200958, 0.0506951, -0.0230264, -0.00224894, -0.00801753,
      -0.00224894, -0.00801753, -0.00224894, -0.00801753, -0.00445632,
      -0.095475, -0.0402359, -0.0402359, -0.0402359, -0.0873567, -0.00569686,
      -0.00569686, -0.00569686, -0.0230264, 0.0157992, -0.00801753, 0.0115445,
      -0.00801753, 0.0115445, -0.00801753, 0.0115445, 0.00415998, 0.0891262,
      0.141933, -0.141933, -0.141933, 0.0815477, 0.0200958, -0.0200958,
      -0.0200958, -0.00224894, -0.00801753, 0.0506951, -0.0230264, -0.00224894,
      -0.00801753, -0.00224894, -0.00801753, -0.00445632, -0.095475, -0.0402359,
      0.0402359, 0.0402359, -0.0873567, -0.00569686, 0.00569686, 0.00569686,
      -0.00801753, 0.0115445, -0.0230264, 0.0157992, -0.00801753, 0.0115445,
      -0.00801753, 0.0115445, 0.00415998, 0.0891262, -0.141933, -0.141933,
      0.141933, 0.0815477, -0.0200958, -0.0200958, 0.0200958, -0.00224894,
      -0.00801753, -0.00224894, -0.00801753, 0.0506951, -0.0230264, -0.00224894,
      -0.00801753, -0.00445632, -0.095475, 0.0402359, 0.0402359, -0.0402359,
      -0.0873567, 0.00569686, 0.00569686, -0.00569686, -0.00801753, 0.0115445,
      -0.00801753, 0.0115445, -0.0230264, 0.0157992, -0.00801753, 0.0115445,
      0.00415998, 0.0891262, -0.141933, 0.141933, -0.141933, 0.0815477,
      -0.0200958, 0.0200958, -0.0200958, -0.00224894, -0.00801753, -0.00224894,
      -0.00801753, -0.00224894, -0.00801753, 0.0506951, -0.0230264, -0.00445632,
      -0.095475, 0.0402359, -0.0402359, 0.0402359, -0.0873567, 0.00569686,
      -0.00569686, 0.00569686, -0.00801753, 0.0115445, -0.00801753, 0.0115445,
      -0.00801753, 0.0115445, -0.0230264, 0.0157992;

  ERIs eris;
  eris.Initialize_4c_small_molecule(aobasis);
  eris.CalculateERIs_4c_small_molecule(dmat);

  Eigen::MatrixXd eris_ref = Eigen::MatrixXd::Zero(17, 17);
  eris_ref << 7.97316, 1.45564, -3.0651e-17, 8.17169e-17, 3.42098e-16, 1.39992,
      -3.24766e-18, 1.13521e-17, 4.75743e-17, 0.145934, 0.627171, 0.145934,
      0.627171, 0.145934, 0.627171, 0.145934, 0.627171, 1.45564, 6.28995,
      4.29881e-17, 4.47071e-16, 1.36225e-15, 4.46207, 4.45165e-17, 2.4258e-16,
      6.14373e-16, 1.0517, 2.29374, 1.0517, 2.29374, 1.0517, 2.29374, 1.0517,
      2.29374, -3.0651e-17, 4.29881e-17, 6.26692, 3.16473e-16, 4.80361e-17,
      5.64633e-17, 2.77119, 1.33411e-16, -2.0052e-18, 0.861649, 0.693449,
      0.861649, 0.693449, -0.861649, -0.693449, -0.861649, -0.693449,
      8.17169e-17, 4.47071e-16, 3.16473e-16, 6.26692, 6.55882e-17, 3.86325e-16,
      1.33411e-16, 2.77119, 6.69489e-17, 0.861649, 0.693449, -0.861649,
      -0.693449, -0.861649, -0.693449, 0.861649, 0.693449, 3.42098e-16,
      1.36225e-15, 4.80361e-17, 6.55882e-17, 6.26692, 1.04265e-15, -2.0052e-18,
      6.69489e-17, 2.77119, 0.861649, 0.693449, -0.861649, -0.693449, 0.861649,
      0.693449, -0.861649, -0.693449, 1.39992, 4.46207, 5.64633e-17,
      3.86325e-16, 1.04265e-15, 4.6748, 1.26956e-16, 3.96496e-16, 8.46433e-16,
      1.45414, 2.8438, 1.45414, 2.8438, 1.45414, 2.8438, 1.45414, 2.8438,
      -3.24766e-18, 4.45165e-17, 2.77119, 1.33411e-16, -2.0052e-18, 1.26956e-16,
      3.60471, 1.19577e-16, -4.02136e-17, 1.12112, 1.13632, 1.12112, 1.13632,
      -1.12112, -1.13632, -1.12112, -1.13632, 1.13521e-17, 2.4258e-16,
      1.33411e-16, 2.77119, 6.69489e-17, 3.96496e-16, 1.19577e-16, 3.60471,
      1.28251e-16, 1.12112, 1.13632, -1.12112, -1.13632, -1.12112, -1.13632,
      1.12112, 1.13632, 4.75743e-17, 6.14373e-16, -2.0052e-18, 6.69489e-17,
      2.77119, 8.46433e-16, -4.02136e-17, 1.28251e-16, 3.60471, 1.12112,
      1.13632, -1.12112, -1.13632, 1.12112, 1.13632, -1.12112, -1.13632,
      0.145934, 1.0517, 0.861649, 0.861649, 0.861649, 1.45414, 1.12112, 1.12112,
      1.12112, 3.82992, 2.41107, 0.0414422, 0.512709, 0.0414422, 0.512709,
      0.0414422, 0.512709, 0.627171, 2.29374, 0.693449, 0.693449, 0.693449,
      2.8438, 1.13632, 1.13632, 1.13632, 2.41107, 3.3433, 0.512709, 1.4449,
      0.512709, 1.4449, 0.512709, 1.4449, 0.145934, 1.0517, 0.861649, -0.861649,
      -0.861649, 1.45414, 1.12112, -1.12112, -1.12112, 0.0414422, 0.512709,
      3.82992, 2.41107, 0.0414422, 0.512709, 0.0414422, 0.512709, 0.627171,
      2.29374, 0.693449, -0.693449, -0.693449, 2.8438, 1.13632, -1.13632,
      -1.13632, 0.512709, 1.4449, 2.41107, 3.3433, 0.512709, 1.4449, 0.512709,
      1.4449, 0.145934, 1.0517, -0.861649, -0.861649, 0.861649, 1.45414,
      -1.12112, -1.12112, 1.12112, 0.0414422, 0.512709, 0.0414422, 0.512709,
      3.82992, 2.41107, 0.0414422, 0.512709, 0.627171, 2.29374, -0.693449,
      -0.693449, 0.693449, 2.8438, -1.13632, -1.13632, 1.13632, 0.512709,
      1.4449, 0.512709, 1.4449, 2.41107, 3.3433, 0.512709, 1.4449, 0.145934,
      1.0517, -0.861649, 0.861649, -0.861649, 1.45414, -1.12112, 1.12112,
      -1.12112, 0.0414422, 0.512709, 0.0414422, 0.512709, 0.0414422, 0.512709,
      3.82992, 2.41107, 0.627171, 2.29374, -0.693449, 0.693449, -0.693449,
      2.8438, -1.13632, 1.13632, -1.13632, 0.512709, 1.4449, 0.512709, 1.4449,
      0.512709, 1.4449, 2.41107, 3.3433;
  bool eris_check = eris.getERIs().isApprox(eris_ref, 0.00001);
  BOOST_CHECK_EQUAL(eris_check, 1);

  eris.CalculateEXX_4c_small_molecule(dmat);

  Eigen::MatrixXd exx_ref = Eigen::MatrixXd::Zero(17, 17);
  exx_ref << 0.389974, 0.688493, 3.79471e-17, 7.81168e-17, 1.08518e-15,
      0.517741, 4.72468e-16, -9.48677e-17, 5.73814e-16, 0.127329, 0.269914,
      0.127329, 0.269914, 0.127329, 0.269914, 0.127329, 0.269914, 0.688493,
      2.50146, -1.02362e-16, 1.06661e-15, 3.44768e-15, 1.9867, 1.61085e-15,
      2.45057e-16, 2.26081e-15, 0.565584, 1.07459, 0.565584, 1.07459, 0.565584,
      1.07459, 0.565584, 1.07459, 4.46971e-17, -1.27916e-16, 2.183, 2.72192e-15,
      4.18152e-16, -1.07828e-15, 1.09857, 1.98561e-15, -3.66135e-16, 0.398514,
      0.296396, 0.398514, 0.296396, -0.398514, -0.296396, -0.398514, -0.296396,
      7.76458e-17, 1.06603e-15, 2.72067e-15, 2.183, 5.41643e-16, 2.43269e-15,
      5.75359e-16, 1.09857, 9.80009e-16, 0.398514, 0.296396, -0.398514,
      -0.296396, -0.398514, -0.296396, 0.398514, 0.296396, 1.08939e-15,
      3.43893e-15, 4.20912e-16, 5.42781e-16, 2.183, 3.25489e-15, -5.41257e-16,
      1.86621e-15, 1.09857, 0.398514, 0.296396, -0.398514, -0.296396, 0.398514,
      0.296396, -0.398514, -0.296396, 0.517741, 1.9867, -1.09083e-15,
      2.45266e-15, 3.22787e-15, 1.65289, 1.02023e-15, 1.07747e-15, 2.28546e-15,
      0.499817, 0.909896, 0.499817, 0.909896, 0.499817, 0.909896, 0.499817,
      0.909896, 4.7234e-16, 1.60133e-15, 1.09857, 5.68733e-16, -5.4011e-16,
      1.0231e-15, 0.646708, 6.65839e-16, -6.59225e-16, 0.250323, 0.180503,
      0.250323, 0.180503, -0.250323, -0.180503, -0.250323, -0.180503,
      -9.44025e-17, 2.32187e-16, 1.9847e-15, 1.09857, 1.8418e-15, 1.07759e-15,
      6.64172e-16, 0.646708, 1.47368e-15, 0.250323, 0.180503, -0.250323,
      -0.180503, -0.250323, -0.180503, 0.250323, 0.180503, 5.74192e-16,
      2.26622e-15, -3.64074e-16, 9.70211e-16, 1.09857, 2.28268e-15,
      -6.57724e-16, 1.47071e-15, 0.646708, 0.250323, 0.180503, -0.250323,
      -0.180503, 0.250323, 0.180503, -0.250323, -0.180503, 0.127329, 0.565584,
      0.398514, 0.398514, 0.398514, 0.499817, 0.250323, 0.250323, 0.250323,
      0.562844, 0.51555, 0.0484527, 0.204984, 0.0484527, 0.204984, 0.0484527,
      0.204984, 0.269914, 1.07459, 0.296396, 0.296396, 0.296396, 0.909896,
      0.180503, 0.180503, 0.180503, 0.51555, 0.66257, 0.204984, 0.451934,
      0.204984, 0.451934, 0.204984, 0.451934, 0.127329, 0.565584, 0.398514,
      -0.398514, -0.398514, 0.499817, 0.250323, -0.250323, -0.250323, 0.0484527,
      0.204984, 0.562844, 0.51555, 0.0484527, 0.204984, 0.0484527, 0.204984,
      0.269914, 1.07459, 0.296396, -0.296396, -0.296396, 0.909896, 0.180503,
      -0.180503, -0.180503, 0.204984, 0.451934, 0.51555, 0.66257, 0.204984,
      0.451934, 0.204984, 0.451934, 0.127329, 0.565584, -0.398514, -0.398514,
      0.398514, 0.499817, -0.250323, -0.250323, 0.250323, 0.0484527, 0.204984,
      0.0484527, 0.204984, 0.562844, 0.51555, 0.0484527, 0.204984, 0.269914,
      1.07459, -0.296396, -0.296396, 0.296396, 0.909896, -0.180503, -0.180503,
      0.180503, 0.204984, 0.451934, 0.204984, 0.451934, 0.51555, 0.66257,
      0.204984, 0.451934, 0.127329, 0.565584, -0.398514, 0.398514, -0.398514,
      0.499817, -0.250323, 0.250323, -0.250323, 0.0484527, 0.204984, 0.0484527,
      0.204984, 0.0484527, 0.204984, 0.562844, 0.51555, 0.269914, 1.07459,
      -0.296396, 0.296396, -0.296396, 0.909896, -0.180503, 0.180503, -0.180503,
      0.204984, 0.451934, 0.204984, 0.451934, 0.204984, 0.451934, 0.51555,
      0.66257;
  bool exxs_check = eris.getEXX().isApprox(exx_ref, 0.00001);
  BOOST_CHECK_EQUAL(exxs_check, 1);
}

BOOST_AUTO_TEST_CASE(threecenter) {

  ofstream xyzfile("molecule.xyz");
  xyzfile << " 5" << endl;
  xyzfile << " methane" << endl;
  xyzfile << " C            .000000     .000000     .000000" << endl;
  xyzfile << " H            .629118     .629118     .629118" << endl;
  xyzfile << " H           -.629118    -.629118     .629118" << endl;
  xyzfile << " H            .629118    -.629118    -.629118" << endl;
  xyzfile << " H           -.629118     .629118    -.629118" << endl;
  xyzfile.close();

  ofstream basisfile("3-21G.xml");
  basisfile << "<basis name=\"3-21G\">" << endl;
  basisfile << "  <element name=\"H\">" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << endl;
  basisfile << "      <constant decay=\"5.447178e+00\">" << endl;
  basisfile << "        <contractions factor=\"1.562850e-01\" type=\"S\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"8.245470e-01\">" << endl;
  basisfile << "        <contractions factor=\"9.046910e-01\" type=\"S\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << endl;
  basisfile << "      <constant decay=\"1.831920e-01\">" << endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"S\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "  </element>" << endl;
  basisfile << "  <element name=\"C\">" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << endl;
  basisfile << "      <constant decay=\"1.722560e+02\">" << endl;
  basisfile << "        <contractions factor=\"6.176690e-02\" type=\"S\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"2.591090e+01\">" << endl;
  basisfile << "        <contractions factor=\"3.587940e-01\" type=\"S\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"5.533350e+00\">" << endl;
  basisfile << "        <contractions factor=\"7.007130e-01\" type=\"S\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"SP\">" << endl;
  basisfile << "      <constant decay=\"3.664980e+00\">" << endl;
  basisfile << "        <contractions factor=\"-3.958970e-01\" type=\"S\"/>"
            << endl;
  basisfile << "        <contractions factor=\"2.364600e-01\" type=\"P\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"7.705450e-01\">" << endl;
  basisfile << "        <contractions factor=\"1.215840e+00\" type=\"S\"/>"
            << endl;
  basisfile << "        <contractions factor=\"8.606190e-01\" type=\"P\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"SP\">" << endl;
  basisfile << "      <constant decay=\"1.958570e-01\">" << endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"S\"/>"
            << endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"P\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "  </element>" << endl;
  basisfile << "</basis>" << endl;
  basisfile.close();

  Orbitals orbitals;
  orbitals.QMAtoms().LoadFromXYZ("molecule.xyz");
  BasisSet basis;
  basis.LoadBasisSet("3-21G.xml");

  AOBasis aobasis;
  aobasis.AOBasisFill(basis, orbitals.QMAtoms());

  Eigen::MatrixXd dmat = Eigen::MatrixXd::Zero(17, 17);
  dmat << 0.00157539, 0.0337504, -3.73717e-16, -2.65681e-16, 9.65234e-18,
      0.0308764, 7.13611e-17, 1.02507e-16, -3.09238e-17, 0.0041604, -0.00445629,
      0.0041604, -0.00445629, 0.0041604, -0.00445629, 0.0041604, -0.00445629,
      0.0337504, 0.723053, -3.57432e-15, -2.9052e-15, 3.71509e-16, 0.661482,
      2.15632e-15, 2.59062e-15, -6.39175e-16, 0.0891305, -0.0954696, 0.0891305,
      -0.0954696, 0.0891305, -0.0954696, 0.0891305, -0.0954696, -3.73717e-16,
      -3.57432e-15, 1.52199, -1.00243e-17, -3.01821e-15, 4.79258e-15, 0.215491,
      -7.65321e-15, 1.92163e-15, 0.141933, -0.0402357, 0.141933, -0.0402357,
      -0.141933, 0.0402357, -0.141933, 0.0402357, -2.65681e-16, -2.9052e-15,
      -1.00243e-17, 1.52199, 8.85959e-16, 1.71603e-15, 9.42566e-15, 0.215491,
      -6.60818e-15, 0.141933, -0.0402357, -0.141933, 0.0402357, -0.141933,
      0.0402357, 0.141933, -0.0402357, 9.65234e-18, 3.71509e-16, -3.01821e-15,
      8.85959e-16, 1.52199, -1.2338e-15, 5.28812e-15, 6.19506e-15, 0.215491,
      0.141933, -0.0402357, -0.141933, 0.0402357, 0.141933, -0.0402357,
      -0.141933, 0.0402357, 0.0308764, 0.661482, 4.79258e-15, 1.71603e-15,
      -1.2338e-15, 0.605153, 3.11423e-15, 2.98929e-15, -8.07556e-16, 0.0815406,
      -0.0873399, 0.0815406, -0.0873399, 0.0815406, -0.0873399, 0.0815406,
      -0.0873399, 7.13611e-17, 2.15632e-15, 0.215491, 9.42566e-15, 5.28812e-15,
      3.11423e-15, 0.0305104, 2.51026e-16, 1.08143e-15, 0.0200957, -0.00569679,
      0.0200957, -0.00569679, -0.0200957, 0.00569679, -0.0200957, 0.00569679,
      1.02507e-16, 2.59062e-15, -7.65321e-15, 0.215491, 6.19506e-15,
      2.98929e-15, 2.51026e-16, 0.0305104, -7.72888e-17, 0.0200957, -0.00569679,
      -0.0200957, 0.00569679, -0.0200957, 0.00569679, 0.0200957, -0.00569679,
      -3.09238e-17, -6.39175e-16, 1.92163e-15, -6.60818e-15, 0.215491,
      -8.07556e-16, 1.08143e-15, -7.72888e-17, 0.0305104, 0.0200957,
      -0.00569679, -0.0200957, 0.00569679, 0.0200957, -0.00569679, -0.0200957,
      0.00569679, 0.0041604, 0.0891305, 0.141933, 0.141933, 0.141933, 0.0815406,
      0.0200957, 0.0200957, 0.0200957, 0.0506951, -0.0230251, -0.00224893,
      -0.00801631, -0.00224893, -0.00801631, -0.00224893, -0.00801631,
      -0.00445629, -0.0954696, -0.0402357, -0.0402357, -0.0402357, -0.0873399,
      -0.00569679, -0.00569679, -0.00569679, -0.0230251, 0.0157965, -0.00801631,
      0.0115418, -0.00801631, 0.0115418, -0.00801631, 0.0115418, 0.0041604,
      0.0891305, 0.141933, -0.141933, -0.141933, 0.0815406, 0.0200957,
      -0.0200957, -0.0200957, -0.00224893, -0.00801631, 0.0506951, -0.0230251,
      -0.00224893, -0.00801631, -0.00224893, -0.00801631, -0.00445629,
      -0.0954696, -0.0402357, 0.0402357, 0.0402357, -0.0873399, -0.00569679,
      0.00569679, 0.00569679, -0.00801631, 0.0115418, -0.0230251, 0.0157965,
      -0.00801631, 0.0115418, -0.00801631, 0.0115418, 0.0041604, 0.0891305,
      -0.141933, -0.141933, 0.141933, 0.0815406, -0.0200957, -0.0200957,
      0.0200957, -0.00224893, -0.00801631, -0.00224893, -0.00801631, 0.0506951,
      -0.0230251, -0.00224893, -0.00801631, -0.00445629, -0.0954696, 0.0402357,
      0.0402357, -0.0402357, -0.0873399, 0.00569679, 0.00569679, -0.00569679,
      -0.00801631, 0.0115418, -0.00801631, 0.0115418, -0.0230251, 0.0157965,
      -0.00801631, 0.0115418, 0.0041604, 0.0891305, -0.141933, 0.141933,
      -0.141933, 0.0815406, -0.0200957, 0.0200957, -0.0200957, -0.00224893,
      -0.00801631, -0.00224893, -0.00801631, -0.00224893, -0.00801631,
      0.0506951, -0.0230251, -0.00445629, -0.0954696, 0.0402357, -0.0402357,
      0.0402357, -0.0873399, 0.00569679, -0.00569679, 0.00569679, -0.00801631,
      0.0115418, -0.00801631, 0.0115418, -0.00801631, 0.0115418, -0.0230251,
      0.0157965;

  Eigen::MatrixXd mos = Eigen::MatrixXd::Zero(17, 17);
  mos << -0.0280659, 4.09009e-16, 8.10359e-17, -3.17589e-17, 2.60409e-16,
      5.98083e-17, 5.86316e-16, -0.0686399, -2.05288e-16, -3.64613e-16,
      2.35016e-16, 0.0567761, 2.78239e-16, 2.13909e-15, -8.89567e-16, 0.207008,
      1.00756, -0.601271, 6.10444e-15, 5.5398e-16, 6.1698e-17, 2.54555e-15,
      9.36257e-16, 5.65665e-15, -0.748531, -1.47712e-15, -3.2823e-15,
      2.12599e-15, 0.426415, 2.11064e-15, 1.76729e-14, -7.30842e-15, 1.68086,
      -0.376496, -5.74943e-15, -0.867437, 0.0897498, 0.0221836, 0.483062,
      0.275478, -0.141328, -2.38283e-15, -0.52216, 0.262923, -0.132998,
      -8.22658e-16, 0.0754187, -0.212918, 0.116965, 2.00618e-15, -2.58415e-17,
      1.05977e-15, -0.0684321, -0.764063, 0.415353, 0.29223, -0.31914, 0.376779,
      6.37317e-16, -0.109947, 0.0772177, 0.584309, 1.26088e-15, 0.113063,
      0.139163, 0.180425, -1.58122e-15, -2.05324e-17, -1.39766e-15, -0.0621626,
      -0.411274, -0.766801, -0.10229, 0.389195, 0.408993, 2.28634e-15, 0.273366,
      0.53327, -0.0190346, 9.96897e-16, -0.215016, -0.00150619, 0.1359,
      1.36764e-15, -1.32384e-17, -0.55007, 8.57097e-16, -7.88251e-16,
      2.16043e-15, -4.00107e-15, 2.87815e-15, -2.96673e-15, -0.187648,
      9.25968e-15, 4.17991e-15, -6.22492e-15, -0.315635, -1.9449e-15,
      -5.25747e-14, 1.93335e-14, -4.61576, 0.19997, -3.028e-15, -0.122816,
      0.0127073, 0.00314088, 0.00603592, 0.00344213, -0.00176591, 7.69767e-15,
      1.63739, -0.824478, 0.417058, 5.69265e-15, -0.308788, 0.871755, -0.47889,
      -9.31376e-15, 2.0897e-17, -2.34629e-15, -0.00968898, -0.10818, 0.058808,
      0.00365145, -0.00398769, 0.00470789, 2.27294e-15, 0.344773, -0.24214,
      -1.83228, -3.79824e-15, -0.462914, -0.569779, -0.738719, 4.8908e-15,
      1.27283e-16, 3.77373e-16, -0.00880132, -0.0582304, -0.108568, -0.00127812,
      0.00486303, 0.00511041, 1.86452e-15, -0.857224, -1.67223, 0.059689,
      -4.15328e-15, 0.880343, 0.00616681, -0.556418, -5.76928e-15, 4.37339e-17,
      -0.0741184, -0.0930716, -0.101237, -0.0307056, -0.470865, -0.241752,
      -0.450884, 0.320794, -0.149702, 0.364472, 0.180388, 0.603045, 0.0500511,
      0.141963, -0.817302, 0.0972599, 0.0265551, 0.0793899, 0.0263843,
      0.0286989, 0.00870452, -0.25194, -0.129351, -0.241248, 0.246325,
      -0.300217, 0.730926, 0.361756, -0.412795, -0.125917, -0.357144, 2.05613,
      1.14533, -0.0365659, -0.0741184, -0.0687144, 0.117976, 0.0348431,
      -0.205082, -0.143724, 0.648644, 0.320794, -0.28609, -0.145038, -0.291388,
      0.603045, -0.334571, 0.661279, 0.376048, 0.0972599, 0.0265551, 0.0793899,
      0.0194794, -0.0334442, -0.00987743, -0.109731, -0.0769004, 0.347061,
      0.246325, -0.573736, -0.290864, -0.58436, -0.412795, 0.8417, -1.66362,
      -0.946047, 1.14533, -0.0365659, -0.0741184, 0.0814777, 0.0245298,
      -0.112311, 0.613999, -0.302847, -0.121418, 0.320794, 0.377852, 0.0805922,
      -0.196274, 0.603045, 0.761104, -0.136281, 0.304613, 0.0972599, 0.0265551,
      0.0793899, -0.0230976, -0.00695379, 0.0318382, 0.328524, -0.162041,
      -0.0649658, 0.246325, 0.757758, 0.161623, -0.393615, -0.412795, -1.91475,
      0.342849, -0.766334, 1.14533, -0.0365659, -0.0741184, 0.0803083,
      -0.0412691, 0.108173, 0.0619484, 0.688323, -0.0763414, 0.320794,
      0.0579405, -0.300027, 0.307274, 0.603045, -0.476584, -0.666962, 0.13664,
      0.0972599, 0.0265551, 0.0793899, -0.0227661, 0.0116991, -0.0306653,
      0.0331459, 0.368292, -0.040847, 0.246325, 0.116196, -0.601685, 0.616218,
      -0.412795, 1.19897, 1.67792, -0.343754, 1.14533, -0.0365659;

  ERIs eris;
  eris.Initialize(aobasis, aobasis);
  eris.CalculateEXX(dmat);
  Eigen::MatrixXd eri_d = eris.getERIs();
  eris.CalculateEXX(mos.block(0, 0, 17, 4), dmat);
  Eigen::MatrixXd eri_mo = eris.getERIs();

  bool compare_eris = eri_mo.isApprox(eri_d, 1e-4);
  BOOST_CHECK_EQUAL(compare_eris, 1);
}

BOOST_AUTO_TEST_CASE(fourcenter_direct) {

  ofstream xyzfile("molecule.xyz");
  xyzfile << " 5" << endl;
  xyzfile << " methane" << endl;
  xyzfile << " C            .000000     .000000     .000000" << endl;
  xyzfile << " H            .629118     .629118     .629118" << endl;
  xyzfile << " H           -.629118    -.629118     .629118" << endl;
  xyzfile << " H            .629118    -.629118    -.629118" << endl;
  xyzfile << " H           -.629118     .629118    -.629118" << endl;
  xyzfile.close();

  ofstream basisfile("3-21G.xml");
  basisfile << "<basis name=\"3-21G\">" << endl;
  basisfile << "  <element name=\"H\">" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << endl;
  basisfile << "      <constant decay=\"5.447178e+00\">" << endl;
  basisfile << "        <contractions factor=\"1.562850e-01\" type=\"S\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"8.245470e-01\">" << endl;
  basisfile << "        <contractions factor=\"9.046910e-01\" type=\"S\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << endl;
  basisfile << "      <constant decay=\"1.831920e-01\">" << endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"S\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "  </element>" << endl;
  basisfile << "  <element name=\"C\">" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << endl;
  basisfile << "      <constant decay=\"1.722560e+02\">" << endl;
  basisfile << "        <contractions factor=\"6.176690e-02\" type=\"S\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"2.591090e+01\">" << endl;
  basisfile << "        <contractions factor=\"3.587940e-01\" type=\"S\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"5.533350e+00\">" << endl;
  basisfile << "        <contractions factor=\"7.007130e-01\" type=\"S\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"SP\">" << endl;
  basisfile << "      <constant decay=\"3.664980e+00\">" << endl;
  basisfile << "        <contractions factor=\"-3.958970e-01\" type=\"S\"/>"
            << endl;
  basisfile << "        <contractions factor=\"2.364600e-01\" type=\"P\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"7.705450e-01\">" << endl;
  basisfile << "        <contractions factor=\"1.215840e+00\" type=\"S\"/>"
            << endl;
  basisfile << "        <contractions factor=\"8.606190e-01\" type=\"P\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"SP\">" << endl;
  basisfile << "      <constant decay=\"1.958570e-01\">" << endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"S\"/>"
            << endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"P\"/>"
            << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "  </element>" << endl;
  basisfile << "</basis>" << endl;
  basisfile.close();

  Orbitals orbitals;
  orbitals.QMAtoms().LoadFromXYZ("molecule.xyz");
  BasisSet basis;
  basis.LoadBasisSet("3-21G.xml");

  AOBasis aobasis;
  aobasis.AOBasisFill(basis, orbitals.QMAtoms());

  Eigen::MatrixXd dmat = Eigen::MatrixXd::Zero(17, 17);
  dmat << 0.00157507, 0.0337454, 4.48905e-16, -5.93152e-16, 7.87133e-17,
      0.030876, 2.51254e-16, -1.49094e-16, 5.77899e-17, 0.00415998, -0.00445632,
      0.00415998, -0.00445632, 0.00415998, -0.00445632, 0.00415998, -0.00445632,
      0.0337454, 0.722983, 2.66427e-15, -4.44783e-15, 3.45846e-16, 0.661507,
      4.39854e-15, -2.02475e-15, 1.04832e-15, 0.0891262, -0.095475, 0.0891262,
      -0.095475, 0.0891262, -0.095475, 0.0891262, -0.095475, 4.48905e-16,
      2.66427e-15, 1.52199, 2.88658e-15, 2.09034e-15, -7.94212e-15, 0.215492,
      2.8727e-15, -1.40513e-15, 0.141933, -0.0402359, 0.141933, -0.0402359,
      -0.141933, 0.0402359, -0.141933, 0.0402359, -5.93152e-16, -4.44783e-15,
      2.88658e-15, 1.52199, -2.31759e-15, 9.21105e-15, -2.22045e-15, 0.215492,
      1.6263e-15, 0.141933, -0.0402359, -0.141933, 0.0402359, -0.141933,
      0.0402359, 0.141933, -0.0402359, 7.87133e-17, 3.45846e-16, 2.09034e-15,
      -2.31759e-15, 1.52199, 2.98902e-15, -2.04958e-15, 4.79738e-15, 0.215492,
      0.141933, -0.0402359, -0.141933, 0.0402359, 0.141933, -0.0402359,
      -0.141933, 0.0402359, 0.030876, 0.661507, -7.94212e-15, 9.21105e-15,
      2.98902e-15, 0.605259, 2.55488e-15, 2.7779e-17, 1.33759e-15, 0.0815477,
      -0.0873567, 0.0815477, -0.0873567, 0.0815477, -0.0873567, 0.0815477,
      -0.0873567, 2.51254e-16, 4.39854e-15, 0.215492, -2.22045e-15,
      -2.04958e-15, 2.55488e-15, 0.0305108, 3.29597e-17, -5.29036e-16,
      0.0200958, -0.00569686, 0.0200958, -0.00569686, -0.0200958, 0.00569686,
      -0.0200958, 0.00569686, -1.49094e-16, -2.02475e-15, 2.8727e-15, 0.215492,
      4.79738e-15, 2.7779e-17, 3.29597e-17, 0.0305108, 9.55941e-16, 0.0200958,
      -0.00569686, -0.0200958, 0.00569686, -0.0200958, 0.00569686, 0.0200958,
      -0.00569686, 5.77899e-17, 1.04832e-15, -1.40513e-15, 1.6263e-15, 0.215492,
      1.33759e-15, -5.29036e-16, 9.55941e-16, 0.0305108, 0.0200958, -0.00569686,
      -0.0200958, 0.00569686, 0.0200958, -0.00569686, -0.0200958, 0.00569686,
      0.00415998, 0.0891262, 0.141933, 0.141933, 0.141933, 0.0815477, 0.0200958,
      0.0200958, 0.0200958, 0.0506951, -0.0230264, -0.00224894, -0.00801753,
      -0.00224894, -0.00801753, -0.00224894, -0.00801753, -0.00445632,
      -0.095475, -0.0402359, -0.0402359, -0.0402359, -0.0873567, -0.00569686,
      -0.00569686, -0.00569686, -0.0230264, 0.0157992, -0.00801753, 0.0115445,
      -0.00801753, 0.0115445, -0.00801753, 0.0115445, 0.00415998, 0.0891262,
      0.141933, -0.141933, -0.141933, 0.0815477, 0.0200958, -0.0200958,
      -0.0200958, -0.00224894, -0.00801753, 0.0506951, -0.0230264, -0.00224894,
      -0.00801753, -0.00224894, -0.00801753, -0.00445632, -0.095475, -0.0402359,
      0.0402359, 0.0402359, -0.0873567, -0.00569686, 0.00569686, 0.00569686,
      -0.00801753, 0.0115445, -0.0230264, 0.0157992, -0.00801753, 0.0115445,
      -0.00801753, 0.0115445, 0.00415998, 0.0891262, -0.141933, -0.141933,
      0.141933, 0.0815477, -0.0200958, -0.0200958, 0.0200958, -0.00224894,
      -0.00801753, -0.00224894, -0.00801753, 0.0506951, -0.0230264, -0.00224894,
      -0.00801753, -0.00445632, -0.095475, 0.0402359, 0.0402359, -0.0402359,
      -0.0873567, 0.00569686, 0.00569686, -0.00569686, -0.00801753, 0.0115445,
      -0.00801753, 0.0115445, -0.0230264, 0.0157992, -0.00801753, 0.0115445,
      0.00415998, 0.0891262, -0.141933, 0.141933, -0.141933, 0.0815477,
      -0.0200958, 0.0200958, -0.0200958, -0.00224894, -0.00801753, -0.00224894,
      -0.00801753, -0.00224894, -0.00801753, 0.0506951, -0.0230264, -0.00445632,
      -0.095475, 0.0402359, -0.0402359, 0.0402359, -0.0873567, 0.00569686,
      -0.00569686, 0.00569686, -0.00801753, 0.0115445, -0.00801753, 0.0115445,
      -0.00801753, 0.0115445, -0.0230264, 0.0157992;

  ERIs eris1;
  ERIs eris2;
  eris1.Initialize_4c_screening(aobasis, 1e-10);
  eris2.Initialize_4c_small_molecule(aobasis);
  eris1.CalculateERIs_4c_direct(aobasis, dmat);
  eris2.CalculateERIs_4c_small_molecule(dmat);

  bool check_eris = eris1.getERIs().isApprox(eris2.getERIs(), 0.001);
  if (!check_eris) {
    std::cout << eris1.getERIs() << std::endl;
    std::cout << eris2.getERIs() << std::endl;
  }
  BOOST_CHECK_EQUAL(check_eris, 1);
}

BOOST_AUTO_TEST_SUITE_END()
