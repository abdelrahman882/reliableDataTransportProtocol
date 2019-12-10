//
// Created by abdelrahman on 12/5/19.
//
#include <iostream>
#include <stab.h>
using namespace std;


/* Data-only packets */
struct packet {
     /* Header */
     uint16_t cksum; /* Optional bonus part */
     uint16_t len;
     uint32_t seqno;
     /* Data */
     char data[500]; /* Not always 500 bytes, can be less */
 };


/* Ack-only packets are only 8 bytes */
struct ack_packet {
     uint16_t cksum; /* Optional bonus part */
     uint16_t len;
     uint32_t ackno;
 };
//
//void pTOb(char * buffer , packet * p ){
//    buffer[0]=(uint32_t) p->cksum >> 8;
//    buffer[1] =(uint32_t) p->cksum ;
//    buffer[2] = (uint32_t)p->len >>8;
//    buffer[3]= (uint32_t)p->len;
//    buffer[4]=(uint32_t)p->seqno>>24;
//    buffer[5]=(uint32_t)p->seqno>>16;
//    buffer[6]=(uint32_t)p->seqno>>8;
//    buffer[7]=(uint32_t)p->seqno;
//    for (int i=0;i<500;i++) {
//        buffer[8+i] = p->data[i];
//    }
//}
//void bTOp(char * buffer , packet * p ){
//    p->cksum = (uint32_t)buffer[0] << 8 | (uint32_t) buffer[1];
//    p->len=(uint32_t)buffer[2] << 8 | (uint32_t) buffer[3];
//    p->seqno = (uint32_t)buffer[4] << 24 | (uint32_t) buffer[5] <<16|(uint32_t)buffer[6] << 8 | (uint32_t) buffer[7];
//    for (int i=0;i<500;i++) {
//        p->data[i] = buffer[8+i];
//    }
//}
//
//void apTOb(char * buffer , ack_packet* p ){
//    buffer[0]=(uint32_t) p->cksum >> 8;
//    buffer[1] = (uint32_t)p->cksum ;
//    buffer[2] = (uint32_t)p->len >>8;
//    buffer[3]= (uint32_t)p->len;
//    buffer[4]=(uint32_t)p->ackno>>24;
//    buffer[5]=(uint32_t)p->ackno>>16;
//    buffer[6]=(uint32_t)p->ackno>>8;
//    buffer[7]=(uint32_t)p->ackno;
//}
//
//void bTOap(char * buffer , ack_packet * p ){
//    p->cksum = (uint32_t)buffer[0] << 8 | (uint32_t) buffer[1];
//    p->len=(uint32_t)buffer[2] << 8 | (uint32_t) buffer[3];
//    p->ackno = (uint32_t)buffer[4] << 32 | (uint32_t) buffer[5] <<16|(uint32_t)buffer[6] << 8 | (uint32_t) buffer[7];
//
//}
