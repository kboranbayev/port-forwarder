#include "handlers.h"

void DieWithError (char *error)
{
    perror(error);
    exit(EXIT_FAILURE);
}

void traffic (int src, int dst) 
{
    char buf[BUFLEN];
    int r, i, j;

    r = read(src, buf, BUFLEN);

    while (r > 0) {
        i = 0;

        while (i < r) {
            j = write(dst, buf + i, r - i);

            if (j == -1) {
                DieWithError("write");
            }

            i += j;
        }

        r = read(src, buf, BUFLEN);
    }

    shutdown(src, SHUT_RD);
    shutdown(dst, SHUT_WR);
    close(src);
    close(dst);
    exit(EXIT_SUCCESS);
}

int open_forwarding_socket (char *forward_name, int forward_port) 
{
    int forward_socket;
    struct hostent *forward;
    struct sockaddr_in forward_address;

    forward = gethostbyname(forward_name);

    if (forward == NULL) {
        DieWithError("gethostbyname");
    }

    memset((char *) &forward_address, 0, sizeof(forward_address));
    forward_address.sin_family = AF_INET;
    bcopy((char *)forward->h_addr, (char *) &forward_address.sin_addr.s_addr, forward->h_length);
    forward_address.sin_port = htons(forward_port);

    forward_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (forward_socket == -1) {
        DieWithError("socket");
    }

    if (connect(forward_socket, (struct sockaddr *) &forward_address, sizeof(forward_address)) == -1) {
        DieWithError("connect");
    }

    return forward_socket;
}

void handle_forward (int client_socket, char *forward_name, int forward_port) 
{
    int forward_socket;
    pid_t down_pid;

    forward_socket = open_forwarding_socket(forward_name, forward_port);

    down_pid = fork();

    if (down_pid == -1) {
        DieWithError("fork");
    }

    if (down_pid == 0) {
        // inbound traffic
        traffic(forward_socket, client_socket);
    } else {
        // outbound traffic
        traffic(client_socket, forward_socket);
    }
}

int getLineCount (char *file) 
{
    FILE *fp;
    int count = 0;
    char c;
    
    fp = fopen(file, "r");
    
    if (fp == NULL) {
        DieWithError ("Could not open file");
    }
    
    for (c = getc(fp); c != EOF; c = getc(fp)) 
        if (c == '\n') // Increment count if this character is newline 
            count = count + 1; 
    fclose(fp);
    return count;
}

struct IP_PORT * parseFile (char *file, int maxLine) 
{
    char *line_buf = NULL;
    size_t line_buf_size = 0;
    int line_count = 0;
    FILE *fp = fopen(file, "r");
    if (!fp)
    {
        fprintf(stderr, "Error opening file '%s'\n", file);
        exit(1);
    }

    struct IP_PORT *pairs = malloc(maxLine * sizeof(struct IP_PORT ));
    
    /* Get the first line of the file. */
    getline(&line_buf, &line_buf_size, fp);

    /* Loop through until we are done with the file. */
    while (line_count < maxLine)
    {
        char * p = strtok(line_buf, "-");   // src port
        char * ip = strtok(NULL, ":");      // dst IP
        char * p1 = strtok(NULL, "\n");     // dst port

        pairs[line_count].src_port = atoi(p);
        pairs[line_count].dst_port = atoi(p1);
        
        strncpy(pairs[line_count].ip_addr, ip, sizeof(pairs[line_count].ip_addr));
        
        line_count++;

        /* Get the next line */
        getline(&line_buf, &line_buf_size, fp);
    }

    /* Free the allocated line buffer */
    free(line_buf);
    line_buf = NULL;

    /* Close the file now that we are done with it */
    fclose(fp);

    return pairs;
}
