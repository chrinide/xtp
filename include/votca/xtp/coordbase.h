#ifndef _VOTCA_XTP_COORDBASE
#define _VOTCA_XTP_COORDBASE
#include <string>
#include <votca/xtp/qmatom.h>
#include <votca/xtp/orbitals.h>
#include <votca/xtp/eigen.h>

namespace votca { namespace xtp{

enum CoordType{CARTESIAN, INTERNAL};

class CoordBase{
public:

    CoordBase();

    Eigen::VectorXd Vector();
    Eigen::VectorXd operator()();
    const Orbitals& System();
protected:
    // CoordBase cannot be instantiated
    CoordBase(const CoordType& , const Orbitals& orb);
    const CoordType _type;
    const std::vector<QMAtom*>& _qmMolecule;
    const Orbitals& _system;
    const int _numAtoms;
    std::vector<double> _vector;
};
typedef CoordBase CoordSystem;
} //xtp
} //votca

#endif