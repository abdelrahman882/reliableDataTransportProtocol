//
// Created by abdelrahman on 11/1/19.
//
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <ctime>


#define MAX_TO 30
#define PORT 8080
#define queueSize 4
#define MAXCLIENTS 10
#define BUFFER_SIZE 1024
#define  MAX_NUM_OF_REQS 100 // set -ve number for the effect of inf

using namespace std;



pthread_t * clients[MAXCLIENTS] ={nullptr};
bool empty[MAXCLIENTS] ={true};
int socketN[MAXCLIENTS]= {-1};
int numberOfClients = 0;
struct timeval tv;
char * strToChar(string *s){
    // assigning value to string s

    int n = s->length();

    // declaring character array
    char * char_array = new char[n];

    // copying the contents of the
    // string to char array
    strcpy(char_array, s->c_str());

   return char_array;
}

void get(char * buffer,int acLen,int id){
    char path[30]={0};
    int ind =0;
    for(int i =0,j=0; i < acLen && j<2&&ind<30;i++){
        if(buffer[i]==' '){
            j++;
            i++;
            continue;
        }
        if(j==1){
            path[ind] = buffer[i] ;
            ind++;
        }

    }
    printf("%d THREAD || path:%s\n",id,path);
    fstream file;
   file.open(path,ios::in|ios::binary);

    if(!file)
    {
        printf("%d THREAD || error opening file",id);
        char const * msg = "HTTP/1.1 404 Not Found\n\n";
        send(socketN[id] , msg , strlen(msg)+2 , 0 );
        printf(" %d THREAD || replied to client with 404 \n" , id );
        return;
    }



    file.seekg(0,ios::end);
    int size  = (int )file.tellg();
    char *data = new char[size];
    file.seekg(0,ios::beg);
    string s = "HTTP/1.1 200 OK\nContent-Length:"+to_string(size)+"\r\n\r\n";
    char const * msg = strToChar(&s);
    send(socketN[id] , msg , strlen(msg), 0 );
    file.read(data,size);
    char const * msg2 = data;
    send(socketN[id] , msg2 , size , 0 );
    printf("thread %i sent data to client  \n" , id );
    file.close(); //close files
}


