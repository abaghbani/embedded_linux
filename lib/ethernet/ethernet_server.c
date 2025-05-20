#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>

#define PORT_NUMBER		3333

int server(void)
{
	printf("Configuring environment... ");
	char data[2];
	struct sockaddr_in server_add;
	struct sockaddr_in client;
	int client_len = sizeof(client);
	int id, idReuse=1, son, aux;
	printf("done!\n");

	printf("Creating socket... ");
	id = socket(AF_INET, SOCK_STREAM, 0);		// ipv4 , tcp
	if (id == -1)
		return -1;
	printf("done!\n");

	printf("Configuring socket... ");
	if(setsockopt(id,SOL_SOCKET,SO_REUSEADDR,&idReuse,sizeof(idReuse)) == -1)
		return -1;
	printf("done!\n");

	printf("Binding... ");
	server_add.sin_port = htons(PORT_NUMBER);
	server_add.sin_family = AF_INET;
	server_add.sin_addr.s_addr = INADDR_ANY;
	if(bind(id, (struct sockaddr *)&server_add, sizeof(server_add)) == -1)
	{
		close (id);
		return -1;
	}
	printf("done!\n");

	printf("Listening... ");
	if(listen(id, 3) == -1)
	{
		close(id);
		return -1;
	}
	printf("done!\n");
	
	printf("Accepting... \n");
	son = accept(id, (struct sockaddr *)&client, (socklen_t*)&client_len);
	if (son == -1)
		return -1;
	printf("done!\n");
	printf("Client port = %d \n", ntohs(client.sin_port));
	printf("Client addr = %s \n", inet_ntoa(client.sin_addr));
	
	printf("Reading... ");
	aux = read(son, data , 1);
	if(aux!=1)
		return -1;
	printf("\"%c\" ", data[0]);
	printf("done!\n");

	printf("Writing \"S\"... ");
	aux = send(son, "S", 1, MSG_NOSIGNAL);
	if(aux < 0)
		return -1;
	printf("done!\n");

	return 0;
}