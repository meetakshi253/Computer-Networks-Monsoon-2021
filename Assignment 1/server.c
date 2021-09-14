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
#include <pthread.h>
#include <sys/sendfile.h>

#define MAX_BUF 1024
char info[100];
int p = 0;
struct processinfo
{
    int pid;
    char pname[20];
    int pcpu;
};
struct processinfo PROCESSES[30000];

int comparator(const void *p1, const void *p2)
{
    struct processinfo pr1 = *(const struct processinfo *)p1;
    struct processinfo pr2 = *(const struct processinfo *)p2;
    return (pr2.pcpu - pr1.pcpu);
}

void writeToFile(int N)
{
    FILE *file;
    char info[50];
    file = fopen("ProcessData.txt", "w");
    if (file == NULL)
    {
        perror("fopen");
        exit(1);
    }
    if (p - 1 < N)
    {
        N = p - 1;
    }

    for (int i = 0; i < N; i++)
    {
        int length = 0;
        memset(info, 0, 50);
        length += snprintf(info + length, MAX_BUF - length, "%d, ", PROCESSES[i].pid);
        length += snprintf(info + length, sizeof(PROCESSES[i].pname)/sizeof(char), "%s, ", PROCESSES[i].pname);
        length += snprintf(info + length, MAX_BUF - length, "%d\n", PROCESSES[i].pcpu);
        fprintf(file, "%s", info);
    }
    fclose(file);
} 

void processdata(char *stat)
{
    char *token;
    int local = 0, count = 0;
    token = strtok(stat, " ");
    while (token != NULL)
    {
        if (count == 0)
        {
            PROCESSES[p].pid = atoi(token);
        }
        if (count == 1)
        {
            // memset(PROCESSES[p].pname, 0,20);
            strcpy(PROCESSES[p].pname, token);
        }
        if (count == 13)
        {
            local += atoi(token);
        }
        if (count == 14)
        {
            local += atoi(token);
            PROCESSES[p].pcpu = local;
            int length = 0;
            if (PROCESSES[p].pcpu > 10)
                p++;
            return;
        }
        count++;
        token = strtok(NULL, " ");
    }
}

void maxproc(int N)
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
            //printf("filepath:: %s ", pathname);
            if (fd == -1)
            {
                perror("open");
                close(fd);
                continue;
            }
            memset(stat, 0, 200);
            int sz = read(fd, stat, sizeof(stat));
            if (sz == -1)
            {
                perror("read");
                close(fd);
                continue;
            }

            processdata(stat);
            close(fd);
        }
    }

    qsort(PROCESSES, p - 1, sizeof(struct processinfo), comparator);
    writeToFile(N);
}

void *handleClient(void *connfd)
{
    int sockfd = *(int *)connfd;
    int msize;
    char N[10];
    char buf[100];
    while (1)
    {
        msize = recv(sockfd, N, 10, 0);
        if (strlen(N) > 0)
        {
            maxproc(atoi(N));
            printf("Received N= %d\n", atoi(N));
            printf("Sending file with info of top %d CPU consuming processes...  ", atoi(N));
            break;
        }
    }

    int file = open("ProcessData.txt", O_RDONLY);
    if (file == -1)
    {
        perror("file open");
        exit(1);
    }
    int sent = sendfile(sockfd, file, NULL, 5000);
    if (sent < 0)
    {
        perror("sendfile");
        exit(1);
    }

    printf("File sent\n");

    while (1)
    {
        msize = recv(sockfd, buf, 100, 0);
        if (strlen(buf) > 0)
        {
            printf("Received info (pid, pname, cpu time) about the client's top CPU consuming process: %s\n", buf);
            break;
        }
    }

    free(connfd);
    close(sockfd);
    printf("Closed connection with client\n");
    printf("---------------------------------------------------------------------------------\n");
    fflush(stdout);
    return 0;
}

int main()
{
    int sockfd, connfd, addrlen, *clientSocket;
    struct sockaddr_in server, client;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
    }
    printf("Socket created\n");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("bind");
        return 1;
    }
    printf("Server bound to port\n");

    listen(sockfd, 5);

    printf("Server is listening for connections...\n");
    addrlen = sizeof(struct sockaddr_in);

    while ((connfd = accept(sockfd, (struct sockaddr *)&client, (socklen_t *)&addrlen)))
    {
        printf("\n\nConnection accepted from %s:%d\n", inet_ntoa(client.sin_addr), htons(client.sin_port));

        pthread_t thread;
        clientSocket = malloc(1);
        *clientSocket = connfd;

        if (pthread_create(&thread, NULL, handleClient, (void *)clientSocket) < 0)
        {
            perror("thread");
            return 1;
        }
    }

    if (connfd < 0)
    {
        perror("accept");
        return 1;
    }

    return 0;
}
