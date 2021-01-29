//TODO: Where command is entered. add a console output
//requesting user for input
//not just a blank as it is now
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#define PORT 8080

int main(int argc, char const *argv[])
{
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char message[1024] = {0};
	char buffer[1024] = {0};
	int length;
	char *result;
	int i = 0;
	int j = 0;
	int check = 0;
	char username[12] = {0};
	char district[12] = {0};

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}

	printf("Health Oficer Username:");
	scanf(" %s", username);
	getchar();
	send(sock, username, strlen(username), 0);
	printf("Your District:");
	scanf(" %s", district);
	getchar();
	send(sock, district, strlen(district), 0);
	printf("\n");

	//Adding provision for a Client user to type a message
	printf("Please Enter A Command\n");
	printf("----------------------------------------------------------------\n");
	printf("Please use commnads exactly as they appear!!\n");
	printf("1) To register a patient: Addpatient patient_name(Tonny_Trevor),date(YYYY-MM-DD),gender(M/F),status(A/S)\n");
	printf("2) To submit new patients from a file: Addpatient filename.txt\n");
	printf("3) To check status of the file: Check_status\n");
	printf("4) To Search for records by name or date: Search criteria \n\te.g Search Tonny_Baw or Search 2021-01-28\n");
	printf("5) To End session: exit\n");
	printf("----------------------------------------------------------------\n");

	do
	{
		fgets(message, 1024, stdin); //gets commands from the client

		length = strlen(message);
		send(sock, message, strlen(message), 0); //sends commands to the server

		printf("Command Sent!\n");
		valread = recv(sock, buffer, 1024, 0);
		buffer[valread] = '\0';
		
		if (strcmp(buffer, "exit") != 0)
		{
			printf("%s\n", buffer);
			printf("\n");
		}
		else
		{
			//close connection if server response is exit
			return 0;
		}
		
		//remove the new line got from fgets so as to check if
		//command(message) is exit
		//message[strcspn(message, "\n")] = 0;
	} while ((strcmp(message, "Exit") != 0) && (strcmp(message, "exit") != 0));
	return 0;
}
