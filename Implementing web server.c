// Student ID : 20143068
// Name : Seo Junkyo

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int portnum = 0;
int main(int argc, char *argv[])  //get the argument as a port number
{
	struct sockaddr_in server, remote;
	int request_sock, new_sock;
	int bytesread;
	int addrlen;
	char buf[BUFSIZ];

	if (argc != 2) {
		(void) fprintf(stderr,"usage: %s portnum\n", argv[0]);
		exit(1);
	}
	if ((request_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket");
		exit(1);
	}
	printf("Student ID : 20143068\n");
	printf("Name : Junkyo Seo\n");

  // Create a Server Socket //
	memset((void *)&server, 0, sizeof(server));
	server.sin_family = AF_INET; //IPv4 address system
	server.sin_addr.s_addr = INADDR_ANY; //input the IP address of server
	server.sin_port = htons((u_short)atoi(argv[1])); //input the port number

	if (bind(request_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("bind");  //create a socket using bind function
		exit(1);
	}
	if (listen(request_sock, SOMAXCONN) < 0) {  //wait the access of client
		perror("listen");
		exit(1);
	}

	while(1)
	{
		addrlen = sizeof(remote);
		new_sock = accept(request_sock, (struct sockaddr *)&remote, &addrlen); //accept the request of client
		if (new_sock < 0) {
			perror("accept");
			exit(1);
		}
		printf("connection from host %s, port %d, socket %d\n",
		inet_ntoa(remote.sin_addr), ntohs(remote.sin_port), new_sock);
		//inet_ntoa : convert the IP address of struct type to string type
		//ntohs : convert the value of u_short type to host byte order

		while(1)
		{
			bytesread = read(new_sock, buf, sizeof(buf)-1);
			if (bytesread <= 0) {
				printf("server: end of file on %d\n", new_sock);
				if (close(new_sock))
					perror("close");
				break;
			}

			buf[bytesread] = '\0';
			printf("%s", buf);

			char *wbuf; //the array that has contents of the file for a client
			char *header = "HTTP/1.1 200 OK\r\n\n"; //HTTP response header (It notices that the request is successful)
			int size; //the number of the bytes of the file that is going to be sent to a client

			FILE *fp = fopen("biga.html","r");
			fseek(fp, 0, SEEK_END); //send the file pointer to the end of the file in order to find the number of bytes of the file
			size = ftell(fp); //return the position indicating the file pointer using ftell function

			wbuf = malloc(size);
			memset(wbuf, 0, size); //Initialize the buffer
			fseek(fp, 0, SEEK_SET); //Initialize the position of file pointer to get the contents of the file
			fread(wbuf, size, 1, fp); //read the data from file stream
			fclose(fp);

			send(new_sock, header, strlen(header), 0); //send HTTP response header to a client
			send(new_sock, wbuf, strlen(wbuf), 0); //send contents of the file to a client

			printf("finish %d %d\n", size, strlen(wbuf)); //the number of bytes sent and the number of bytes of the file
			free(wbuf); //Release memory that is allocated in the buffer
		}
	}
}
