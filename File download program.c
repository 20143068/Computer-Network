// Student ID : 20143068
// Name : Junkyo Seo

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

#define PROMPT() {printf("\n> ");fflush(stdout);}
#define GETCMD "get"
#define QUITCMD "quit"

int count(char *c, char x);  //파일 이름을 구하기 위해 count를 구하는 함수

int main() {
	int socktoserver = -1;
	char buf[BUFSIZ];
	struct hostent *hostp;
	struct sockaddr_in server;
	int sock;

	printf("Student ID : 20143068\n");
	printf("Name : Junkyo Seo\n");

	for (;;) {
			PROMPT();
		if (!fgets(buf, BUFSIZ - 1, stdin)) {
			if (ferror(stdin)) {
				perror("stdin");
				exit(1);
			}
			exit(0);
		}

		char *cmd = strtok(buf, " \t\n\r");

		if((cmd == NULL) || (strcmp(cmd, "") == 0)) {
			PROMPT();
			continue;
		} else if(strcasecmp(cmd, QUITCMD) == 0) {
				exit(0);
		}

		if(!strcasecmp(cmd, GETCMD) == 0) {
			printf("Wrong command %s\n", cmd);
			PROMPT();
			continue;
		}

		if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
			perror("socket");
			exit(1);
		}

		// connect to a server
		char *hostname = strtok(NULL, " \t\n\r");
		char *pnum = strtok(NULL, " ");
		char *filename = strtok(NULL, " \t\n\r");

		if ((hostp = gethostbyname(hostname)) == 0) {
					fprintf(stderr, "%s: unknown host\n", hostname);
					exit(1);
		}

		memset((void *) &server, 0, sizeof (server));  //sever구조체에 내용 복사
		server.sin_family = AF_INET;
		server.sin_port = htons((u_short)atoi(pnum));
		memcpy((void *) &server.sin_addr, hostp->h_addr, hostp->h_length);

		if (connect(sock, (struct sockaddr *)&server, sizeof (server)) < 0) {
			(void) close(sock);
			fprintf(stderr, "connect");
			exit(1);
		}
		
		char message[300] = "GET ";  //request message를 저장할 배열
		strcat(message, filename);
		char message2[30] = " HTTP/1.0\r\n";
		strcat(message, message2);
		char message3[20] = "Host: ";
		strcat(message, message3);
		strcat(message, hostname);
		char message4[50] = "\r\nUser-agent: HW2/1.0\r\nConnection: close\r\n\r\n";
		strcat(message, message4);

		send(sock, message, strlen(message), 0);  //server에 request message 전송

		//파일 이름 구하기
		char *subfn;  //파일 이름이 저장될 변수
		int cnt1 = count(filename, '/');
		subfn = strtok(filename, "/");
		while (cnt1 > 1)  // /의 갯수를 구하고, 마지막 / 전까지 토큰으로 자른다.
		{
			subfn = strtok(NULL, "/");
			cnt1--;
		}

		FILE *fp = fopen(subfn, "w");

		char fname[100];
		strcpy(fname, subfn);  //파일 이름 복사

		char *tmp;
		char *hdpos;  //헤더의 위치를 저장
		char filebuf[BUFSIZ];  //읽을 파일의 내용이 저장될 버퍼
		char *fsize;  //string형식으로 읽어올 response message에 있는 파일의 크기
		int rplen = recv(sock, buf, BUFSIZ, 0); //response message 길이 계산
		int totalSize = 0;
		int percent;  //다운로드 진행상황을 표시할 비율
		int hdsize;  //헤더의 크기
		int bytes;  //스트링형의 byte 크기를 정수 형태로 변환하여 저장할 정수

		//response header에서 Content-Length값 읽기
		tmp = strstr(buf, "Content-Length"); //Content-Length로 시작하는 위치를 받아옴
		hdpos = strstr(tmp, "\r\n\r\n");
		hdpos += 4;  //\r\n\r\n 문자만큼 위치 이동
		hdsize = hdpos - buf;  //헤더의 크기 계산
		memcpy(filebuf, &buf[hdsize], rplen - hdsize); //filebuf에 file 시작 부분 복사

		fwrite(filebuf, rplen - hdsize, 1, fp);
		totalSize += rplen - hdsize;  //총 response message에서 header를 뺀 것이 file의 총 크기
		memset(filebuf, 0, BUFSIZ);

		//response message에서 파일 크기 구하기
		fsize = strtok(tmp, "\n");
		fsize += 16;  //헤더에서 파일 크기를 구하기 위해 위치 이동 (Content-Length: 가 총 16bytes)
		bytes = atoi(fsize);  //string형 fsize를 정수형으로 변환
		printf("Total Size %d bytes\n", bytes);
		memset(buf, 0, BUFSIZ);
		int fileSize = bytes;
		percent = 10;

		//파일을 다운로드받고, 전체 용량의 10%를 받을 때마다 메시지 출력
		while((rplen = read(sock, buf, BUFSIZ)) > 0) {
			totalSize += rplen;
			//전송된 파일 사이즈가 총 파일 사이즈보다 크면 break
			while(totalSize >= fileSize * percent/100) {
				printf("Current Downloading %d / %d (bytes) %d%%\n", (fileSize * percent / 100), fileSize, percent);
				percent += 10;

				if (percent > 100) {
					printf("Download Complete: %s, %d/%d\n", fname, fileSize, fileSize);
				}
			}
			if(totalSize >= bytes) {
				rplen -= (totalSize - bytes);
				fwrite(buf, rplen, 1, fp);
				memset(buf, 0, BUFSIZ);
				break;
			}
			else {
			fwrite(buf, rplen, 1, fp);
			memset(buf, 0, BUFSIZ);
			}
		}
		
		fclose(fp);
		close(sock);
	}
}

int count(char *c, char x)  // 특정 문자의 갯수를 구하는 함수
{
	int i, cnt;
	cnt = 0;
	for(i = 0; c[i] != '\0' ; i++)
	{
		if(c[i] == x)
				cnt++;
		else
		 		continue;
	}
	return cnt;
}
