/* 
 * Copyright 2009-2018 The VOTCA Development Team (http://www.votca.org)
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
 */

#include "kmclifetime.h"
#include <votca/xtp/gnode.h>
#include <votca/tools/property.h>
#include <votca/tools/constants.h>
#include <boost/format.hpp>
#include <votca/ctp/topology.h>
#include <locale>
#include <votca/xtp/eigen.h>


using namespace std;

namespace votca {
    namespace xtp {

        
        
    void KMCLifetime::Initialize( tools::Property *options) {
            
    std::string key = "options." + Identify();

    _insertions=options->ifExistsReturnElseThrowRuntimeError<unsigned int>(key+".numberofinsertions");
    _seed=options->ifExistsReturnElseThrowRuntimeError<int>(key+".seed");
    _numberofcharges=options->ifExistsReturnElseThrowRuntimeError<int>(key+".numberofcharges");
    _injection_name=options->ifExistsReturnElseThrowRuntimeError<std::string>(key+".injectionpattern");
    _lifetimefile=options->ifExistsReturnElseThrowRuntimeError<string>(key+".lifetimefile");
    
    
    _probfile=options->ifExistsReturnElseReturnDefault<std::string>(key+".decayprobfile","");

    _maxrealtime=options->ifExistsReturnElseReturnDefault<double>(key+".maxrealtime",1E10);
     _trajectoryfile=options->ifExistsReturnElseReturnDefault<std::string>(key+".trajectoryfile","trajectory.csv");
    _temperature=options->ifExistsReturnElseReturnDefault<double>(key+".temperature",300);
    _rates=options->ifExistsReturnElseReturnDefault<std::string>(key+".rates","statefile");

     std::string subkey=key+".carrierenergy";
    if (options->exists(subkey)) {
        _do_carrierenergy=options->ifExistsReturnElseReturnDefault<bool>(subkey+".run",false);
        _energy_outputfile = options->ifExistsReturnElseReturnDefault<std::string>(subkey+".outputfile","energy.tab");
         _alpha = options->ifExistsReturnElseReturnDefault<double>(subkey+".alpha",0.3);
         _outputsteps=options->ifExistsReturnElseReturnDefault<double>(subkey+".outputsteps",100);

    } else {
        _do_carrierenergy=false;
    }

    _carriertype = 2;
    cout << "carrier type:"<<CarrierInttoLongString(_carriertype) << endl;
    _field=tools::vec(0.0);


    if (_rates != "statefile" && _rates != "calculate") {
        cout << "WARNING in kmclifetime: Invalid option rates. Valid options are 'statefile' or 'calculate'. Setting it to 'statefile'." << endl;
        _rates = "statefile";
    }
    
    lengthdistribution = options->ifExistsReturnElseReturnDefault<double>(key+".jumplengthdist",0);
    if(lengthdistribution>0){
        dolengthdistributon=true;
    }
    
    return;
    }
    
    
    void KMCLifetime::WriteDecayProbability(string filename){
        
        Eigen::VectorXd outrates=Eigen::VectorXd::Zero(_nodes.size());
        Eigen::VectorXd inrates=Eigen::VectorXd::Zero(_nodes.size());
        Eigen::VectorXd decayrates=Eigen::VectorXd::Ones(_nodes.size());
        
        for (unsigned i=0;i<_nodes.size();i++){
            GNode* node=_nodes[i];
            if(node->hasdecay){
                for(const GLink& event:node->events){
                    if(event.decayevent){
                        decayrates[i]=event.rate;
                    }else{
                        inrates[event.destination]+=event.rate;
                        outrates[i]+=event.rate;
                    }   
                }
            }
        }
        outrates=decayrates.cwiseQuotient(outrates+decayrates);
        inrates=decayrates.cwiseQuotient(inrates+decayrates);
        
        fstream probs;
        probs.open ( filename.c_str(), fstream::out);
        probs<<"#SiteID, Relative Prob outgoing, Relative Prob ingoing"<<endl;
        for (unsigned i=0;i<_nodes.size();i++){
            probs<<_nodes[i]->id<<" "<<outrates[i]<<" "<<inrates[i]<<endl;
        }
        probs.close();
        return;
    }

        
    void KMCLifetime::ReadLifetimeFile(std::string filename){
        tools::Property xml;
        load_property_from_xml(xml, filename);
        list<tools::Property*> jobProps = xml.Select("lifetimes.site");
        if (jobProps.size()!=_nodes.size()){
            throw  runtime_error((boost::format("The number of sites in the sqlfile: %i does not match the number in the lifetimefile: %i")
                    % _nodes.size() % jobProps.size()).str()); 
        }

        for (list<tools::Property*> ::iterator  it = jobProps.begin(); it != jobProps.end(); ++it) {
            int site_id =(*it)->getAttribute<int>("id")-1;
            double lifetime=boost::lexical_cast<double>((*it)->value());
            bool check=false;
            for (unsigned i=0;i<_nodes.size();i++){
                if (_nodes[i]->id==site_id && !(_nodes[i]->hasdecay)){
                    _nodes[i]->AddDecayEvent(1.0/lifetime);
                    check=true;
                    break;
                }
                else if(_nodes[i]->id==site_id && _nodes[i]->hasdecay){
                    throw runtime_error((boost::format("Node %i appears twice in your list") %site_id).str());
                } 

            }
            if (!check){
            throw runtime_error((boost::format("Site from file with id: %i not found in sql") %site_id).str());
            }
        }

        return;
    }

        
    void  KMCLifetime::RunVSSM(ctp::Topology *top) {

        int realtime_start = time(NULL);
        cout << endl << "Algorithm: VSSM for Multiple Charges with finite Lifetime" << endl;
        cout << "number of charges: " << _numberofcharges << endl;
        cout << "number of nodes: " << _nodes.size() << endl;

        if (_numberofcharges > _nodes.size()) {
            throw runtime_error("ERROR in kmclifetime: specified number of charges is greater than the number of nodes. This conflicts with single occupation.");
        }

        fstream traj;
        fstream energyfile;

        cout << "Writing trajectory to " <<  _trajectoryfile << "." << endl; 
        traj.open ( _trajectoryfile.c_str(), fstream::out);
        traj << "#Simtime [s]\t Insertion\t Carrier ID\t Lifetime[s]\tSteps\t Last Segment\t x_travelled[nm]\t y_travelled[nm]\t z_travelled[nm]"<<endl;

        if(_do_carrierenergy){

            cout << "Tracking the energy of one charge carrier and exponential average with alpha=" << _alpha << " to "<<_energy_outputfile << endl;
            energyfile.open(_energy_outputfile.c_str(),fstream::out);
            energyfile << "Simtime [s]\tSteps\tCarrier ID\tEnergy_a="<<_alpha<<"[eV]"<<endl;
        }

        // Injection
        cout << endl << "injection method: " << _injectionmethod << endl;

        RandomlyCreateCharges();

        unsigned insertioncount = 0;
        unsigned long step=0;
        double simtime=0.0;

        std::vector<int> forbiddennodes;
        std::vector<int> forbiddendests;

        time_t now = time(0);
        tm* localtm = localtime(&now);
        cout << "Run started at " << asctime(localtm) << endl;

        double avlifetime=0.0;
        double meanfreepath=0.0;
        tools::vec difflength=tools::vec(0,0,0);
    
        double avgenergy=_carriers[0]->getCurrentEnergy();
        int     carrieridold=_carriers[0]->id;

        while (insertioncount < _insertions) {
            if ((time(NULL) - realtime_start) > _maxrealtime * 60. * 60.) {
                cout << endl << "Real time limit of " << _maxrealtime << " hours (" << int(_maxrealtime * 60 * 60 + 0.5) << " seconds) has been reached. Stopping here." << endl << endl;
                break;
            }

     
            double cumulated_rate = 0;

            for (unsigned int i = 0; i < _carriers.size(); i++) {
                cumulated_rate += _carriers[i]->getCurrentEscapeRate();
            }
            if (cumulated_rate == 0) { // this should not happen: no possible jumps defined for a node
                throw runtime_error("ERROR in kmclifetime: Incorrect rates in the database file. All the escape rates for the current setting are 0.");
            }
            // go forward in time
            double dt=Promotetime(cumulated_rate);

            if(_do_carrierenergy){
                bool print=false;
                if (_carriers[0]->id>carrieridold){
                    avgenergy=_carriers[0]->getCurrentEnergy();
                    print=true;
                    carrieridold=_carriers[0]->id;
                }
                else if(step%_outputsteps==0){
                    avgenergy=_alpha*_carriers[0]->getCurrentEnergy()+(1-_alpha)*avgenergy;
                    print=true;
                }
                if(print){
                    energyfile << simtime<<"\t"<<step <<"\t"<<_carriers[0]->id<<"\t"<<avgenergy<<endl;                  
                }
            }

            simtime += dt;
            step++;
            for (unsigned int i = 0; i < _carriers.size(); i++) {
                _carriers[i]->updateLifetime(dt);
                _carriers[i]->updateSteps(1);
                _carriers[i]->updateOccupationtime(dt);
            }

            ResetForbiddenlist(forbiddennodes);
            bool secondlevel=true;
            while (secondlevel){

                // determine which carrier will escape
                GNode* newnode=NULL;
                Chargecarrier* affectedcarrier=ChooseAffectedCarrier(cumulated_rate);

               

                if (CheckForbidden(affectedcarrier->getCurrentNodeId(), forbiddennodes)) {
                    continue;
                }

                // determine where it will jump to
                ResetForbiddenlist(forbiddendests);

                while (true) {
                    // LEVEL 2

                    newnode = NULL;
                    GLink* event=ChooseHoppingDest(affectedcarrier->getCurrentNode());

                    if (event->decayevent){
                       
                        avlifetime+=affectedcarrier->getLifetime();
                        meanfreepath+=tools::abs(affectedcarrier->dr_travelled);
                        difflength+=tools::elementwiseproduct(affectedcarrier->dr_travelled,affectedcarrier->dr_travelled);
                        traj << simtime<<"\t"<<insertioncount<< "\t"<< affectedcarrier->id<<"\t"<< affectedcarrier->getLifetime()<<"\t"<<affectedcarrier->getSteps()<<"\t"<< affectedcarrier->getCurrentNodeId()+1<<"\t"<<affectedcarrier->dr_travelled.getX()<<"\t"<<affectedcarrier->dr_travelled.getY()<<"\t"<<affectedcarrier->dr_travelled.getZ()<<endl;
                        if( tools::globals::verbose &&(_insertions<1500 ||insertioncount% (_insertions/1000)==0 || insertioncount<0.001*_insertions)){
                            std::cout << "\rInsertion " << insertioncount+1<<" of "<<_insertions;
                            std::cout << std::flush;
                        }
                        RandomlyAssignCarriertoSite(affectedcarrier);
                        affectedcarrier->resetCarrier();
                        insertioncount++;
                        affectedcarrier->id=_numberofcharges-1+insertioncount;
                        secondlevel=false;
                        break;
                            }
                    else{
                    newnode = _nodes[event->destination];
                    }

                    // check after the event if this was allowed
                    if (CheckForbidden(newnode->id, forbiddendests)) {
                        continue;
                    }

                    // if the new segment is unoccupied: jump; if not: add to forbidden list and choose new hopping destination
                    if (newnode->occupied) {
                        if (CheckSurrounded(affectedcarrier->getCurrentNode(), forbiddendests)) {     
                            AddtoForbiddenlist(affectedcarrier->getCurrentNodeId(), forbiddennodes);
                            break; // select new escape node (ends level 2 but without setting level1step to 1)
                        }
                        AddtoForbiddenlist(newnode->id, forbiddendests);
                        continue; // select new destination
                    } else {
                        affectedcarrier->jumpfromCurrentNodetoNode(newnode);
                        affectedcarrier->dr_travelled += event->dr;
                        AddtoJumplengthdistro(event,dt);
                        secondlevel=false;

                        break; // this ends LEVEL 2 , so that the time is updated and the next MC step started
                    }


                    // END LEVEL 2
                }
                // END LEVEL 1
            }
        }


        cout<<endl;
        cout << "Total runtime:\t\t\t\t\t"<< simtime << " s"<< endl;
        cout << "Total KMC steps:\t\t\t\t"<< step << endl;
        cout << "Average lifetime:\t\t\t\t"<<avlifetime/insertioncount<< " s"<<endl;
        cout << "Mean freepath\t l=<|r_x-r_o|> :\t\t"<<(meanfreepath/insertioncount)<< " nm"<<endl;
        cout << "Average diffusionlength\t d=sqrt(<(r_x-r_o)^2>)\t"<<sqrt(abs(difflength)/insertioncount)<< " nm"<<endl;
        cout<<endl;

        PrintJumplengthdistro();
        
        vector< ctp::Segment* >& seg = top->Segments();

        for (unsigned i = 0; i < seg.size(); i++) {
            double occupationprobability=_nodes[i]->occupationtime / simtime;
            seg[i]->setOcc(occupationprobability,_carriertype);
        }
        traj.close();
        if(_do_carrierenergy){
            energyfile.close();
        }
        return;
    }



    bool KMCLifetime::EvaluateFrame(ctp::Topology *top) {
        std::cout << "-----------------------------------" << std::endl;
        std::cout << "      KMCLIFETIME started" << std::endl;
        std::cout << "-----------------------------------" << std::endl << std::endl;

        // Initialise random number generator
        if (votca::tools::globals::verbose) {
            cout << endl << "Initialising random number generator" << endl;
        }
        std::srand(_seed); // srand expects any integer in order to initialise the random number generator
        _RandomVariable = tools::Random2();
        _RandomVariable.init(rand(), rand(), rand(), rand());
        LoadGraph(top);
        ReadLifetimeFile(_lifetimefile);
        
        
        
        if(_rates == "calculate")
        {
           cout << "Calculating rates (i.e. rates from state file are not used)." << endl;
            InitialRates();
        }
        else
        {
            cout << "Using rates from state file." << endl;
        }
        if(_probfile!=""){
            WriteDecayProbability(_probfile);
        }
        RunVSSM(top);

        time_t now = time(0);
        tm* localtm = localtime(&now);
        std::cout << "      KMCLIFETIME finished at:" <<asctime(localtm) <<  std::endl;

        return true;
    }
    
    }
}
