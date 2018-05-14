/* 
 * tcpserver.c - A simple TCP echo server 
 * usage: tcpserver <port>
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/md5.h>

#define BUFSIZE 1024

#if 0
/* 
 * Structs exported from in.h
 */

/* Internet address */
struct in_addr {
  unsigned int s_addr; 
};

/* Internet style socket address */
struct sockaddr_in  {
  unsigned short int sin_family; /* Address family */
  unsigned short int sin_port;   /* Port number */
  struct in_addr sin_addr;	 /* IP address */
  unsigned char sin_zero[...];   /* Pad to size of 'struct sockaddr' */
};

/*
 * Struct exported from netdb.h
 */

/* Domain name service (DNS) host entry */
struct hostent {
  char    *h_name;        /* official name of host */
  char    **h_aliases;    /* alias list */
  int     h_addrtype;     /* host address type */
  int     h_length;       /* length of address */
  char    **h_addr_list;  /* list of addresses */
}
#endif

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) {
  int parentfd; /* parent socket */
  int childfd; /* child socket */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buffer */
  char buf2[BUFSIZE]; /* message buffer */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */
  int x, y, i, i_last, count=0, filesize, fcnt=0;
  char *token;
  char *filename;
  FILE *fp;


  /* 
   * check command line arguments 
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /* 
   * socket: create the parent socket 
   */
  parentfd = socket(AF_INET, SOCK_STREAM, 0);
  if (parentfd < 0) 
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));

  /* this is an Internet address */
  serveraddr.sin_family = AF_INET;

  /* let the system figure out our IP address */
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

  /* this is the port we will listen on */
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(parentfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

  /* 
   * listen: make this socket ready to accept connection requests 
   */
  if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */ 
    error("ERROR on listen");
  printf("Server Running ....\n");
  /* 
   * main loop: wait for a connection request, echo input line, 
   * then close connection.
   */
  clientlen = sizeof(clientaddr);
  while (1) {

    /* 
     * accept: wait for a connection request 
     */
    childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
    if (childfd < 0) 
      error("ERROR on accept");
    fcnt++;
    if(fork() == 0)
    {
        //close(parentfd);
        /* 
         * gethostbyaddr: determine who sent the message 
         */
        
        hostaddrp = inet_ntoa(clientaddr.sin_addr);
        if (hostaddrp == NULL)
          error("ERROR on inet_ntoa\n");
        printf("%d : Server established connection with %s\n", fcnt, hostaddrp);
        /* 
         * read: read input string from the client
         */
        bzero(buf2, BUFSIZE);
        n = read(childfd, buf2, BUFSIZE);
        if (n < 0) 
          error("ERROR reading from socket");
      	buf2[n]='\0';
        if(strcmp(buf2, "Error!")==0)
        {
        	printf("%d : Closing connection with client. Incorrect filename at client side\n\n", fcnt);
        	close(childfd);
        	continue;
        }
        printf("%d : server received %d bytes: %s\n",fcnt , n, buf2);
        /* 
         * write: echo the input string back to the client 
         */
        n = write(childfd, buf2, strlen(buf2));
        if (n < 0) 
          error("ERROR writing to socket");
        
        //filesize extraction!
      	i_last=-1;
      	for(i=0;i<strlen(buf2);i++)
      		if(buf2[i]==' ')
      		{
      			if(i_last == i-1)
      			{
      				i_last=i;
      				continue;
      			}
      			count++;
      			i_last=i;
      		}
      	count-=2;
        token = strtok(buf2, " ");
        token = strtok(NULL, " ");
        filename = token;
        while(count>0)
        {
        	token = strtok(NULL, " ");
        	strcat(filename," ");
        	strcat(filename,token);
        	count--;
        }
        token = strtok(NULL, " ");
        filesize = atoi(token);
        printf("%d : Filename = %s, Filesize = %d\n", fcnt, filename, filesize);    
        fp = fopen(filename, "wb");
        x=filesize/BUFSIZE + 1 ;
        y = filesize;
        while(y)
        {
            //printf("%d : ",x--);
            bzero(buf, BUFSIZE);
            n = recv(childfd, buf, BUFSIZE,0);
            if (n < 0) 
              error("ERROR reading from socket");
            //printf("server received %d bytes\n", n);
            
            buf[n]= '\0';
            fwrite(buf, 1, n, fp);
            y = y-n;
            /*n = write(childfd, buf, strlen(buf));
            if (n < 0) 
              error("ERROR writing to socket");*/
        }
        fclose(fp);

        //MD5 Checksum computation
        fp = fopen(filename, "rb");
        int bytes;
        MD5_CTX mdContext;
        char data[BUFSIZE];
        unsigned char c[MD5_DIGEST_LENGTH];
        MD5_Init (&mdContext);
        while ((bytes = fread (data, 1, BUFSIZE, fp)) != 0)
            MD5_Update (&mdContext, data, bytes);
        MD5_Final (c,&mdContext);
        printf("%d : ", fcnt);
        for(i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
        	printf("%02x", c[i]);
        }
        printf("\n%d : Finished finding MD5 Checksum!\n", fcnt);
        fclose(fp);
        n = send(childfd, c, MD5_DIGEST_LENGTH, 0);
        if (n < 0) 
        	error("ERROR writing to socket");
        printf("%d : Connection with client closed\n\n", fcnt);
        close(childfd);
    }
  }
}
