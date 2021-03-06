#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#ifdef SIMULATION
#define DSPRINT printf
#endif

#define UEVENT_BUFFER_SIZE 2048

unsigned char monitor_hotplug_in = 0;

static int init_hotplug_sock()
{
	const int buffersize = 1024;
	int ret;
	int fd;
	struct sockaddr_nl snl;

	bzero(&snl, sizeof(struct sockaddr_nl));

	snl.nl_family = AF_NETLINK;
	snl.nl_pid = getpid();
	snl.nl_groups = 1;

	fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (fd == -1)
	{
		perror("socket");
		return -1;
	}

	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));

	ret = bind(fd, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));
	if (ret < 0)
	{
		perror("bind");
		close(fd);
		return -1;
	}

	return fd;
}

void hotplug_monitor(void)
{
	int monitor_fd = init_hotplug_sock();
	char buf[UEVENT_BUFFER_SIZE * 2] = {0,};

	if (monitor_fd < 0) {
		DSPRINT("create hotplug socket failed.\n");
		return;
	}

	while (1) {
		recv(monitor_fd, &buf, sizeof(buf), 0);
		if (strstr(buf, "/drm/"))
			monitor_hotplug_in = 1;
		bzero(&buf, sizeof(buf));
	}
}

int main(int argc, char* argv[])
{
	int hotplug_sock = init_hotplug_sock();

	while(1)
	{
		/* Netlink message buffer */
		char buf[UEVENT_BUFFER_SIZE * 2] = {0};
		recv(hotplug_sock, &buf, sizeof(buf), 0);
		DSPRINT("%s\n", buf);
	}
	return 0;
}
