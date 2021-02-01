#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8080

int fdmax, sockfd, nsockfd;
fd_set master;
int headerprinted = 0;
char *filename = {0};

void replaceeverychar(char *str, char oldChar, char newChar);
void replacefirstchar(char *str, char oldChar, char newChar);
void replacefirstcharWithString(char *str, char *oldChar, char *newStr);

void connect_await(int *socketInit, struct sockaddr_in *address)
{
    int opt = 1;

    // Creating socket file descriptor
    if ((*socketInit = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(*socketInit, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(*socketInit, (struct sockaddr *)address,
             sizeof(struct sockaddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(*socketInit, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("\nServer Waiting for client connection...\n");
    fflush(stdout);
}
void connect_accept(fd_set *master, int *fdmax, int *socketNew, int socketInit, int addressLength, struct sockaddr_in *client_addr)
{
    if ((*socketNew = accept(socketInit, (struct sockaddr *)&client_addr,
                             (socklen_t *)&addressLength)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    else
    {
        FD_SET(*socketNew, master);
        if (*socketNew > *fdmax)
        {
            *fdmax = *socketNew;
        }
        printf("\n==Client connection request recieved.==\n");
    }
    headerprinted = 0;
}

void createSession(char *clientDetails, char *username, char *district);

void attendtoClient(int new_socket, char *username, char *buffer);

int main(int argc, char const *argv[])
{

    int new_socket;
    struct sockaddr_in address, client_addr;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    char district[12] = {0};
    char username[12] = {0};
    
    //fd_set master; //declared globally instead, since it has to be modified in global function
    fd_set read_fds;
    int i;

    sockfd = 0;
    nsockfd = 0;
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    connect_await(&sockfd, &address);

    FD_SET(sockfd, &master);
    fdmax = sockfd;

    while (1)
    {
        read_fds = master;
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("select");
            exit(4);
        }

        for (i = 0; i <= fdmax; i++)
        {
            if (FD_ISSET(i, &read_fds))
            {
                if (i == sockfd)
                    connect_accept(&master, &fdmax, &nsockfd, sockfd, addrlen, &client_addr);
                else
                {
                    int nbytes_recvd;
                    new_socket = nsockfd;
                    char clientDetails[1024] = {0};
                    strcpy(district, "");

                    if ((nbytes_recvd = read(i, clientDetails, sizeof(clientDetails))) <= 0)
                    {
                        if (nbytes_recvd == 0)
                        {
                            printf("Socket %d has closed unexpectedly!!\n", new_socket);
                            printf("***********************************\n");
                            printf("\nServer Waiting for client connection...\n");
                        }
                        else
                        {
                            perror("session");
                        }
                        close(new_socket);
                        FD_CLR(new_socket, &master);
                    }
                    else
                    {

                        createSession(clientDetails, username, district);

                        attendtoClient(new_socket, username, buffer);
                    }
                }
            }
        }
    }
    return 0;
}

void createSession(char *clientDetails, char *username, char *district)
{
    if (headerprinted == 0)
    {
        printf("*******************************\n");
        printf("Starting new Session:\n");
        printf("=======================\n");
        headerprinted = 1;
    }

    char *usernameTemp1, *districtTemp1;
    char clientDetailsCopy1[1024] = {0};
    char clientDetailsCopy2[1024] = {0};

    //Create 2 copys of client details.
    strcpy(clientDetailsCopy1, clientDetails);
    strcpy(clientDetailsCopy2, clientDetails);

    //Extract username from clientDetailsCopy1.
    usernameTemp1 = strtok(clientDetailsCopy1, "\n");
    strcpy(username, usernameTemp1);

    //Extracting district from clientDetailsCopy2.
    districtTemp1 = strchr(clientDetailsCopy2, '\n');
    districtTemp1 += 1;
    strcpy(district, districtTemp1);

    printf("Health Officer : %s\n", username);
    printf("District : %s\n", district);

    filename = strcat(district, ".txt");
}

void attendtoClient(int new_socket, char *username, char *buffer)
{

    int valread;
    char *result;
    char *result1;
    char *result2;
    char *result3;
    char result4[1024] = "";
    int length;
    FILE *fp, *fPt;
    char *server_resp = {0};
    char reading[1024] = "";
    char reading1[1024] = "";

    //This if is to ensure a non empty filename is used.
    if (strlen(filename) > 4)
    {
        fp = fopen(filename, "a");
        if (!fp)
        {
            printf("Failed to open file. Possibly invalid file name\n");
        }
        fclose(fp);
    }

    do
    {
        valread = recv(new_socket, buffer, 1024, 0);
        buffer[valread] = '\0';
        length = strlen(buffer);

        if (result = strstr(buffer, "Addpatient"))
        {
            if (result1 = strstr(buffer, "txt"))
            {
                //Add patient details from uploading the patient file
                result2 = strchr(buffer, ' ');
                result2 += 1;
                result3 = strtok(result2, "\n");

                fp = fopen(result3, "r");
                if (!fp)
                {
                    printf("Something wrong: Can't find file!\n");

                    server_resp = "File cannot be found!";
                    send(new_socket, server_resp, strlen(server_resp), 0);
                }
                else
                {
                    while (!feof(fp))
                    {
                        fgets(reading, 1024, fp);
                        printf("%s", reading); //writing data to file
                        strcat(reading1, reading);
                        strcpy(reading, "");
                    }
                    fclose(fp);

                    fp = fopen(filename, "a");
                    if (!fp)
                    {
                        printf("Something wrong: Failed to open district file\n");
                    }
                    else
                    {
                        fprintf(fp, "%s", reading1);
                        fclose(fp);

                        printf("\n%s Successful\n", buffer);
                        server_resp = "Addpatient list Successful";
                        send(new_socket, server_resp, strlen(server_resp), 0);
                    }
                }
            }
            else
            {
                printf("******Adding from commandline");
                //Adds the patient details using the commandline
                result2 = strchr(buffer, ' ');
                result2 += 1;
                result3 = strtok(result2, ".");

                while (result3 != NULL)
                {
                    char coma[] = ",";
                    strcat(result4, result3);
                    result3 = strtok(NULL, ".");
                }

                replaceeverychar(result4, '_', ' ');
                //replacefirstcharWithString(result4,",", "\t");
                printf("%s\n", result4);

                fp = fopen(filename, "a+");
                fseek(fp, 12, SEEK_SET);
                if (!fp)
                {
                    printf("Something wrong\n");
                }

                int detailsLen = strlen(result4);
                //the newline character from the details so ass to concat HO's name
                if (result4[detailsLen - 1] == '\n')
                    result4[detailsLen - 1] = ',';
                //attach HO's name to the patient details
                strcat(result4, username);
                //replace teh comma delimeter with a space.
                replaceeverychar(result4, ',', ' ');
                fprintf(fp, "\n%s", result4); //adding the files to the patient file
                fclose(fp);
                strcpy(result4, ""); //reseting the string

                printf("Addpatient Successful\n");
                server_resp = "Addpatient Successful";
                send(new_socket, server_resp, strlen(server_resp), 0);
            }
        }
        else if (result = strstr(buffer, "Check_status"))
        {
            fp = fopen(filename, "r");
            if (!fp)
            {
                printf("Failed to open dstrict file to check status\n");
                server_resp = "District file missing.\nFailed to check status\n";
                send(new_socket, server_resp, strlen(server_resp), 0);
            }
            else
            {
                printf("\nchecking for how many patients are in the file...\n");
                int count = 0;
                char temp;
                //loop to check every character in the file
                for (temp = getc(fp); temp != EOF; temp = getc(fp))
                {
                    //check if a new line character is got
                    if (temp == '\n')
                    {
                        count = count + 1;
                    }
                }

                printf("Found %d registered patients in this district file\n", count);
                fclose(fp);
                char out[1024];
                snprintf(out, sizeof(out), "There are %d registered patients in your district file", count);

                send(new_socket, out, strlen(out), 0);
            }
        }
        else if (strncmp(buffer, "Search", 6) == 0)
        {
            //get command minus the Search word.
            result2 = strchr(buffer, ' ');
            result2 += 1;
            //remove the newline char at the end.
            result3 = strtok(result2, "\n");
            //result3 is the search term

            char line[200] = {0};
            char **resultsArray = NULL;
            int count = 0; //array index counter

            //get each line
            fp = fopen(filename, "r");
            if (!fp)
            {
                printf("Failed to open dstrict file for search\n");
                server_resp = "District file missing.\nFailed to check status\n";
                send(new_socket, server_resp, strlen(server_resp), 0);
            }
            else
            {
                printf("\nSearching file...\n");
                while (fgets(line, 200, fp))
                {
                    line[strcspn(line, "\n")] = 0; //removes the new line character

                    //Structure of line is:
                    //Sirname Firstname Date Gender Status HOname
                    //remove HOname, Status and Gender using this for loop.
                    // since they are not part of the criteria.
                    //There are 3 spaces after Date thats why loop runs 3 times
                    //Loop removes everything after a space starting from the end.
                    for (int i = 0; i < 3; i++)
                    {
                        char *del = &line[strlen(line)];

                        while (del > line && *del != ' ')
                            del--;

                        if (*del == ' ')
                            *del = '\0';
                    }
                    //New line contains only Sirname, Firstname and date.
                    //check for query word in line
                    if (strstr(line, result3) != NULL)
                    {
                        //adding results to array of results.
                        count++;
                        resultsArray = (char **)realloc(resultsArray, (count + 1) * sizeof(*resultsArray));
                        resultsArray[count - 1] = (char *)malloc(sizeof(line));
                        strcpy(resultsArray[count - 1], line);
                    }
                }
                printf("Search complete!\n");
                //gave it a very large size to accomodate very large search results.
                //incase we have very many search results to print to the client.
                char out[2097152];
                if (!resultsArray)
                {
                    printf("No results found\n");
                    strcpy(out, "No results found");
                    send(new_socket, out, strlen(out), 0);
                }
                else
                {
                    printf("%d Result(s) found, sending results...\n", count);
                    snprintf(out, sizeof(out), "%d Result(s) found\n", count);
                    strcat(out, "====================\n");
                    int i = 0;
                    while (i < count)
                    {
                        char resultLine[1024];
                        snprintf(resultLine, sizeof(resultLine), "%s\n", resultsArray[i]);
                        strcat(out, resultLine);
                        i++;
                    }
                    strcat(out, "====================");

                    send(new_socket, out, strlen(out), 0);
                }
                fclose(fp);
            }
        }
        else if (result = strstr(buffer, "exit"))
        {
            printf("\nClosing session on client(%d) request\n", new_socket);
            printf("**************************************\n");
            printf("\nServer Waiting for client connection...\n");

            server_resp = "exit";
            send(new_socket, server_resp, strlen(server_resp), 0);
            close(new_socket);
            FD_CLR(new_socket, &master);
            break;
        }
        else if (strcmp(buffer, "") == 0)
        {
            //Checks if socket closed
            printf("\nClosed\n");
            //Breaks from loop for attending to client
            //in order to request for a new session.
            break;
        }
        else
        {
            //Run if an invalid command is recieved.
            if (strstr(buffer, "\n"))
            {
                buffer[strcspn(buffer, "\n")] = 0;
            }

            printf("\nInvalid command recieved.\n");
            printf("Ignoring-(%s)\n", buffer);
            server_resp = "\nInvalid command!.\nPlease refer to list above for accepted commands!";
            send(new_socket, server_resp, strlen(server_resp), 0);
        }

    } while ((strcmp(buffer, "exit") != 0));
}

void replaceeverychar(char *str, char oldChar, char newChar)
{
    //Replace every occurrence of a character
    int i = 0;

    for (i = 0; str[i]; i++)
    {
        if (str[i] == oldChar)
        {
            str[i] = newChar;
        }
    }
}

void replacefirstchar(char *str, char oldChar, char newChar)
{
    //Replace first occurrence of a character
    int i = 0;

    /* Run till end of string */
    while (str[i] != '\0')
    {
        /* If an occurrence of character is found */
        if (str[i] == oldChar)
        {
            str[i] = newChar;
            break;
        }

        i++;
    }
}

void replacefirstcharWithString(char *str, char *oldChar, char *newStr)
{
    // Get the resulting length: str, plus newStr,
    // plus terminator, minus one replaced character
    // (the last two cancelling each other out).
    char *s3 = malloc(strlen(str) + strlen(newStr));

    // The number of characters of str that are not oldChar.
    // This does search for "any character from the second
    //# is oldChar
    // *string*", so "#", not '#'.
    size_t pos = strcspn(str, oldChar);

    // Copy up to '#' from str
    strncpy(s3, str, pos);

    // Append s2
    strcat(s3, newStr);

    // Append from str after the oldChar
    strcat(s3, str + pos + 1);
    strcpy(str, "");
    strcat(str, s3);
}
