#ifndef _VOTCA_XTP_CARTESIAN_COORD
#define _VOTCA_XTP_CARTESIAN_COORD

#include <votca/xtp/qmatom.h>
#include <votca/tools/vec.h>
#include <votca/xtp/coordbase.h>
#include <votca/xtp/orbitals.h>
#include <votca/xtp/eigen.h>

namespace votca { namespace xtp {

class CartesianCoords: public CoordBase{
public:
    CartesianCoords(const Orbitals& orb);
};


}// xtp
} // votca
#endif // _VOTCA_XTP_CARTESIAN_COORD
