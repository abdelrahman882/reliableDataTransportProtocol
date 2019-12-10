//
// Created by abdelrahman on 12/5/19.
//

#ifndef SOCKETPROGRAMMING_STATE_H
#define SOCKETPROGRAMMING_STATE_H


#include "Actions.h"
#include <unordered_map>
#include "Params.h"

class State {
public:
    struct stateAction{
        State *next;
        void (*updater) (Params * p );
    };
private:
    stateAction **nextState ;
    Params * p ;
public:
    string name ="non";
    State(Params * pp, int max,string n){
        this->name = n;
        this->p  = pp;
        this->nextState = new stateAction*[max];
    }
    void setNextState(int ac , State * next ,void (*update)(Params *)){
        stateAction * st = new stateAction();
        st->next = next;
        st->updater = update;
        this->nextState[ac]= st;
    }
    State * getNextState(int ac){

        stateAction * cur = *(this->nextState + ac);
        (*cur->updater)(p);
        if(this != cur->next)
        printf("\non state %s action %d new params cwnd, ssthres, dupCnt: %f %f %d next state :%s \n"
                ,this->name.c_str() , ac , p->cwnd,p->ssthresh,p->dupACKcount , cur->next->name.c_str() );
        return (cur)->next;
    }
};


#endif //SOCKETPROGRAMMING_STATE_H
