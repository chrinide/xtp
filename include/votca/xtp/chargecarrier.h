/*
 * Copyright 2009-2017 The VOTCA Development Team (http://www.votca.org)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * author: Kordt
 */

#ifndef VOTCA_XTP_CHARGECARRIER_H
#define	VOTCA_XTP_CHARGECARRIER_H

#include <votca/xtp/gnode.h>
#include <votca/xtp/glink.h>


namespace votca { namespace xtp {
    
  
       
        
            class Chargecarrier
            {
                public:
                    Chargecarrier(int id): _id(id),lifetime(0.0),steps(0),_dr_travelled(Eigen::Vector3d::Zero()),node(NULL){};
                    bool hasNode(){return (node!=NULL);}
                    void updateLifetime(double dt) { lifetime+=dt;}
                    void updateOccupationtime(double dt){ node->occupationtime+=dt;}
                    void updateSteps(unsigned t) { steps+=t;}
                    void resetCarrier() { lifetime=0;steps=0; _dr_travelled=Eigen::Vector3d::Zero();}
                    double getLifetime()const{return lifetime;}
                    unsigned getSteps()const{return steps;}
                    int getCurrentNodeId()const{return node->id;}
                    double getCurrentEnergy()const{return node->siteenergy;}
                    const Eigen::Vector3d& getCurrentPosition()const{return node->position;}
                    double getCurrentEscapeRate()const{return node->escape_rate;}
                    GNode& getCurrentNode()const{return *node;}

                    void ReleaseNode(){
                        node->occupied=false;
                    }
                    
                    void settoNote(GNode *newnode){node=newnode;
                        node->occupied=true;}

                    void jumpAccordingEvent(const GLink& event){
                        ReleaseNode();
                        settoNote(event.destination);
                        _dr_travelled+=event.dr;
                    }

                    const Eigen::Vector3d& get_dRtravelled() const {
                        return _dr_travelled;
                    }

                    int getId() const {
                        return _id;
                    }
                   void setId(int id) {
                       _id=id;
                    }
   
                private:
                    int _id;
                    double lifetime;
                    unsigned steps; 
                    Eigen::Vector3d _dr_travelled;
                    GNode *node;
                   
            };
     



}}


#endif	// VOTCA_XTP_CHARGECARRIER_H
