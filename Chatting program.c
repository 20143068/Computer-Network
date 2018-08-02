/*
Student ID : 20143068
Name : Seo Junkyo
*/

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void display();


int peertcpSocket = -1;

int main(int argc, char **argv) {

	int tcpServ_sock;

	struct sockaddr_in tcpServer_addr;
	struct sockaddr_in tcpClient_addr;
	struct sockaddr_in newTcp_addr;

	int clnt_len;

	fd_set reads, temps;
	int fd_max;

	char command[1024];
	char tmpCmd[1024];	//입력받은 데이터를 임시로 저장

	char *tcpport = NULL;
	char *userid = NULL;
	char *tStr;
	char *blank = " ";
	char *cmd[3];  //토큰 단위로 자른 문자열을 저장하는 포인터 배열
	struct hostent *hostp;
	int bytesread;
	if(argc != 3) {
		printf("Usage : %s <tcpport> <userid>\n", argv[0]);
		exit(1);
	}

	display();

	// NEED TO CREATE A SOCKET FOR TCP SERVER

	tcpServ_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(tcpServ_sock < 0) {
		perror("socket");
		exit(1);
	}

	memset(&tcpServer_addr, 0, sizeof(tcpServer_addr));
	tcpServer_addr.sin_family = AF_INET;
	tcpServer_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	tcpServer_addr.sin_port = htons(atoi(argv[1]));

	// NEED TO bind
	if(bind(tcpServ_sock, (struct sockaddr *) &tcpServer_addr, sizeof(tcpServer_addr)) < 0) {
		perror("bind");
		exit(1);
	}

	// NEED TO listen
	if(listen(tcpServ_sock, SOMAXCONN) < 0) {
		perror("listen");
		exit(1);
	}

	// initialize the select mask variables and set the mask with stdin and the tcp server socket

	FD_ZERO(&reads);
	FD_SET(tcpServ_sock, &reads);
	FD_SET(fileno(stdin), &reads);
	tcpport = argv[1];
	userid = argv[2];
	fd_max = tcpServ_sock;

	printf("%s> \n", userid);

	while(1) {
		int nfound;

		temps = reads;

		nfound = select(fd_max+1, &temps, 0, 0, NULL);

		if(FD_ISSET(fileno(stdin), &temps)) {  //client측
			// Input from the keyboard
			fgets(command, sizeof(command), stdin);
			FD_CLR(fileno(stdin), &temps);
			strcpy(tmpCmd, command);
			tStr = strtok(tmpCmd, blank);  //공백을 기준으로 문자열 분리
			cmd[0] = tStr;

			if(!strcmp(cmd[0], "@talk")){    //client가 server에게 @talk로 대화 요청을 시도할 때
				cmd[1] = strtok(NULL, blank);
				cmd[2] = strtok(NULL, blank);
				hostp = gethostbyname(cmd[1]);

				memset((void *)&newTcp_addr, 0, sizeof(newTcp_addr));
				newTcp_addr.sin_family = AF_INET;
				newTcp_addr.sin_port = htons(atoi(cmd[2]));
				memcpy((void *)&newTcp_addr.sin_addr, hostp->h_addr, hostp->h_length);

				close(peertcpSocket);
				peertcpSocket = socket(PF_INET, SOCK_STREAM, 0);
				FD_SET(peertcpSocket, &reads);
				fd_max = peertcpSocket;

				if(connect(peertcpSocket, (struct sockaddr*)&newTcp_addr, sizeof(newTcp_addr)) < 0) {
					perror("connect");
					exit(1);
				}
			}

			else if(!strcmp(command, "@quit\n")) {  //@quit에 의한 연결 종료 처리
				close(peertcpSocket);
				fd_max = peertcpSocket = tcpServ_sock;
				break;
			}

			else {
				strcpy(tmpCmd, userid);
				strcat(tmpCmd," : ");
				strcat(tmpCmd, command);
				write(peertcpSocket, tmpCmd, strlen(tmpCmd));
			}
			printf("%s> \n", userid);
		}

		else if(FD_ISSET(tcpServ_sock, &temps))  //서버측, 서버가 클라이언트의 연결을 허용
		{
			clnt_len = sizeof(tcpClient_addr);
			peertcpSocket = accept(tcpServ_sock, (struct sockaddr*)&tcpClient_addr, &clnt_len);
			if(peertcpSocket < 0) {
				perror("accept");
				exit(1);
			}

			FD_SET(peertcpSocket, &reads);
			fd_max = peertcpSocket;
			fprintf(stdout,"Connection form host %s, port %d, socket %d\n",
					inet_ntoa(tcpClient_addr.sin_addr), ntohs(tcpClient_addr.sin_port), peertcpSocket);  //연결된 client 정보 출력
		}

		else if(FD_ISSET(peertcpSocket, &temps))  //client측 메시지 처리
		{
			bytesread = read(peertcpSocket, command, sizeof(command));

			if(bytesread < 0) {
				perror("read");
				exit(1);
			}

			else if (bytesread == 0){
				fprintf(stdout, "Connection Closed %d\n", peertcpSocket);
				close(peertcpSocket);
				fd_max = peertcpSocket = tcpServ_sock;
			}

			else {
				write(1, command, bytesread);
				printf("\n");
			}
		}
	}//while End
	close(tcpServ_sock);
	return 0;
}//main End

void display() {
	printf("Student ID : 20143068 \n");
	printf("Name : Seo Junkyo  \n");
}
