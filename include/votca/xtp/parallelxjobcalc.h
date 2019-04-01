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
/// For an earlier history see ctp repo commit
/// 77795ea591b29e664153f9404c8655ba28dc14e9

#ifndef VOTCA_XTP_PARALLELXJOBCALC_H
#define VOTCA_XTP_PARALLELXJOBCALC_H

#include <votca/tools/mutex.h>
#include <votca/xtp/job.h>
#include <votca/xtp/jobcalculator.h>
#include <votca/xtp/progressobserver.h>
#include <votca/xtp/qmthread.h>

namespace votca {
namespace xtp {

template <typename jobtype>
class ParallelXJobCalc : public JobCalculator {

 public:
  class JobOperator;

  virtual std::string Identify() = 0;

  bool EvaluateFrame(Topology &top);
  void CustomizeLogger(QMThread &thread);
  virtual jobtype::JobResult EvalJob(Topology &top, const jobtype &job,
                                     QMThread &thread) = 0;

  // ======================================== //
  // XJOB OPERATOR (THREAD)                   //
  // ======================================== //

  class JobOperator : public QMThread {
   public:
    JobOperator(int id, Topology &top, ParallelXJobCalc<jobtype> &master)
        : _id(id), _top(top), _master(master){};
    ~JobOperator(){};

    void Run();

   private:
    Topology &_top;
    ParallelXJobCalc<jobtype> &_master;
  };

 protected:
  std::vector<jobtype> _XJobs;
  std::string _jobfile = "";
};

}  // namespace xtp
}  // namespace votca

#endif  // VOTCA_XTP_PARALLELXJOBCALC_H