void post(char * buffer,int acLen,int id){
    char path[30]={0};
    int ind =0;
    for(int i =0,j=0; i < acLen && j<2&&ind<30;i++){
        if(buffer[i]==' '){
            j++;
            i++;
            continue;
        }
        if(j==1){
            path[ind] = buffer[i] ;
            ind++;
        }

    }
    printf("path:%s\n",path);
    fstream file;
    file.open(path,ios::out);
    file.close();
    file.open(path,ios::out|ios::app|ios::binary);

    if(!file){
        printf("\n path not found \n ");
        char const * errMsg = "HTTP/1.1 404 Not Found\n\n";
        send(socketN[id],errMsg , strlen(errMsg)+2, 0 );
        return;
    }
    char const * msg = "HTTP/1.1 200 OK\n\n";
    send(socketN[id] , msg , strlen(msg)+2 , 0 );
    int valread=0;
    int totR=0;
    do{

        valread = read(socketN[id], buffer, BUFFER_SIZE);
        totR += valread;
        if(valread== 0 ){ //EOF sent
            break;
        }
       // printf("read bytes :%d , total : %d\n\n",valread,totR );
        file.write(buffer,valread);
    }while ( true );
    printf("tot read : %d",totR);
    file.close(); //close file
}
struct argu {
    char * buf;
    int len;
    int id;
};
//void *pipline(void * p){
//    argu *a = (argu*)p;
//    char * buffer = a->buf;
//    int valread  = a->len;
//    int id = a->id;
//
//    //parse request and preform the action
//    char type[5] = {0};
//    int i = 0;
//    for (i = 0; i < valread && i < 4; i++) {
//        type[i] = buffer[i];
//        if (strcasecmp(type, "GET") == 0) {
//            get(buffer, valread, id);
//            break;
//        } else if (strcasecmp(type, "POST") == 0) {
//            post(buffer, valread, id);
//            break;
//        } else if (i == valread - 1 || i == 3) {
//            perror("UNEXPECTED REQUEST -NOT POST NOR GET-\n");
//            empty[id] = true;
//            return NULL;
//        }
//    }
//
//}
void *handle_client(void * arg){
    int id =*((int * )arg);
    printf("\ninitialized thread with id = %d,sockN = %d , connection set..\n",id,socketN[id]);
    int reqs = 0 ;
        while (reqs != MAX_NUM_OF_REQS) {
            char buffer[BUFFER_SIZE] = {0};
            int valread = 0;
            valread = read(socketN[id], &buffer, BUFFER_SIZE);
            if (valread == 0) {
                continue;
            } else if (valread == -1) {
                printf("%d THREAD || Time out\n",id);
                empty[id] = true;
                return NULL;
            }
            printf("%d THREAD || client sent : %s\n\n",id, buffer);


//            argu * a = new argu();
//            a->buf = buffer;
//            a->len = valread;
//            a->id = id;
//            pthread_t t ;
//            pthread_create(&t, NULL, pipline,a );



//            char tbuf[BUFFER_SIZE] = {0};
//            int sPtr = 0;
//            for(int i=0;i<valread-1;i++){
//                string s =
//
//            }

            char * buffi = buffer;
            int len =0;
            for(int j =2 ; j< valread;j++) {
                if (buffer[j] == '\n' && buffer[j - 2] == '\n') {
                    len+=1;
                    //parse request and preform the action
                    char type[5] = {0};
                    int i = 0;
                    for (i = 0; i < len && i < 4; i++) {
                        type[i] = buffi[i];
                        if (strcasecmp(type, "GET") == 0) {
                            get(buffi, len, id);
                            break;
                        } else if (strcasecmp(type, "POST") == 0) {
                            post(buffi, len, id);
                            break;
                        } else if (i == len - 1 || i == 3) {
                            printf("%d THREAD || unexpected request", id);
                            continue;
                        }
                    }
                    reqs++;
                    buffi = buffi + len * sizeof(char);
                    len = 0;
                    j ++;
                } else {
                    len++;
                    continue;
                }


            }
        }

    printf("thread terminated , connection closed with %d",socketN[id]);
    empty[id] = true;
    return NULL;
}
int getEmptyThread(int socN){
    int runningThreads =0 ;
    int id = -1;
    for(int  i =0 ; i < MAXCLIENTS;i++){
        if(empty[i]&&id==-1){
            printf("\nfound empty thread %d .. \n",i);
            empty[i]= false;
            socketN[i]= socN;
            if(clients[i]== nullptr){
                pthread_t t ;
                clients[i] = &t;
            }
            id =  i;
        }
        if(!empty[i]){
            runningThreads++;
        }
    }
    numberOfClients = runningThreads;
    return id ;
}

int main(int argc, char const *argv[])
{

    int server_fd, new_socket,res;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);


    // Creating socket file descriptor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if ( server_fd == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // attaching socket to the port 80
    // setsockopt Returns 0 on success, -1 for errors.
    tv.tv_sec = MAX_TO;
    tv.tv_usec = 0;
    res = setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO,(const char*)&tv, sizeof tv);
    if (res != 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Forcefully attaching socket to the port 80
    res = bind(server_fd, (struct sockaddr *)&address,
               sizeof(address));
    if (res<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    res = listen(server_fd, queueSize);
    if ( res< 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    //set all threads flags to be empty
    for(int i =0 ; i < MAXCLIENTS;i++){
        empty[i] = true;
    }
    int MAX_CLIENTS = 100; // maximum client to serve generally , set to -1 if u want to serve inf
    while(MAX_CLIENTS != 0) {
        new_socket = accept(server_fd, (struct sockaddr *) &address,
                            (socklen_t *) &addrlen);
        if (new_socket < 0) {
            continue;
        }else{
            printf("\nMAIN || client connecting .. \n");
            int id = getEmptyThread(new_socket); //searches for empty threads and updates number of clients
            printf("MAIN || client was given id %d\n",id);
            if(id == -1 ){
                printf("\nMAIN || client ignored ,server already has max number of clients to serve in parallel\n");
                continue;
            }
            int * arg ;
            arg=  &id;
            pthread_create(clients[id], NULL, handle_client,arg );
            if (numberOfClients != 0 ) tv.tv_sec =int( MAX_TO /(double) numberOfClients);
            printf("MAIN || timeout value updated : %f",MAX_TO /(double) numberOfClients);
        }
        MAX_CLIENTS--;
    }

    return 0;
}





