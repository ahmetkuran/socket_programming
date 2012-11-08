/**********************************
tcp_ser.c: the source file of the server in tcp transmission 
***********************************/


#include "headsock.h"

#define BACKLOG 10

void str_ser(int sockfd);                                                        // transmitting and receiving function

int main(void)
{
  //file descriptors
  int sockfd, con_fd, ret;
  //address holders
  struct sockaddr_in my_addr;
  struct sockaddr_in their_addr;
  int sin_size;

  //	char *buf;
  //pid, oh yeah...
  pid_t pid;
  // Create a socket.AF_INET = IPv4. SOCK_STREAM is not a sock. from man page:
  // byte stream that provides reliable two way communication
  sockfd = socket(AF_INET, SOCK_STREAM, 0);          //create socket
  //C style error checking. Just coz
  if (sockfd <0)
    {
      printf("error in socket!");
      exit(1);
    }
  //existentialism follows....
  my_addr.sin_family = AF_INET;
  //htons is host to network short
  my_addr.sin_port = htons(MYTCP_PORT);

  my_addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("172.0.0.1");
  //erm, man says this is deprecated... ah well
  bzero(&(my_addr.sin_zero), 8);
  //bind socket and the nice address.
  ret = bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));                //bind socket
  if (ret <0)
    {
      printf("error in binding");
      exit(1);
    }
  //
  // A  willingness to accept incoming connections and a queue limit
  // for incoming connections are specified with listen().
  // Heh.                           
  ret = listen(sockfd, BACKLOG);                              //listen
  if (ret <0) {
    printf("error in listening");
    exit(1);
  }

  while (1)
    {
      printf("waiting for data\n");
      sin_size = sizeof (struct sockaddr_in);
      con_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);            //accept the packet
      if (con_fd <0)
        {
          printf("error in accept\n");
          exit(1);
        }

      if ((pid = fork())==0)                                         // creat acception process
        {
          close(sockfd);
          str_ser(con_fd);                                          //receive packet and response
          close(con_fd);
          exit(0);
        }
      else close(con_fd);                                         //parent process
    }
  close(sockfd);
  exit(0);
}

void str_ser(int sockfd)
{

  char buf[BUFSIZE];
  //coz, CAPS are in fashion these days...
  FILE *fp;
  struct pack_so recvs;
  char recvp[DATALEN];
  struct ack_so ack;

  int end, recvn = 0,ackn =0;
  long lseek=0;
  end = 0;
	
  printf("receiving data!\n");

  while(!end)
    {
      //recieve a message from my socket...;)
      if ((recvn= recv(sockfd, &recvs, sizeof(recvs), 0))==-1)                                   //receive the packet
        {
          printf("error when receiving\n");
          exit(1);
        }
      recvn = recvs.len;
      //Some nice debug info
      printf("size of recvn:%d\n",recvn);
      printf("packet recieved\n");
      printf("packet number:%d\n",recvs.num);
      printf("size:%d\n",sizeof(recvs));
      //check for (manual)EOF!!!
      if (recvs.data[recvn-1] == '\0')									//if it is the end of the file
        {
          end = 1;
          //almost forgot this:
          recvn --;
        }
      //C style, copy data from socket to the nice buffer
      memcpy((buf+lseek), recvs.data, recvn);
      //seek for buffer. Yeah yeah, pointer arithmetic
      lseek += recvn;
      //So the clientses know I got their call
      ack.num = recvs.num;
      //Don't really need this so..
      ack.len = 0;
      
      //send ACK! kinda ballsy to use 2 instead of sizeof
      if ((ackn = send(sockfd, &ack, 2, 0))==-1)
        {
          printf("send error!");							      //OH NOES
          exit(1);
        }
		
      
      printf("sent ack\n");
    }
	
  // When all the kings's horses are done..
  if ((fp = fopen ("myTCPreceive.txt","wt")) == NULL)
    {
      printf("File doesn't exit\n");
      exit(0);
    }

  fwrite (buf , 1 , lseek , fp);					//write data into file
  fclose(fp);
  printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)lseek);
}
