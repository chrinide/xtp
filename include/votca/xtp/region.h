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
#include <votca/xtp/checkpoint.h>

#ifndef VOTCA_XTP_REGION_H
#define	VOTCA_XTP_REGION_H

/**
* \brief base class to derive regions from
*
* 
* 
*/

namespace votca { namespace xtp {
   
class Region{
    public:

        virtual ~Region() {};
               

        virtual void WriteToCpt(CheckpointWriter& w)const =0 ;

        virtual void ReadFromCpt(CheckpointReader& r)=0;

        virtual int size()const=0;
    
};
    
}}

#endif	// VOTCA_XTP_REGION_H