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

#include "kmcmultiple.h"
#include <votca/xtp/gnode.h>
#include <votca/tools/property.h>
#include <votca/tools/constants.h>
#include <boost/format.hpp>
#include <votca/ctp/topology.h>
#include <locale>


using namespace std;

namespace votca {
    namespace xtp {

    


void KMCMultiple::Initialize(tools::Property *options){
    std::string key = "options." + Identify();

    _runtime=options->ifExistsReturnElseThrowRuntimeError<double>(key+".runtime");
    _seed=options->ifExistsReturnElseThrowRuntimeError<int>(key+".seed");
    _numberofcharges=options->ifExistsReturnElseThrowRuntimeError<int>(key+".numberofcharges");
    _injection_name=options->ifExistsReturnElseThrowRuntimeError<std::string>(key+".injectionpattern");
  _intermediateoutput_frequency=options->ifExistsReturnElseReturnDefault<int>(key+".intermediateoutput",1E9);

        _maxrealtime=options->ifExistsReturnElseReturnDefault<double>(key+".maxrealtime",1E10);
        _trajectoryfile=options->ifExistsReturnElseReturnDefault<std::string>(key+".trajectoryfile","trajectory.csv");
        _temperature=options->ifExistsReturnElseReturnDefault<double>(key+".temperature",300);
        _rates=options->ifExistsReturnElseReturnDefault<std::string>(key+".rates","statefile");
        
     
        _injectionmethod = options->ifExistsReturnElseReturnDefault<std::string>(key+".injectionmethod","random");
	
        if (_injectionmethod != "random"){
	    cout << "WARNING in kmcmultiple: Unknown injection method. It will be set to random injection." << endl;
            _injectionmethod = "random";
        }
         _field = options->ifExistsReturnElseReturnDefault<tools::vec>(key+".field",tools::vec(0,0,0));
         double mtonm=1E9;
       _field /=mtonm ;//Converting from V/m to V/nm 
      
        _outputtime = options->ifExistsReturnElseReturnDefault<double>(key+".outputtime",0);
        _timefile = options->ifExistsReturnElseReturnDefault<std::string>(key+".timefile","timedependence.csv");
	
        std::string carriertype=options->ifExistsReturnElseReturnDefault<std::string>(key+".carriertype","e");
        _carriertype=StringtoCarriertype(carriertype);
     
        
        lengthdistribution = options->ifExistsReturnElseReturnDefault<double>(key+".jumplengthdist",0);
        if(lengthdistribution>0){
            dolengthdistributon=true;
        }

        return;
}      
        


void KMCMultiple::RunVSSM(ctp::Topology *top)
{

    int realtime_start = time(NULL);
    cout << endl << "Algorithm: VSSM for Multiple Charges" << endl;
    cout << "number of charges: " << _numberofcharges << endl;
    cout << "number of nodes: " << _nodes.size() << endl;
    
    bool checkifoutput=(_outputtime != 0);
    double nexttrajoutput=0;
//    double nexttrajoutput=_runtime;
    unsigned long maxsteps=_runtime;
    unsigned long outputstep=_outputtime;
    bool stopontime=false;
    

    if(_runtime > 100){ 
        cout << "stop condition: " << maxsteps << " steps." << endl;
        
        if(checkifoutput){
	    cout << "output frequency: ";
            cout << "every " << outputstep << " steps." << endl;
        }
    }
    else{
        stopontime =true;
        cout << "stop condition: " << _runtime << " seconds runtime." << endl;
       
        if(checkifoutput){
	    cout << "output frequency: ";
            cout << "every " << _outputtime << " seconds." << endl;
        }
    }
    cout << "(If you specify runtimes larger than 100 kmcmultiple assumes that you are specifying the number of steps for both runtime and outputtime.)" << endl;
    
    if(!stopontime && _outputtime != 0 && floor(_outputtime) != _outputtime){
        throw runtime_error("ERROR in kmcmultiple: runtime was specified in steps (>100) and outputtime in seconds (not an integer). Please use the same units for both input parameters.");
    }
    
    if(_numberofcharges > _nodes.size()){
        throw runtime_error("ERROR in kmcmultiple: specified number of charges is greater than the number of nodes. This conflicts with single occupation.");
    }

    fstream traj;
    fstream tfile;
    
    if(checkifoutput){   
        
        cout << "Writing trajectory to " << _trajectoryfile << "." << endl; 
        traj.open (_trajectoryfile.c_str(), fstream::out);
    
        traj << "'time[s]'\t";
        traj << "'steps'\t";
        for(unsigned int i=0; i<_numberofcharges; i++){
            traj << "'carrier" << i+1 << "_x'\t";    
            traj << "'carrier" << i+1 << "_y'\t";    
            traj << "'carrier" << i+1 << "_z";    
            if(i<_numberofcharges-1){
                traj << "'\t";
            }
        }
        traj << endl;

        cout << "Writing time dependence of energy and mobility to " << _timefile << "." << endl; 
        tfile.open (_timefile.c_str(), fstream::out);
        tfile << "time[s]\t steps\tenergy_per_carrier[eV]\tmobility[nm**2/Vs]\tdistance_fielddirection[nm]\tdistance_absolute[nm]" << endl;
        
    }

    double absolute_field = tools::abs(_field);

    RandomlyCreateCharges();
    vector<tools::vec> startposition(_numberofcharges,tools::vec(0.0));
    for(unsigned int i=0; i<_numberofcharges; i++) {
        startposition[i]=_carriers[i]->getCurrentPosition();
    }
    
    
    traj << 0 << "\t";
    traj << 0 << "\t";
    for(unsigned int i=0; i<_numberofcharges; i++) {
        traj << startposition[i].getX()  << "\t";
        traj << startposition[i].getY() << "\t";
        traj << startposition[i].getZ();
        if (i<_numberofcharges-1) {
            traj << "\t";
        }
        else{
            traj << endl;
        }
    }
  
    vector<int> forbiddennodes;
    vector<int> forbiddendests;
    
    tools::matrix avgdiffusiontensor;
    avgdiffusiontensor.ZeroMatrix();
    
    unsigned long diffusionresolution=1000;
    double simtime = 0.0;
    unsigned long step = 0;
    
    while(((stopontime && simtime < _runtime) || (!stopontime && step < maxsteps))){
        
        if((time(NULL) - realtime_start) > _maxrealtime*60.*60.)
        {
            cout  << endl << "Real time limit of " << _maxrealtime << " hours (" << int(_maxrealtime*60*60 +0.5) <<" seconds) has been reached. Stopping here." << endl << endl;
            break;
        }
        
        double cumulated_rate = 0;
        for(unsigned int i=0; i<_numberofcharges; i++)
        {
            cumulated_rate += _carriers[i]->getCurrentEscapeRate();
        }
        if(cumulated_rate == 0)
        {   // this should not happen: no possible jumps defined for a node
            throw runtime_error("ERROR in kmcmultiple: Incorrect rates in the database file. All the escape rates for the current setting are 0.");
        }
        
        double dt =Promotetime(cumulated_rate);
        
        simtime += dt;
        step++;
        if(tools::globals::verbose) {cout << "simtime += " << dt << endl << endl;}
        
        
        
        
        for(unsigned int i=0; i<_numberofcharges; i++)
        {
            _carriers[i]->updateOccupationtime(dt);
        }

        
        ResetForbiddenlist(forbiddennodes);
        bool level1step = true;
        while(level1step){

            // determine which electron will escape
            
            GNode* newnode= NULL;
            Chargecarrier* affectedcarrier=ChooseAffectedCarrier(cumulated_rate); 
            
            if(CheckForbidden(affectedcarrier->getCurrentNodeId(), forbiddennodes)) {continue;}
            
            // determine where it will jump to
            ResetForbiddenlist(forbiddendests);
            while(true){
            // LEVEL 2
                if(tools::globals::verbose) {cout << "There are " <<affectedcarrier->getCurrentNode()->events.size() << " possible jumps for this charge:"; }
              

                GLink* event=ChooseHoppingDest(affectedcarrier->getCurrentNode());
                newnode = _nodes[event->destination];
                if(newnode==affectedcarrier->getCurrentNode()){
                    cout<<event->dr<<endl;
                }

                if(newnode == NULL){
                    if(tools::globals::verbose) {
                        cout << endl << "Node " << affectedcarrier->getCurrentNodeId()+1  << " is SURROUNDED by forbidden destinations and zero rates. "
                                "Adding it to the list of forbidden nodes. After that: selection of a new escape node." << endl; 
                    }
                    AddtoForbiddenlist(affectedcarrier->getCurrentNodeId(), forbiddennodes);
                    break; // select new escape node (ends level 2 but without setting level1step to 1)
                }
                if(tools::globals::verbose) {cout << endl << "Selected jump: " << newnode->id+1 << endl; }
                
                // check after the event if this was allowed
                if(CheckForbidden(newnode->id, forbiddendests)){
                    if(tools::globals::verbose) {cout << "Node " << newnode->id+1  << " is FORBIDDEN. Now selection new hopping destination." << endl; }
                    continue;
                }

                // if the new segment is unoccupied: jump; if not: add to forbidden list and choose new hopping destination
                if(newnode->occupied){
                    if(CheckSurrounded(affectedcarrier->getCurrentNode(), forbiddendests)){
                        if(tools::globals::verbose) {
                            cout << "Node " << affectedcarrier->getCurrentNodeId()+1  << " is SURROUNDED by forbidden destinations. "
                                    "Adding it to the list of forbidden nodes. After that: selection of a new escape node." << endl; 
                        }
                        AddtoForbiddenlist(affectedcarrier->getCurrentNodeId(), forbiddennodes);
                        break; // select new escape node (ends level 2 but without setting level1step to 1)
                    }
                    if(tools::globals::verbose) {cout << "Selected segment: " << newnode->id+1 << " is already OCCUPIED. Added to forbidden list." << endl << endl;}
                    AddtoForbiddenlist(newnode->id, forbiddendests);
                    if(tools::globals::verbose) {cout << "Now choosing different hopping destination." << endl; }
                    continue; // select new destination
                }
                else{
                    affectedcarrier->jumpfromCurrentNodetoNode(newnode);
                    affectedcarrier->dr_travelled +=event->dr;
                    AddtoJumplengthdistro(event,dt);
                    level1step = false;
                    if(tools::globals::verbose) {cout << "Charge has jumped to segment: " << newnode->id+1 << "." << endl;}
                    
                    break; // this ends LEVEL 2 , so that the time is updated and the next MC step started
                }

                if(tools::globals::verbose) {cout << "." << endl;}
            // END LEVEL 2
            }
        // END LEVEL 1
        }    
              
        //outputstuff
        
        if(step%diffusionresolution==0){     
            for(unsigned int i=0; i<_numberofcharges; i++){
                avgdiffusiontensor += (_carriers[i]->dr_travelled)|(_carriers[i]->dr_travelled);
            }
        }
        
        if (step != 0 && step % _intermediateoutput_frequency == 0) {

            if (absolute_field == 0) {
                unsigned long diffusionsteps = step / diffusionresolution;
                tools::matrix result = avgdiffusiontensor / (diffusionsteps * 2 * simtime * _numberofcharges);
                cout << endl << "Step: " << step << " Diffusion tensor averaged over all carriers (nm^2/s):" << endl << result << endl;
            } else {
                double average_mobility = 0;
                cout << endl << "Mobilities (nm^2/Vs): " << endl;
                for (unsigned int i = 0; i < _numberofcharges; i++) {
                    tools::vec velocity = _carriers[i]->dr_travelled / simtime;
                    cout << std::scientific << "    charge " << i + 1 << ": mu=" << (velocity * _field) / (absolute_field * absolute_field) << endl;
                    average_mobility += (velocity * _field) / (absolute_field * absolute_field);
                }
                average_mobility /= _numberofcharges;
                cout << std::scientific << "  Overall average mobility in field direction <mu>=" << average_mobility << " nm^2/Vs  " << endl;
            }
        }

        
        
        if(checkifoutput) { 
            bool outputsteps=(!stopontime && step%outputstep==0);
            bool outputtime=(stopontime && simtime>nexttrajoutput);
            if(outputsteps || outputtime){
                // write to trajectory file
                nexttrajoutput = simtime + _outputtime;
                traj << simtime << "\t";
            traj << step << "\t";
                for(unsigned int i=0; i<_numberofcharges; i++) {
                    traj << startposition[i].getX() + _carriers[i]->dr_travelled.getX() << "\t";
                    traj << startposition[i].getY() + _carriers[i]->dr_travelled.getY() << "\t";
                    traj << startposition[i].getZ() + _carriers[i]->dr_travelled.getZ();
                    if (i<_numberofcharges-1) {
                        traj << "\t";
                    }
                    else{
                        traj << endl;
                    }
                }
                
              
                double currentenergy = 0;
                double currentmobility = 0;
                tools::vec dr_travelled_current = tools::vec (0,0,0);
                double dr_travelled_field=0.0;
                tools::vec avgvelocity_current = tools::vec(0,0,0);
                if(absolute_field != 0){
                    for(unsigned int i=0; i<_numberofcharges; i++){
                        dr_travelled_current += _carriers[i]->dr_travelled;
                        currentenergy += _carriers[i]->getCurrentEnergy();
                    }
                    dr_travelled_current /= _numberofcharges;
                    currentenergy /= _numberofcharges;
                    avgvelocity_current = dr_travelled_current/simtime; 
                    currentmobility = (avgvelocity_current*_field) /absolute_field/absolute_field;
                    dr_travelled_field=(dr_travelled_current*_field)/absolute_field;
                }
                
                tfile << simtime << "\t"<< step << "\t"<< currentenergy << "\t" << currentmobility << "\t" <<
                        dr_travelled_field << "\t" << tools::abs(dr_travelled_current)<<"\t"<< endl;
              
            }
        }
      
    }//KMC 
    
    
    
    if(checkifoutput)
    {   
        traj.close();
        tfile.close();
    }

    
    vector< ctp::Segment* >& seg = top->Segments();
    for (unsigned i = 0; i < seg.size(); i++) {
            double occupationprobability=_nodes[i]->occupationtime / simtime;
            seg[i]->setOcc(occupationprobability,_carriertype);
        }

    cout << endl << "finished KMC simulation after " << step << " steps." << endl;
    cout << "simulated time " << simtime << " seconds." << endl;
    cout << "runtime: ";
    cout << endl << endl;
    
    tools::vec avg_dr_travelled = tools::vec (0,0,0);
    for(unsigned int i=0; i<_numberofcharges; i++){
        cout << std::scientific << "    charge " << i+1 << ": " << _carriers[i]->dr_travelled/simtime << endl;
        avg_dr_travelled += _carriers[i]->dr_travelled;
    }
    avg_dr_travelled /= _numberofcharges;
    
    tools::vec avgvelocity = avg_dr_travelled/simtime; 
    cout << std::scientific << "  Overall average velocity (nm/s): " << avgvelocity << endl;

    cout << endl << "Distances travelled (nm): " << endl;
    for(unsigned int i=0; i<_numberofcharges; i++){
        cout << std::scientific << "    charge " << i+1 << ": " << _carriers[i]->dr_travelled << endl;
    }
    
    // calculate mobilities
   
    if (absolute_field != 0){
        double average_mobility = 0;
        cout << endl << "Mobilities (nm^2/Vs): " << endl;
        for(unsigned int i=0; i<_numberofcharges; i++){
            tools::vec velocity = _carriers[i]->dr_travelled/simtime;
            cout << std::scientific << "    charge " << i+1 << ": mu=" << (velocity*_field)/(absolute_field*absolute_field) << endl;
            average_mobility += (velocity*_field) /(absolute_field*absolute_field);
        }
        average_mobility /= _numberofcharges;
        cout << std::scientific << "  Overall average mobility in field direction <mu>=" << average_mobility << " nm^2/Vs  " << endl;
      }
    cout << endl;
    
    // calculate diffusion tensor
    unsigned long diffusionsteps=step/diffusionresolution;
    avgdiffusiontensor /= (diffusionsteps*2*simtime*_numberofcharges);
    cout<<endl<<"Diffusion tensor averaged over all carriers (nm^2/s):" << endl << avgdiffusiontensor << endl;
    
  

    tools::matrix::eigensystem_t diff_tensor_eigensystem;
    cout<<endl<<"Eigenvalues: "<<endl<<endl;
    avgdiffusiontensor.SolveEigensystem(diff_tensor_eigensystem);
    for(int i=0; i<=2; i++)
    {
        cout<<"Eigenvalue: "<<diff_tensor_eigensystem.eigenvalues[i]<<endl<<"Eigenvector: ";
               
        cout<<diff_tensor_eigensystem.eigenvecs[i].x()<<"   ";
        cout<<diff_tensor_eigensystem.eigenvecs[i].y()<<"   ";
        cout<<diff_tensor_eigensystem.eigenvecs[i].z()<<endl<<endl;
    }
    
    // calculate average mobility from the Einstein relation
    if (absolute_field == 0){
        cout << "The following value is calculated using the Einstein relation and assuming an isotropic medium" << endl;
       double avgD  = 1./3. * (diff_tensor_eigensystem.eigenvalues[0] + diff_tensor_eigensystem.eigenvalues[1] + diff_tensor_eigensystem.eigenvalues[2] );
       double average_mobility = std::abs(avgD / tools::conv::kB / _temperature);
       cout << std::scientific << "  Overall average mobility <mu>=" << average_mobility << " nm^2/Vs "  << endl;
    }
    
  PrintJumplengthdistro();
    

    
    return;
}




bool KMCMultiple::EvaluateFrame(ctp::Topology *top){
    std::cout << std::endl;      
    std::cout << "-----------------------------------" << std::endl;      
    std::cout << "      KMC FOR MULTIPLE CHARGES" << std::endl;
    std::cout << "-----------------------------------" << std::endl << std::endl;      
 
    // Initialise random number generator
    if(tools::globals::verbose) { cout << endl << "Initialising random number generator" << endl; }
    srand(_seed); // srand expects any integer in order to initialise the random number generator
    _RandomVariable = tools::Random2();
    _RandomVariable.init(rand(), rand(), rand(), rand());
    
    LoadGraph(top);
    
        
        if(_rates == "calculate")
        {
            cout << "Calculating rates (i.e. rates from state file are not used)." << endl;
            InitialRates();
        }
        else
        {
            cout << "Using rates from state file." << endl;
        }
    

    RunVSSM(top);

    return true;
}
    
    }
}
