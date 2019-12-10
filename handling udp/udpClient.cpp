//
// Created by abdelrahman on 12/6/19.
//

// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "Packets.cpp"
#include <pthread.h>
#include <mutex>
#include <fstream>
#define PORT     8080
#define MAXLINE 508


struct queue{
    packet * p = nullptr;
    queue * next = nullptr;
};

queue * head = new queue();
std::mutex ql;
bool recDone =false;


struct thPar{
    int sfd=1;
    struct  sockaddr_in * servaddr;
    int *len;
};

void *rec(void * arg) {   //receive and throw in the queue
    thPar *p = ((thPar *) arg);
    queue * curr = head ;
    while (1) {
        auto data = new packet();
        int n = recvfrom(p->sfd, data, sizeof(packet),
                         MSG_WAITALL, (struct sockaddr *) p->servaddr,
                         reinterpret_cast<socklen_t *>(p->len));
        printf("data received with seq %d\n",data->seqno);
        if(data->len == 5){
            break;
        }
        ql.lock();
        if(head->next == nullptr){
            curr = head;
        }
        curr->next = new queue();
        curr->p = data;
        curr = curr->next;
        ql.unlock();
    }
    recDone=true;

}
// Driver code
int main() {
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in  servaddr;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;


    auto *ack = new ack_packet();
    auto * data = new packet();
    data->len=0;
    string path = "big.txt";
    for(char& c : path) {
        data->data[data->len]=c;
        data->len++;
    }
    data->data[data->len] ='\0';
    data->len+=8;
    int n, len;
    socklen_t sendsize = sizeof(servaddr);
    len = sendsize;
    sendto(sockfd, data, sizeof(packet),
           MSG_CONFIRM, (const struct sockaddr *) &servaddr,
           sizeof(servaddr));
    printf("file path is sent %s.\n",data->data);
//    pthread_t *th= new pthread_t();
//    thPar  *p= new thPar();
//    p->sfd = sockfd;
//    p->len=&len;
//    p->servaddr = &servaddr;

//    pthread_create(th, NULL, rec, p);

//        ql.lock();
//        if(head->next== nullptr&&recDone){
//            ql.unlock();
//            break;
//        }else if(head->next== nullptr){
//            ql.unlock();
//            continue;
//        }

//        data = head->next->p;
//        head->next = head->next->next;
//        ql.unlock();


    fstream file;
    file.open("filerec.txt",ios::out);
    file.close();
    file.open("filerec.txt",ios::out|ios::app|ios::binary);

    if(!file){
        printf("\n path not found \n ");
    }
    int expSeqNum=-1;
    while(1) {
        n = recvfrom(sockfd, data, sizeof(packet),
                         MSG_WAITALL, (struct sockaddr *)& servaddr,
                         reinterpret_cast<socklen_t *>(&len));
        printf("data received with seq %d\n",data->seqno);
        if(expSeqNum==-1){
            expSeqNum = data->seqno;
        }

        if(data->len == 8){
            break;
        }else if(expSeqNum == data->seqno){
            expSeqNum++;

            file.write(data->data,data->len-8);
            //append here

        }
        //GBN
        ack->ackno = data->seqno+1;
        sendto(sockfd, ack, sizeof(ack_packet),
               MSG_CONFIRM, (const struct sockaddr *) &servaddr,
               sizeof(servaddr));
        printf("ack sent. %d\n",ack->ackno);
    }
    file.close(); //close file

    close(sockfd);
    return 0;
}