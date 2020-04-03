#define TRUE                1
#define FALSE               0
#define EPOLL_QUEUE_LEN     256
#define BUFLEN              1024
#define IPV4_LEN            20     // allocation bytes for ipv4 address length

struct mySocket 
{
    int sd;
    int fd;
    int epoll_fd;
    int port;
    struct sockaddr_in listen_addr;
};

struct IP_PORT
{
    char ip_addr[IPV4_LEN];
    int src_port;
    int dst_port;
};
