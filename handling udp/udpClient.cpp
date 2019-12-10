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
#include <poll.h>
#include <sstream>

int PORT    = 8080;
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
    int t =0;


    while (1) {
        auto data = new packet();
        int n = recvfrom(p->sfd, data, sizeof(packet),
                         0, (struct sockaddr *) p->servaddr,
                         reinterpret_cast<socklen_t *>(p->len));
        if(t>100){
            printf("err");
            return 0;
        }
        if(n<0){
            printf("e1");
            t++;
            continue;
        }

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
    string sa="127.0.0.1", fname = "a.txt";
    std::ifstream infile("client.in");
    std::string line;
    std::getline(infile, line);
    sa=line;
    std::getline(infile, line);
    std::istringstream is(line);
    is >> PORT;
    std::getline(infile, line);
    fname=line;

    printf("%s %s %d",sa.c_str(),fname.c_str(),PORT);

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
    string path = fname;
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



    auto *fd =new pollfd();
    int res;

    fd->fd = sockfd;
    fd->events = POLLIN;
    res = poll(fd, 1, 4000); // 1000 ms timeout
    if (res == 0)
    {
        printf( " TO >>>>>>> \n");
        return 0 ;
        // timeout
    }
    else if (res == -1)
    {
        printf("ERR");
        return 0;
        // error
    }

    while(1) {
        n = recvfrom(sockfd, data, sizeof(packet),
                         MSG_WAITALL, (struct sockaddr *)& servaddr,
                         reinterpret_cast<socklen_t *>(&len));
        printf("data received with seq %d\n",data->seqno);
        if(!checksum(data)){
            printf("checksum err");
            continue;
        }
        if(expSeqNum==-1){
            expSeqNum = data->seqno;
        }

        if(data->len == 8){
            break;
        }else if(expSeqNum == data->seqno&&GBN){
            expSeqNum++;
            file.write(data->data,data->len-8);
            //append here

        } else{

        }
        //GBN
        if(GBN) {
            ack->ackno = expSeqNum;
        }else{
            ack->ackno =data->seqno+1;
        }

        if(random()%100 <0 ){
            usleep(1000);
        }
        addCS(ack);
        sendto(sockfd, ack, sizeof(ack_packet),
               MSG_CONFIRM, (const struct sockaddr *) &servaddr,
               sizeof(servaddr));
        printf("ack sent. %d\n",ack->ackno);
    }
    file.close(); //close file

    close(sockfd);
    return 0;
}