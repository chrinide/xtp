#ifndef VOTCA_XTP_PAIRCALCULATOR2_H
#define VOTCA_XTP_PAIRCALCULATOR2_H

#include <votca/xtp/qmcalculator.h>

namespace votca {
namespace xtp {

class PairCalculator2 : public QMCalculator {
 public:
  PairCalculator2(){};
  virtual ~PairCalculator2(){};

  bool EvaluateFrame(Topology *top);
  virtual void EvaluatePair(Topology *top, QMPair *pair){};
};

bool PairCalculator2::EvaluateFrame(Topology *top) {

  // Rigidify if (a) not rigid yet (b) rigidification at all possible
  if (!top->isRigid()) {
    bool isRigid = top->Rigidify();
    if (!isRigid) {
      return 0;
    }
  } else {
    std::cout << std::endl << "... ... System is already rigidified.";
  }
  std::cout << std::endl;

  QMNBList &nblist = top->NBList();

  QMNBList::iterator pit;
  for (pit = nblist.begin(); pit != nblist.end(); pit++) {

    EvaluatePair(top, *pit);

    if ((*pit)->getId() == -1) {

      std::string pairId = boost::lexical_cast<std::string>((*pit)->getId());
      std::string pdbname = "Pair" + pairId + ".pdb";
    }
  }

  return 1;
}

}  // namespace xtp
}  // namespace votca

#endif  // VOTCA_XTP_PAIRCALCULATOR2_H
