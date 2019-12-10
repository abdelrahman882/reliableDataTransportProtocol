//
// Created by abdelrahman on 12/5/19.
//

#ifndef SOCKETPROGRAMMING_PARAMS_H
#define SOCKETPROGRAMMING_PARAMS_H


class Params{
public:
    float cwnd ;
    float ssthresh = 64000;
    int dupACKcount = 0;
    int MSS = 500;
    bool retransmit=false;
    Params(){
        cwnd= 1 * MSS;
    }
};

#endif //SOCKETPROGRAMMING_PARAMS_H
