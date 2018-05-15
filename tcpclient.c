/* 
 * tcpclient.c - A simple TCP client
 * usage: tcpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <openssl/md5.h>

#define BUFSIZE 1024

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}


int main(int argc, char **argv) {
    int sockfd, portno, n;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char *buf;
    buf = (char*)malloc(sizeof(char)*BUFSIZE);
    /*char buf[BUFSIZE];*/
    char filename[BUFSIZE];
    FILE *fd;
    int filesize, x;
    char sizebuf[BUFSIZE];

    /* check command line arguments */
    if (argc != 3) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* connect: create a connection with the server */
    if (connect(sockfd, &serveraddr, sizeof(serveraddr)) < 0) 
      error("ERROR connecting");

    /* get message line from the user */
    printf("Please enter name of file: ");
    bzero(filename, BUFSIZE);
    fgets(filename, BUFSIZE, stdin);
    filename[strlen(filename)-1]='\0';
    fd = fopen(filename, "rb");
    if(fd==NULL)
    {
        strcpy(buf, "Error!");
        n = write(sockfd, buf, strlen(buf));
        if (n < 0) 
          error("ERROR writing to socket");
        printf("Error opening file!\n");
        close(sockfd);
        exit(1);
    }
    
    printf("\nMD5 checksum of file %s = ", filename);
    int i, bytes;
    MD5_CTX mdContext;
    char data[BUFSIZE];
    unsigned char c[MD5_DIGEST_LENGTH];
    unsigned char c1[MD5_DIGEST_LENGTH];
    MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, BUFSIZE, fd)) != 0)
        MD5_Update (&mdContext, data, bytes);
    MD5_Final (c,&mdContext);
    for(i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", c[i]);
    printf("\n");

    fseek(fd, 0, SEEK_SET); // seek back to beginning of file
    fseek(fd, 0, SEEK_END); // seek to end of file
    filesize = ftell(fd); // get current file pointer
    fseek(fd, 0, SEEK_SET); // seek back to beginning of file

    bzero(buf, BUFSIZE);
    strcpy(buf, "Hello! ");
    strcat(buf, filename);
    strcat(buf," ");
    
    x = snprintf(sizebuf, BUFSIZE, "%d", filesize);
    strcat(buf, sizebuf);
    /* send the message line to the server */
    // message : Hello! <filename> <filesize>
    n = write(sockfd, buf, strlen(buf));
    if (n < 0) 
      error("ERROR writing to socket");

    /* print the server's reply */
    bzero(buf, BUFSIZE);
    n = read(sockfd, buf, BUFSIZE);
    if (n < 0) 
      error("ERROR reading from socket");
    printf("Echo from server: %s\n", buf);
    

    int check=0;
    while(!feof(fd))
    {
        bzero(buf, BUFSIZE);
        check = fread(buf, 1, BUFSIZE, fd);
        //printf("***Size read from file : %d***\n",check);
        n = send(sockfd, buf,check,0);
        if (n < 0) 
          error("ERROR writing to socket");
        
        /* print the server's reply */
        /*bzero(buf, BUFSIZE);
        n = read(sockfd, buf, BUFSIZE); 
        if (n < 0) 
          error("ERROR reading from socket");
        printf("Echo from server: %s", buf);*/ 
    }
    bzero(buf, BUFSIZE);
    n = recv(sockfd, c1, MD5_DIGEST_LENGTH, 0);
    if (n < 0) 
      error("ERROR reading from socket");
    int flag = 1;
    printf("\nMD5 checksum received from server : ");
    for(i = 0; i < MD5_DIGEST_LENGTH; i++) 
    {
        printf("%02x", c1[i]);
        if(c[i]!=c1[i])
            flag=0;
    }
    if(flag)
        printf("\nMD5 Matched\n");
    else
        printf("\nMD5 Not Matched\n");
    close(sockfd);
    return 0;
printf("dsdf");
printf("ZDV");
}
