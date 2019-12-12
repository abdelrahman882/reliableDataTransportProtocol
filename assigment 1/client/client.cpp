//
// Created by abdelrahman on 11/1/19.
//

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;
#define PORT 8080
#define B_SIZE 4096

char * strToCharPtr (string s){

    int n = s.length();
    char * char_array = new char[n];

    strcpy(char_array, s.c_str());
    return  char_array;
}
int main(int argc, char const *argv[])
{
    int sock = 0, valread;
    char buffer[B_SIZE] = {0};



    // Convert IPv4 and IPv6 addresses from text to binary form

//client_get file-path host-name (port-number)
    std::ifstream file("input.txt");
    std::string str;
    while (true) {
        std::getline(file, str);
        int pN = 80;
        int s1= str.find(' ');
        string req = str.substr(0, s1);
        int s2 = str.find(' ',s1+1);
        string path = str.substr(s1+1 , s2-s1-1);
        int s3 = str.find(' ',s2+1);
        string hn;
        string pn = "";
        if(s3 > 0 ){
            hn = str.substr(s2+1 , s3-s2-1);
            pn = str.substr(s3+1 , str.length()-s3-1);
            stringstream stream1(pn);
            stream1 >> pN;
        }else{
            hn = str.substr(s2+1 , str.length()-s2-1);
        }
        printf("%s,%s,%s,%d\n\n\n" , req.c_str() , path.c_str() , hn.c_str(),pN);


        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Socket creation error \n");
            return -1;
        }
        struct sockaddr_in serv_addr = *(new sockaddr_in());

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(pN);
        if(inet_pton(AF_INET, hn.c_str(), &serv_addr.sin_addr)<=0)
        {
            printf("\nInvalid address/ Address not supported \n");
            return -1;
        }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("\nConnection Failed \n");
            return -1;
        }




        string m = req + " " +path+ " http/1.1\r\n\r\n";
        char const *msg = strToCharPtr(m);
        send(sock, msg, m.length() , 0);
        valread = read(sock, buffer, B_SIZE);
        int response = 0;
        for (int i = 0; i < valread; i++) {
            if ((buffer[i] == '2' || buffer[i] == '4')) {
                response = (buffer[i] - 48) * 100 +
                           (buffer[i + 1] - 48) * 10 +
                           (buffer[i + 2] - 48);
                break;
            }
        }
        printf("\n server responded with respose code = %d\n", response);
        if (response != 200) {
            printf("\n 404 ERR\n");
            break;
        }

        if (req == "POST") {
            printf("POST");

            fstream file;
            file.open(path.substr(1), ios::in | ios::binary);


            file.seekg(0, ios::end);
            int size = (int) file.tellg();
            char *data = new char[size];
            file.seekg(0, ios::beg);
            file.read(data, size);
            send(sock, data, size, 0);

        } else {
            fstream file;
            file.open(path.substr(1),ios::out| std::ofstream::trunc);
            file.close();
            file.open(path.substr(1),ios::out|ios::in|ios::app|ios::binary);
            string t = buffer;
            string cl = "Content-Length:";
            string rn = "\r\n\r\n";
            stringstream ss (
                    t.substr(t.find(cl)+cl.length(),t.find(rn,t.find(cl))-t.find(cl)+cl.length())
            );
            int len =0;
            ss>>len;
            printf("%d , --- > ",len);
            int s = t.find("\r\n\r\n")+4;
            if(s>=0&& s<valread)
            file.write(t.substr(s,len > valread-s ? valread:len).c_str(), len > valread-s ? valread-s:len);

            int tot =0;
            int ovr = valread;
            if (len > valread - s) do{
                if(ovr-s + tot >= len)break;
                valread = read(sock, buffer, B_SIZE);
                tot+=valread;
                printf("%d-",tot + B_SIZE-s);
                if(valread<= 0 ){ //EOF sent
                    break;
                }
                file.write(buffer,valread);
            }while ( true );

            file.close(); //close file
        }


    }
    return 0;
}







/*

        string getPost;
        cout << "enter 1 for post any thing else for get";
        getline(cin, getPost);
        if (getPost == "1") {//post
            cout << "enter post request : ";
            string line;
            getline(cin, line);

            cout << "enter file to upload : ";
            string line2;
            getline(cin, line2);

            char const *msg = strToCharPtr(line);
            printf("%s", msg);
            send(sock, msg, line.length() + 2, 0);
            do {
                valread = read(sock, buffer, 1024);
                printf("server sent : %s\n", buffer);
            } while (buffer[valread - 1] != '\n' && valread != 0);
            int response = 0;
            for (int i = 0; i < 1024; i++) {
                if ((buffer[i] == '2' || buffer[i] == '4')) {
                    response = (buffer[i] - 48) * 100 +
                               (buffer[i + 1] - 48) * 10 +
                               (buffer[i + 2] - 48);
                    break;
                }
            }
            printf("\n server responded with respose code = %d\n", response);
            if (response != 200) {
                printf("\n dir doesn't exist\n");
                return 0;
            }

            fstream file;
            file.open(line2, ios::in | ios::binary);


            file.seekg(0, ios::end);
            int size = (int) file.tellg();
            char *data = new char[size];
            file.seekg(0, ios::beg);
            file.read(data, size);
            printf("\nsending %d bytes ..\n\n\n", size);
            send(sock, data, size, 0);

        } else {
            cout << "enter GET request: ";
            string line;
            getline(cin, line);
            char const *msg = strToCharPtr(line);
            send(sock, msg, line.length() + 2, 0);

            printf("server reply : \n\n");
            do {
                printf("\n\n\nBLOCK : \n");
                valread = read(sock, buffer, 1024);
                printf("%d\n\n", valread);
                printf("%s\n", buffer);
            } while (buffer[valread - 1] != '\n');

        }

 * */