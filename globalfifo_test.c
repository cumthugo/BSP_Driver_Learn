#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

int fd = -1;

void input_handler(int signum)
{
	char data[100];
	int len;
	printf("receive a signal from globalfifo, signalnum:%d\n",signum);
	
	if(fd != -1)
	{
		len = read(fd,&data,100);
		data[len] = 0;
		printf("Read data: %s\n",data);
	}
}

void main()
{
	int oflags;
	fd = open("/dev/globalfifo",O_RDWR, S_IRUSR | S_IWUSR);
	if (fd != -1)
	{
		signal(SIGIO,input_handler);
		fcntl(fd,F_SETOWN,getpid());
		oflags = fcntl(fd,F_GETFL);
		fcntl(fd,F_SETFL,oflags|FASYNC);
		while(1){
			sleep(100);
		}
	}
	else
	{
		printf("device open failure\n");
	}
}