/********************************************************           
    * wetalk.c 			   					               *   
    *                                                      *   
    * Author:  Dakshil Shah, shah329                       *   
    *                                                      *   
    * CS 536 lab4 Q2							           *   
    *                                                      *   
    * Usage:                                               *   
    *      wetalk.out portno 							   *   
    ********************************************************/
    /*
//give input address to chat with as eg:
?sslab01 20000
?192.168.1.1 20000
where ? is the prompt and there is a space between the address and portnumber. Please use only one space else error of invalid address
on request, c accepts, n rejects, q quits, other char do nothing.
while chatting q ends program and e ends chat 
    */  
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <termios.h>            //termios, TCSANOW, ECHO, ICANON
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/signal.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/syscall.h>

int sd;

struct sockaddr_in addr1,addr_to_connect,addr_connected;
socklen_t len=sizeof(addr1);	
char buffer[51],buffer2[51],buffer3[51];
int port2,setup=0,flag=0;
int c;   
static struct termios oldt, newt;
int i=-1;
int j,terminate=0;

void udptimeout( int sig ) 
{
}
void sigpoll( int sig ) 
{
				//if chat connection accepted
				if(setup==1)
				{

					if(recvfrom(sd, buffer2, 51, 0, (struct sockaddr *)&addr_connected, &len)<0)
					{
		        		perror("\nerror in recv poll");
	        		}
	        		//checl if end of chat requested
	        		if(strlen(buffer2)==1 && buffer2[0]=='E')
	        		{
	        			printf("\n| chat terminated\n");
	        			terminate=1;
	        		}
	        		//check if chat message
	        		else if(buffer2[0]=='D')
	        		{

	        			printf("\n|%.*s\n",(int)strlen(buffer2),buffer2 + 1);
	        		}
	        		else
	        			printf("\nerror\n");
	        		memset(&buffer2, 0, sizeof(buffer2));
	        		fflush(stdout);
			        flag=1;



				}
		
}

