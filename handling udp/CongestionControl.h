//
// Created by abdelrahman on 12/5/19.
//

#ifndef SOCKETPROGRAMMING_CONGESTIONCONTROL_H
#define SOCKETPROGRAMMING_CONGESTIONCONTROL_H

#include "State.h"
#include "Actions.h"
#include "Params.h"

#define newACK "new ACK"
#define dupACK "dup ACK"
#define TO "timeout"
#define dupACK3 "3 dupAck"
#define  cwndGss "cwnd>=ssthresh"
#define NUM_OF_ACTIONS 5

class CongestionControl {
private:
    State * currState ;
    Actions * acs;
public:
    Params * p ;
    CongestionControl(){
        CongestionControl *curr = this;
        string ls [] = {newACK,TO,dupACK,dupACK3,cwndGss};
        acs = new Actions(ls,NUM_OF_ACTIONS);
        this->p = new Params();
        State * slowStart= new State(p,NUM_OF_ACTIONS,"slow start"), *conAv= new State(p,NUM_OF_ACTIONS,"congestion avoidance") , * fastRec = new State(p,NUM_OF_ACTIONS,"fast recovery");
        this->currState = slowStart;


        //setting slowStart

        slowStart->setNextState(acs->getActionID(newACK),slowStart,[](Params * pp)->void{
            pp->cwnd += pp->MSS;
            pp->dupACKcount =0;
            //Transmit new Seg
        });
        slowStart->setNextState(acs->getActionID(dupACK),slowStart,[]( Params * pp)->void{
            pp->dupACKcount++;
        });
        slowStart->setNextState(acs->getActionID(TO),slowStart,[](Params * pp)->void{
            pp->ssthresh = pp->cwnd /2;
            pp->cwnd = 1 * pp->MSS;
            pp->dupACKcount=0;
            pp->retransmit= true;
        });
        slowStart->setNextState(acs->getActionID(dupACK3),fastRec,[](Params * pp)->void{
            pp->ssthresh = pp->cwnd /2;
            pp->cwnd = pp->ssthresh+ 3 * pp->MSS;
            pp->retransmit= true;
        });
        slowStart->setNextState(acs->getActionID(cwndGss),conAv,[](Params * pp)->void{
            //NOTHING
        });



        //congestion avoidance
        conAv->setNextState(acs->getActionID(newACK),conAv,[](Params * pp)->void{
            pp->cwnd += pp->MSS * (pp->MSS/pp->cwnd);
            pp->dupACKcount =0;
            //Transmit new Seg
        });
        conAv->setNextState(acs->getActionID(dupACK),conAv,[](Params * pp)->void{
            pp->dupACKcount++;
        });
        conAv->setNextState(acs->getActionID(TO),slowStart,[](Params * pp)->void{
            pp->ssthresh = pp->cwnd /2;
            pp->cwnd = 1 * pp->MSS;
            pp->dupACKcount=0;
            pp->retransmit = true;
        });
        conAv->setNextState(acs->getActionID(dupACK3),fastRec,[](Params * pp)->void{
            pp->ssthresh = pp->cwnd /2;
            pp->cwnd = pp->ssthresh+ 3 * pp->MSS;
            pp->retransmit = true;
        });
        conAv->setNextState(acs->getActionID(cwndGss),conAv,[](Params * pp)->void{
            //NOTHING
        });




        //fast  recovery
        fastRec->setNextState(acs->getActionID(newACK),conAv,[](Params * pp)->void{
            pp->cwnd += pp->ssthresh;
            pp->dupACKcount =0;
            //Transmit new Seg
        });
        fastRec->setNextState(acs->getActionID(dupACK),fastRec,[](Params * pp)->void{
            pp->cwnd += pp->MSS;
            //transmit new seg
        });
        fastRec->setNextState(acs->getActionID(TO),slowStart,[](Params * pp)->void{
            pp->ssthresh = pp->cwnd /2;
            pp->cwnd = 1 ;
            pp->dupACKcount=0;
            pp->retransmit = true;
        });
        fastRec->setNextState(acs->getActionID(dupACK3),fastRec,[](Params * pp)->void{
            //NOTHING
        });
        fastRec->setNextState(acs->getActionID(cwndGss),fastRec,[](Params * pp)->void{
            //NOTHING
        });



    }
//    void control(string action){
//        currState = currState->getNextState(acs->getActionID(action));
//    }
    void dupAck(){
        currState = currState->getNextState(acs->getActionID(dupACK));
        if(p->dupACKcount==3){
            currState = currState->getNextState(acs->getActionID(dupACK3));
            if(p->cwnd >= p->ssthresh){
                currState = currState->getNextState(acs->getActionID(cwndGss));
            }

        }

    }
    void newAck(){
        currState = currState->getNextState(acs->getActionID(newACK));
        if(p->cwnd >= p->ssthresh){
            currState = currState->getNextState(acs->getActionID(cwndGss));
        }
    }


};


#endif //SOCKETPROGRAMMING_CONGESTIONCONTROL_H
