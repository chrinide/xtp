
/*
 * File:   lambdaout.cc
 * Author: mayfalk
 *
 * Created on May 20, 2011, 12:21 PM
 */


#include <stdlib.h>
#include "lambdaout.h"
#include <math.h>
#include <list>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <fstream>

void CalcLambdaOut::Initialize(QMTopology *top, Property *options) {
    _options = options;

    if (options->exists("options.lambda_params.lambda_method")) {
       if (options->get("options.lambda_params.lambda_method").as<string > () == "constant") {
        //_lambda_method = &CalcLambdaOut::const_lambda;
        if (options->exists("options.lambda_params.lambdaout")) {
            cout << "Thank you for choosing the constant lambda method"<<endl;
            _lambda_const = options->get("options.lambda_params.lambdaout").as<double>();
            cout << "Using a constant lambda outer of " << _lambda_const << endl;
        } else {
            _lambda_const = 0.0;
            cout << "Warning: no lambda outer sphere defined, using lambdaout=0." << endl;
        }
    } else if (options->get("options.lambda_params.lambda_method").as<string > () == "spheres") {
        //_lambda_method = &CalcLambdaOut::spheres_lambda;
        cout << "Thank you for choosing the spheres lambda method"<<endl;
        if (options->exists("options.lambda_params.pekar")) {
            _pekar = options->get("options.lambda_params.pekar").as<double>();
            cout << "Using a Pekar factor of " << _pekar << endl;
        } else {
            _pekar = 0.05;
            cout << "Warning: no Pekar factor defined, using pekar =0.05" << endl;
        }
    }
    else if (options->get("options.lambda_params.lambda_method").as<string > () == "dielectric") {
        //_lambda_method = &CalcLambdaOut::dielectric_lambda;
        cout << "Thank you for choosing the dielectric lambda method"<<endl;
        if (options->exists("options.lambda_params.pekar")) {
            _pekar = options->get("options.lambda_params.pekar").as<double>();
            cout << "Using a Pekar factor of " << _pekar << endl;
        } else {
            _pekar = 0.05;
            cout << "Warning: no Pekar factor defined, using pekar =0.05" << endl;
        }
    } else throw std::runtime_error("Error in CalcLambdaOut::Initialize : no such lambda method, should be constant, spheres or dielectric");
    } else throw std::runtime_error("Error in CalcLambdaOut::Initialize : no lambda method specified, should be constant, spheres or dielectric");
}

