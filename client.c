//
// Created by mouha on 05/12/2022.
//
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>

char *parseP(char **argv, int n, int index,int);

char *parseR(char **, int, int,int);

void SocketConnect(int, char *, char *);

char *parseURl(char **, int, int *,int);

int main(int argc, char *argv[]) {
    //variable initialization
    if(argc < 2) {
        printf("Usage: client [-p n <text>] [-r n < pr1=value1"
               " pr2=value2 …>] <URL>\n");
        exit(1);
    }
    int i, port, flag;
    char *P, *R, *path;
    int rArg, pArg, rIndex, pIndex, Uindex = -1;
    for (i = 0; i < argc; i++) {
        if ((flag = strcmp(argv[i], "-r")) == 0)
            break;
        if(strcmp(argv[i], "-") == 0) {
            printf("Usage: client [-p n <text>] [-r n < pr1=value1"
                   " pr2=value2 …>] <URL>\n");
            exit(1);
        }
    }

    //index of r arguments
    rIndex = i + 2;
    if ((rArg = atoi(argv[i + 1])) == 0 || strcmp(argv[i + 1], "0") == 0) {
        if (flag == 0) {
            printf("Usage: client [-p n <text>] [-r n < pr1=value1"
                   " pr2=value2 …>] <URL>\n");

            exit(1);
        }
        R = "";
    } else {
        R = parseR(argv, rArg, rIndex,argc);
    }

    //find p args
    for (i = 0; i < argc; i++)
        if ((flag = strcmp(argv[i], "-p")) == 0)
            break;
    if ((pArg = atoi(argv[i + 1])) == 0 || strcmp(argv[i + 1], "0") == 0) {
        if (flag == 0) {
            printf("Usage: client [-p n <text>] [-r n < pr1=value1"
                   " pr2=value2 …>] <URL>\n");

            exit(1);
        }
        P = "";
    } else {
        pIndex = i;
        P = parseP(argv, pArg, pIndex,argc);
    }

//find url index
    char* strstrChecker;
    for (int j = 0; j < argc; ++j) {
        if ((strstrChecker = strstr(argv[j], "http://")) != NULL) {
            Uindex = j;
            break;
        }
    }
    if(strstrChecker == NULL){
        printf("Usage: client [-p n <text>] [-r n < pr1=value1"
               " pr2=value2 …>] <URL>\n");

        exit(1);
    }
    char *URL = parseURl(argv, Uindex, &port,argc);
    char *temp;

    //check the path after the url
    if ((temp = strstr(argv[Uindex] + 7, "/")) != NULL) {

        if ((path = (char *) malloc(sizeof(char) * (strlen(temp) + 1))) == NULL) {
            perror("Malloc Failed");
            exit(1);
        }
        strcpy(path, temp);
    } else {
        path = "/";

    }
    //request message

    char *req;
    char all[pArg+(rArg*10) + 200];
    //determine what request to send and build the request
    if (strcmp(P,"")!=0) {
        req = "POST";
        sprintf(all, "%s %s%s HTTP/1.0\r\nHost: %s\r\nContent-Length: %d\r\n\r\n%s", req, path, R, URL, (int)strlen(P),P);

    } else {
        req = "GET";
        sprintf(all, "%s %s%s HTTP/1.0\r\nHost: %s\r\n\r\n", req, path, R, URL);

    }


    printf("HTTP request =\n%s\nLEN = %d\n", all, (int)strlen(all));
    SocketConnect(port, URL, all);
    //free all mallocs
    if(temp!=NULL)
        free(path);
    free(URL);

    if(strcmp(P,"")!=0)
        free(P);
    if(strcmp(R,"")!=0)
        free(R);
    return 0;
}

//this function parses the r arguments  into a char* and organizes them according to what's needed
char *parseR(char **argv, int n, int index,int argc) {
    int count = 0;
    for (int i = index; i <= index + n-1; ++i) {
        if(strstr(argv[i],"=") != NULL)
            count += strlen(argv[i]);
        else{
            printf("Usage: client [-p n <text>] [-r n < pr1=value1"
                   " pr2=value2 …>] <URL>\n");

            exit(1);
        }
    }

    count += n;

    char *str;
    if ((str = (char *) calloc(count,sizeof(char)+1)) == NULL) {
        perror("calloc Failed");
        exit(1);
    }
    str[0] = '?';
    for (int i = index; i < n + index; ++i) {
        strcat(str, argv[i]);
        strcat(str, "&");
    }
    str[strlen(str) - 1] = '\0';
    return str;
}
//this function parses the p arguments  into a string and returns the P argument according to the specified size in the parameter
char *parseP(char **argv, int n, int index,int argc) {
    char *str;
    if (((str = (char *) malloc(sizeof(char) * (n + 1))) == NULL)) {
        perror("Malloc Failed");
        exit(1);
    }
    if(strlen(argv[index+2])!=n){
        printf("Usage: client [-p n <text>] [-r n < pr1=value1"
               " pr2=value2 …>] <URL>\n");

        exit(1);
    }

    strcpy(str, argv[index + 2]);
    return str;
}

//this function gets the url index and parses the domain into a string and the path to a different string and the port number in an int *parameter
char *parseURl(char **argv, int Uindex, int *port,int argc) {
    char *domain;
    if ((domain = (char *) malloc(sizeof(char) * (strlen(argv[Uindex])))) == NULL) {
        perror("Malloc Failed");
        exit(1);
    }
    strcpy(domain, &argv[Uindex][7]);
    char *e;
    int index;

    e = strchr(domain, ':');
    index = (int) (e - domain);
    if (index >= strlen(domain) || index < 0) {
        *port = 80;
    } else {
        *port = atoi(domain + index + 1);
    }

    if(e!=NULL)
        domain[index] = '\0';
    if(*port <= 0 || *port >= 65536){
        printf("Usage: client [-p n <text>] [-r n < pr1=value1"
               " pr2=value2 …>] <URL>\n");

        exit(1);
    }

    e = strchr(domain, '/');
    index = (int) (e - domain);
    if (e != NULL)
        domain[index] = '\0';
    return domain;
}

//this function is responsible for sending the request and doing all the socket initialization and the system calls then it writes the request to the socket
// and then reads the response from the server
void SocketConnect(int port, char *domain, char *request) {
    //socket structs initialization
    struct sockaddr_in serv_addr;
    struct hostent *hostnm;
    hostnm = gethostbyname(domain);

    if (hostnm == NULL) {
        herror("gethostbynamefailes\n");
        exit(2);
    }
    serv_addr.sin_family = PF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = *((unsigned long *) hostnm->h_addr);


    // socket descriptor initialization

    int fd;

    if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket");
        exit(1);
    }

    int client_fd;

    if ((client_fd = connect(fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0) {
        perror("connection failed");
        exit(1);
    }

    if((write(fd, request, strlen(request))) < 0) {
        perror("writing Failed");
        exit(1);
    }
    unsigned char buffer[1024] = {0};
    int size,bytes=0;
    while (((size=read(fd, buffer, 1023)) != 0)) {
        int index=0;
        while(index!=size){
            printf("%c",buffer[index]);
            index++;
        }
        bytes += size;
        explicit_bzero(buffer, sizeof(buffer));

    }
    if(size<0){
        perror("Read Failed");
        exit(1);
    }
    printf("\n Total received response bytes: %d\n",bytes);
    //close connection and the socket
    shutdown(fd, SHUT_RD);
    close(client_fd);
    close(fd);
}
