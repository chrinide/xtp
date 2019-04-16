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

#ifndef __XTP_MULLIKEN__H
#define __XTP_MULLIKEN__H

#include <votca/tools/elements.h>
#include <votca/xtp/aobasis.h>
#include <votca/xtp/qmatom.h>

/**
 * \brief Takes a list of atoms, and the corresponding density matrix and puts
 * out a table of mulliken partial charges
 *
 *
 *
 */

namespace votca {
namespace xtp {

class Mulliken {
 public:
  Mulliken() {}
  ~Mulliken(){};

  void EvaluateMulliken(std::vector<QMAtom*>& _atomlist,
                        const Eigen::MatrixXd& _dmat, const AOBasis& basis,
                        bool _do_transition);
};

}  // namespace xtp
}  // namespace votca

#endif /* ESPFIT_H */
