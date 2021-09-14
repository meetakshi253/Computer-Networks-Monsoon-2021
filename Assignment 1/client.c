#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define PORT 8888
#define MAX_BUF 100
char info[100];
int max = -1;

void processdata(char *stat)
{
    char *token, *pid, *pname;
    int local = 0, count = 0;
    token = strtok(stat, " ");
    while (token != NULL)
    {
        if (count == 0)
        {
            pid = token;
        }
        else if (count == 1)
        {
            pname = token;
        }
        else if (count == 13)
        {
            local += atoi(token);
        }
        else if (count == 14)
        {
            local += atoi(token);
            if (local > max)
            {
                max = local;

                int length = 0;
                memset(info, 0, strlen(info));
                length += snprintf(info + length, MAX_BUF - length, "%s, ", pid);
                length += snprintf(info + length, MAX_BUF - length, "%s, ", pname);
                length += snprintf(info + length, MAX_BUF - length, "%d", local);
            }
            return;
        }
        count++;
        token = strtok(NULL, " ");
    }
}

void maxproc()
{
    struct dirent *files;
    char *pathname = malloc(MAX_BUF);
    char str[8];
    char stat[200];
    DIR *dir = opendir("/proc");
    if (dir == NULL)
    {
        printf("Cannot open directory\n");
        exit(1);
    }
    while ((files = readdir(dir)) != NULL)
    {
        sprintf(str, "%d", atoi(files->d_name));
        if (strlen(files->d_name) == strlen(str))
        {
            int length = 0;
            length += snprintf(pathname + length, MAX_BUF - length, "/proc/");
            length += snprintf(pathname + length, MAX_BUF - length, "%s", str);
            length += snprintf(pathname + length, MAX_BUF - length, "/stat");
            int fd = open(pathname, O_RDONLY);
            if (fd == -1)
            {
                perror(pathname);
                continue;
            }
            int sz = read(fd, stat, sizeof(stat));
            if (sz == -1)
            {
                perror("read");
                continue;
            }
            processdata(stat);
        }
    }
}

int main()
{
    struct sockaddr_in server, client;
    char N[10];
    char buf[5000];
    //int filesize=0;
    FILE *newfile;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(1);
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect");
        exit(1);
    }

    printf("Connection to server established.\n");
    while (1)
    {
        printf("Enter the number of processes: ");
        scanf("%s", &N[0]);
        if (strlen(N) > 0)
        {
            send(sockfd, N, strlen(N), 0); //0 means no flag
            break;
        }
    }

    char name[40];
    
      printf("namemmee%s ", name);


    newfile = fopen("ServerProcessData.txt", "w");
    if (newfile == NULL)
    {
        perror("open");
        exit(1);
    }

    int fn = 0;
    fwrite("---------------Client---------------------", sizeof(char), fn, newfile);

    while (1)
    {

        fn = recv(sockfd, buf, 5000, 0);
        if (fn > 0)
        {
            fwrite(buf, sizeof(char), fn, newfile);
            printf("Received file. Stored as ServerProcessData.txt\n");
            break;
        }
    }

    fclose(newfile);

    maxproc();
    if (strlen(info) > 0)
    {
        printf("Info on the file that consumes the most CPU (pid, pname, cpu time) on client's computer: %s\n", info);
        fn = send(sockfd, info, strlen(info), 0);
        if (fn < 0)
        {
            perror("send");
            exit(1);
        }
        printf("Sent info of the most CPU consuming process to Server.\n");
    }

    printf("Closing connection with server\n");
    close(sockfd);
    return 0;
}
