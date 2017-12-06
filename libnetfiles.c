#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "libnetfiles.h"
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>



/*
Opens a file.
Returns: the new file descriptor or -1 if an error occured and sets errno
Otherwise, returns 0  
Flags: O_RDONLY=0, O_WRONLY=1, O_RDWR=2
*/
int netopen(const char *pathname, int flags ){
 
    if(isInitialized != 0){
        printf("ERROR: init wasn't called or hostname doesn't exist  \n");
        h_errno = HOST_NOT_FOUND;
        return -1;
    }

    if(flags == 0 || flags == 1 || flags == 2 ){
        printf("Valid flag\n");
    }
    else{
        printf("Invalid netopen flag\n");
        return -1;
        
    }
    int sentbytes1, sentbytes2, bytesReceived, connectCheck;
    struct addrinfo *clientinfo;
    int results[5];
  

    for(clientinfo = clientlist; clientinfo != NULL; clientinfo = clientinfo ->ai_next){
        //Searching to see if there is a socket to use
   
        sockfd = socket(clientlist->ai_family, clientlist->ai_socktype, clientlist->ai_protocol);
        if(sockfd == -1){  //Error cannot create socket
            perror("Error: ");
            printf("Error Socket couldn't be created\n");
            continue;
        }
        
        connectCheck = connect(sockfd, clientinfo->ai_addr, clientinfo->ai_addrlen);
        if(connectCheck == -1) {
            close(sockfd);
            perror("Error: ");
            continue;
        }
        else{
            break;
        }
    }  //end of linked list loop

    if(clientinfo == NULL){
        fprintf(stderr, "Error: Can't find a socket to connect to\n");
        return -1;
    }
 
//At this point, has created a socket with SocketFD = sockfd
//And connected this socket to the server  


    int pathLength = strlen(pathname);

    char *path = (char*) malloc (pathLength +1) ;

    strcpy (path, pathname);
    path[pathLength] = '\0';

    int sentLength = strlen(path);
    printf("Sent lenght: %d\n", sentLength);
//Sending the INDICATOR to OPEN (0) along with the flags, and filemode. 

    int indArr[5];
    indArr[0] = 0;
    indArr[1] = flags;
    indArr[2] = 0;
    indArr[3] = serv_mode;
    indArr[4] = sentLength;

    sentbytes1 = send(sockfd, indArr, 5, 0);
    printf("bytes sent for indicator: %d\n", sentbytes1);
    
    if(sentbytes1 == -1){
        perror("Error: ");
        exit(1);
    }
 
//Sending the PATHNAME

//int sentLength = strlen(path); 
  sentbytes2 = send(sockfd, path, sentLength, 0);

    if(sentbytes2 == -1){
        perror("Error: ");
        exit(1);
    }
 

  bytesReceived = recv(sockfd, results, 2*sizeof(int), 0);

  if(bytesReceived == -1){
      perror("Error: ");
      exit(1);
  }


  if(results[0] == -1) {

      errno = results[1];
      perror("Error with netopen: ");
      return -1;
  }
    
  free(path);
  return results[0];
}



/*
Reads a remote file
Returns an integer with the number of bytes that were read.
Returns -1 and sets the errno value appropriately if an error was caught. 
 */

