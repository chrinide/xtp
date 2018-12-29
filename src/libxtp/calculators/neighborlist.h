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


#ifndef __VOTCA_XTP_NEIGHBORLIST_H
#define __VOTCA_XTP_NEIGHBORLIST_H

#include <votca/tools/globals.h>
#include <votca/xtp/qmcalculator.h>
#include <votca/xtp/qmpair.h>
#include <votca/xtp/qmnblist.h>
#include <votca/xtp/topology.h>
#include <votca/xtp/segment.h>
#include <votca/xtp/fragment.h>
#include <votca/xtp/atom.h>
#include <votca/tools/property.h>
#include <boost/progress.hpp>
#include <boost/format.hpp>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace votca {
    namespace xtp {

        class Neighborlist : public QMCalculator {
        public:

            std::string Identify() {
                return "neighborlist";
            }

            void Initialize(tools::Property *options);
            bool EvaluateFrame(Topology *top);
            void DetClassicalPairs(Topology* top);

        private:

            std::vector<std::string> _included_segments;
            std::map< std::string, std::map< std::string, double> > _cutoffs;
            bool _useConstantCutoff;
            double _constantCutoff;
            bool _useExcitonCutoff;
            double _excitonqmCutoff;
            std::string _pairfilename;
            bool _generate_from_file;

        };

        void Neighborlist::Initialize(tools::Property *options) {

            // update options with the VOTCASHARE defaults   
            UpdateWithDefaults(options, "xtp");
            std::string key = "options." + Identify();

            std::list< tools::Property* > segs = options->Select(key + ".segments");

            for (tools::Property* segprop : segs) {
                std::string types = segprop->get("type").as< std::string>();
                double cutoff = segprop->get("cutoff").as<double>();

                tools::Tokenizer tok(types, " ");
                std::vector< std::string > names;
                tok.ToVector(names);

                if (names.size() != 2) {
                    throw std::runtime_error("ERROR: Faulty pair definition for cut-off's: Need two segment names separated by a space");
                }
                _cutoffs[names[0]][names[1]] = cutoff;
                _cutoffs[names[1]][names[0]] = cutoff;
                if (std::find(_included_segments.begin(), _included_segments.end(), names[0]) == _included_segments.end()) {
                    _included_segments.push_back(names[0]);
                }
                if (std::find(_included_segments.begin(), _included_segments.end(), names[1]) == _included_segments.end()) {
                    _included_segments.push_back(names[1]);
                }
            }

            if (options->exists(key + ".constant")) {
                _useConstantCutoff = true;
                _constantCutoff = options->get(key + ".constant").as< double >();
            } else {
                _useConstantCutoff = false;
            }
            if (options->exists(key + ".exciton_cutoff")) {
                _useExcitonCutoff = true;
                _excitonqmCutoff = options->get(key + ".exciton_cutoff").as< double >();
            } else {
                _useExcitonCutoff = false;
            }
            if (options->exists(key + ".generate_from")) {
                _generate_from_file = true;
                _pairfilename = options->get(key + ".generate_from").as< std::string >();
            } else {
                _generate_from_file = false;
                _pairfilename = "nofile";
            }

        }

        void Neighborlist::DetClassicalPairs(Topology* top){
            std::cout << std::endl << " ... ... Determining classical pairs " << std::endl;
            for (QMPair* pair:top->NBList()) {
                tools::vec r1;
                tools::vec r2;
                Segment* seg1=pair->Seg1();
                Segment* seg2=pair->Seg2();
                bool stopLoop = false;
                for (Fragment* frag1:seg1->Fragments()) {
                    if (stopLoop) {
                        break;
                    }
                    for (Fragment* frag2:seg2->Fragments()) {
                        r1 = frag1->getPos();
                        r2 = frag2->getPos();
                        if (tools::abs(top->PbShortestConnect(r1, r2)) > _excitonqmCutoff) {
                            pair->setType(QMPair::PairType::Excitoncl);
                            continue;
                        } else {
                            pair->setType(QMPair::PairType::Hopping);
                            stopLoop = true;
                            break;
                        }
                    } /* exit loop frag2 */
                } /* exit loop frag1 */
            } //Type 3 Exciton_classical approx
        }

        bool Neighborlist::EvaluateFrame(Topology *top) {
            top->NBList().Cleanup();

                if (tools::globals::verbose) {
                    std::cout << std::endl << "... ..." << std::flush;
                }

                const tools::matrix box = top->getBox();
                double min = box.get(0, 0);
                if (min > box.get(1, 1)) {
                    min = box.get(1, 1);
                }
                if (min > box.get(2, 2)) {
                    min = box.get(2, 2);
                }

                std::vector< Segment* > segs;
                for (Segment* seg:top->Segments()) {
                    if (_useConstantCutoff || std::find(_included_segments.begin(), _included_segments.end(), seg->getName()) != _included_segments.end()) {
                        segs.push_back(seg);
                        seg->calcPos();
                        seg->calcApproxSize();
                    }
                }
                std::cout << std::endl;
                std::cout << "Evaluating " << segs.size() << " segments for neighborlist. " 
                        << top->Segments().size() - segs.size() << " segments are not taken into account as specified" << std::endl;
                if (!_useConstantCutoff) {
                    std::cout << "The following segments are used in the neigborlist creation" << std::endl;
                    std::cout << "\t" << std::flush;
                    for (const std::string& st:_included_segments) {
                        std::cout << " " << st << std::flush;
                    }
                    std::cout << std::endl;
                }

                std::cout << "\r ... ... Evaluating " << std::flush;
                std::vector<std::string> skippedpairs;

                for (std::vector< Segment* >::iterator segit1 = segs.begin(); segit1 < segs.end(); ++segit1) {
                    Segment *seg1 = *segit1;

                    std::vector< Segment* > ::iterator segit2;
                    double cutoff;
                    tools::vec r1;
                    tools::vec r2;

                    if (tools::globals::verbose) {
                        std::cout << "\r ... ... NB List Seg " << seg1->getId() << std::flush;
                    }

                    for (segit2 = segit1 + 1; segit2 < segs.end(); ++segit2) {
                        Segment *seg2 = *segit2;
                        if (!_useConstantCutoff) {
                            try {
                                cutoff = _cutoffs.at(seg1->getName())
                                        .at(seg2->getName());
                            } catch (const std::exception& out_of_range) {
                                std::string pairstring = seg1->getName() + "/" + seg2->getName();
                                if (std::find(skippedpairs.begin(), skippedpairs.end(), pairstring) == skippedpairs.end()) {
                                    skippedpairs.push_back(pairstring);
                                }
                                continue;
                            }
                        }
                        else {
                            cutoff = _constantCutoff;
                        }

                        if (cutoff > 0.5 * min) {
                            throw std::runtime_error((boost::format("Cutoff is larger than half the box size. Maximum allowed cutoff is %1$1.1f") % (0.5 * min)).str());
                        }
                        double cutoff2 = cutoff*cutoff;
                        tools::vec segdistance = top->PbShortestConnect(seg1->getPos(), seg2->getPos());
                        double segdistance2 = segdistance*segdistance;
                        double outside = cutoff + seg1->getApproxSize() + seg2->getApproxSize();

                        if (segdistance2 < cutoff2) {
                            top->NBList().Add(seg1, seg2);
                        } else if (segdistance2 > (outside * outside)) {
                            continue;
                        } else {
                            bool stopLoop = false;
                            for (Fragment* frag1:seg1->Fragments()) {
                                if (stopLoop) {
                                    break;
                                }
                                r1 = frag1->getPos();
                                for (Fragment* frag2:seg2->Fragments()) {
                                    r2 = frag2->getPos();
                                    tools::vec distance = top->PbShortestConnect(r1, r2);
                                    double dist2 = distance*distance;
                                    if (dist2 > cutoff2) {
                                        continue;
                                    } else {
                                        top->NBList().Add(seg1, seg2);
                                        stopLoop = true;
                                        break;
                                    }
                                } /* exit loop frag2 */
                            } /* exit loop frag1 */
                        }
                    } /* exit loop seg2 */
                } /* exit loop seg1 */

                if (skippedpairs.size() > 0) {
                    std::cout << "WARNING: No cut-off specified for segment pairs of type " << std::endl;
                    for (const std::string& st:skippedpairs) {
                        std::cout << st << std::endl;
                    }
                    std::cout << "pairs were skipped" << std::endl;
                }

                std::cout << std::endl << " ... ... Created " << top->NBList().size() << " direct pairs.";
                if (_useExcitonCutoff) {
                    DetClassicalPairs(top);
                }
            return true;
        }

    }
}

#endif  /* __NEIGHBORLIST2_H */
