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

#ifndef VOTCA_XTP_ESPFIT_H
#define	VOTCA_XTP_ESPFIT_H

#include <votca/xtp/grid.h>
#include <votca/xtp/orbitals.h>
#include <votca/xtp/aobasis.h>
/**
* \brief Takes a list of atoms, and the corresponding density matrix and puts out a table of partial charges
*
* 
* 
*/

namespace votca { namespace xtp {
    
class Espfit{
public:
    
    struct ConstraintRegion{
         std::vector<int> atomindices;
         double charge;
     };
    
    Espfit(Logger *log):_do_svd(true) {_log = log;
    _conditionnumber=1e-8;
    _pairconstraint.resize(0);
    _regionconstraint.resize(0);
    }
   
   void setUseSVD(double conditionnumber){_do_svd=true;_conditionnumber=conditionnumber;}
    
   void setPairConstraint(std::vector< std::pair<int,int> > pairconstraint){
       _pairconstraint=pairconstraint;
   }
   
   void setRegionConstraint(std::vector< ConstraintRegion > regionconstraint){
       _regionconstraint=regionconstraint;
   }
    // on grid very fast
    void Fit2Density(Orbitals& orbitals,const QMState& state, const AOBasis &basis,std::string gridsize);
    // not so fast
    void Fit2Density_analytic(Orbitals& orbitals,const QMState& state, const AOBasis &basis);
private:
    
     Logger *_log;
     bool _do_svd;
     double _conditionnumber;
     
     std::vector< std::pair<int,int> > _pairconstraint; //  pairconstraint[i] is all the atomindices which have the same charge     
     
     std::vector< ConstraintRegion > _regionconstraint;
 
    void EvalNuclearPotential(const QMMolecule& atoms, Grid& grid);
   
     // Fits partial charges to Potential on a grid, constrains net charge
    void FitPartialCharges(Orbitals& orbitals,const Grid& grid,double netcharge  );
    
};
}}

#endif	// VOTCA_XTP_ESPFIT_H 
