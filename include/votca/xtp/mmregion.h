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


#ifndef VOTCA_XTP_MMREGION_H
#define	VOTCA_XTP_MMREGION_H


#include <votca/xtp/region.h>
#include <votca/xtp/polarsegment.h>

namespace votca { namespace xtp {

class MMRegion: public Region{
    public:

        void WriteToCpt(CheckpointWriter& w)const;

        void ReadFromCpt(CheckpointReader& r);

        int size()const{return _segments.size();}

        std::vector<PolarSegment>::iterator begin(){return _segments.begin();}
        std::vector<PolarSegment>::iterator end(){return _segments.end();}

        std::vector<PolarSegment>::const_iterator begin()const{return _segments.begin();}
        std::vector<PolarSegment>::const_iterator end()const{return _segments.end();}

        void push_back(const PolarSegment& seg){
            _segments.push_back(seg);
        }
        
    private:

        std::vector<PolarSegment> _segments;

};

}}

#endif /* VOTCA_XTP_MMREGION_H */

