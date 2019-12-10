//
// Created by abdelrahman on 12/6/19.
//

// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include "Packets.cpp"
#include "CongestionControl.h"
#include <mutex>
#include <fstream>
#define PORT   8080
#define MAXLINE 508
#define GBN true
#define ALPHA 0.125
#define  BETA 0.25
class node{
public:
    int v = -1;
    int seqNum = -1;
    node*next = nullptr;
    bool acked= false;
    time_t stamp = 0;

};
void markAcked(node * head, int a){

    while(head->next != nullptr){
        if(head->v == a ){
            head->acked=true;
            return;
        }
        head = head->next;
    }

}

struct thPar{
    int sfd=1;
    string *path = new string ();
    struct  sockaddr_in * cliaddr = nullptr;
    int len=0;
    CongestionControl * cc = new CongestionControl();
    int lastAck =-1;
    std::mutex *ccm = new std::mutex();
    bool done = false;
    node * head = new node(); //this list is the window head ;
    int LLlen=0;
    time_t timeoutInterval =1000;


};
long getErtt(long prevertt,float alpha , long rtt ){
    return (1-alpha) * prevertt + alpha * rtt;
}
long getDevRTT(long prevDevrtt,float beta , long rtt , long ertt){
    return (1-beta) * prevDevrtt + beta * abs(rtt-ertt);
}
time_t updateTO(long ERTT , long DevRTT){
    return ERTT + 4*DevRTT;
}

void *rec(void * arg) {   //receive and throw in the queue
    thPar *p = ((thPar *) arg);
    auto * ack = new ack_packet();
    int len, n=-1;
    len = sizeof(*p->cliaddr);
    while(!p->done) {
        n=-1;
        while (n==-1)
        n = recvfrom(p->sfd, ack, sizeof(ack_packet),
                     MSG_WAITALL, (struct sockaddr *) p->cliaddr,
                     reinterpret_cast<socklen_t *>(&p->len));
    //    printf("ack rec with ack no : %d\n", ack->ackno);
        l1:p->ccm->lock();
            if(p->head->next== nullptr){
                //printf("unexpected logic");
                p->ccm->unlock();
                goto l1;
            }

            int expAck =p->head->next->seqNum+1; //the expected ack
       // printf("exp : %d\n", expAck);


            if(ack->ackno >= expAck && GBN){            //GBN
                p->cc->newAck();
                    while (p->head->next != nullptr && p->head->next->next != nullptr&&p->head->next->seqNum  <ack->ackno){
                        node *temp = p->head->next;
                        p->head->next = p->head->next->next;
                        p->LLlen--;
                        delete temp;// deleting the acked node
                        
                    }

            } else if(ack->ackno < expAck && GBN){
                p->cc->dupAck();
            } else if (ack->ackno == expAck&& ! GBN){               //SR
                p->cc->newAck();
                while (p->LLlen >0  && p->head->next->acked){//SR
                    expAck++;
                    node *temp = p->head->next;
                    p->head->next = p->head->next->next;
                    p->LLlen--;
                    delete temp;// deleting the acked node
                }
            }else if (ack->ackno > expAck && ! GBN ){                //SR
                markAcked(p->head,ack->ackno);
            }

        p->ccm->unlock();
    }
}
void *handle_client(void * arg) {
    thPar *p= ((thPar *) arg);
    auto *data = new packet();
    data->seqno=1;

    printf("path:%s\n",p->path->c_str());
    fstream file;

    file.open(*p->path,ios::out|ios::in|ios::app|ios::binary);

    if(!file){
        printf("\n path not found \n ");
    }

    file.seekg(0,ios::end);
    int size  = (int )file.tellg();
    file.seekg(0,ios::beg);


    float counter = (float)size/500;
    int toread = 500;
    node *tail = p->head;
    printf("\nsize is %d , counter is %f\n",size,counter);


    while(counter > 0 ) {
        if(counter<1){
            toread = size % 500;
        }
       // printf("\ncurrent counter %f , to read %d\n",counter,toread);

        p->ccm->lock();
                float numPackToSend = p->cc->p->cwnd / (float)p->cc->p->MSS ;
               // printf("\nnumber of packets allowed is %f len of history is %d \n",numPackToSend,p->LLlen);
                if(p->LLlen >= numPackToSend){ //allow sending within the window
                    p->ccm->unlock();
                    continue;
                }

                if(p->cc->p->retransmit){             //rollback
                    printf("retr.\n");
                    node * h = p->head;
                    file.seekg(h->next->v);
                    data->seqno = h->next->seqNum-1;
                    counter = (float)(size - file.tellg())/500;
                    toread=500;
                    p->LLlen = 0;
                    h->next = nullptr; // GOBACK N
                    tail=h;
                    p->cc->p->retransmit= false;
                }else{                          //adding current vars to history
                    tail->next = new node();
                    tail = tail->next;
                    tail->v = file.tellg();
                    tail->stamp = time(NULL);
                    tail->seqNum = data->seqno+1;
                    p->LLlen++;
                }
        p->ccm->unlock();



        file.read(data->data ,toread);
        data->seqno +=1;
        data->len = toread+8;



        sendto(p->sfd, data, sizeof(packet),
               MSG_CONFIRM, (const struct sockaddr *) p->cliaddr,
               p->len);
      //  printf("data sent seq no %d.\n", data->seqno);
        counter--;
    }


    data->len = 8;
    sendto(p->sfd, data, sizeof(packet),
           MSG_CONFIRM, (const struct sockaddr *) p->cliaddr,
           p->len);
    p->done=true;
    file.close();
}
int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,
              sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    auto *data = new packet();
    int c =100;
    while (c!=0) {
        int len, n;
        socklen_t sendsize = sizeof(servaddr);
        len = sendsize;
        n = recvfrom(sockfd,data , sizeof(packet),
                     MSG_WAITALL, (struct sockaddr *) &cliaddr,
                     reinterpret_cast<socklen_t *>(&len));


        auto *client = new pthread_t();
        auto *th= new pthread_t();
        auto *p=new thPar();
        p->path->append( data->data);
        printf("path : %s",p->path->c_str());
        p->sfd = sockfd;
        p->cliaddr = &cliaddr;
        p->len = len;
        printf("creatingThreads...\n");
        pthread_create(th, NULL, rec, p);
        pthread_create(client, NULL, handle_client, p);
        while (1);
        c--;
    }

    return 0;
}