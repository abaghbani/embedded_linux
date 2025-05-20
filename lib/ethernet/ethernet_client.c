#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT_NUMBER		3333
#define HOST_NAME		"sdr"

int client(void)
{
	printf("Configuring environment... ");

	char data[2];
	struct sockaddr_in dir;
	struct hostent *host;
	int aux, id;

	dir.sin_port = PORT_NUMBER;
	dir.sin_family = AF_INET;
	host = gethostbyname(HOST_NAME);
	if(host == NULL)
		return -1;
	dir.sin_addr.s_addr = ((struct in_addr *)(host->h_addr))->s_addr;
	printf("done!\n");

	printf("Creating socket... ");
	id = socket(AF_INET, SOCK_STREAM, 0);
	if(id == -1)
		return -1;
	printf("done!\n");

	printf("Connecting... ");
	if(connect(id, (struct sockaddr *)&dir, sizeof(dir)) == -1)
		return -1;
	printf("done!\n");

	printf("Writing \"C\"... ");
	aux = send(id, "C", 1, MSG_NOSIGNAL);
	if(aux < 0)
		return -1;
	printf("done!\n");

	printf("Reading... ");
	aux = read(id, data , 1);
	if(aux!=1)
		return -1;
	printf("\"%c\"", data[0]);
	printf(" done!\n");
	return 0;
}