ssize_t netread(int fildes, void *buf, size_t nbyte) {


    if(isInitialized != 0){
        printf("ERROR: Init was not called or hostname does not exist  \n");
        h_errno = HOST_NOT_FOUND;
        return -1;
    }

    struct addrinfo *clientinfo;
    int connectCheck;


    for(clientinfo = clientlist; clientinfo != NULL; clientinfo = clientinfo ->ai_next){
   //Going throught the linked list checking for a useable socket to use.                                                                                                                                       
   
        sockfd = socket(clientlist->ai_family, clientlist->ai_socktype, clientlist->ai_protocol);
        if(sockfd == -1){  //Error on creating a socket for server
            perror("Error: ");
            printf("Error Socket could not be created for client at this particular socket. Try entereing a new PORT NUM\n");
            continue;
        }
        
        connectCheck = connect(sockfd, clientinfo->ai_addr, clientinfo->ai_addrlen);
        if(connectCheck == -1) {
            close(sockfd);
            perror("Error: ");
            continue;
        }
        else{
            break;
        }
    }  //end of linked list loop

    if(clientinfo == NULL){
        fprintf(stderr, "Error: Couldn't find a socket to connect to. Try entering a new PORT NUM.\n");
        return -1;
    }


    //Sending the INDICATOR to READ (1), the file descriptor for the file, and number of bytes to be read.
  
    int bytesReceived, sentbytes1, sentbytes2;
    int indArr[3];

    int nBytes = (int) nbyte;

    indArr[0] = 1;
    indArr[1] = fildes;
    indArr[2] = nBytes;

    sentbytes1 = send(sockfd, indArr, 10, 0);

    if(sentbytes1 == -1){
        perror("error: ");
        exit(1);
    }
    
//Sending the PATHNAME
    char *path = "A";
    printf("New path in READ: %s\n", path);
    int sentLength = strlen(path);
    sentbytes2 = send(sockfd, path, sentLength, 0);
    printf("bytes sent path: %d\n", sentbytes2);
    
    if(sentbytes2 == -1){
        perror("error: ");
        exit(1);
    }
    //Recv results from the server

    int results [4];
    //results[0] = "# of bytes read" on success, "-1" on error.
    //results[1] = "0" on success, "Errno value" on error
    bytesReceived = recv(sockfd, results, 2*sizeof(int), 0);
    //bytesReceived = recv(sockfd, buff, 100, 0);
    if(bytesReceived == -1){
        perror("Error: ");
        exit(1);
    }

    //Setting the errno value
    if(results[0] == -1) {
        errno = results[1];
        perror("Error: ");
        return -1;
    }

    return results[0];

}


/*
Writes to a remote file
Returns an int with the number of bytes actually written to the file. 
Returns -1 and sets the appropriate errno value if error is caught. 
 */
ssize_t netwrite(int fildes, const void *buf, size_t nbyte) {

    struct addrinfo *clientinfo;
    int connectCheck;

    if(isInitialized != 0){
        printf("ERROR: Init was not called or hostname does not exist  \n");
        h_errno = HOST_NOT_FOUND;
        return -1;
    }

    for(clientinfo = clientlist; clientinfo != NULL; clientinfo = clientinfo ->ai_next){
        //Going throught the linked list to check for a useable socket to use.
   
        sockfd = socket(clientlist->ai_family, clientlist->ai_socktype, clientlist->ai_protocol);
        if(sockfd == -1){  //Error on creating a socket for server
            perror("Error: ");
            printf("Error Socket could not be created for client at this particular socketry entering a new PORT NUM.\n");
            continue;
        }
        connectCheck = connect(sockfd, clientinfo->ai_addr, clientinfo->ai_addrlen);
        
        if(connectCheck == -1) {
            close(sockfd);
            perror("Error: ");
            continue;
        }
        else{
            break;
        }
    }  //end of linked list loop

    if(clientinfo == NULL){
        fprintf(stderr, "Error: Couldn't find a socket to connect to. Try entering a new PORT NUM.\n");
        return -1;
    }
    
    //Sending the INDICATOR to WRITE (2), the file descriptor and the number of bytes to write.
    int bytesReceived, sentbytes1, sentbytes2, nBytes;
    int indArr[3];

    nBytes = (int) nbyte;
 
    indArr[0] = 2;
    indArr[1] = fildes;
    indArr[2] = nBytes;
    printf("nbytes: %d\n", nBytes);
    sentbytes1 = send(sockfd, indArr, 10, 0);
  
    if(sentbytes1 == -1){
        perror("error: ");
        exit(1);
    }

    //Sending the BUFFER
    int buffLen = strlen(buf);
    char path[buffLen + 1];
    strcpy(path, buf);
    printf("Buffer sent in WRITE: %s\n", path);
    sentbytes2 = send(sockfd, path, buffLen, 0);

    if(sentbytes2 == -1){
        perror("error: ");
        exit(1);
    }


    int results [4];
    bytesReceived = recv(sockfd, results, 2*sizeof(int), 0);
    
    //bytesReceived = recv(sockfd, buff, 100, 0);
    if(bytesReceived == -1){
        perror("error: ");
        exit(1);
    }
    
    if(results[0] == -1) {
        errno = results[1];
        perror("Error: ");
        return -1;
    }

    return results[0];


}