bool CalcLambdaOut::EvaluateFrame(QMTopology *top) {
        if (_options->get("options.lambda_params.lambda_method").as<string > () == "constant") {
            const_lambda(top);
           }
        else if (_options->get("options.lambda_params.lambda_method").as<string > () == "spheres") {
           spheres_lambda(top);
        }
        else if (_options->get("options.lambda_params.lambda_method").as<string > () == "dielectric") {
           dielectric_lambda(top);
       }
    return true;
}


    void CalcLambdaOut::const_lambda(QMTopology *top) {
        QMNBList& nblist=top->nblist();
        for (QMNBList::iterator ipair = nblist.begin(); ipair != nblist.end(); ++ipair) {
        QMPair *pair = *ipair;
        QMCrgUnit *crg1 = pair->Crg1();
        QMCrgUnit *crg2 = pair->Crg2();
        double lambda = _lambda_const;
        pair->setLambdaOuter(lambda);
        cout << "lambda out [eV] for pair " << crg1->getId() << " and " << crg2->getId() << " is " << lambda << "\n";
   }
    }

    void CalcLambdaOut::spheres_lambda(QMTopology *top) {
        QMNBList& nblist=top->nblist();
        for (QMNBList::iterator ipair = nblist.begin(); ipair != nblist.end(); ++ipair) {

        QMPair *pair = *ipair;
        QMCrgUnit *crg1 = pair->Crg1();
        QMCrgUnit *crg2 = pair->Crg2();
            double R_one = crg1->getType()->getOptions()->get("R_lambda").as<double>();
            double R_two = crg2->getType()->getOptions()->get("R_lambda").as<double>();

            double distance = pair->dist();
            double e = 1.602176487e-19;
            double epsilon_zero = 8.85418782e-12;
            double nanometer = 1.0e-09;
            double lambda = _pekar * e / (4.0 * M_PI * epsilon_zero)*(1.0 / (2.0 * R_one * nanometer) + 1.0 / (2.0 * R_two * nanometer) - 1.0 / (distance * nanometer));
        pair->setLambdaOuter(lambda);
        cout << "lambda out [eV] for pair " << crg1->getId() << " and " << crg2->getId() << " at distance "<< distance << " is " << lambda << "\n";
    }
     }

    void CalcLambdaOut::dielectric_lambda(QMTopology *top) {
        QMNBList& nblist=top->nblist();
        for (QMNBList::iterator ipair = nblist.begin(); ipair != nblist.end(); ++ipair) {
        QMPair *pair = *ipair;
        double distance = pair->dist();
        QMCrgUnit *crg1 = pair->Crg1();
        QMCrgUnit *crg2 = pair->Crg2();
        double lambda = 0.0;
           
            vector<QMCrgUnit *> lcharges = top->CrgUnits();
            Topology atop;
            atop.setBox(top->getBox());
            //Add the beads of charge units i and j to the atop
            Molecule *molcrg1 = top->AddAtomisticBeads(crg1, &atop);
            Molecule *molcrg2 = top->AddAtomisticBeads(crg2, &atop);
            //Add the rest of the beads to atop
            for (vector<QMCrgUnit *>::iterator itl = lcharges.begin(); itl != lcharges.end(); itl++) {
                if (crg1->getId() == (*itl)->getId()) continue;
                if (crg2->getId() == (*itl)->getId()) continue;
                top->AddAtomisticBeads(*itl, &atop);
            }
           //Loop over all beads exterior to crgunits i and j
            
            for (BeadContainer::iterator ibead = atop.Beads().begin(); ibead != atop.Beads().end(); ++ibead) {
                Bead *bk = *ibead;
                                
                if (bk->getMolecule()->getUserData<QMCrgUnit>()->getId() == crg1->getId()) continue;
                if (bk->getMolecule()->getUserData<QMCrgUnit>()->getId() == crg2->getId()) continue;
                //cout << "chosen bead number:" << bk->getMolecule()->getUserData<QMCrgUnit>()->getId() << endl;

                double Dx = 0.0;
                double Dy = 0.0;
                double Dz = 0.0;
                double nanometer = 1.0e-09;
                double nanometer3 = 1.0e-27;
                double elementary_charge = 1.60217646e-19;
                double epsilon_zero = 8.85418782e-12;
                vec bcs, diff , dist;
                //Get shortest distance to crgunit i TOP OR ATOP???
                //diff = top->BCShortestConnection(crg1->GetCom(), bk->getMolecule()->getUserData<QMCrgUnit>()->GetCom());
                 bcs = atop.BCShortestConnection(crg1->GetCom(), bk->getMolecule()->getUserData<QMCrgUnit>()->GetCom());
                 dist =  bk->getMolecule()->getUserData<QMCrgUnit>()->GetCom() - crg1->GetCom();
                 diff = bcs - dist;
                //Loop over all bead of crgunit i
                for (int bi = 0; bi < molcrg1->BeadCount(); ++bi) {
                    Bead *beadi = molcrg1->getBead(bi);
                    //Compute deltaQ
                    double charge_of_bead_i_charged = crg1->getType()->GetCrgUnit().getChargesCrged()->mpls[bi];
                    double charge_of_bead_i_neutral = crg1->getType()->GetCrgUnit().getChargesNeutr()->mpls[bi];
                    double delta_Q = charge_of_bead_i_charged - charge_of_bead_i_neutral;
                    vec r_v = bk->getPos()-(beadi->getPos() + diff);
                    double r = abs(r_v);
                    double r3 = r * r * r;
                    //cout << "delta Q:" << delta_Q << "|r3: " <<r3 << "|r_v.getX "<<r_v.getX()<<endl;
                    Dx = Dx + elementary_charge * delta_Q / (4.0 * M_PI * r3 * nanometer3) * r_v.getX() * nanometer;
                    Dy = Dy + elementary_charge * delta_Q / (4.0 * M_PI * r3 * nanometer3) * r_v.getY() * nanometer;
                    Dz = Dz + elementary_charge * delta_Q / (4.0 * M_PI * r3 * nanometer3) * r_v.getZ() * nanometer;
                }
                //cout << "D due to mol i:"<<D<<endl;
                //Get shortest distance to crgunit j TOP OR ATOP???
                //diff = top->BCShortestConnection(crg2->GetCom(), bk->getMolecule()->getUserData<QMCrgUnit>()->GetCom());
                 
                bcs = atop.BCShortestConnection(crg2->GetCom(), bk->getMolecule()->getUserData<QMCrgUnit>()->GetCom());
                dist =  bk->getMolecule()->getUserData<QMCrgUnit>()->GetCom() - crg2->GetCom();
                diff = bcs - dist;
                //Loop over all bead of crgunit j
                for (int bj = 0; bj < molcrg2->BeadCount(); ++bj) {
                    Bead *beadj = molcrg2->getBead(bj);
                    //Compute deltaQ
                    double charge_of_bead_j_charged = crg2->getType()->GetCrgUnit().getChargesCrged()->mpls[bj];
                    double charge_of_bead_j_neutral = crg2->getType()->GetCrgUnit().getChargesNeutr()->mpls[bj];
                    double delta_Q = charge_of_bead_j_neutral - charge_of_bead_j_charged;
                    vec r_v = bk->getPos()-(beadj->getPos() + diff);
                    double r = abs(r_v);
                    double r3 = r * r * r;
                  //  cout << "delta Q:" << delta_Q << "|r3: " <<r3 << "|r_v.getX "<<r_v.getX()<<endl;
                    Dx = Dx + elementary_charge * delta_Q / (4.0 * M_PI * r3 * nanometer3) * r_v.getX() * nanometer;
                    Dy = Dy + elementary_charge * delta_Q / (4.0 * M_PI * r3 * nanometer3) * r_v.getY() * nanometer;
                    Dz = Dz + elementary_charge * delta_Q / (4.0 * M_PI * r3 * nanometer3) * r_v.getZ() * nanometer;
                }
                //cout << "Dx :"<<Dx<<endl; cout << "Dy :"<<Dy<<endl; cout << "Dz :"<<Dz<<endl;

                //cout << "box volume:"<<atop.BoxVolume()<< " and beads:"<<atop.BeadCount()<<endl;
                lambda = lambda + (Dx*Dx+Dy*Dy+Dz*Dz)  * _pekar * atop.BoxVolume()*nanometer3/atop.BeadCount()  * 1/(2.0 * epsilon_zero * elementary_charge);
            }
        pair->setLambdaOuter(lambda);
        cout << "lambda out [eV] for pair " << crg1->getId() << " and " << crg2->getId() <<" at distance "<< distance << " is " << lambda << "\n";
       }
    }



