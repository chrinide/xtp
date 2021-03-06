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
 * distributed under the License is distributed on an "A_ol I_ol" BA_olI_ol,
 * WITHOUT WARRANTIE_ol OR CONDITION_ol OF ANY KIND, either express or implied.
 * _olee the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <votca/xtp/aomatrix.h>

#include <votca/xtp/aobasis.h>

#include <vector>

#include <votca/tools/constants.h>

namespace votca {
namespace xtp {

void AOQuadrupole_Potential::FillBlock(Eigen::Block<Eigen::MatrixXd>& matrix,
                                       const AOShell* shell_row,
                                       const AOShell* shell_col) {

  const double pi = boost::math::constants::pi<double>();

  std::vector<double> quadrupole = apolarsite->getQ2();
  tools::vec position = apolarsite->getPos() * tools::conv::nm2bohr;
  double nm22bohr2 = tools::conv::nm2bohr * tools::conv::nm2bohr;
  for (double& entry : quadrupole) {
    entry *= -nm22bohr2;
  }
  // I am not sure the order definition or anything is correct apolarsite object
  // orders them as Q20, Q21c, Q21s, Q22c, Q22s

  // q_01 etc are cartesian tensor multipole moments according to
  // https://en.wikipedia.org/wiki/Quadrupole so transform apolarsite into
  // cartesian and then multiply by 2 (difference stone definition/wiki
  // definition) not sure about unit conversion
  double q_00 = -quadrupole[0] + sqrt(3) * quadrupole[3];
  double q_01 = sqrt(3) * quadrupole[4];
  double q_02 = sqrt(3) * quadrupole[1];
  double q_12 = sqrt(3) * quadrupole[2];
  double q_11 =
      -quadrupole[0] -
      sqrt(3) * quadrupole[3];  // tensor is traceless, q_22 = - (q_00 + q_11)
  // shell info, only lmax tells how far to go
  int lmax_row = shell_row->getLmax();
  int lmax_col = shell_col->getLmax();
  int lsum = lmax_row + lmax_col;
  // set size of internal block for recursion
  int nrows = this->getBlockSize(lmax_row);
  int ncols = this->getBlockSize(lmax_col);

  // initialize local matrix block for unnormalized cartesians

  Eigen::MatrixXd quad = Eigen::MatrixXd::Zero(nrows, ncols);

  // cout << nuc.size1() << ":" << nuc.size2() << endl;

  /* FOR CONTRACTED FUNCTIONS, ADD LOOP OVER ALL DECAYS IN CONTRACTION
   * MULTIPLY THE TRANSFORMATION MATRICES BY APPROPRIATE CONTRACTION
   * COEFFICIENTS, AND ADD TO matrix(i,j)
   */

  int n_orbitals[] = {1, 4, 10, 20, 35, 56, 84};

  int nx[] = {0, 1, 0, 0, 2, 1, 1, 0, 0, 0, 3, 2, 2, 1, 1, 1, 0, 0,
              0, 0, 4, 3, 3, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0};

  int ny[] = {0, 0, 1, 0, 0, 1, 0, 2, 1, 0, 0, 1, 0, 2, 1, 0, 3, 2,
              1, 0, 0, 1, 0, 2, 1, 0, 3, 2, 1, 0, 4, 3, 2, 1, 0};

  int nz[] = {0, 0, 0, 1, 0, 0, 1, 0, 1, 2, 0, 0, 1, 0, 1, 2, 0, 1,
              2, 3, 0, 0, 1, 0, 1, 2, 0, 1, 2, 3, 0, 1, 2, 3, 4};

  int i_less_x[] = {0,  0,  0,  0,  1,  2,  3, 0, 0,  0,  4,  5,
                    6,  7,  8,  9,  0,  0,  0, 0, 10, 11, 12, 13,
                    14, 15, 16, 17, 18, 19, 0, 0, 0,  0,  0};

  int i_less_y[] = {0,  0, 0,  0,  0,  1, 0,  2,  3,  0,  0, 4,
                    0,  5, 6,  0,  7,  8, 9,  0,  0,  10, 0, 11,
                    12, 0, 13, 14, 15, 0, 16, 17, 18, 19, 0};

  int i_less_z[] = {0,  0,  0, 0,  0,  0,  1, 0,  2,  3,  0,  0,
                    4,  0,  5, 6,  0,  7,  8, 9,  0,  0,  10, 0,
                    11, 12, 0, 13, 14, 15, 0, 16, 17, 18, 19};

  // get shell positions
  const tools::vec& pos_row = shell_row->getPos();
  const tools::vec& pos_col = shell_col->getPos();
  const tools::vec diff = pos_row - pos_col;
  // initialize some helper

  double distsq = (diff * diff);

  // iterate over Gaussians in this shell_row
  for (AOShell::GaussianIterator itr = shell_row->begin();
       itr != shell_row->end(); ++itr) {
    // iterate over Gaussians in this shell_col
    // get decay constant
    const double decay_row = itr->getDecay();

    for (AOShell::GaussianIterator itc = shell_col->begin();
         itc != shell_col->end(); ++itc) {
      // get decay constant
      const double decay_col = itc->getDecay();

      const double zeta = decay_row + decay_col;
      const double fak = 0.5 / zeta;
      const double fak2 = 2.0 * fak;
      const double xi = decay_row * decay_col * fak2;

      double exparg = xi * distsq;
      // check if distance between postions is big, then skip step
      if (exparg > 30.0) {
        continue;
      }

      // some helpers
      double PmA0 =
          fak2 * (decay_row * pos_row.getX() + decay_col * pos_col.getX()) -
          pos_row.getX();
      double PmA1 =
          fak2 * (decay_row * pos_row.getY() + decay_col * pos_col.getY()) -
          pos_row.getY();
      double PmA2 =
          fak2 * (decay_row * pos_row.getZ() + decay_col * pos_col.getZ()) -
          pos_row.getZ();

      double PmB0 =
          fak2 * (decay_row * pos_row.getX() + decay_col * pos_col.getX()) -
          pos_col.getX();
      double PmB1 =
          fak2 * (decay_row * pos_row.getY() + decay_col * pos_col.getY()) -
          pos_col.getY();
      double PmB2 =
          fak2 * (decay_row * pos_row.getZ() + decay_col * pos_col.getZ()) -
          pos_col.getZ();

      double PmC0 =
          fak2 * (decay_row * pos_row.getX() + decay_col * pos_col.getX()) -
          position.getX();
      double PmC1 =
          fak2 * (decay_row * pos_row.getY() + decay_col * pos_col.getY()) -
          position.getY();
      double PmC2 =
          fak2 * (decay_row * pos_row.getZ() + decay_col * pos_col.getZ()) -
          position.getZ();

      const double U = zeta * (PmC0 * PmC0 + PmC1 * PmC1 + PmC2 * PmC2);

      // +3 quadrupole, +2 dipole, +1 nuclear attraction integrals
      const std::vector<double> FmU = XIntegrate(lsum + 3, U);

      typedef boost::multi_array<double, 3> ma_type;
      typedef boost::multi_array<double, 4> ma4_type;  //////////////////
      ma_type nuc3(boost::extents[nrows][ncols][lsum + 1]);
      ma4_type dip4(boost::extents[nrows][ncols][3][lsum + 1]);
      ma4_type quad4(boost::extents[nrows][ncols][5][lsum + 1]);
      typedef ma_type::index index;

      for (index i = 0; i < nrows; ++i) {
        for (index j = 0; j < ncols; ++j) {
          for (index m = 0; m < lsum + 1; ++m) {
            nuc3[i][j][m] = 0.;
          }
        }
      }

      for (index i = 0; i < nrows; ++i) {
        for (index j = 0; j < ncols; ++j) {
          for (index k = 0; k < 3; ++k) {
            for (index m = 0; m < lsum + 1; ++m) {
              dip4[i][j][k][m] = 0.;
            }
          }
        }
      }

      for (index i = 0; i < nrows; ++i) {
        for (index j = 0; j < ncols; ++j) {
          for (index k = 0; k < 5; ++k) {
            for (index m = 0; m < lsum + 1; ++m) {
              quad4[i][j][k][m] = 0.;
            }
          }
        }
      }

      // (s-s element normiert )
      double _prefactor = 4. * sqrt(2. / pi) * pow(decay_row * decay_col, .75) *
                          fak2 * exp(-exparg);
      for (int m = 0; m < lsum + 1; m++) {
        nuc3[0][0][m] = _prefactor * FmU[m];
      }
      //------------------------------------------------------

      // Integrals     p - s
      if (lmax_row > 0) {
        for (int m = 0; m < lsum; m++) {
          nuc3[Cart::x][0][m] = PmA0 * nuc3[0][0][m] - PmC0 * nuc3[0][0][m + 1];
          nuc3[Cart::y][0][m] = PmA1 * nuc3[0][0][m] - PmC1 * nuc3[0][0][m + 1];
          nuc3[Cart::z][0][m] = PmA2 * nuc3[0][0][m] - PmC2 * nuc3[0][0][m + 1];
        }
      }
      //------------------------------------------------------

      // Integrals     d - s
      if (lmax_row > 1) {
        for (int m = 0; m < lsum - 1; m++) {
          double term = fak * (nuc3[0][0][m] - nuc3[0][0][m + 1]);
          nuc3[Cart::xx][0][m] = PmA0 * nuc3[Cart::x][0][m] -
                                 PmC0 * nuc3[Cart::x][0][m + 1] + term;
          nuc3[Cart::xy][0][m] =
              PmA0 * nuc3[Cart::y][0][m] - PmC0 * nuc3[Cart::y][0][m + 1];
          nuc3[Cart::xz][0][m] =
              PmA0 * nuc3[Cart::z][0][m] - PmC0 * nuc3[Cart::z][0][m + 1];
          nuc3[Cart::yy][0][m] = PmA1 * nuc3[Cart::y][0][m] -
                                 PmC1 * nuc3[Cart::y][0][m + 1] + term;
          nuc3[Cart::yz][0][m] =
              PmA1 * nuc3[Cart::z][0][m] - PmC1 * nuc3[Cart::z][0][m + 1];
          nuc3[Cart::zz][0][m] = PmA2 * nuc3[Cart::z][0][m] -
                                 PmC2 * nuc3[Cart::z][0][m + 1] + term;
        }
      }
      //------------------------------------------------------

      // Integrals     f - s
      if (lmax_row > 2) {
        for (int m = 0; m < lsum - 2; m++) {
          nuc3[Cart::xxx][0][m] =
              PmA0 * nuc3[Cart::xx][0][m] - PmC0 * nuc3[Cart::xx][0][m + 1] +
              2 * fak * (nuc3[Cart::x][0][m] - nuc3[Cart::x][0][m + 1]);
          nuc3[Cart::xxy][0][m] =
              PmA1 * nuc3[Cart::xx][0][m] - PmC1 * nuc3[Cart::xx][0][m + 1];
          nuc3[Cart::xxz][0][m] =
              PmA2 * nuc3[Cart::xx][0][m] - PmC2 * nuc3[Cart::xx][0][m + 1];
          nuc3[Cart::xyy][0][m] =
              PmA0 * nuc3[Cart::yy][0][m] - PmC0 * nuc3[Cart::yy][0][m + 1];
          nuc3[Cart::xyz][0][m] =
              PmA0 * nuc3[Cart::yz][0][m] - PmC0 * nuc3[Cart::yz][0][m + 1];
          nuc3[Cart::xzz][0][m] =
              PmA0 * nuc3[Cart::zz][0][m] - PmC0 * nuc3[Cart::zz][0][m + 1];
          nuc3[Cart::yyy][0][m] =
              PmA1 * nuc3[Cart::yy][0][m] - PmC1 * nuc3[Cart::yy][0][m + 1] +
              2 * fak * (nuc3[Cart::y][0][m] - nuc3[Cart::y][0][m + 1]);
          nuc3[Cart::yyz][0][m] =
              PmA2 * nuc3[Cart::yy][0][m] - PmC2 * nuc3[Cart::yy][0][m + 1];
          nuc3[Cart::yzz][0][m] =
              PmA1 * nuc3[Cart::zz][0][m] - PmC1 * nuc3[Cart::zz][0][m + 1];
          nuc3[Cart::zzz][0][m] =
              PmA2 * nuc3[Cart::zz][0][m] - PmC2 * nuc3[Cart::zz][0][m + 1] +
              2 * fak * (nuc3[Cart::z][0][m] - nuc3[Cart::z][0][m + 1]);
        }
      }
      //------------------------------------------------------

      // Integrals     g - s
      if (lmax_row > 3) {
        for (int m = 0; m < lsum - 3; m++) {
          double term_xx =
              fak * (nuc3[Cart::xx][0][m] - nuc3[Cart::xx][0][m + 1]);
          double term_yy =
              fak * (nuc3[Cart::yy][0][m] - nuc3[Cart::yy][0][m + 1]);
          double term_zz =
              fak * (nuc3[Cart::zz][0][m] - nuc3[Cart::zz][0][m + 1]);
          nuc3[Cart::xxxx][0][m] = PmA0 * nuc3[Cart::xxx][0][m] -
                                   PmC0 * nuc3[Cart::xxx][0][m + 1] +
                                   3 * term_xx;
          nuc3[Cart::xxxy][0][m] =
              PmA1 * nuc3[Cart::xxx][0][m] - PmC1 * nuc3[Cart::xxx][0][m + 1];
          nuc3[Cart::xxxz][0][m] =
              PmA2 * nuc3[Cart::xxx][0][m] - PmC2 * nuc3[Cart::xxx][0][m + 1];
          nuc3[Cart::xxyy][0][m] = PmA0 * nuc3[Cart::xyy][0][m] -
                                   PmC0 * nuc3[Cart::xyy][0][m + 1] + term_yy;
          nuc3[Cart::xxyz][0][m] =
              PmA1 * nuc3[Cart::xxz][0][m] - PmC1 * nuc3[Cart::xxz][0][m + 1];
          nuc3[Cart::xxzz][0][m] = PmA0 * nuc3[Cart::xzz][0][m] -
                                   PmC0 * nuc3[Cart::xzz][0][m + 1] + term_zz;
          nuc3[Cart::xyyy][0][m] =
              PmA0 * nuc3[Cart::yyy][0][m] - PmC0 * nuc3[Cart::yyy][0][m + 1];
          nuc3[Cart::xyyz][0][m] =
              PmA0 * nuc3[Cart::yyz][0][m] - PmC0 * nuc3[Cart::yyz][0][m + 1];
          nuc3[Cart::xyzz][0][m] =
              PmA0 * nuc3[Cart::yzz][0][m] - PmC0 * nuc3[Cart::yzz][0][m + 1];
          nuc3[Cart::xzzz][0][m] =
              PmA0 * nuc3[Cart::zzz][0][m] - PmC0 * nuc3[Cart::zzz][0][m + 1];
          nuc3[Cart::yyyy][0][m] = PmA1 * nuc3[Cart::yyy][0][m] -
                                   PmC1 * nuc3[Cart::yyy][0][m + 1] +
                                   3 * term_yy;
          nuc3[Cart::yyyz][0][m] =
              PmA2 * nuc3[Cart::yyy][0][m] - PmC2 * nuc3[Cart::yyy][0][m + 1];
          nuc3[Cart::yyzz][0][m] = PmA1 * nuc3[Cart::yzz][0][m] -
                                   PmC1 * nuc3[Cart::yzz][0][m + 1] + term_zz;
          nuc3[Cart::yzzz][0][m] =
              PmA1 * nuc3[Cart::zzz][0][m] - PmC1 * nuc3[Cart::zzz][0][m + 1];
          nuc3[Cart::zzzz][0][m] = PmA2 * nuc3[Cart::zzz][0][m] -
                                   PmC2 * nuc3[Cart::zzz][0][m + 1] +
                                   3 * term_zz;
        }
      }
      //------------------------------------------------------

      if (lmax_col > 0) {

        // Integrals     s - p
        for (int m = 0; m < lmax_col; m++) {
          nuc3[0][Cart::x][m] = PmB0 * nuc3[0][0][m] - PmC0 * nuc3[0][0][m + 1];
          nuc3[0][Cart::y][m] = PmB1 * nuc3[0][0][m] - PmC1 * nuc3[0][0][m + 1];
          nuc3[0][Cart::z][m] = PmB2 * nuc3[0][0][m] - PmC2 * nuc3[0][0][m + 1];
        }
        //------------------------------------------------------

        // Integrals     p - p
        if (lmax_row > 0) {
          for (int m = 0; m < lmax_col; m++) {
            double term = fak * (nuc3[0][0][m] - nuc3[0][0][m + 1]);
            for (int i = 1; i < 4; i++) {
              nuc3[i][Cart::x][m] = PmB0 * nuc3[i][0][m] -
                                    PmC0 * nuc3[i][0][m + 1] + nx[i] * term;
              nuc3[i][Cart::y][m] = PmB1 * nuc3[i][0][m] -
                                    PmC1 * nuc3[i][0][m + 1] + ny[i] * term;
              nuc3[i][Cart::z][m] = PmB2 * nuc3[i][0][m] -
                                    PmC2 * nuc3[i][0][m + 1] + nz[i] * term;
            }
          }
        }
        //------------------------------------------------------

        // Integrals     d - p     f - p     g - p
        for (int m = 0; m < lmax_col; m++) {
          for (int i = 4; i < n_orbitals[lmax_row]; i++) {
            int nx_i = nx[i];
            int ny_i = ny[i];
            int nz_i = nz[i];
            int ilx_i = i_less_x[i];
            int ily_i = i_less_y[i];
            int ilz_i = i_less_z[i];
            nuc3[i][Cart::x][m] =
                PmB0 * nuc3[i][0][m] - PmC0 * nuc3[i][0][m + 1] +
                nx_i * fak * (nuc3[ilx_i][0][m] - nuc3[ilx_i][0][m + 1]);
            nuc3[i][Cart::y][m] =
                PmB1 * nuc3[i][0][m] - PmC1 * nuc3[i][0][m + 1] +
                ny_i * fak * (nuc3[ily_i][0][m] - nuc3[ily_i][0][m + 1]);
            nuc3[i][Cart::z][m] =
                PmB2 * nuc3[i][0][m] - PmC2 * nuc3[i][0][m + 1] +
                nz_i * fak * (nuc3[ilz_i][0][m] - nuc3[ilz_i][0][m + 1]);
          }
        }
        //------------------------------------------------------

      }  // end if (lmax_col > 0)

      if (lmax_col > 1) {

        // Integrals     s - d
        for (int m = 0; m < lmax_col - 1; m++) {
          double term = fak * (nuc3[0][0][m] - nuc3[0][0][m + 1]);
          nuc3[0][Cart::xx][m] = PmB0 * nuc3[0][Cart::x][m] -
                                 PmC0 * nuc3[0][Cart::x][m + 1] + term;
          nuc3[0][Cart::xy][m] =
              PmB0 * nuc3[0][Cart::y][m] - PmC0 * nuc3[0][Cart::y][m + 1];
          nuc3[0][Cart::xz][m] =
              PmB0 * nuc3[0][Cart::z][m] - PmC0 * nuc3[0][Cart::z][m + 1];
          nuc3[0][Cart::yy][m] = PmB1 * nuc3[0][Cart::y][m] -
                                 PmC1 * nuc3[0][Cart::y][m + 1] + term;
          nuc3[0][Cart::yz][m] =
              PmB1 * nuc3[0][Cart::z][m] - PmC1 * nuc3[0][Cart::z][m + 1];
          nuc3[0][Cart::zz][m] = PmB2 * nuc3[0][Cart::z][m] -
                                 PmC2 * nuc3[0][Cart::z][m + 1] + term;
        }
        //------------------------------------------------------

        // Integrals     p - d     d - d     f - d     g - d
        for (int m = 0; m < lmax_col - 1; m++) {
          for (int i = 1; i < n_orbitals[lmax_row]; i++) {
            int nx_i = nx[i];
            int ny_i = ny[i];
            int nz_i = nz[i];
            int ilx_i = i_less_x[i];
            int ily_i = i_less_y[i];
            int ilz_i = i_less_z[i];
            double term = fak * (nuc3[i][0][m] - nuc3[i][0][m + 1]);
            nuc3[i][Cart::xx][m] =
                PmB0 * nuc3[i][Cart::x][m] - PmC0 * nuc3[i][Cart::x][m + 1] +
                nx_i * fak *
                    (nuc3[ilx_i][Cart::x][m] - nuc3[ilx_i][Cart::x][m + 1]) +
                term;
            nuc3[i][Cart::xy][m] =
                PmB0 * nuc3[i][Cart::y][m] - PmC0 * nuc3[i][Cart::y][m + 1] +
                nx_i * fak *
                    (nuc3[ilx_i][Cart::y][m] - nuc3[ilx_i][Cart::y][m + 1]);
            nuc3[i][Cart::xz][m] =
                PmB0 * nuc3[i][Cart::z][m] - PmC0 * nuc3[i][Cart::z][m + 1] +
                nx_i * fak *
                    (nuc3[ilx_i][Cart::z][m] - nuc3[ilx_i][Cart::z][m + 1]);
            nuc3[i][Cart::yy][m] =
                PmB1 * nuc3[i][Cart::y][m] - PmC1 * nuc3[i][Cart::y][m + 1] +
                ny_i * fak *
                    (nuc3[ily_i][Cart::y][m] - nuc3[ily_i][Cart::y][m + 1]) +
                term;
            nuc3[i][Cart::yz][m] =
                PmB1 * nuc3[i][Cart::z][m] - PmC1 * nuc3[i][Cart::z][m + 1] +
                ny_i * fak *
                    (nuc3[ily_i][Cart::z][m] - nuc3[ily_i][Cart::z][m + 1]);
            nuc3[i][Cart::zz][m] =
                PmB2 * nuc3[i][Cart::z][m] - PmC2 * nuc3[i][Cart::z][m + 1] +
                nz_i * fak *
                    (nuc3[ilz_i][Cart::z][m] - nuc3[ilz_i][Cart::z][m + 1]) +
                term;
          }
        }
        //------------------------------------------------------

      }  // end if (lmax_col > 1)

      if (lmax_col > 2) {

        // Integrals     s - f
        for (int m = 0; m < lmax_col - 2; m++) {
          nuc3[0][Cart::xxx][m] =
              PmB0 * nuc3[0][Cart::xx][m] - PmC0 * nuc3[0][Cart::xx][m + 1] +
              2 * fak * (nuc3[0][Cart::x][m] - nuc3[0][Cart::x][m + 1]);
          nuc3[0][Cart::xxy][m] =
              PmB1 * nuc3[0][Cart::xx][m] - PmC1 * nuc3[0][Cart::xx][m + 1];
          nuc3[0][Cart::xxz][m] =
              PmB2 * nuc3[0][Cart::xx][m] - PmC2 * nuc3[0][Cart::xx][m + 1];
          nuc3[0][Cart::xyy][m] =
              PmB0 * nuc3[0][Cart::yy][m] - PmC0 * nuc3[0][Cart::yy][m + 1];
          nuc3[0][Cart::xyz][m] =
              PmB0 * nuc3[0][Cart::yz][m] - PmC0 * nuc3[0][Cart::yz][m + 1];
          nuc3[0][Cart::xzz][m] =
              PmB0 * nuc3[0][Cart::zz][m] - PmC0 * nuc3[0][Cart::zz][m + 1];
          nuc3[0][Cart::yyy][m] =
              PmB1 * nuc3[0][Cart::yy][m] - PmC1 * nuc3[0][Cart::yy][m + 1] +
              2 * fak * (nuc3[0][Cart::y][m] - nuc3[0][Cart::y][m + 1]);
          nuc3[0][Cart::yyz][m] =
              PmB2 * nuc3[0][Cart::yy][m] - PmC2 * nuc3[0][Cart::yy][m + 1];
          nuc3[0][Cart::yzz][m] =
              PmB1 * nuc3[0][Cart::zz][m] - PmC1 * nuc3[0][Cart::zz][m + 1];
          nuc3[0][Cart::zzz][m] =
              PmB2 * nuc3[0][Cart::zz][m] - PmC2 * nuc3[0][Cart::zz][m + 1] +
              2 * fak * (nuc3[0][Cart::z][m] - nuc3[0][Cart::z][m + 1]);
        }
        //------------------------------------------------------

        // Integrals     p - f     d - f     f - f     g - f
        for (int m = 0; m < lmax_col - 2; m++) {
          for (int i = 1; i < n_orbitals[lmax_row]; i++) {
            int nx_i = nx[i];
            int ny_i = ny[i];
            int nz_i = nz[i];
            int ilx_i = i_less_x[i];
            int ily_i = i_less_y[i];
            int ilz_i = i_less_z[i];
            double term_x =
                2 * fak * (nuc3[i][Cart::x][m] - nuc3[i][Cart::x][m + 1]);
            double term_y =
                2 * fak * (nuc3[i][Cart::y][m] - nuc3[i][Cart::y][m + 1]);
            double term_z =
                2 * fak * (nuc3[i][Cart::z][m] - nuc3[i][Cart::z][m + 1]);
            nuc3[i][Cart::xxx][m] =
                PmB0 * nuc3[i][Cart::xx][m] - PmC0 * nuc3[i][Cart::xx][m + 1] +
                nx_i * fak *
                    (nuc3[ilx_i][Cart::xx][m] - nuc3[ilx_i][Cart::xx][m + 1]) +
                term_x;
            nuc3[i][Cart::xxy][m] =
                PmB1 * nuc3[i][Cart::xx][m] - PmC1 * nuc3[i][Cart::xx][m + 1] +
                ny_i * fak *
                    (nuc3[ily_i][Cart::xx][m] - nuc3[ily_i][Cart::xx][m + 1]);
            nuc3[i][Cart::xxz][m] =
                PmB2 * nuc3[i][Cart::xx][m] - PmC2 * nuc3[i][Cart::xx][m + 1] +
                nz_i * fak *
                    (nuc3[ilz_i][Cart::xx][m] - nuc3[ilz_i][Cart::xx][m + 1]);
            nuc3[i][Cart::xyy][m] =
                PmB0 * nuc3[i][Cart::yy][m] - PmC0 * nuc3[i][Cart::yy][m + 1] +
                nx_i * fak *
                    (nuc3[ilx_i][Cart::yy][m] - nuc3[ilx_i][Cart::yy][m + 1]);
            nuc3[i][Cart::xyz][m] =
                PmB0 * nuc3[i][Cart::yz][m] - PmC0 * nuc3[i][Cart::yz][m + 1] +
                nx_i * fak *
                    (nuc3[ilx_i][Cart::yz][m] - nuc3[ilx_i][Cart::yz][m + 1]);
            nuc3[i][Cart::xzz][m] =
                PmB0 * nuc3[i][Cart::zz][m] - PmC0 * nuc3[i][Cart::zz][m + 1] +
                nx_i * fak *
                    (nuc3[ilx_i][Cart::zz][m] - nuc3[ilx_i][Cart::zz][m + 1]);
            nuc3[i][Cart::yyy][m] =
                PmB1 * nuc3[i][Cart::yy][m] - PmC1 * nuc3[i][Cart::yy][m + 1] +
                ny_i * fak *
                    (nuc3[ily_i][Cart::yy][m] - nuc3[ily_i][Cart::yy][m + 1]) +
                term_y;
            nuc3[i][Cart::yyz][m] =
                PmB2 * nuc3[i][Cart::yy][m] - PmC2 * nuc3[i][Cart::yy][m + 1] +
                nz_i * fak *
                    (nuc3[ilz_i][Cart::yy][m] - nuc3[ilz_i][Cart::yy][m + 1]);
            nuc3[i][Cart::yzz][m] =
                PmB1 * nuc3[i][Cart::zz][m] - PmC1 * nuc3[i][Cart::zz][m + 1] +
                ny_i * fak *
                    (nuc3[ily_i][Cart::zz][m] - nuc3[ily_i][Cart::zz][m + 1]);
            nuc3[i][Cart::zzz][m] =
                PmB2 * nuc3[i][Cart::zz][m] - PmC2 * nuc3[i][Cart::zz][m + 1] +
                nz_i * fak *
                    (nuc3[ilz_i][Cart::zz][m] - nuc3[ilz_i][Cart::zz][m + 1]) +
                term_z;
          }
        }
        //------------------------------------------------------

      }  // end if (lmax_col > 2)

      if (lmax_col > 3) {

        // Integrals     s - g
        for (int m = 0; m < lmax_col - 3; m++) {
          double term_xx =
              fak * (nuc3[0][Cart::xx][m] - nuc3[0][Cart::xx][m + 1]);
          double term_yy =
              fak * (nuc3[0][Cart::yy][m] - nuc3[0][Cart::yy][m + 1]);
          double term_zz =
              fak * (nuc3[0][Cart::zz][m] - nuc3[0][Cart::zz][m + 1]);
          nuc3[0][Cart::xxxx][m] = PmB0 * nuc3[0][Cart::xxx][m] -
                                   PmC0 * nuc3[0][Cart::xxx][m + 1] +
                                   3 * term_xx;
          nuc3[0][Cart::xxxy][m] =
              PmB1 * nuc3[0][Cart::xxx][m] - PmC1 * nuc3[0][Cart::xxx][m + 1];
          nuc3[0][Cart::xxxz][m] =
              PmB2 * nuc3[0][Cart::xxx][m] - PmC2 * nuc3[0][Cart::xxx][m + 1];
          nuc3[0][Cart::xxyy][m] = PmB0 * nuc3[0][Cart::xyy][m] -
                                   PmC0 * nuc3[0][Cart::xyy][m + 1] + term_yy;
          nuc3[0][Cart::xxyz][m] =
              PmB1 * nuc3[0][Cart::xxz][m] - PmC1 * nuc3[0][Cart::xxz][m + 1];
          nuc3[0][Cart::xxzz][m] = PmB0 * nuc3[0][Cart::xzz][m] -
                                   PmC0 * nuc3[0][Cart::xzz][m + 1] + term_zz;
          nuc3[0][Cart::xyyy][m] =
              PmB0 * nuc3[0][Cart::yyy][m] - PmC0 * nuc3[0][Cart::yyy][m + 1];
          nuc3[0][Cart::xyyz][m] =
              PmB0 * nuc3[0][Cart::yyz][m] - PmC0 * nuc3[0][Cart::yyz][m + 1];
          nuc3[0][Cart::xyzz][m] =
              PmB0 * nuc3[0][Cart::yzz][m] - PmC0 * nuc3[0][Cart::yzz][m + 1];
          nuc3[0][Cart::xzzz][m] =
              PmB0 * nuc3[0][Cart::zzz][m] - PmC0 * nuc3[0][Cart::zzz][m + 1];
          nuc3[0][Cart::yyyy][m] = PmB1 * nuc3[0][Cart::yyy][m] -
                                   PmC1 * nuc3[0][Cart::yyy][m + 1] +
                                   3 * term_yy;
          nuc3[0][Cart::yyyz][m] =
              PmB2 * nuc3[0][Cart::yyy][m] - PmC2 * nuc3[0][Cart::yyy][m + 1];
          nuc3[0][Cart::yyzz][m] = PmB1 * nuc3[0][Cart::yzz][m] -
                                   PmC1 * nuc3[0][Cart::yzz][m + 1] + term_zz;
          nuc3[0][Cart::yzzz][m] =
              PmB1 * nuc3[0][Cart::zzz][m] - PmC1 * nuc3[0][Cart::zzz][m + 1];
          nuc3[0][Cart::zzzz][m] = PmB2 * nuc3[0][Cart::zzz][m] -
                                   PmC2 * nuc3[0][Cart::zzz][m + 1] +
                                   3 * term_zz;
        }
        //------------------------------------------------------

        // Integrals     p - g     d - g     f - g     g - g
        for (int m = 0; m < lmax_col - 3; m++) {
          for (int i = 1; i < n_orbitals[lmax_row]; i++) {
            int nx_i = nx[i];
            int ny_i = ny[i];
            int nz_i = nz[i];
            int ilx_i = i_less_x[i];
            int ily_i = i_less_y[i];
            int ilz_i = i_less_z[i];
            double term_xx =
                fak * (nuc3[i][Cart::xx][m] - nuc3[i][Cart::xx][m + 1]);
            double term_yy =
                fak * (nuc3[i][Cart::yy][m] - nuc3[i][Cart::yy][m + 1]);
            double term_zz =
                fak * (nuc3[i][Cart::zz][m] - nuc3[i][Cart::zz][m + 1]);
            nuc3[i][Cart::xxxx][m] = PmB0 * nuc3[i][Cart::xxx][m] -
                                     PmC0 * nuc3[i][Cart::xxx][m + 1] +
                                     nx_i * fak *
                                         (nuc3[ilx_i][Cart::xxx][m] -
                                          nuc3[ilx_i][Cart::xxx][m + 1]) +
                                     3 * term_xx;
            nuc3[i][Cart::xxxy][m] =
                PmB1 * nuc3[i][Cart::xxx][m] -
                PmC1 * nuc3[i][Cart::xxx][m + 1] +
                ny_i * fak *
                    (nuc3[ily_i][Cart::xxx][m] - nuc3[ily_i][Cart::xxx][m + 1]);
            nuc3[i][Cart::xxxz][m] =
                PmB2 * nuc3[i][Cart::xxx][m] -
                PmC2 * nuc3[i][Cart::xxx][m + 1] +
                nz_i * fak *
                    (nuc3[ilz_i][Cart::xxx][m] - nuc3[ilz_i][Cart::xxx][m + 1]);
            nuc3[i][Cart::xxyy][m] = PmB0 * nuc3[i][Cart::xyy][m] -
                                     PmC0 * nuc3[i][Cart::xyy][m + 1] +
                                     nx_i * fak *
                                         (nuc3[ilx_i][Cart::xyy][m] -
                                          nuc3[ilx_i][Cart::xyy][m + 1]) +
                                     term_yy;
            nuc3[i][Cart::xxyz][m] =
                PmB1 * nuc3[i][Cart::xxz][m] -
                PmC1 * nuc3[i][Cart::xxz][m + 1] +
                ny_i * fak *
                    (nuc3[ily_i][Cart::xxz][m] - nuc3[ily_i][Cart::xxz][m + 1]);
            nuc3[i][Cart::xxzz][m] = PmB0 * nuc3[i][Cart::xzz][m] -
                                     PmC0 * nuc3[i][Cart::xzz][m + 1] +
                                     nx_i * fak *
                                         (nuc3[ilx_i][Cart::xzz][m] -
                                          nuc3[ilx_i][Cart::xzz][m + 1]) +
                                     term_zz;
            nuc3[i][Cart::xyyy][m] =
                PmB0 * nuc3[i][Cart::yyy][m] -
                PmC0 * nuc3[i][Cart::yyy][m + 1] +
                nx_i * fak *
                    (nuc3[ilx_i][Cart::yyy][m] - nuc3[ilx_i][Cart::yyy][m + 1]);
            nuc3[i][Cart::xyyz][m] =
                PmB0 * nuc3[i][Cart::yyz][m] -
                PmC0 * nuc3[i][Cart::yyz][m + 1] +
                nx_i * fak *
                    (nuc3[ilx_i][Cart::yyz][m] - nuc3[ilx_i][Cart::yyz][m + 1]);
            nuc3[i][Cart::xyzz][m] =
                PmB0 * nuc3[i][Cart::yzz][m] -
                PmC0 * nuc3[i][Cart::yzz][m + 1] +
                nx_i * fak *
                    (nuc3[ilx_i][Cart::yzz][m] - nuc3[ilx_i][Cart::yzz][m + 1]);
            nuc3[i][Cart::xzzz][m] =
                PmB0 * nuc3[i][Cart::zzz][m] -
                PmC0 * nuc3[i][Cart::zzz][m + 1] +
                nx_i * fak *
                    (nuc3[ilx_i][Cart::zzz][m] - nuc3[ilx_i][Cart::zzz][m + 1]);
            nuc3[i][Cart::yyyy][m] = PmB1 * nuc3[i][Cart::yyy][m] -
                                     PmC1 * nuc3[i][Cart::yyy][m + 1] +
                                     ny_i * fak *
                                         (nuc3[ily_i][Cart::yyy][m] -
                                          nuc3[ily_i][Cart::yyy][m + 1]) +
                                     3 * term_yy;
            nuc3[i][Cart::yyyz][m] =
                PmB2 * nuc3[i][Cart::yyy][m] -
                PmC2 * nuc3[i][Cart::yyy][m + 1] +
                nz_i * fak *
                    (nuc3[ilz_i][Cart::yyy][m] - nuc3[ilz_i][Cart::yyy][m + 1]);
            nuc3[i][Cart::yyzz][m] = PmB1 * nuc3[i][Cart::yzz][m] -
                                     PmC1 * nuc3[i][Cart::yzz][m + 1] +
                                     ny_i * fak *
                                         (nuc3[ily_i][Cart::yzz][m] -
                                          nuc3[ily_i][Cart::yzz][m + 1]) +
                                     term_zz;
            nuc3[i][Cart::yzzz][m] =
                PmB1 * nuc3[i][Cart::zzz][m] -
                PmC1 * nuc3[i][Cart::zzz][m + 1] +
                ny_i * fak *
                    (nuc3[ily_i][Cart::zzz][m] - nuc3[ily_i][Cart::zzz][m + 1]);
            nuc3[i][Cart::zzzz][m] = PmB2 * nuc3[i][Cart::zzz][m] -
                                     PmC2 * nuc3[i][Cart::zzz][m + 1] +
                                     nz_i * fak *
                                         (nuc3[ilz_i][Cart::zzz][m] -
                                          nuc3[ilz_i][Cart::zzz][m + 1]) +
                                     3 * term_zz;
          }
        }
        //------------------------------------------------------

      }  // end if (lmax_col > 3)

      // (s-s element normiert )
      double _prefactor_dip = 2. * zeta * _prefactor;
      for (int m = 0; m < lsum + 1; m++) {
        dip4[0][0][0][m] = PmC0 * _prefactor_dip * FmU[m + 1];
        dip4[0][0][1][m] = PmC1 * _prefactor_dip * FmU[m + 1];
        dip4[0][0][2][m] = PmC2 * _prefactor_dip * FmU[m + 1];
      }
      //------------------------------------------------------

      // Integrals     p - s
      if (lmax_row > 0) {
        for (int m = 0; m < lsum; m++) {
          for (int k = 0; k < 3; k++) {
            dip4[Cart::x][0][k][m] = PmA0 * dip4[0][0][k][m] -
                                     PmC0 * dip4[0][0][k][m + 1] +
                                     (k == 0) * nuc3[0][0][m + 1];
            dip4[Cart::y][0][k][m] = PmA1 * dip4[0][0][k][m] -
                                     PmC1 * dip4[0][0][k][m + 1] +
                                     (k == 1) * nuc3[0][0][m + 1];
            dip4[Cart::z][0][k][m] = PmA2 * dip4[0][0][k][m] -
                                     PmC2 * dip4[0][0][k][m + 1] +
                                     (k == 2) * nuc3[0][0][m + 1];
          }
        }
      }
      //------------------------------------------------------

      // Integrals     d - s
      if (lmax_row > 1) {
        for (int m = 0; m < lsum - 1; m++) {
          for (int k = 0; k < 3; k++) {
            double term = fak * (dip4[0][0][k][m] - dip4[0][0][k][m + 1]);
            dip4[Cart::xx][0][k][m] = PmA0 * dip4[Cart::x][0][k][m] -
                                      PmC0 * dip4[Cart::x][0][k][m + 1] +
                                      (k == 0) * nuc3[Cart::x][0][m + 1] + term;
            dip4[Cart::xy][0][k][m] = PmA0 * dip4[Cart::y][0][k][m] -
                                      PmC0 * dip4[Cart::y][0][k][m + 1] +
                                      (k == 0) * nuc3[Cart::y][0][m + 1];
            dip4[Cart::xz][0][k][m] = PmA0 * dip4[Cart::z][0][k][m] -
                                      PmC0 * dip4[Cart::z][0][k][m + 1] +
                                      (k == 0) * nuc3[Cart::z][0][m + 1];
            dip4[Cart::yy][0][k][m] = PmA1 * dip4[Cart::y][0][k][m] -
                                      PmC1 * dip4[Cart::y][0][k][m + 1] +
                                      (k == 1) * nuc3[Cart::y][0][m + 1] + term;
            dip4[Cart::yz][0][k][m] = PmA1 * dip4[Cart::z][0][k][m] -
                                      PmC1 * dip4[Cart::z][0][k][m + 1] +
                                      (k == 1) * nuc3[Cart::z][0][m + 1];
            dip4[Cart::zz][0][k][m] = PmA2 * dip4[Cart::z][0][k][m] -
                                      PmC2 * dip4[Cart::z][0][k][m + 1] +
                                      (k == 2) * nuc3[Cart::z][0][m + 1] + term;
          }
        }
      }
      //------------------------------------------------------

      // Integrals     f - s
      if (lmax_row > 2) {
        for (int m = 0; m < lsum - 2; m++) {
          for (int k = 0; k < 3; k++) {
            dip4[Cart::xxx][0][k][m] =
                PmA0 * dip4[Cart::xx][0][k][m] -
                PmC0 * dip4[Cart::xx][0][k][m + 1] +
                (k == 0) * nuc3[Cart::xx][0][m + 1] +
                2 * fak * (dip4[Cart::x][0][k][m] - dip4[Cart::x][0][k][m + 1]);
            dip4[Cart::xxy][0][k][m] = PmA1 * dip4[Cart::xx][0][k][m] -
                                       PmC1 * dip4[Cart::xx][0][k][m + 1] +
                                       (k == 1) * nuc3[Cart::xx][0][m + 1];
            dip4[Cart::xxz][0][k][m] = PmA2 * dip4[Cart::xx][0][k][m] -
                                       PmC2 * dip4[Cart::xx][0][k][m + 1] +
                                       (k == 2) * nuc3[Cart::xx][0][m + 1];
            dip4[Cart::xyy][0][k][m] = PmA0 * dip4[Cart::yy][0][k][m] -
                                       PmC0 * dip4[Cart::yy][0][k][m + 1] +
                                       (k == 0) * nuc3[Cart::yy][0][m + 1];
            dip4[Cart::xyz][0][k][m] = PmA0 * dip4[Cart::yz][0][k][m] -
                                       PmC0 * dip4[Cart::yz][0][k][m + 1] +
                                       (k == 0) * nuc3[Cart::yz][0][m + 1];
            dip4[Cart::xzz][0][k][m] = PmA0 * dip4[Cart::zz][0][k][m] -
                                       PmC0 * dip4[Cart::zz][0][k][m + 1] +
                                       (k == 0) * nuc3[Cart::zz][0][m + 1];
            dip4[Cart::yyy][0][k][m] =
                PmA1 * dip4[Cart::yy][0][k][m] -
                PmC1 * dip4[Cart::yy][0][k][m + 1] +
                (k == 1) * nuc3[Cart::yy][0][m + 1] +
                2 * fak * (dip4[Cart::y][0][k][m] - dip4[Cart::y][0][k][m + 1]);
            dip4[Cart::yyz][0][k][m] = PmA2 * dip4[Cart::yy][0][k][m] -
                                       PmC2 * dip4[Cart::yy][0][k][m + 1] +
                                       (k == 2) * nuc3[Cart::yy][0][m + 1];
            dip4[Cart::yzz][0][k][m] = PmA1 * dip4[Cart::zz][0][k][m] -
                                       PmC1 * dip4[Cart::zz][0][k][m + 1] +
                                       (k == 1) * nuc3[Cart::zz][0][m + 1];
            dip4[Cart::zzz][0][k][m] =
                PmA2 * dip4[Cart::zz][0][k][m] -
                PmC2 * dip4[Cart::zz][0][k][m + 1] +
                (k == 2) * nuc3[Cart::zz][0][m + 1] +
                2 * fak * (dip4[Cart::z][0][k][m] - dip4[Cart::z][0][k][m + 1]);
          }
        }
      }
      //------------------------------------------------------

      // Integrals     g - s
      if (lmax_row > 3) {
        for (int m = 0; m < lsum - 3; m++) {
          for (int k = 0; k < 3; k++) {
            double term_xx =
                fak * (dip4[Cart::xx][0][k][m] - dip4[Cart::xx][0][k][m + 1]);
            double term_yy =
                fak * (dip4[Cart::yy][0][k][m] - dip4[Cart::yy][0][k][m + 1]);
            double term_zz =
                fak * (dip4[Cart::zz][0][k][m] - dip4[Cart::zz][0][k][m + 1]);
            dip4[Cart::xxxx][0][k][m] = PmA0 * dip4[Cart::xxx][0][k][m] -
                                        PmC0 * dip4[Cart::xxx][0][k][m + 1] +
                                        (k == 0) * nuc3[Cart::xxx][0][m + 1] +
                                        3 * term_xx;
            dip4[Cart::xxxy][0][k][m] = PmA1 * dip4[Cart::xxx][0][k][m] -
                                        PmC1 * dip4[Cart::xxx][0][k][m + 1] +
                                        (k == 1) * nuc3[Cart::xxx][0][m + 1];
            dip4[Cart::xxxz][0][k][m] = PmA2 * dip4[Cart::xxx][0][k][m] -
                                        PmC2 * dip4[Cart::xxx][0][k][m + 1] +
                                        (k == 2) * nuc3[Cart::xxx][0][m + 1];
            dip4[Cart::xxyy][0][k][m] = PmA0 * dip4[Cart::xyy][0][k][m] -
                                        PmC0 * dip4[Cart::xyy][0][k][m + 1] +
                                        (k == 0) * nuc3[Cart::xyy][0][m + 1] +
                                        term_yy;
            dip4[Cart::xxyz][0][k][m] = PmA1 * dip4[Cart::xxz][0][k][m] -
                                        PmC1 * dip4[Cart::xxz][0][k][m + 1] +
                                        (k == 1) * nuc3[Cart::xxz][0][m + 1];
            dip4[Cart::xxzz][0][k][m] = PmA0 * dip4[Cart::xzz][0][k][m] -
                                        PmC0 * dip4[Cart::xzz][0][k][m + 1] +
                                        (k == 0) * nuc3[Cart::xzz][0][m + 1] +
                                        term_zz;
            dip4[Cart::xyyy][0][k][m] = PmA0 * dip4[Cart::yyy][0][k][m] -
                                        PmC0 * dip4[Cart::yyy][0][k][m + 1] +
                                        (k == 0) * nuc3[Cart::yyy][0][m + 1];
            dip4[Cart::xyyz][0][k][m] = PmA0 * dip4[Cart::yyz][0][k][m] -
                                        PmC0 * dip4[Cart::yyz][0][k][m + 1] +
                                        (k == 0) * nuc3[Cart::yyz][0][m + 1];
            dip4[Cart::xyzz][0][k][m] = PmA0 * dip4[Cart::yzz][0][k][m] -
                                        PmC0 * dip4[Cart::yzz][0][k][m + 1] +
                                        (k == 0) * nuc3[Cart::yzz][0][m + 1];
            dip4[Cart::xzzz][0][k][m] = PmA0 * dip4[Cart::zzz][0][k][m] -
                                        PmC0 * dip4[Cart::zzz][0][k][m + 1] +
                                        (k == 0) * nuc3[Cart::zzz][0][m + 1];
            dip4[Cart::yyyy][0][k][m] = PmA1 * dip4[Cart::yyy][0][k][m] -
                                        PmC1 * dip4[Cart::yyy][0][k][m + 1] +
                                        (k == 1) * nuc3[Cart::yyy][0][m + 1] +
                                        3 * term_yy;
            dip4[Cart::yyyz][0][k][m] = PmA2 * dip4[Cart::yyy][0][k][m] -
                                        PmC2 * dip4[Cart::yyy][0][k][m + 1] +
                                        (k == 2) * nuc3[Cart::yyy][0][m + 1];
            dip4[Cart::yyzz][0][k][m] = PmA1 * dip4[Cart::yzz][0][k][m] -
                                        PmC1 * dip4[Cart::yzz][0][k][m + 1] +
                                        (k == 1) * nuc3[Cart::yzz][0][m + 1] +
                                        term_zz;
            dip4[Cart::yzzz][0][k][m] = PmA1 * dip4[Cart::zzz][0][k][m] -
                                        PmC1 * dip4[Cart::zzz][0][k][m + 1] +
                                        (k == 1) * nuc3[Cart::zzz][0][m + 1];
            dip4[Cart::zzzz][0][k][m] = PmA2 * dip4[Cart::zzz][0][k][m] -
                                        PmC2 * dip4[Cart::zzz][0][k][m + 1] +
                                        (k == 2) * nuc3[Cart::zzz][0][m + 1] +
                                        3 * term_zz;
          }
        }
      }
      //------------------------------------------------------

      if (lmax_col > 0) {

        // Integrals     s - p
        for (int m = 0; m < lmax_col; m++) {
          for (int k = 0; k < 3; k++) {
            dip4[0][Cart::x][k][m] = PmB0 * dip4[0][0][k][m] -
                                     PmC0 * dip4[0][0][k][m + 1] +
                                     (k == 0) * nuc3[0][0][m + 1];
            dip4[0][Cart::y][k][m] = PmB1 * dip4[0][0][k][m] -
                                     PmC1 * dip4[0][0][k][m + 1] +
                                     (k == 1) * nuc3[0][0][m + 1];
            dip4[0][Cart::z][k][m] = PmB2 * dip4[0][0][k][m] -
                                     PmC2 * dip4[0][0][k][m + 1] +
                                     (k == 2) * nuc3[0][0][m + 1];
          }
        }
        //------------------------------------------------------

        // Integrals     p - p
        if (lmax_row > 0) {
          for (int m = 0; m < lmax_col; m++) {
            for (int i = 1; i < 4; i++) {
              for (int k = 0; k < 3; k++) {
                double term = fak * (dip4[0][0][k][m] - dip4[0][0][k][m + 1]);
                dip4[i][Cart::x][k][m] =
                    PmB0 * dip4[i][0][k][m] - PmC0 * dip4[i][0][k][m + 1] +
                    (k == 0) * nuc3[i][0][m + 1] + nx[i] * term;
                dip4[i][Cart::y][k][m] =
                    PmB1 * dip4[i][0][k][m] - PmC1 * dip4[i][0][k][m + 1] +
                    (k == 1) * nuc3[i][0][m + 1] + ny[i] * term;
                dip4[i][Cart::z][k][m] =
                    PmB2 * dip4[i][0][k][m] - PmC2 * dip4[i][0][k][m + 1] +
                    (k == 2) * nuc3[i][0][m + 1] + nz[i] * term;
              }
            }
          }
        }
        //------------------------------------------------------

        // Integrals     d - p     f - p     g - p
        for (int m = 0; m < lmax_col; m++) {
          for (int i = 4; i < n_orbitals[lmax_row]; i++) {
            int nx_i = nx[i];
            int ny_i = ny[i];
            int nz_i = nz[i];
            int ilx_i = i_less_x[i];
            int ily_i = i_less_y[i];
            int ilz_i = i_less_z[i];
            for (int k = 0; k < 3; k++) {
              dip4[i][Cart::x][k][m] =
                  PmB0 * dip4[i][0][k][m] - PmC0 * dip4[i][0][k][m + 1] +
                  (k == 0) * nuc3[i][0][m + 1] +
                  nx_i * fak *
                      (dip4[ilx_i][0][k][m] - dip4[ilx_i][0][k][m + 1]);
              dip4[i][Cart::y][k][m] =
                  PmB1 * dip4[i][0][k][m] - PmC1 * dip4[i][0][k][m + 1] +
                  (k == 1) * nuc3[i][0][m + 1] +
                  ny_i * fak *
                      (dip4[ily_i][0][k][m] - dip4[ily_i][0][k][m + 1]);
              dip4[i][Cart::z][k][m] =
                  PmB2 * dip4[i][0][k][m] - PmC2 * dip4[i][0][k][m + 1] +
                  (k == 2) * nuc3[i][0][m + 1] +
                  nz_i * fak *
                      (dip4[ilz_i][0][k][m] - dip4[ilz_i][0][k][m + 1]);
            }
          }
        }
        //------------------------------------------------------

      }  // end if (lmax_col > 0)

      if (lmax_col > 1) {

        // Integrals     s - d
        for (int m = 0; m < lmax_col - 1; m++) {
          for (int k = 0; k < 3; k++) {
            double term = fak * (dip4[0][0][k][m] - dip4[0][0][k][m + 1]);
            dip4[0][Cart::xx][k][m] = PmB0 * dip4[0][Cart::x][k][m] -
                                      PmC0 * dip4[0][Cart::x][k][m + 1] +
                                      (k == 0) * nuc3[0][Cart::x][m + 1] + term;
            dip4[0][Cart::xy][k][m] = PmB0 * dip4[0][Cart::y][k][m] -
                                      PmC0 * dip4[0][Cart::y][k][m + 1] +
                                      (k == 0) * nuc3[0][Cart::y][m + 1];
            dip4[0][Cart::xz][k][m] = PmB0 * dip4[0][Cart::z][k][m] -
                                      PmC0 * dip4[0][Cart::z][k][m + 1] +
                                      (k == 0) * nuc3[0][Cart::z][m + 1];
            dip4[0][Cart::yy][k][m] = PmB1 * dip4[0][Cart::y][k][m] -
                                      PmC1 * dip4[0][Cart::y][k][m + 1] +
                                      (k == 1) * nuc3[0][Cart::y][m + 1] + term;
            dip4[0][Cart::yz][k][m] = PmB1 * dip4[0][Cart::z][k][m] -
                                      PmC1 * dip4[0][Cart::z][k][m + 1] +
                                      (k == 1) * nuc3[0][Cart::z][m + 1];
            dip4[0][Cart::zz][k][m] = PmB2 * dip4[0][Cart::z][k][m] -
                                      PmC2 * dip4[0][Cart::z][k][m + 1] +
                                      (k == 2) * nuc3[0][Cart::z][m + 1] + term;
          }
        }
        //------------------------------------------------------

        // Integrals     p - d     d - d     f - d     g - d
        for (int m = 0; m < lmax_col - 1; m++) {
          for (int i = 1; i < n_orbitals[lmax_row]; i++) {
            int nx_i = nx[i];
            int ny_i = ny[i];
            int nz_i = nz[i];
            int ilx_i = i_less_x[i];
            int ily_i = i_less_y[i];
            int ilz_i = i_less_z[i];
            for (int k = 0; k < 3; k++) {
              double term = fak * (dip4[i][0][k][m] - dip4[i][0][k][m + 1]);
              dip4[i][Cart::xx][k][m] = PmB0 * dip4[i][Cart::x][k][m] -
                                        PmC0 * dip4[i][Cart::x][k][m + 1] +
                                        (k == 0) * nuc3[i][Cart::x][m + 1] +
                                        nx_i * fak *
                                            (dip4[ilx_i][Cart::x][k][m] -
                                             dip4[ilx_i][Cart::x][k][m + 1]) +
                                        term;
              dip4[i][Cart::xy][k][m] = PmB0 * dip4[i][Cart::y][k][m] -
                                        PmC0 * dip4[i][Cart::y][k][m + 1] +
                                        (k == 0) * nuc3[i][Cart::y][m + 1] +
                                        nx_i * fak *
                                            (dip4[ilx_i][Cart::y][k][m] -
                                             dip4[ilx_i][Cart::y][k][m + 1]);
              dip4[i][Cart::xz][k][m] = PmB0 * dip4[i][Cart::z][k][m] -
                                        PmC0 * dip4[i][Cart::z][k][m + 1] +
                                        (k == 0) * nuc3[i][Cart::z][m + 1] +
                                        nx_i * fak *
                                            (dip4[ilx_i][Cart::z][k][m] -
                                             dip4[ilx_i][Cart::z][k][m + 1]);
              dip4[i][Cart::yy][k][m] = PmB1 * dip4[i][Cart::y][k][m] -
                                        PmC1 * dip4[i][Cart::y][k][m + 1] +
                                        (k == 1) * nuc3[i][Cart::y][m + 1] +
                                        ny_i * fak *
                                            (dip4[ily_i][Cart::y][k][m] -
                                             dip4[ily_i][Cart::y][k][m + 1]) +
                                        term;
              dip4[i][Cart::yz][k][m] = PmB1 * dip4[i][Cart::z][k][m] -
                                        PmC1 * dip4[i][Cart::z][k][m + 1] +
                                        (k == 1) * nuc3[i][Cart::z][m + 1] +
                                        ny_i * fak *
                                            (dip4[ily_i][Cart::z][k][m] -
                                             dip4[ily_i][Cart::z][k][m + 1]);
              dip4[i][Cart::zz][k][m] = PmB2 * dip4[i][Cart::z][k][m] -
                                        PmC2 * dip4[i][Cart::z][k][m + 1] +
                                        (k == 2) * nuc3[i][Cart::z][m + 1] +
                                        nz_i * fak *
                                            (dip4[ilz_i][Cart::z][k][m] -
                                             dip4[ilz_i][Cart::z][k][m + 1]) +
                                        term;
            }
          }
        }
        //------------------------------------------------------

      }  // end if (lmax_col > 1)

      if (lmax_col > 2) {

        // Integrals     s - f
        for (int m = 0; m < lmax_col - 2; m++) {
          for (int k = 0; k < 3; k++) {
            dip4[0][Cart::xxx][k][m] =
                PmB0 * dip4[0][Cart::xx][k][m] -
                PmC0 * dip4[0][Cart::xx][k][m + 1] +
                (k == 0) * nuc3[0][Cart::xx][m + 1] +
                2 * fak * (dip4[0][Cart::x][k][m] - dip4[0][Cart::x][k][m + 1]);
            dip4[0][Cart::xxy][k][m] = PmB1 * dip4[0][Cart::xx][k][m] -
                                       PmC1 * dip4[0][Cart::xx][k][m + 1] +
                                       (k == 1) * nuc3[0][Cart::xx][m + 1];
            dip4[0][Cart::xxz][k][m] = PmB2 * dip4[0][Cart::xx][k][m] -
                                       PmC2 * dip4[0][Cart::xx][k][m + 1] +
                                       (k == 2) * nuc3[0][Cart::xx][m + 1];
            dip4[0][Cart::xyy][k][m] = PmB0 * dip4[0][Cart::yy][k][m] -
                                       PmC0 * dip4[0][Cart::yy][k][m + 1] +
                                       (k == 0) * nuc3[0][Cart::yy][m + 1];
            dip4[0][Cart::xyz][k][m] = PmB0 * dip4[0][Cart::yz][k][m] -
                                       PmC0 * dip4[0][Cart::yz][k][m + 1] +
                                       (k == 0) * nuc3[0][Cart::yz][m + 1];
            dip4[0][Cart::xzz][k][m] = PmB0 * dip4[0][Cart::zz][k][m] -
                                       PmC0 * dip4[0][Cart::zz][k][m + 1] +
                                       (k == 0) * nuc3[0][Cart::zz][m + 1];
            dip4[0][Cart::yyy][k][m] =
                PmB1 * dip4[0][Cart::yy][k][m] -
                PmC1 * dip4[0][Cart::yy][k][m + 1] +
                (k == 1) * nuc3[0][Cart::yy][m + 1] +
                2 * fak * (dip4[0][Cart::y][k][m] - dip4[0][Cart::y][k][m + 1]);
            dip4[0][Cart::yyz][k][m] = PmB2 * dip4[0][Cart::yy][k][m] -
                                       PmC2 * dip4[0][Cart::yy][k][m + 1] +
                                       (k == 2) * nuc3[0][Cart::yy][m + 1];
            dip4[0][Cart::yzz][k][m] = PmB1 * dip4[0][Cart::zz][k][m] -
                                       PmC1 * dip4[0][Cart::zz][k][m + 1] +
                                       (k == 1) * nuc3[0][Cart::zz][m + 1];
            dip4[0][Cart::zzz][k][m] =
                PmB2 * dip4[0][Cart::zz][k][m] -
                PmC2 * dip4[0][Cart::zz][k][m + 1] +
                (k == 2) * nuc3[0][Cart::zz][m + 1] +
                2 * fak * (dip4[0][Cart::z][k][m] - dip4[0][Cart::z][k][m + 1]);
          }
        }
        //------------------------------------------------------

        // Integrals     p - f     d - f     f - f     g - f
        for (int m = 0; m < lmax_col - 2; m++) {
          for (int i = 1; i < n_orbitals[lmax_row]; i++) {
            int nx_i = nx[i];
            int ny_i = ny[i];
            int nz_i = nz[i];
            int ilx_i = i_less_x[i];
            int ily_i = i_less_y[i];
            int ilz_i = i_less_z[i];
            for (int k = 0; k < 3; k++) {
              double term_x =
                  2 * fak *
                  (dip4[i][Cart::x][k][m] - dip4[i][Cart::x][k][m + 1]);
              double term_y =
                  2 * fak *
                  (dip4[i][Cart::y][k][m] - dip4[i][Cart::y][k][m + 1]);
              double term_z =
                  2 * fak *
                  (dip4[i][Cart::z][k][m] - dip4[i][Cart::z][k][m + 1]);
              dip4[i][Cart::xxx][k][m] = PmB0 * dip4[i][Cart::xx][k][m] -
                                         PmC0 * dip4[i][Cart::xx][k][m + 1] +
                                         (k == 0) * nuc3[i][Cart::xx][m + 1] +
                                         nx_i * fak *
                                             (dip4[ilx_i][Cart::xx][k][m] -
                                              dip4[ilx_i][Cart::xx][k][m + 1]) +
                                         term_x;
              dip4[i][Cart::xxy][k][m] = PmB1 * dip4[i][Cart::xx][k][m] -
                                         PmC1 * dip4[i][Cart::xx][k][m + 1] +
                                         (k == 1) * nuc3[i][Cart::xx][m + 1] +
                                         ny_i * fak *
                                             (dip4[ily_i][Cart::xx][k][m] -
                                              dip4[ily_i][Cart::xx][k][m + 1]);
              dip4[i][Cart::xxz][k][m] = PmB2 * dip4[i][Cart::xx][k][m] -
                                         PmC2 * dip4[i][Cart::xx][k][m + 1] +
                                         (k == 2) * nuc3[i][Cart::xx][m + 1] +
                                         nz_i * fak *
                                             (dip4[ilz_i][Cart::xx][k][m] -
                                              dip4[ilz_i][Cart::xx][k][m + 1]);
              dip4[i][Cart::xyy][k][m] = PmB0 * dip4[i][Cart::yy][k][m] -
                                         PmC0 * dip4[i][Cart::yy][k][m + 1] +
                                         (k == 0) * nuc3[i][Cart::yy][m + 1] +
                                         nx_i * fak *
                                             (dip4[ilx_i][Cart::yy][k][m] -
                                              dip4[ilx_i][Cart::yy][k][m + 1]);
              dip4[i][Cart::xyz][k][m] = PmB0 * dip4[i][Cart::yz][k][m] -
                                         PmC0 * dip4[i][Cart::yz][k][m + 1] +
                                         (k == 0) * nuc3[i][Cart::yz][m + 1] +
                                         nx_i * fak *
                                             (dip4[ilx_i][Cart::yz][k][m] -
                                              dip4[ilx_i][Cart::yz][k][m + 1]);
              dip4[i][Cart::xzz][k][m] = PmB0 * dip4[i][Cart::zz][k][m] -
                                         PmC0 * dip4[i][Cart::zz][k][m + 1] +
                                         (k == 0) * nuc3[i][Cart::zz][m + 1] +
                                         nx_i * fak *
                                             (dip4[ilx_i][Cart::zz][k][m] -
                                              dip4[ilx_i][Cart::zz][k][m + 1]);
              dip4[i][Cart::yyy][k][m] = PmB1 * dip4[i][Cart::yy][k][m] -
                                         PmC1 * dip4[i][Cart::yy][k][m + 1] +
                                         (k == 1) * nuc3[i][Cart::yy][m + 1] +
                                         ny_i * fak *
                                             (dip4[ily_i][Cart::yy][k][m] -
                                              dip4[ily_i][Cart::yy][k][m + 1]) +
                                         term_y;
              dip4[i][Cart::yyz][k][m] = PmB2 * dip4[i][Cart::yy][k][m] -
                                         PmC2 * dip4[i][Cart::yy][k][m + 1] +
                                         (k == 2) * nuc3[i][Cart::yy][m + 1] +
                                         nz_i * fak *
                                             (dip4[ilz_i][Cart::yy][k][m] -
                                              dip4[ilz_i][Cart::yy][k][m + 1]);
              dip4[i][Cart::yzz][k][m] = PmB1 * dip4[i][Cart::zz][k][m] -
                                         PmC1 * dip4[i][Cart::zz][k][m + 1] +
                                         (k == 1) * nuc3[i][Cart::zz][m + 1] +
                                         ny_i * fak *
                                             (dip4[ily_i][Cart::zz][k][m] -
                                              dip4[ily_i][Cart::zz][k][m + 1]);
              dip4[i][Cart::zzz][k][m] = PmB2 * dip4[i][Cart::zz][k][m] -
                                         PmC2 * dip4[i][Cart::zz][k][m + 1] +
                                         (k == 2) * nuc3[i][Cart::zz][m + 1] +
                                         nz_i * fak *
                                             (dip4[ilz_i][Cart::zz][k][m] -
                                              dip4[ilz_i][Cart::zz][k][m + 1]) +
                                         term_z;
            }
          }
        }
        //------------------------------------------------------

      }  // end if (lmax_col > 2)

      if (lmax_col > 3) {

        // Integrals     s - g
        for (int m = 0; m < lmax_col - 3; m++) {
          for (int k = 0; k < 3; k++) {
            double term_xx =
                fak * (dip4[0][Cart::xx][k][m] - dip4[0][Cart::xx][k][m + 1]);
            double term_yy =
                fak * (dip4[0][Cart::yy][k][m] - dip4[0][Cart::yy][k][m + 1]);
            double term_zz =
                fak * (dip4[0][Cart::zz][k][m] - dip4[0][Cart::zz][k][m + 1]);
            dip4[0][Cart::xxxx][k][m] = PmB0 * dip4[0][Cart::xxx][k][m] -
                                        PmC0 * dip4[0][Cart::xxx][k][m + 1] +
                                        (k == 0) * nuc3[0][Cart::xxx][m + 1] +
                                        3 * term_xx;
            dip4[0][Cart::xxxy][k][m] = PmB1 * dip4[0][Cart::xxx][k][m] -
                                        PmC1 * dip4[0][Cart::xxx][k][m + 1] +
                                        (k == 1) * nuc3[0][Cart::xxx][m + 1];
            dip4[0][Cart::xxxz][k][m] = PmB2 * dip4[0][Cart::xxx][k][m] -
                                        PmC2 * dip4[0][Cart::xxx][k][m + 1] +
                                        (k == 2) * nuc3[0][Cart::xxx][m + 1];
            dip4[0][Cart::xxyy][k][m] = PmB0 * dip4[0][Cart::xyy][k][m] -
                                        PmC0 * dip4[0][Cart::xyy][k][m + 1] +
                                        (k == 0) * nuc3[0][Cart::xyy][m + 1] +
                                        term_yy;
            dip4[0][Cart::xxyz][k][m] = PmB1 * dip4[0][Cart::xxz][k][m] -
                                        PmC1 * dip4[0][Cart::xxz][k][m + 1] +
                                        (k == 1) * nuc3[0][Cart::xxz][m + 1];
            dip4[0][Cart::xxzz][k][m] = PmB0 * dip4[0][Cart::xzz][k][m] -
                                        PmC0 * dip4[0][Cart::xzz][k][m + 1] +
                                        (k == 0) * nuc3[0][Cart::xzz][m + 1] +
                                        term_zz;
            dip4[0][Cart::xyyy][k][m] = PmB0 * dip4[0][Cart::yyy][k][m] -
                                        PmC0 * dip4[0][Cart::yyy][k][m + 1] +
                                        (k == 0) * nuc3[0][Cart::yyy][m + 1];
            dip4[0][Cart::xyyz][k][m] = PmB0 * dip4[0][Cart::yyz][k][m] -
                                        PmC0 * dip4[0][Cart::yyz][k][m + 1] +
                                        (k == 0) * nuc3[0][Cart::yyz][m + 1];
            dip4[0][Cart::xyzz][k][m] = PmB0 * dip4[0][Cart::yzz][k][m] -
                                        PmC0 * dip4[0][Cart::yzz][k][m + 1] +
                                        (k == 0) * nuc3[0][Cart::yzz][m + 1];
            dip4[0][Cart::xzzz][k][m] = PmB0 * dip4[0][Cart::zzz][k][m] -
                                        PmC0 * dip4[0][Cart::zzz][k][m + 1] +
                                        (k == 0) * nuc3[0][Cart::zzz][m + 1];
            dip4[0][Cart::yyyy][k][m] = PmB1 * dip4[0][Cart::yyy][k][m] -
                                        PmC1 * dip4[0][Cart::yyy][k][m + 1] +
                                        (k == 1) * nuc3[0][Cart::yyy][m + 1] +
                                        3 * term_yy;
            dip4[0][Cart::yyyz][k][m] = PmB2 * dip4[0][Cart::yyy][k][m] -
                                        PmC2 * dip4[0][Cart::yyy][k][m + 1] +
                                        (k == 2) * nuc3[0][Cart::yyy][m + 1];
            dip4[0][Cart::yyzz][k][m] = PmB1 * dip4[0][Cart::yzz][k][m] -
                                        PmC1 * dip4[0][Cart::yzz][k][m + 1] +
                                        (k == 1) * nuc3[0][Cart::yzz][m + 1] +
                                        term_zz;
            dip4[0][Cart::yzzz][k][m] = PmB1 * dip4[0][Cart::zzz][k][m] -
                                        PmC1 * dip4[0][Cart::zzz][k][m + 1] +
                                        (k == 1) * nuc3[0][Cart::zzz][m + 1];
            dip4[0][Cart::zzzz][k][m] = PmB2 * dip4[0][Cart::zzz][k][m] -
                                        PmC2 * dip4[0][Cart::zzz][k][m + 1] +
                                        (k == 2) * nuc3[0][Cart::zzz][m + 1] +
                                        3 * term_zz;
          }
        }
        //------------------------------------------------------

        // Integrals     p - g     d - g     f - g     g - g
        for (int m = 0; m < lmax_col - 3; m++) {
          for (int i = 1; i < n_orbitals[lmax_row]; i++) {
            int nx_i = nx[i];
            int ny_i = ny[i];
            int nz_i = nz[i];
            int ilx_i = i_less_x[i];
            int ily_i = i_less_y[i];
            int ilz_i = i_less_z[i];
            for (int k = 0; k < 3; k++) {
              double term_xx =
                  fak * (dip4[i][Cart::xx][k][m] - dip4[i][Cart::xx][k][m + 1]);
              double term_yy =
                  fak * (dip4[i][Cart::yy][k][m] - dip4[i][Cart::yy][k][m + 1]);
              double term_zz =
                  fak * (dip4[i][Cart::zz][k][m] - dip4[i][Cart::zz][k][m + 1]);
              dip4[i][Cart::xxxx][k][m] =
                  PmB0 * dip4[i][Cart::xxx][k][m] -
                  PmC0 * dip4[i][Cart::xxx][k][m + 1] +
                  (k == 0) * nuc3[i][Cart::xxx][m + 1] +
                  nx_i * fak *
                      (dip4[ilx_i][Cart::xxx][k][m] -
                       dip4[ilx_i][Cart::xxx][k][m + 1]) +
                  3 * term_xx;
              dip4[i][Cart::xxxy][k][m] =
                  PmB1 * dip4[i][Cart::xxx][k][m] -
                  PmC1 * dip4[i][Cart::xxx][k][m + 1] +
                  (k == 1) * nuc3[i][Cart::xxx][m + 1] +
                  ny_i * fak *
                      (dip4[ily_i][Cart::xxx][k][m] -
                       dip4[ily_i][Cart::xxx][k][m + 1]);
              dip4[i][Cart::xxxz][k][m] =
                  PmB2 * dip4[i][Cart::xxx][k][m] -
                  PmC2 * dip4[i][Cart::xxx][k][m + 1] +
                  (k == 2) * nuc3[i][Cart::xxx][m + 1] +
                  nz_i * fak *
                      (dip4[ilz_i][Cart::xxx][k][m] -
                       dip4[ilz_i][Cart::xxx][k][m + 1]);
              dip4[i][Cart::xxyy][k][m] =
                  PmB0 * dip4[i][Cart::xyy][k][m] -
                  PmC0 * dip4[i][Cart::xyy][k][m + 1] +
                  (k == 0) * nuc3[i][Cart::xyy][m + 1] +
                  nx_i * fak *
                      (dip4[ilx_i][Cart::xyy][k][m] -
                       dip4[ilx_i][Cart::xyy][k][m + 1]) +
                  term_yy;
              dip4[i][Cart::xxyz][k][m] =
                  PmB1 * dip4[i][Cart::xxz][k][m] -
                  PmC1 * dip4[i][Cart::xxz][k][m + 1] +
                  (k == 1) * nuc3[i][Cart::xxz][m + 1] +
                  ny_i * fak *
                      (dip4[ily_i][Cart::xxz][k][m] -
                       dip4[ily_i][Cart::xxz][k][m + 1]);
              dip4[i][Cart::xxzz][k][m] =
                  PmB0 * dip4[i][Cart::xzz][k][m] -
                  PmC0 * dip4[i][Cart::xzz][k][m + 1] +
                  (k == 0) * nuc3[i][Cart::xzz][m + 1] +
                  nx_i * fak *
                      (dip4[ilx_i][Cart::xzz][k][m] -
                       dip4[ilx_i][Cart::xzz][k][m + 1]) +
                  term_zz;
              dip4[i][Cart::xyyy][k][m] =
                  PmB0 * dip4[i][Cart::yyy][k][m] -
                  PmC0 * dip4[i][Cart::yyy][k][m + 1] +
                  (k == 0) * nuc3[i][Cart::yyy][m + 1] +
                  nx_i * fak *
                      (dip4[ilx_i][Cart::yyy][k][m] -
                       dip4[ilx_i][Cart::yyy][k][m + 1]);
              dip4[i][Cart::xyyz][k][m] =
                  PmB0 * dip4[i][Cart::yyz][k][m] -
                  PmC0 * dip4[i][Cart::yyz][k][m + 1] +
                  (k == 0) * nuc3[i][Cart::yyz][m + 1] +
                  nx_i * fak *
                      (dip4[ilx_i][Cart::yyz][k][m] -
                       dip4[ilx_i][Cart::yyz][k][m + 1]);
              dip4[i][Cart::xyzz][k][m] =
                  PmB0 * dip4[i][Cart::yzz][k][m] -
                  PmC0 * dip4[i][Cart::yzz][k][m + 1] +
                  (k == 0) * nuc3[i][Cart::yzz][m + 1] +
                  nx_i * fak *
                      (dip4[ilx_i][Cart::yzz][k][m] -
                       dip4[ilx_i][Cart::yzz][k][m + 1]);
              dip4[i][Cart::xzzz][k][m] =
                  PmB0 * dip4[i][Cart::zzz][k][m] -
                  PmC0 * dip4[i][Cart::zzz][k][m + 1] +
                  (k == 0) * nuc3[i][Cart::zzz][m + 1] +
                  nx_i * fak *
                      (dip4[ilx_i][Cart::zzz][k][m] -
                       dip4[ilx_i][Cart::zzz][k][m + 1]);
              dip4[i][Cart::yyyy][k][m] =
                  PmB1 * dip4[i][Cart::yyy][k][m] -
                  PmC1 * dip4[i][Cart::yyy][k][m + 1] +
                  (k == 1) * nuc3[i][Cart::yyy][m + 1] +
                  ny_i * fak *
                      (dip4[ily_i][Cart::yyy][k][m] -
                       dip4[ily_i][Cart::yyy][k][m + 1]) +
                  3 * term_yy;
              dip4[i][Cart::yyyz][k][m] =
                  PmB2 * dip4[i][Cart::yyy][k][m] -
                  PmC2 * dip4[i][Cart::yyy][k][m + 1] +
                  (k == 2) * nuc3[i][Cart::yyy][m + 1] +
                  nz_i * fak *
                      (dip4[ilz_i][Cart::yyy][k][m] -
                       dip4[ilz_i][Cart::yyy][k][m + 1]);
              dip4[i][Cart::yyzz][k][m] =
                  PmB1 * dip4[i][Cart::yzz][k][m] -
                  PmC1 * dip4[i][Cart::yzz][k][m + 1] +
                  (k == 1) * nuc3[i][Cart::yzz][m + 1] +
                  ny_i * fak *
                      (dip4[ily_i][Cart::yzz][k][m] -
                       dip4[ily_i][Cart::yzz][k][m + 1]) +
                  term_zz;
              dip4[i][Cart::yzzz][k][m] =
                  PmB1 * dip4[i][Cart::zzz][k][m] -
                  PmC1 * dip4[i][Cart::zzz][k][m + 1] +
                  (k == 1) * nuc3[i][Cart::zzz][m + 1] +
                  ny_i * fak *
                      (dip4[ily_i][Cart::zzz][k][m] -
                       dip4[ily_i][Cart::zzz][k][m + 1]);
              dip4[i][Cart::zzzz][k][m] =
                  PmB2 * dip4[i][Cart::zzz][k][m] -
                  PmC2 * dip4[i][Cart::zzz][k][m + 1] +
                  (k == 2) * nuc3[i][Cart::zzz][m + 1] +
                  nz_i * fak *
                      (dip4[ilz_i][Cart::zzz][k][m] -
                       dip4[ilz_i][Cart::zzz][k][m + 1]) +
                  3 * term_zz;
            }
          }
        }
        //------------------------------------------------------

      }  // end if (lmax_col > 3)

      double fact = 1. / 3.;
      double fac0[] = {fact, fact, 0., 2. * fact, 0.};
      double fac1[] = {fact, 0., fact, 0., 2. * fact};
      double fac2[] = {0., fact, fact, -2. * fact, -2. * fact};

      int ind0[] = {1, 2, 0, 0, 0};
      int ind1[] = {0, 0, 2, 0, 1};
      int ind2[] = {0, 0, 1, 2, 2};

      // (s-s element normiert )
      double _prefactor_quad = (4. * zeta * zeta * _prefactor) / 3.;
      for (int m = 0; m < lsum + 1; m++) {
        quad4[0][0][0][m] = PmC0 * PmC1 * _prefactor_quad * FmU[m + 2];
        quad4[0][0][1][m] = PmC0 * PmC2 * _prefactor_quad * FmU[m + 2];
        quad4[0][0][2][m] = PmC1 * PmC2 * _prefactor_quad * FmU[m + 2];
        quad4[0][0][3][m] =
            (PmC0 * PmC0 - PmC2 * PmC2) * _prefactor_quad * FmU[m + 2];
        quad4[0][0][4][m] =
            (PmC1 * PmC1 - PmC2 * PmC2) * _prefactor_quad * FmU[m + 2];
      }
      //------------------------------------------------------

      // Integrals     p - s
      if (lmax_row > 0) {
        for (int m = 0; m < lsum; m++) {
          for (int k = 0; k < 5; k++) {
            quad4[Cart::x][0][k][m] = PmA0 * quad4[0][0][k][m] -
                                      PmC0 * quad4[0][0][k][m + 1] +
                                      fac0[k] * dip4[0][0][ind0[k]][m + 1];
            quad4[Cart::y][0][k][m] = PmA1 * quad4[0][0][k][m] -
                                      PmC1 * quad4[0][0][k][m + 1] +
                                      fac1[k] * dip4[0][0][ind1[k]][m + 1];
            quad4[Cart::z][0][k][m] = PmA2 * quad4[0][0][k][m] -
                                      PmC2 * quad4[0][0][k][m + 1] +
                                      fac2[k] * dip4[0][0][ind2[k]][m + 1];
          }
        }
      }
      //------------------------------------------------------

      // Integrals     d - s
      if (lmax_row > 1) {
        for (int m = 0; m < lsum - 1; m++) {
          for (int k = 0; k < 5; k++) {
            double term = fak * (quad4[0][0][k][m] - quad4[0][0][k][m + 1]);
            quad4[Cart::xx][0][k][m] =
                PmA0 * quad4[Cart::x][0][k][m] -
                PmC0 * quad4[Cart::x][0][k][m + 1] +
                fac0[k] * dip4[Cart::x][0][ind0[k]][m + 1] + term;
            quad4[Cart::xy][0][k][m] =
                PmA0 * quad4[Cart::y][0][k][m] -
                PmC0 * quad4[Cart::y][0][k][m + 1] +
                fac0[k] * dip4[Cart::y][0][ind0[k]][m + 1];
            quad4[Cart::xz][0][k][m] =
                PmA0 * quad4[Cart::z][0][k][m] -
                PmC0 * quad4[Cart::z][0][k][m + 1] +
                fac0[k] * dip4[Cart::z][0][ind0[k]][m + 1];
            quad4[Cart::yy][0][k][m] =
                PmA1 * quad4[Cart::y][0][k][m] -
                PmC1 * quad4[Cart::y][0][k][m + 1] +
                fac1[k] * dip4[Cart::y][0][ind1[k]][m + 1] + term;
            quad4[Cart::yz][0][k][m] =
                PmA1 * quad4[Cart::z][0][k][m] -
                PmC1 * quad4[Cart::z][0][k][m + 1] +
                fac1[k] * dip4[Cart::z][0][ind1[k]][m + 1];
            quad4[Cart::zz][0][k][m] =
                PmA2 * quad4[Cart::z][0][k][m] -
                PmC2 * quad4[Cart::z][0][k][m + 1] +
                fac2[k] * dip4[Cart::z][0][ind2[k]][m + 1] + term;
          }
        }
      }
      //------------------------------------------------------

      // Integrals     f - s
      if (lmax_row > 2) {
        for (int m = 0; m < lsum - 2; m++) {
          for (int k = 0; k < 5; k++) {
            quad4[Cart::xxx][0][k][m] =
                PmA0 * quad4[Cart::xx][0][k][m] -
                PmC0 * quad4[Cart::xx][0][k][m + 1] +
                fac0[k] * dip4[Cart::xx][0][ind0[k]][m + 1] +
                2 * fak *
                    (quad4[Cart::x][0][k][m] - quad4[Cart::x][0][k][m + 1]);
            quad4[Cart::xxy][0][k][m] =
                PmA1 * quad4[Cart::xx][0][k][m] -
                PmC1 * quad4[Cart::xx][0][k][m + 1] +
                fac1[k] * dip4[Cart::xx][0][ind1[k]][m + 1];
            quad4[Cart::xxz][0][k][m] =
                PmA2 * quad4[Cart::xx][0][k][m] -
                PmC2 * quad4[Cart::xx][0][k][m + 1] +
                fac2[k] * dip4[Cart::xx][0][ind2[k]][m + 1];
            quad4[Cart::xyy][0][k][m] =
                PmA0 * quad4[Cart::yy][0][k][m] -
                PmC0 * quad4[Cart::yy][0][k][m + 1] +
                fac0[k] * dip4[Cart::yy][0][ind0[k]][m + 1];
            quad4[Cart::xyz][0][k][m] =
                PmA0 * quad4[Cart::yz][0][k][m] -
                PmC0 * quad4[Cart::yz][0][k][m + 1] +
                fac0[k] * dip4[Cart::yz][0][ind0[k]][m + 1];
            quad4[Cart::xzz][0][k][m] =
                PmA0 * quad4[Cart::zz][0][k][m] -
                PmC0 * quad4[Cart::zz][0][k][m + 1] +
                fac0[k] * dip4[Cart::zz][0][ind0[k]][m + 1];
            quad4[Cart::yyy][0][k][m] =
                PmA1 * quad4[Cart::yy][0][k][m] -
                PmC1 * quad4[Cart::yy][0][k][m + 1] +
                fac1[k] * dip4[Cart::yy][0][ind1[k]][m + 1] +
                2 * fak *
                    (quad4[Cart::y][0][k][m] - quad4[Cart::y][0][k][m + 1]);
            quad4[Cart::yyz][0][k][m] =
                PmA2 * quad4[Cart::yy][0][k][m] -
                PmC2 * quad4[Cart::yy][0][k][m + 1] +
                fac2[k] * dip4[Cart::yy][0][ind2[k]][m + 1];
            quad4[Cart::yzz][0][k][m] =
                PmA1 * quad4[Cart::zz][0][k][m] -
                PmC1 * quad4[Cart::zz][0][k][m + 1] +
                fac1[k] * dip4[Cart::zz][0][ind1[k]][m + 1];
            quad4[Cart::zzz][0][k][m] =
                PmA2 * quad4[Cart::zz][0][k][m] -
                PmC2 * quad4[Cart::zz][0][k][m + 1] +
                fac2[k] * dip4[Cart::zz][0][ind2[k]][m + 1] +
                2 * fak *
                    (quad4[Cart::z][0][k][m] - quad4[Cart::z][0][k][m + 1]);
          }
        }
      }
      //------------------------------------------------------

      // Integrals     g - s
      if (lmax_row > 3) {
        for (int m = 0; m < lsum - 3; m++) {
          for (int k = 0; k < 5; k++) {
            double term_xx =
                fak * (quad4[Cart::xx][0][k][m] - quad4[Cart::xx][0][k][m + 1]);
            double term_yy =
                fak * (quad4[Cart::yy][0][k][m] - quad4[Cart::yy][0][k][m + 1]);
            double term_zz =
                fak * (quad4[Cart::zz][0][k][m] - quad4[Cart::zz][0][k][m + 1]);
            quad4[Cart::xxxx][0][k][m] =
                PmA0 * quad4[Cart::xxx][0][k][m] -
                PmC0 * quad4[Cart::xxx][0][k][m + 1] +
                fac0[k] * dip4[Cart::xxx][0][ind0[k]][m + 1] + 3 * term_xx;
            quad4[Cart::xxxy][0][k][m] =
                PmA1 * quad4[Cart::xxx][0][k][m] -
                PmC1 * quad4[Cart::xxx][0][k][m + 1] +
                fac1[k] * dip4[Cart::xxx][0][ind1[k]][m + 1];
            quad4[Cart::xxxz][0][k][m] =
                PmA2 * quad4[Cart::xxx][0][k][m] -
                PmC2 * quad4[Cart::xxx][0][k][m + 1] +
                fac2[k] * dip4[Cart::xxx][0][ind2[k]][m + 1];
            quad4[Cart::xxyy][0][k][m] =
                PmA0 * quad4[Cart::xyy][0][k][m] -
                PmC0 * quad4[Cart::xyy][0][k][m + 1] +
                fac0[k] * dip4[Cart::xyy][0][ind0[k]][m + 1] + term_yy;
            quad4[Cart::xxyz][0][k][m] =
                PmA1 * quad4[Cart::xxz][0][k][m] -
                PmC1 * quad4[Cart::xxz][0][k][m + 1] +
                fac1[k] * dip4[Cart::xxz][0][ind1[k]][m + 1];
            quad4[Cart::xxzz][0][k][m] =
                PmA0 * quad4[Cart::xzz][0][k][m] -
                PmC0 * quad4[Cart::xzz][0][k][m + 1] +
                fac0[k] * dip4[Cart::xzz][0][ind0[k]][m + 1] + term_zz;
            quad4[Cart::xyyy][0][k][m] =
                PmA0 * quad4[Cart::yyy][0][k][m] -
                PmC0 * quad4[Cart::yyy][0][k][m + 1] +
                fac0[k] * dip4[Cart::yyy][0][ind0[k]][m + 1];
            quad4[Cart::xyyz][0][k][m] =
                PmA0 * quad4[Cart::yyz][0][k][m] -
                PmC0 * quad4[Cart::yyz][0][k][m + 1] +
                fac0[k] * dip4[Cart::yyz][0][ind0[k]][m + 1];
            quad4[Cart::xyzz][0][k][m] =
                PmA0 * quad4[Cart::yzz][0][k][m] -
                PmC0 * quad4[Cart::yzz][0][k][m + 1] +
                fac0[k] * dip4[Cart::yzz][0][ind0[k]][m + 1];
            quad4[Cart::xzzz][0][k][m] =
                PmA0 * quad4[Cart::zzz][0][k][m] -
                PmC0 * quad4[Cart::zzz][0][k][m + 1] +
                fac0[k] * dip4[Cart::zzz][0][ind0[k]][m + 1];
            quad4[Cart::yyyy][0][k][m] =
                PmA1 * quad4[Cart::yyy][0][k][m] -
                PmC1 * quad4[Cart::yyy][0][k][m + 1] +
                fac1[k] * dip4[Cart::yyy][0][ind1[k]][m + 1] + 3 * term_yy;
            quad4[Cart::yyyz][0][k][m] =
                PmA2 * quad4[Cart::yyy][0][k][m] -
                PmC2 * quad4[Cart::yyy][0][k][m + 1] +
                fac2[k] * dip4[Cart::yyy][0][ind2[k]][m + 1];
            quad4[Cart::yyzz][0][k][m] =
                PmA1 * quad4[Cart::yzz][0][k][m] -
                PmC1 * quad4[Cart::yzz][0][k][m + 1] +
                fac1[k] * dip4[Cart::yzz][0][ind1[k]][m + 1] + term_zz;
            quad4[Cart::yzzz][0][k][m] =
                PmA1 * quad4[Cart::zzz][0][k][m] -
                PmC1 * quad4[Cart::zzz][0][k][m + 1] +
                fac1[k] * dip4[Cart::zzz][0][ind1[k]][m + 1];
            quad4[Cart::zzzz][0][k][m] =
                PmA2 * quad4[Cart::zzz][0][k][m] -
                PmC2 * quad4[Cart::zzz][0][k][m + 1] +
                fac2[k] * dip4[Cart::zzz][0][ind2[k]][m + 1] + 3 * term_zz;
          }
        }
      }
      //------------------------------------------------------

      if (lmax_col > 0) {

        // Integrals     s - p
        for (int m = 0; m < lmax_col; m++) {
          for (int k = 0; k < 5; k++) {
            quad4[0][Cart::x][k][m] = PmB0 * quad4[0][0][k][m] -
                                      PmC0 * quad4[0][0][k][m + 1] +
                                      fac0[k] * dip4[0][0][ind0[k]][m + 1];
            quad4[0][Cart::y][k][m] = PmB1 * quad4[0][0][k][m] -
                                      PmC1 * quad4[0][0][k][m + 1] +
                                      fac1[k] * dip4[0][0][ind1[k]][m + 1];
            quad4[0][Cart::z][k][m] = PmB2 * quad4[0][0][k][m] -
                                      PmC2 * quad4[0][0][k][m + 1] +
                                      fac2[k] * dip4[0][0][ind2[k]][m + 1];
          }
        }
        //------------------------------------------------------

        // Integrals     p - p
        if (lmax_row > 0) {
          for (int m = 0; m < lmax_col; m++) {
            for (int i = 1; i < 4; i++) {
              for (int k = 0; k < 5; k++) {
                double term = fak * (quad4[0][0][k][m] - quad4[0][0][k][m + 1]);
                quad4[i][Cart::x][k][m] =
                    PmB0 * quad4[i][0][k][m] - PmC0 * quad4[i][0][k][m + 1] +
                    fac0[k] * dip4[i][0][ind0[k]][m + 1] + nx[i] * term;
                quad4[i][Cart::y][k][m] =
                    PmB1 * quad4[i][0][k][m] - PmC1 * quad4[i][0][k][m + 1] +
                    fac1[k] * dip4[i][0][ind1[k]][m + 1] + ny[i] * term;
                quad4[i][Cart::z][k][m] =
                    PmB2 * quad4[i][0][k][m] - PmC2 * quad4[i][0][k][m + 1] +
                    fac2[k] * dip4[i][0][ind2[k]][m + 1] + nz[i] * term;
              }
            }
          }
        }
        //------------------------------------------------------

        // Integrals     d - p     f - p     g - p
        for (int m = 0; m < lmax_col; m++) {
          for (int i = 4; i < n_orbitals[lmax_row]; i++) {
            int nx_i = nx[i];
            int ny_i = ny[i];
            int nz_i = nz[i];
            int ilx_i = i_less_x[i];
            int ily_i = i_less_y[i];
            int ilz_i = i_less_z[i];
            for (int k = 0; k < 5; k++) {
              quad4[i][Cart::x][k][m] =
                  PmB0 * quad4[i][0][k][m] - PmC0 * quad4[i][0][k][m + 1] +
                  fac0[k] * dip4[i][0][ind0[k]][m + 1] +
                  nx_i * fak *
                      (quad4[ilx_i][0][k][m] - quad4[ilx_i][0][k][m + 1]);
              quad4[i][Cart::y][k][m] =
                  PmB1 * quad4[i][0][k][m] - PmC1 * quad4[i][0][k][m + 1] +
                  fac1[k] * dip4[i][0][ind1[k]][m + 1] +
                  ny_i * fak *
                      (quad4[ily_i][0][k][m] - quad4[ily_i][0][k][m + 1]);
              quad4[i][Cart::z][k][m] =
                  PmB2 * quad4[i][0][k][m] - PmC2 * quad4[i][0][k][m + 1] +
                  fac2[k] * dip4[i][0][ind2[k]][m + 1] +
                  nz_i * fak *
                      (quad4[ilz_i][0][k][m] - quad4[ilz_i][0][k][m + 1]);
            }
          }
        }
        //------------------------------------------------------

      }  // end if (lmax_col > 0)

      if (lmax_col > 1) {

        // Integrals     s - d
        for (int m = 0; m < lmax_col - 1; m++) {
          for (int k = 0; k < 5; k++) {
            double term = fak * (quad4[0][0][k][m] - quad4[0][0][k][m + 1]);
            quad4[0][Cart::xx][k][m] =
                PmB0 * quad4[0][Cart::x][k][m] -
                PmC0 * quad4[0][Cart::x][k][m + 1] +
                fac0[k] * dip4[0][Cart::x][ind0[k]][m + 1] + term;
            quad4[0][Cart::xy][k][m] =
                PmB0 * quad4[0][Cart::y][k][m] -
                PmC0 * quad4[0][Cart::y][k][m + 1] +
                fac0[k] * dip4[0][Cart::y][ind0[k]][m + 1];
            quad4[0][Cart::xz][k][m] =
                PmB0 * quad4[0][Cart::z][k][m] -
                PmC0 * quad4[0][Cart::z][k][m + 1] +
                fac0[k] * dip4[0][Cart::z][ind0[k]][m + 1];
            quad4[0][Cart::yy][k][m] =
                PmB1 * quad4[0][Cart::y][k][m] -
                PmC1 * quad4[0][Cart::y][k][m + 1] +
                fac1[k] * dip4[0][Cart::y][ind1[k]][m + 1] + term;
            quad4[0][Cart::yz][k][m] =
                PmB1 * quad4[0][Cart::z][k][m] -
                PmC1 * quad4[0][Cart::z][k][m + 1] +
                fac1[k] * dip4[0][Cart::z][ind1[k]][m + 1];
            quad4[0][Cart::zz][k][m] =
                PmB2 * quad4[0][Cart::z][k][m] -
                PmC2 * quad4[0][Cart::z][k][m + 1] +
                fac2[k] * dip4[0][Cart::z][ind2[k]][m + 1] + term;
          }
        }
        //------------------------------------------------------

        // Integrals     p - d     d - d     f - d     g - d
        for (int m = 0; m < lmax_col - 1; m++) {
          for (int i = 1; i < n_orbitals[lmax_row]; i++) {
            int nx_i = nx[i];
            int ny_i = ny[i];
            int nz_i = nz[i];
            int ilx_i = i_less_x[i];
            int ily_i = i_less_y[i];
            int ilz_i = i_less_z[i];
            for (int k = 0; k < 5; k++) {
              double term = fak * (quad4[i][0][k][m] - quad4[i][0][k][m + 1]);
              quad4[i][Cart::xx][k][m] =
                  PmB0 * quad4[i][Cart::x][k][m] -
                  PmC0 * quad4[i][Cart::x][k][m + 1] +
                  fac0[k] * dip4[i][Cart::x][ind0[k]][m + 1] +
                  nx_i * fak *
                      (quad4[ilx_i][Cart::x][k][m] -
                       quad4[ilx_i][Cart::x][k][m + 1]) +
                  term;
              quad4[i][Cart::xy][k][m] =
                  PmB0 * quad4[i][Cart::y][k][m] -
                  PmC0 * quad4[i][Cart::y][k][m + 1] +
                  fac0[k] * dip4[i][Cart::y][ind0[k]][m + 1] +
                  nx_i * fak *
                      (quad4[ilx_i][Cart::y][k][m] -
                       quad4[ilx_i][Cart::y][k][m + 1]);
              quad4[i][Cart::xz][k][m] =
                  PmB0 * quad4[i][Cart::z][k][m] -
                  PmC0 * quad4[i][Cart::z][k][m + 1] +
                  fac0[k] * dip4[i][Cart::z][ind0[k]][m + 1] +
                  nx_i * fak *
                      (quad4[ilx_i][Cart::z][k][m] -
                       quad4[ilx_i][Cart::z][k][m + 1]);
              quad4[i][Cart::yy][k][m] =
                  PmB1 * quad4[i][Cart::y][k][m] -
                  PmC1 * quad4[i][Cart::y][k][m + 1] +
                  fac1[k] * dip4[i][Cart::y][ind1[k]][m + 1] +
                  ny_i * fak *
                      (quad4[ily_i][Cart::y][k][m] -
                       quad4[ily_i][Cart::y][k][m + 1]) +
                  term;
              quad4[i][Cart::yz][k][m] =
                  PmB1 * quad4[i][Cart::z][k][m] -
                  PmC1 * quad4[i][Cart::z][k][m + 1] +
                  fac1[k] * dip4[i][Cart::z][ind1[k]][m + 1] +
                  ny_i * fak *
                      (quad4[ily_i][Cart::z][k][m] -
                       quad4[ily_i][Cart::z][k][m + 1]);
              quad4[i][Cart::zz][k][m] =
                  PmB2 * quad4[i][Cart::z][k][m] -
                  PmC2 * quad4[i][Cart::z][k][m + 1] +
                  fac2[k] * dip4[i][Cart::z][ind2[k]][m + 1] +
                  nz_i * fak *
                      (quad4[ilz_i][Cart::z][k][m] -
                       quad4[ilz_i][Cart::z][k][m + 1]) +
                  term;
            }
          }
        }
        //------------------------------------------------------

      }  // end if (lmax_col > 1)

      if (lmax_col > 2) {

        // Integrals     s - f
        for (int m = 0; m < lmax_col - 2; m++) {
          for (int k = 0; k < 5; k++) {
            quad4[0][Cart::xxx][k][m] =
                PmB0 * quad4[0][Cart::xx][k][m] -
                PmC0 * quad4[0][Cart::xx][k][m + 1] +
                fac0[k] * dip4[0][Cart::xx][ind0[k]][m + 1] +
                2 * fak *
                    (quad4[0][Cart::x][k][m] - quad4[0][Cart::x][k][m + 1]);
            quad4[0][Cart::xxy][k][m] =
                PmB1 * quad4[0][Cart::xx][k][m] -
                PmC1 * quad4[0][Cart::xx][k][m + 1] +
                fac1[k] * dip4[0][Cart::xx][ind1[k]][m + 1];
            quad4[0][Cart::xxz][k][m] =
                PmB2 * quad4[0][Cart::xx][k][m] -
                PmC2 * quad4[0][Cart::xx][k][m + 1] +
                fac2[k] * dip4[0][Cart::xx][ind2[k]][m + 1];
            quad4[0][Cart::xyy][k][m] =
                PmB0 * quad4[0][Cart::yy][k][m] -
                PmC0 * quad4[0][Cart::yy][k][m + 1] +
                fac0[k] * dip4[0][Cart::yy][ind0[k]][m + 1];
            quad4[0][Cart::xyz][k][m] =
                PmB0 * quad4[0][Cart::yz][k][m] -
                PmC0 * quad4[0][Cart::yz][k][m + 1] +
                fac0[k] * dip4[0][Cart::yz][ind0[k]][m + 1];
            quad4[0][Cart::xzz][k][m] =
                PmB0 * quad4[0][Cart::zz][k][m] -
                PmC0 * quad4[0][Cart::zz][k][m + 1] +
                fac0[k] * dip4[0][Cart::zz][ind0[k]][m + 1];
            quad4[0][Cart::yyy][k][m] =
                PmB1 * quad4[0][Cart::yy][k][m] -
                PmC1 * quad4[0][Cart::yy][k][m + 1] +
                fac1[k] * dip4[0][Cart::yy][ind1[k]][m + 1] +
                2 * fak *
                    (quad4[0][Cart::y][k][m] - quad4[0][Cart::y][k][m + 1]);
            quad4[0][Cart::yyz][k][m] =
                PmB2 * quad4[0][Cart::yy][k][m] -
                PmC2 * quad4[0][Cart::yy][k][m + 1] +
                fac2[k] * dip4[0][Cart::yy][ind2[k]][m + 1];
            quad4[0][Cart::yzz][k][m] =
                PmB1 * quad4[0][Cart::zz][k][m] -
                PmC1 * quad4[0][Cart::zz][k][m + 1] +
                fac1[k] * dip4[0][Cart::zz][ind1[k]][m + 1];
            quad4[0][Cart::zzz][k][m] =
                PmB2 * quad4[0][Cart::zz][k][m] -
                PmC2 * quad4[0][Cart::zz][k][m + 1] +
                fac2[k] * dip4[0][Cart::zz][ind2[k]][m + 1] +
                2 * fak *
                    (quad4[0][Cart::z][k][m] - quad4[0][Cart::z][k][m + 1]);
          }
        }
        //------------------------------------------------------

        // Integrals     p - f     d - f     f - f     g - f
        for (int m = 0; m < lmax_col - 2; m++) {
          for (int i = 1; i < n_orbitals[lmax_row]; i++) {
            int nx_i = nx[i];
            int ny_i = ny[i];
            int nz_i = nz[i];
            int ilx_i = i_less_x[i];
            int ily_i = i_less_y[i];
            int ilz_i = i_less_z[i];
            for (int k = 0; k < 5; k++) {
              double term_x =
                  2 * fak *
                  (quad4[i][Cart::x][k][m] - quad4[i][Cart::x][k][m + 1]);
              double term_y =
                  2 * fak *
                  (quad4[i][Cart::y][k][m] - quad4[i][Cart::y][k][m + 1]);
              double term_z =
                  2 * fak *
                  (quad4[i][Cart::z][k][m] - quad4[i][Cart::z][k][m + 1]);
              quad4[i][Cart::xxx][k][m] =
                  PmB0 * quad4[i][Cart::xx][k][m] -
                  PmC0 * quad4[i][Cart::xx][k][m + 1] +
                  fac0[k] * dip4[i][Cart::xx][ind0[k]][m + 1] +
                  nx_i * fak *
                      (quad4[ilx_i][Cart::xx][k][m] -
                       quad4[ilx_i][Cart::xx][k][m + 1]) +
                  term_x;
              quad4[i][Cart::xxy][k][m] =
                  PmB1 * quad4[i][Cart::xx][k][m] -
                  PmC1 * quad4[i][Cart::xx][k][m + 1] +
                  fac1[k] * dip4[i][Cart::xx][ind1[k]][m + 1] +
                  ny_i * fak *
                      (quad4[ily_i][Cart::xx][k][m] -
                       quad4[ily_i][Cart::xx][k][m + 1]);
              quad4[i][Cart::xxz][k][m] =
                  PmB2 * quad4[i][Cart::xx][k][m] -
                  PmC2 * quad4[i][Cart::xx][k][m + 1] +
                  fac2[k] * dip4[i][Cart::xx][ind2[k]][m + 1] +
                  nz_i * fak *
                      (quad4[ilz_i][Cart::xx][k][m] -
                       quad4[ilz_i][Cart::xx][k][m + 1]);
              quad4[i][Cart::xyy][k][m] =
                  PmB0 * quad4[i][Cart::yy][k][m] -
                  PmC0 * quad4[i][Cart::yy][k][m + 1] +
                  fac0[k] * dip4[i][Cart::yy][ind0[k]][m + 1] +
                  nx_i * fak *
                      (quad4[ilx_i][Cart::yy][k][m] -
                       quad4[ilx_i][Cart::yy][k][m + 1]);
              quad4[i][Cart::xyz][k][m] =
                  PmB0 * quad4[i][Cart::yz][k][m] -
                  PmC0 * quad4[i][Cart::yz][k][m + 1] +
                  fac0[k] * dip4[i][Cart::yz][ind0[k]][m + 1] +
                  nx_i * fak *
                      (quad4[ilx_i][Cart::yz][k][m] -
                       quad4[ilx_i][Cart::yz][k][m + 1]);
              quad4[i][Cart::xzz][k][m] =
                  PmB0 * quad4[i][Cart::zz][k][m] -
                  PmC0 * quad4[i][Cart::zz][k][m + 1] +
                  fac0[k] * dip4[i][Cart::zz][ind0[k]][m + 1] +
                  nx_i * fak *
                      (quad4[ilx_i][Cart::zz][k][m] -
                       quad4[ilx_i][Cart::zz][k][m + 1]);
              quad4[i][Cart::yyy][k][m] =
                  PmB1 * quad4[i][Cart::yy][k][m] -
                  PmC1 * quad4[i][Cart::yy][k][m + 1] +
                  fac1[k] * dip4[i][Cart::yy][ind1[k]][m + 1] +
                  ny_i * fak *
                      (quad4[ily_i][Cart::yy][k][m] -
                       quad4[ily_i][Cart::yy][k][m + 1]) +
                  term_y;
              quad4[i][Cart::yyz][k][m] =
                  PmB2 * quad4[i][Cart::yy][k][m] -
                  PmC2 * quad4[i][Cart::yy][k][m + 1] +
                  fac2[k] * dip4[i][Cart::yy][ind2[k]][m + 1] +
                  nz_i * fak *
                      (quad4[ilz_i][Cart::yy][k][m] -
                       quad4[ilz_i][Cart::yy][k][m + 1]);
              quad4[i][Cart::yzz][k][m] =
                  PmB1 * quad4[i][Cart::zz][k][m] -
                  PmC1 * quad4[i][Cart::zz][k][m + 1] +
                  fac1[k] * dip4[i][Cart::zz][ind1[k]][m + 1] +
                  ny_i * fak *
                      (quad4[ily_i][Cart::zz][k][m] -
                       quad4[ily_i][Cart::zz][k][m + 1]);
              quad4[i][Cart::zzz][k][m] =
                  PmB2 * quad4[i][Cart::zz][k][m] -
                  PmC2 * quad4[i][Cart::zz][k][m + 1] +
                  fac2[k] * dip4[i][Cart::zz][ind2[k]][m + 1] +
                  nz_i * fak *
                      (quad4[ilz_i][Cart::zz][k][m] -
                       quad4[ilz_i][Cart::zz][k][m + 1]) +
                  term_z;
            }
          }
        }
        //------------------------------------------------------

      }  // end if (lmax_col > 2)

      if (lmax_col > 3) {

        // Integrals     s - g
        for (int m = 0; m < lmax_col - 3; m++) {
          for (int k = 0; k < 5; k++) {
            double term_xx =
                fak * (quad4[0][Cart::xx][k][m] - quad4[0][Cart::xx][k][m + 1]);
            double term_yy =
                fak * (quad4[0][Cart::yy][k][m] - quad4[0][Cart::yy][k][m + 1]);
            double term_zz =
                fak * (quad4[0][Cart::zz][k][m] - quad4[0][Cart::zz][k][m + 1]);
            quad4[0][Cart::xxxx][k][m] =
                PmB0 * quad4[0][Cart::xxx][k][m] -
                PmC0 * quad4[0][Cart::xxx][k][m + 1] +
                fac0[k] * dip4[0][Cart::xxx][ind0[k]][m + 1] + 3 * term_xx;
            quad4[0][Cart::xxxy][k][m] =
                PmB1 * quad4[0][Cart::xxx][k][m] -
                PmC1 * quad4[0][Cart::xxx][k][m + 1] +
                fac1[k] * dip4[0][Cart::xxx][ind1[k]][m + 1];
            quad4[0][Cart::xxxz][k][m] =
                PmB2 * quad4[0][Cart::xxx][k][m] -
                PmC2 * quad4[0][Cart::xxx][k][m + 1] +
                fac2[k] * dip4[0][Cart::xxx][ind2[k]][m + 1];
            quad4[0][Cart::xxyy][k][m] =
                PmB0 * quad4[0][Cart::xyy][k][m] -
                PmC0 * quad4[0][Cart::xyy][k][m + 1] +
                fac0[k] * dip4[0][Cart::xyy][ind0[k]][m + 1] + term_yy;
            quad4[0][Cart::xxyz][k][m] =
                PmB1 * quad4[0][Cart::xxz][k][m] -
                PmC1 * quad4[0][Cart::xxz][k][m + 1] +
                fac1[k] * dip4[0][Cart::xxz][ind1[k]][m + 1];
            quad4[0][Cart::xxzz][k][m] =
                PmB0 * quad4[0][Cart::xzz][k][m] -
                PmC0 * quad4[0][Cart::xzz][k][m + 1] +
                fac0[k] * dip4[0][Cart::xzz][ind0[k]][m + 1] + term_zz;
            quad4[0][Cart::xyyy][k][m] =
                PmB0 * quad4[0][Cart::yyy][k][m] -
                PmC0 * quad4[0][Cart::yyy][k][m + 1] +
                fac0[k] * dip4[0][Cart::yyy][ind0[k]][m + 1];
            quad4[0][Cart::xyyz][k][m] =
                PmB0 * quad4[0][Cart::yyz][k][m] -
                PmC0 * quad4[0][Cart::yyz][k][m + 1] +
                fac0[k] * dip4[0][Cart::yyz][ind0[k]][m + 1];
            quad4[0][Cart::xyzz][k][m] =
                PmB0 * quad4[0][Cart::yzz][k][m] -
                PmC0 * quad4[0][Cart::yzz][k][m + 1] +
                fac0[k] * dip4[0][Cart::yzz][ind0[k]][m + 1];
            quad4[0][Cart::xzzz][k][m] =
                PmB0 * quad4[0][Cart::zzz][k][m] -
                PmC0 * quad4[0][Cart::zzz][k][m + 1] +
                fac0[k] * dip4[0][Cart::zzz][ind0[k]][m + 1];
            quad4[0][Cart::yyyy][k][m] =
                PmB1 * quad4[0][Cart::yyy][k][m] -
                PmC1 * quad4[0][Cart::yyy][k][m + 1] +
                fac1[k] * dip4[0][Cart::yyy][ind1[k]][m + 1] + 3 * term_yy;
            quad4[0][Cart::yyyz][k][m] =
                PmB2 * quad4[0][Cart::yyy][k][m] -
                PmC2 * quad4[0][Cart::yyy][k][m + 1] +
                fac2[k] * dip4[0][Cart::yyy][ind2[k]][m + 1];
            quad4[0][Cart::yyzz][k][m] =
                PmB1 * quad4[0][Cart::yzz][k][m] -
                PmC1 * quad4[0][Cart::yzz][k][m + 1] +
                fac1[k] * dip4[0][Cart::yzz][ind1[k]][m + 1] + term_zz;
            quad4[0][Cart::yzzz][k][m] =
                PmB1 * quad4[0][Cart::zzz][k][m] -
                PmC1 * quad4[0][Cart::zzz][k][m + 1] +
                fac1[k] * dip4[0][Cart::zzz][ind1[k]][m + 1];
            quad4[0][Cart::zzzz][k][m] =
                PmB2 * quad4[0][Cart::zzz][k][m] -
                PmC2 * quad4[0][Cart::zzz][k][m + 1] +
                fac2[k] * dip4[0][Cart::zzz][ind2[k]][m + 1] + 3 * term_zz;
          }
        }
        //------------------------------------------------------

        // Integrals     p - g     d - g     f - g     g - g
        for (int m = 0; m < lmax_col - 3; m++) {
          for (int i = 1; i < n_orbitals[lmax_row]; i++) {
            int nx_i = nx[i];
            int ny_i = ny[i];
            int nz_i = nz[i];
            int ilx_i = i_less_x[i];
            int ily_i = i_less_y[i];
            int ilz_i = i_less_z[i];
            for (int k = 0; k < 5; k++) {
              double term_xx = fak * (quad4[i][Cart::xx][k][m] -
                                      quad4[i][Cart::xx][k][m + 1]);
              double term_yy = fak * (quad4[i][Cart::yy][k][m] -
                                      quad4[i][Cart::yy][k][m + 1]);
              double term_zz = fak * (quad4[i][Cart::zz][k][m] -
                                      quad4[i][Cart::zz][k][m + 1]);
              quad4[i][Cart::xxxx][k][m] =
                  PmB0 * quad4[i][Cart::xxx][k][m] -
                  PmC0 * quad4[i][Cart::xxx][k][m + 1] +
                  fac0[k] * dip4[i][Cart::xxx][ind0[k]][m + 1] +
                  nx_i * fak *
                      (quad4[ilx_i][Cart::xxx][k][m] -
                       quad4[ilx_i][Cart::xxx][k][m + 1]) +
                  3 * term_xx;
              quad4[i][Cart::xxxy][k][m] =
                  PmB1 * quad4[i][Cart::xxx][k][m] -
                  PmC1 * quad4[i][Cart::xxx][k][m + 1] +
                  fac1[k] * dip4[i][Cart::xxx][ind1[k]][m + 1] +
                  ny_i * fak *
                      (quad4[ily_i][Cart::xxx][k][m] -
                       quad4[ily_i][Cart::xxx][k][m + 1]);
              quad4[i][Cart::xxxz][k][m] =
                  PmB2 * quad4[i][Cart::xxx][k][m] -
                  PmC2 * quad4[i][Cart::xxx][k][m + 1] +
                  fac2[k] * dip4[i][Cart::xxx][ind2[k]][m + 1] +
                  nz_i * fak *
                      (quad4[ilz_i][Cart::xxx][k][m] -
                       quad4[ilz_i][Cart::xxx][k][m + 1]);
              quad4[i][Cart::xxyy][k][m] =
                  PmB0 * quad4[i][Cart::xyy][k][m] -
                  PmC0 * quad4[i][Cart::xyy][k][m + 1] +
                  fac0[k] * dip4[i][Cart::xyy][ind0[k]][m + 1] +
                  nx_i * fak *
                      (quad4[ilx_i][Cart::xyy][k][m] -
                       quad4[ilx_i][Cart::xyy][k][m + 1]) +
                  term_yy;
              quad4[i][Cart::xxyz][k][m] =
                  PmB1 * quad4[i][Cart::xxz][k][m] -
                  PmC1 * quad4[i][Cart::xxz][k][m + 1] +
                  fac1[k] * dip4[i][Cart::xxz][ind1[k]][m + 1] +
                  ny_i * fak *
                      (quad4[ily_i][Cart::xxz][k][m] -
                       quad4[ily_i][Cart::xxz][k][m + 1]);
              quad4[i][Cart::xxzz][k][m] =
                  PmB0 * quad4[i][Cart::xzz][k][m] -
                  PmC0 * quad4[i][Cart::xzz][k][m + 1] +
                  fac0[k] * dip4[i][Cart::xzz][ind0[k]][m + 1] +
                  nx_i * fak *
                      (quad4[ilx_i][Cart::xzz][k][m] -
                       quad4[ilx_i][Cart::xzz][k][m + 1]) +
                  term_zz;
              quad4[i][Cart::xyyy][k][m] =
                  PmB0 * quad4[i][Cart::yyy][k][m] -
                  PmC0 * quad4[i][Cart::yyy][k][m + 1] +
                  fac0[k] * dip4[i][Cart::yyy][ind0[k]][m + 1] +
                  nx_i * fak *
                      (quad4[ilx_i][Cart::yyy][k][m] -
                       quad4[ilx_i][Cart::yyy][k][m + 1]);
              quad4[i][Cart::xyyz][k][m] =
                  PmB0 * quad4[i][Cart::yyz][k][m] -
                  PmC0 * quad4[i][Cart::yyz][k][m + 1] +
                  fac0[k] * dip4[i][Cart::yyz][ind0[k]][m + 1] +
                  nx_i * fak *
                      (quad4[ilx_i][Cart::yyz][k][m] -
                       quad4[ilx_i][Cart::yyz][k][m + 1]);
              quad4[i][Cart::xyzz][k][m] =
                  PmB0 * quad4[i][Cart::yzz][k][m] -
                  PmC0 * quad4[i][Cart::yzz][k][m + 1] +
                  fac0[k] * dip4[i][Cart::yzz][ind0[k]][m + 1] +
                  nx_i * fak *
                      (quad4[ilx_i][Cart::yzz][k][m] -
                       quad4[ilx_i][Cart::yzz][k][m + 1]);
              quad4[i][Cart::xzzz][k][m] =
                  PmB0 * quad4[i][Cart::zzz][k][m] -
                  PmC0 * quad4[i][Cart::zzz][k][m + 1] +
                  fac0[k] * dip4[i][Cart::zzz][ind0[k]][m + 1] +
                  nx_i * fak *
                      (quad4[ilx_i][Cart::zzz][k][m] -
                       quad4[ilx_i][Cart::zzz][k][m + 1]);
              quad4[i][Cart::yyyy][k][m] =
                  PmB1 * quad4[i][Cart::yyy][k][m] -
                  PmC1 * quad4[i][Cart::yyy][k][m + 1] +
                  fac1[k] * dip4[i][Cart::yyy][ind1[k]][m + 1] +
                  ny_i * fak *
                      (quad4[ily_i][Cart::yyy][k][m] -
                       quad4[ily_i][Cart::yyy][k][m + 1]) +
                  3 * term_yy;
              quad4[i][Cart::yyyz][k][m] =
                  PmB2 * quad4[i][Cart::yyy][k][m] -
                  PmC2 * quad4[i][Cart::yyy][k][m + 1] +
                  fac2[k] * dip4[i][Cart::yyy][ind2[k]][m + 1] +
                  nz_i * fak *
                      (quad4[ilz_i][Cart::yyy][k][m] -
                       quad4[ilz_i][Cart::yyy][k][m + 1]);
              quad4[i][Cart::yyzz][k][m] =
                  PmB1 * quad4[i][Cart::yzz][k][m] -
                  PmC1 * quad4[i][Cart::yzz][k][m + 1] +
                  fac1[k] * dip4[i][Cart::yzz][ind1[k]][m + 1] +
                  ny_i * fak *
                      (quad4[ily_i][Cart::yzz][k][m] -
                       quad4[ily_i][Cart::yzz][k][m + 1]) +
                  term_zz;
              quad4[i][Cart::yzzz][k][m] =
                  PmB1 * quad4[i][Cart::zzz][k][m] -
                  PmC1 * quad4[i][Cart::zzz][k][m + 1] +
                  fac1[k] * dip4[i][Cart::zzz][ind1[k]][m + 1] +
                  ny_i * fak *
                      (quad4[ily_i][Cart::zzz][k][m] -
                       quad4[ily_i][Cart::zzz][k][m + 1]);
              quad4[i][Cart::zzzz][k][m] =
                  PmB2 * quad4[i][Cart::zzz][k][m] -
                  PmC2 * quad4[i][Cart::zzz][k][m + 1] +
                  fac2[k] * dip4[i][Cart::zzz][ind2[k]][m + 1] +
                  nz_i * fak *
                      (quad4[ilz_i][Cart::zzz][k][m] -
                       quad4[ilz_i][Cart::zzz][k][m + 1]) +
                  3 * term_zz;
            }
          }
        }
        //------------------------------------------------------

      }  // end if (lmax_col > 3)

      for (int i = 0; i < nrows; i++) {
        for (int j = 0; j < ncols; j++) {
          quad(i, j) =
              q_01 * quad4[i][j][0][0] + q_02 * quad4[i][j][1][0] +
              q_12 * quad4[i][j][2][0] +
              .5 * (q_00 * quad4[i][j][3][0] + q_11 * quad4[i][j][4][0]);
        }
      }

      Eigen::MatrixXd quad_sph =
          getTrafo(*itr).transpose() * quad * getTrafo(*itc);
      // save to matrix

      for (unsigned i = 0; i < matrix.rows(); i++) {
        for (unsigned j = 0; j < matrix.cols(); j++) {
          matrix(i, j) +=
              quad_sph(i + shell_row->getOffset(), j + shell_col->getOffset());
        }
      }

    }  // shell_col Gaussians
  }    // shell_row Gaussians
}

void AOQuadrupole_Potential::Fillextpotential(
    const AOBasis& aobasis,
    const std::vector<std::shared_ptr<ctp::PolarSeg> >& sites) {

  _externalpotential =
      Eigen::MatrixXd::Zero(aobasis.AOBasisSize(), aobasis.AOBasisSize());
  for (unsigned int i = 0; i < sites.size(); i++) {
    for (ctp::APolarSite* site : *(sites[i])) {

      if (site->getRank() > 1) {
        _aomatrix =
            Eigen::MatrixXd::Zero(aobasis.AOBasisSize(), aobasis.AOBasisSize());
        setAPolarSite(site);
        Fill(aobasis);
        _externalpotential += _aomatrix;
      }
    }
  }

  return;
}

}  // namespace xtp
}  // namespace votca