/*
Closes a remote file or returns the errno value if an error was caught
Returns 0 if the file is closed properly. 
Returns -1 and sets the errno appropriately if an error is caught.  
*/

int netclose(int fd) {
 
    struct addrinfo *clientinfo;
    int connectCheck;


    if(isInitialized != 0){
        printf("ERROR: Init was not called or hostname does not exist  \n");
        h_errno = HOST_NOT_FOUND;
        return -1;
    }

    for(clientinfo = clientlist; clientinfo != NULL; clientinfo = clientinfo ->ai_next){
        //Going throught the linked list checking for a useable socket to use.
   
        sockfd = socket(clientlist->ai_family, clientlist->ai_socktype, clientlist->ai_protocol);
        if(sockfd == -1){  //Error on creating a socket for server
            perror("Error: ");
            printf("Error Socket could not be created for client at this particular socket. Try entering a new PORT NUM.\n");
            continue;
        }
        connectCheck = connect(sockfd, clientinfo->ai_addr, clientinfo->ai_addrlen);
        if(connectCheck == -1) {
            close(sockfd);
            perror("Error: ");
            continue;
        }
        else{
            break;
        }
    }  //end of linked list loop

    if(clientinfo == NULL){
        fprintf(stderr, "Error: Couldn't find a socket to connect to. Try entering a new PORT NUM.\n");
        return -1;
    }

    //Sending the INDICATOR to CLOSE (3)
    int bytesReceived, sentbytes1, sentbytes2;
    int indArr[2];
    indArr[0] = 3;
    indArr[1] = fd;

    sentbytes1 = send(sockfd, indArr, 5, 0);

    if(sentbytes1 == -1){
        perror("error: ");
        exit(1);
    }

    //Sending the BUFFER

    char *path = "A";

    sentbytes2 = send(sockfd, path, 5, 0);
    
    if(sentbytes2 == -1){
        perror("error: ");
        exit(1);
    }

    int results [4];
    bytesReceived = recv(sockfd, results, 2*sizeof(int), 0);

    if(bytesReceived == -1){
        perror("error: ");
        exit(1);
    }


    if(results[0] == -1) {
  
        errno = results[1];
        perror("Error: ");
        return -1;
    }

    return results[0];
}



/*Checks that the server can be connected. Returns 0 on success, -1 on error
Filemode unrestricted=0, exclusive=1, transaction=2 
Should be the first function called from a client in order to establish a connection. 
Returns 0 on succes and -1 on error.
*/

int netserverinit(char * hostname,int filemode) {
  
    int addrTest;
    struct addrinfo hints; //servinfo requirements
    char portNum[6]; //hardcoded port number

    serv_mode = filemode;
    memset(&hints, 0, sizeof hints); //makes sure hints is empty
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_STREAM; //TCP stream sockets
  

    snprintf(portNum, 6, "%d", PORT_NUM);

    //sets h_errno
    addrTest = getaddrinfo(hostname, portNum, &hints, &clientlist);

    if(addrTest != 0) {
        h_errno = HOST_NOT_FOUND;  //check if it is a proper errno
        herror("Error: ");
        isInitialized = -1;
        return -1;   //returns -1 if error occurred while initializing server

    }
    isInitialized = 0;
    return 0;
    
}
