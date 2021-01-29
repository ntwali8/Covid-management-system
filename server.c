#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define mkstr(s) #s
#define PORT 8080

struct patientDetails
{
    char name[12];
    char signature[12];
};
void replaceeverychar(char *str, char oldChar, char newChar);
void replacefirstchar(char *str, char oldChar, char newChar);
void replacefirstcharWithString(char *str, char *oldChar, char *newStr);

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread, valread1;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char message[1024];
    int length;
    char *result;
    char *result1;
    char *result2;
    char *result3;
    char *agentname;

    char district[12] = {0};
    char username[12] = {0};
    char signature[12] = {0};
    struct patientDetails t;

    char sign2[15] = {0};
    char result4[1024] = "";
    char result5[100] = "";
    char *result6 = {0};
    char *result7 = {0};
    char reading[1024] = "";
    char reading1[1024] = "";
    char *first;
    FILE *fp, *fPt;

    char *hello = {0};
    char sign1[15] = {0};
    int i = 0;
    int j = 0;
    int check = 0;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    strcpy(district, "");

    //save username and district for reference and more functions
    valread = recv(new_socket, username, 20, 0);
    printf("*******************************\n");
    printf("Starting new Session:\n");
    printf("=======================\n");
    printf("Health Officer : %s\n", username);
    //	strcpy(t.name, username);
    valread = recv(new_socket, district, 8, 0);
    printf("District : %s\n", district);

    result6 = strcat(district, ".txt");

    fp = fopen(result6, "a");
    if (!fp)
    {
        printf("Something wrong\n");
    }
    fclose(fp);

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

                    hello = "File cannot be found!";
                    send(new_socket, hello, strlen(hello), 0);
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

                    fp = fopen(result6, "a");
                    if (!fp)
                    {
                        printf("Something wrong: Failed to open district file\n");
                    }
                    else
                    {
                        fprintf(fp, "%s", reading1);
                        fclose(fp);

                        printf("\n%s Successful\n", buffer);
                        hello = "Addpatient list Successful";
                        send(new_socket, hello, strlen(hello), 0);
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

                fp = fopen(result6, "a+");
                fseek(fp, 12, SEEK_SET);
                if (!fp)
                {
                    printf("Something wrong\n");
                }
                agentname = ",";
                //strcat(honame, username);

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
                hello = "Addpatient Successful";
                send(new_socket, hello, strlen(hello), 0);
            }
        }
        else if (result = strstr(buffer, "Check_status"))
        {
            fp = fopen(result6, "r");
            if (!fp)
            {
                printf("Failed to open dstrict file to check status\n");
                hello = "District file missing.\nFailed to check status\n";
                send(new_socket, hello, strlen(hello), 0);
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
        else if (result = strstr(buffer, "exit"))
        {
            printf("\nClosing session on client request\n");
            hello = "exit";
            send(new_socket, hello, strlen(hello), 0);
        }
        else if (result = strstr(buffer, ""))
        {
            //impelement code to keep the server running
            //waiting for a new connection after the client closes.
            //for now it just terminates
            //removing the code below causes an infinite loop
            printf("\nClosed\n");
            hello = "\nclosed";
            send(new_socket, hello, strlen(hello), 0);
            
        }
        else
        {
            printf("\nInvalid command recieved.");
            printf("\n(%s)", buffer);
            hello = "\nInvalid command!.\nPlease try again";
            send(new_socket, hello, strlen(hello), 0);
        }

        //remove the new line from command(buffer) to check if its exit
		//buffer[strcspn(buffer, "\n")] = 0;
    } while ( (strcmp(buffer, "Exit") != 0) && (strcmp(buffer, "exit") != 0) );
    return 0;
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
