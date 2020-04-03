#include "handlers.h"

int main (int argc, char* argv[])
{
    int arg;
    char *filename;

    switch (argc)
    {
        case 2:
            filename = argv[1];
            break;
        default:
            printf(" Usage: %s [config]\n", argv[0]);
            exit(EXIT_FAILURE);
    }
    
    int maxLine = getLineCount(filename);
    struct IP_PORT *pairs = parseFile (filename, maxLine);
    struct mySocket all_sd[maxLine];
    
    for (int i = 0; i < maxLine; i++)
    {
        all_sd[i].port = pairs[i].src_port;
        if ((all_sd[i].sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            DieWithError("Cannot Create Socket!");
    
        arg = 1;
        if (setsockopt (all_sd[i].sd, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(arg)) == -1)
            DieWithError("setsockopt");
        
        
        if (fcntl (all_sd[i].sd, F_SETFL, O_NONBLOCK | fcntl (all_sd[i].sd, F_GETFL, 0)) == -1)
            DieWithError("fcntl");
        
        // Bind to the specified listening port
        memset (&all_sd[i].listen_addr, 0, sizeof (struct sockaddr_in));
        all_sd[i].listen_addr.sin_family = AF_INET;
        all_sd[i].listen_addr.sin_port = htons(all_sd[i].port);
        all_sd[i].listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        
        if (bind ( all_sd[i].sd, 
                  (struct sockaddr*) &all_sd[i].listen_addr, 
                   sizeof(all_sd[i].listen_addr)
                 ) == -1)
            DieWithError("bind");
        
        if (listen (all_sd[i].sd, 5) == -1)
            DieWithError("listen");
        
        char myIP[16];
        unsigned int len = (unsigned int)sizeof(all_sd[i].listen_addr);
        getsockname(all_sd[i].sd, (struct sockaddr *) &all_sd[i].listen_addr, &len);
        inet_ntop(AF_INET, &all_sd[i].listen_addr.sin_addr, myIP, sizeof(myIP));
        printf(" Listenning on: %s:%d\t===>\t%s:%d\n", myIP, all_sd[i].port, pairs[i].ip_addr, pairs[i].dst_port);
    }

    while (TRUE)
    {

        for (int i = 0; i < maxLine; i++)
        {
            struct sockaddr_in remote_addr;
            socklen_t addr_size = sizeof(struct sockaddr_in);

            int fd_new = accept (all_sd[i].sd, NULL, NULL);

            if (fd_new == -1) 
            {
                if((errno == EAGAIN) || (errno == EWOULDBLOCK)) { continue; }
                else { perror("accept"); break; }
            }
            
            getpeername (fd_new, (struct sockaddr *)&remote_addr, &addr_size);
            
            printf(" Remote Address: %s:%d forwarding to %s:%d\n", inet_ntoa(remote_addr.sin_addr), pairs[i].src_port,  pairs[i].ip_addr, pairs[i].dst_port);

            pid_t up_pid;
            up_pid = fork();
            
            if (up_pid == -1) { DieWithError("fork"); }

            while (up_pid == 0) 
            {
                // let a child process to handle the connection
                handle_forward(fd_new, pairs[i].ip_addr, pairs[i].dst_port);
            }
        }
    }

    return 0;
}