int main(int argc, char *argv[])
{
	
	if (argc!=2)
    {
      printf("USAGE: %s port\n", argv[0]);
      return 0;
    }
	memset(&addr1, 0, sizeof(addr1));
	bzero((char *) &addr1, sizeof(addr1));
	bzero((char *) &addr_connected, sizeof(addr_connected));
	bzero((char *) &addr_to_connect, sizeof(addr_to_connect));
	addr_to_connect.sin_family = AF_INET;
	addr_connected.sin_family = AF_INET;
	memset(&buffer3, 0, sizeof(buffer3));

	addr1.sin_port = htons(atoi(argv[1]));
	addr1.sin_addr.s_addr = INADDR_ANY;

	char *hostname_to_connect;
	char *port2_c;

    


	struct sigaction sa;

	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask,SIGPOLL);
	sigaddset(&sa.sa_mask,SIGALRM);

    sa.sa_handler = udptimeout;//handle alarm and send kill signal to stop recvfrom

    //if (sigfillset(&sa.sa_mask) < 0) /* block everything in handler */
    //    perror("sigfillset() failed");
    sa.sa_flags=0;

    if (sigaction(SIGALRM, &sa, 0) < 0)
        perror("\nsigaction() failed for SIGALRM");
	
	sa.sa_handler = sigpoll;
	if (sigaction(SIGPOLL, &sa, 0) < 0)
        perror("\nsigaction() failed for SIGPOLL");

	sd = socket(AF_INET,SOCK_DGRAM,0);
	

	if (fcntl(sd,F_SETOWN, getpid()) < 0){
		perror("\nfcntl F_SETOWN");
		exit(1);
	}

	//allow receipt of asynchronous I/O signals
	if (fcntl(sd,F_SETFL,FASYNC) <0 ){
		perror("\nfcntl F_SETFL, FASYNC");
		exit(1);
	}



	if(bind(sd,(struct sockaddr *)&addr1, sizeof(addr1))<0)
	{
			perror("\nbind failed");
			return 0;
	}

	if(sd<0)
	{
		perror("\ncant create socket");
		return 0;
	}
	else
	{
		int r;
		struct pollfd pollIO[2];

		/*zero is stdin*/
		pollIO[0].fd = 0;
		pollIO[1].fd = sd;
		pollIO[0].events = POLLIN | POLLPRI;
		pollIO[1].events = POLLIN | POLLPRI;

		while (1) 
		{
			fflush(stdout);
			fflush(stdin);
			if(setup==0 || terminate==1)
			{

				printf("\n?");
				terminate=0;
				setup=0;
				memset(&buffer, 0, sizeof(buffer));
				memset(&buffer2, 0, sizeof(buffer));
			}
			else if(setup==1 && terminate==0)
				 printf("\n>");
			fflush(stdin);
			fflush(stdout);
			// fgets(buffer,51, stdin);
			/*tcgetattr gets the parameters of the current terminal
			    STDIN_FILENO informs tcgetattr to write the settings
			    of stdin to oldt*/
			    tcgetattr( STDIN_FILENO, &oldt);
			    /*settings to copy*/
			    newt = oldt;
			    //put into raw mode
			    newt.c_lflag &= ~(ICANON);          

			    /*new settings will be set to STDIN
			    TCSANOW changes attributes immediately. */
			    tcsetattr( STDIN_FILENO, TCSANOW, &newt);

			//start polling

			memset(&buffer2, 0, sizeof(buffer));

			r= poll(pollIO, 2, -1);
			if (r>0) 
			{

		      /* poll() can always return these */
		      if (pollIO[0].revents & (POLLERR | POLLHUP | POLLNVAL)) 
		      {
		        printf("\nError - poll indicated stdin error");
		        break;
		      }
		      if (pollIO[1].revents & (POLLERR | POLLHUP | POLLNVAL)) 
		      {
		        printf("\nError - poll indicated socket error");
		        break;
		      }
		    }

			//if to read from stdin
			if (pollIO[0].revents & (POLLIN | POLLPRI))
			{

				memset(&buffer, 0, sizeof(buffer));
				
				i=0;
				flag=0;
			    while(1)   
			    {   
			    	
			    	if(flag==1 && terminate==0)
			        {
			        	printf(">%s",buffer );
			        	buffer[i+1]='\0';
			        	fflush(stdout);
			        	flag=0;
			        	
			        }
			        if(terminate==1)
			        {
			    		break;
			    	}
			        fflush(stdin);
			        if(read(0,&c,1)>0)
			        {
			        	//check for backspace or delete key press. ^? means delete a typed character
			        	if(c==127 || c=='\b')
			        	{
			        		fflush(stdin);
					
					        buffer[i-1]='\0';
					        i--;
					    }			        	
			        	else
			        	{
					        fflush(stdin);
					        if(c=='\n')
					        	break;
					        buffer[i]=c;
					        i++;
				    	}
			    	}

			    }
			    if(terminate==1)
			    {
			    		memset(&buffer, 0, sizeof(buffer));
			    		terminate=0;
			    		setup=0;
			    		continue;
			    }
			    //check if end chatting
			    if(buffer[0]=='e' && strlen(buffer)==1)
			    {
			    	memset(&buffer, 0, sizeof(buffer));
			    	memset(&buffer2, 0, sizeof(buffer2));

			    	buffer[0]='E';
			    	terminate=1;
			    }



				//check if end program
				if(strlen(buffer)==1 && buffer[0]=='q' )
				{	
					close(sd);
					return 0;
				}
				if(setup==0 && strlen(buffer)!=1 )
				{
					int finder=0;
					int spaces=0;
					//until end of buffer
					while(buffer[finder]!='\0')
					{
						//valid input is (ip port)
						if(buffer[finder]==' ')
							spaces++;
						finder++;
					}
					//if more than 1 space means wrong command
					if(spaces!=1)
					{
						printf("enter a valid command as: IP PORT\n");
						continue;
					}
					//splitting address and port from input
					hostname_to_connect=strtok(buffer," ");
					port2_c=strtok(NULL," ");
					
					port2=atoi(port2_c);
					

					struct hostent *client_to_connect=gethostbyname(hostname_to_connect);
					if (client_to_connect == NULL) 
					{
					        perror("\nNo such client");
					        continue;
					}
					memset(&addr_to_connect, 0, sizeof(addr_to_connect));
					bzero((char *) &addr_to_connect, sizeof(addr_to_connect));

					addr_to_connect.sin_port = htons((port2));
					bcopy( (char *)client_to_connect->h_addr, (char *)&addr_to_connect.sin_addr.s_addr, client_to_connect->h_length);
					
					if (sendto(sd, "wannatalk", strlen("wannatalk"), 0, (struct sockaddr *)&addr_to_connect, len) < 0) 
					{
						perror("\nsendto failed in wannatalk");
						continue;
					}
					alarm(7);

					memset(&buffer, 0, sizeof(buffer));

					while(recvfrom(sd, buffer, 51, 0, (struct sockaddr *)&addr_connected, &len)<0)
					{
						//if alarm goes off, reset timer and break
						if(errno==EINTR)
		        		{	
		        			alarm(0);
		        			break;
		        		}
	        		}
	        		printf("\n%s\n",buffer);
	        		fflush(stdout);
	        		if(strcmp(buffer,"OK")==0)
	        		{
	        			//go ahead and chat
	        			setup=1;
	        		}
	        		if(strcmp(buffer,"KO")==0)
	        		{
	        			//chat denied
	        			printf("\n| doesn't want to chat\n");

	        			memset(&buffer, 0, sizeof(buffer));
	        			continue;
	        		}
	        		alarm(0);


				}
				else if(setup==1)
				{

					if(terminate==0)
					{
						//append D to message to indicate chat msg
						char msg[51]={'\0'};
						msg[0]='D';
    					strcat(msg, buffer);
    					memset(&buffer, 0, sizeof(buffer));
    					strcat(buffer, msg);

					}
					if (sendto(sd, buffer, strlen(buffer), 0, (struct sockaddr *)&addr_connected, len) < 0) 
					{
						perror("\nsendto failed in buffer");
						continue;
					}
					if(terminate==1)
					{
						//if chat terminate requested.
						setup=0;
						terminate=0;
						continue;
					}
				}

			}//poll socket
			if (pollIO[1].revents & (POLLIN | POLLPRI)) 
			{

				if(setup==0)
				{

					if(recvfrom(sd, buffer, 51, 0, (struct sockaddr *)&addr_to_connect, &len)<0)
					{
						//if alarm goes off, reset timer and break
		        		perror("\nerror in recv poll");
	        		}

	        		printf("\n%s\n",buffer);
		        	fflush(stdout);
					if(strcmp(buffer,"wannatalk")==0)
				    	printf("\n| chat request from %s %d",inet_ntoa(addr_to_connect.sin_addr),ntohs(addr_to_connect.sin_port));
				   	fflush(stdout);
				    printf("\n?\n");
				   	fflush(stdout);
				    memset(&buffer, 0, sizeof(buffer));
				    memset(&buffer2, 0, sizeof(buffer));
				    //if c send ok to chat
				   	fgets(buffer,51, stdin);
				   	if(buffer[0]=='c' && strlen(buffer)==2)
				   	{

				   		addr_connected=addr_to_connect;
				   		setup=1;
					   	if (sendto(sd, "OK", strlen("OK"), 0, (struct sockaddr *)&addr_connected, len) < 0) 
						{
							perror("\nsendto failed in while c");
							return;
						}
					}
					//if n send no chat
					if(buffer[0]=='n' && strlen(buffer)==2)
				   	{

				   		setup=0;
					   	if (sendto(sd, "KO", strlen("KO"), 0, (struct sockaddr *)&addr_to_connect, len) < 0) 
						{
							perror("\nsendto failed in while c");
							return;
						}
					}

				}
			
			}



		}
		/*restore the old settings*/
		tcsetattr( STDIN_FILENO, TCSANOW, &oldt);


	}
		
}

