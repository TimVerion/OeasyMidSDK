#include "OeasySocket.h"
#include "log4cpp.h"

OeasySocket::OeasySocket()
{
	m_socket = 0;
	m_bindPort = 0;
}

OeasySocket::~OeasySocket()
{
	SOCK_CLOSE(m_socket);
}

bool OeasySocket::SetNonBlock(bool nonBlock /*= true*/ )
{
#ifdef _WIN32
	unsigned long mode = nonBlock; //mode: 1-������ģʽ   0-����ģʽ
	int ret = SOCK_IOCTL(m_socket,FIONBIO,(unsigned long *)&mode);//���óɷ�����ģʽ��
	if (ret == -1)
	{
		return false;
	}
	return true;
#else
	int opt = SOCK_IOCTL(m_socket, F_GETFL, 0);  
	if (opt == -1)  
		return false;  
	if (nonBlock)  
		opt |= O_NONBLOCK;  // ���ϡ���������flag
	else  
		opt &= ~O_NONBLOCK;  
	if (SOCK_IOCTL(m_socket, F_SETFL, opt) == -1)  
		return false;  
	return true; 
#endif

}

int OeasySocket::CreateSocket( int af, int type, int protocol )
{
	m_socket= SOCK_CREATE(af,type, protocol);
	if (m_socket <= 0)
	{
		m_socket = SOCK_CREATE(af,type, protocol);
	}
	return m_socket;
}

int OeasySocket::Bind(unsigned short port )
{
	if (port <= 1024)
	{
		return -1;
	}
	m_bindPort = port;
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = OEASY_HTONS(port);
	serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	if(SOCK_BIND(m_socket, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR){
		return -1;
	}
	return 0;
}

int OeasySocket::Listen( int backlog )
{
	return SOCK_LISTEN(m_socket, backlog);
}

bool OeasySocket::Connect( char* ip, unsigned short port )
{
	struct sockaddr_in servaddr;
	servaddr.sin_port = OEASY_HTONS(port);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);
	int ret = SOCK_CONNECT(m_socket, (const struct sockaddr *)&servaddr, sizeof(sockaddr));
	if (ret == -1)
	{
		OEASYLOG_E("connect failed ret = %d", ret);
		return false;
	}
	return true;
}

bool OeasySocket::Accept(OeasySocket* acceptSocket)
{
	int socket = SOCK_ACCEPT(m_socket, NULL, NULL);
	if (socket == -1)  
		OEASYLOG_E("OeasySocket:Accept failed");
		return false;  
	acceptSocket->setinnerSocket(socket);
	return true;  
}

bool OeasySocket::CloseSocket(int socket)
{
	if (socket == 0)
	{
		SOCK_CLOSE(m_socket);
	}else{
		SOCK_CLOSE(socket);
	}
	return true;
}

int OeasySocket::Readn( int socket, char* buf, int len )
{
	char *tempbuf = buf;
	int nleft = len;
	int nread = 0;
	while(nleft > 0){
		if((nread = SOCK_RECV(socket, tempbuf, nleft, 0)) <= 0 ){
			if (errno == EINTR)
			{
				 //�����ȡ�����Ǳ��źŴ����, ��˵�������Լ����� 
				continue;
			}else{
				//GetLastError()����-10053�������������ݿɶ�
				return -1;
			}
		}
		else if(nread == 0)  //û���ݿɶ���
		{
			return len-nleft; 
		}		
		nleft -= nread;
		tempbuf += nread;
	}
	return len;
}


int OeasySocket::recv( void *buf, size_t n, int flags, struct sockaddr_in* addr )
{
	fd_set fdr;
	FD_ZERO(&fdr);
	FD_SET(m_socket, &fdr);
	struct timeval	_timeout;
	_timeout.tv_sec = 0;
	_timeout.tv_usec = 50000;
	flags = SOCK_SELECT(m_socket + 1, &fdr, NULL, NULL, &_timeout);
	if(flags > 0){
		if (0 == (flags = SOCK_RECV(m_socket, (char*)buf, n, 0)))
		{
			return -1;
		}
		return flags;
	}else if (flags < 0){
		return flags;
	}else{
		return 0;
	}
}

int OeasySocket::Writen( int socket, char* buf, int len )
{
	int nwritten = 0;
	int nleft = len;
	char *tempbuf = buf;
	while(nleft > 0){
		if ((nwritten = SOCK_SEND(socket, tempbuf, nleft, 0)) <= 0)
		{
			if (errno == EINTR)
			{
				continue;
			}else{
				return -1;
			}
		}
		//��� ==0��˵����ʲôҲûд��, ���Լ���д  
		else if (nwritten == 0)  
			continue;  
		nleft -= nwritten;
		tempbuf += nwritten;
	}
	return len;
}

int OeasySocket::Getsocket()
{
	return m_socket;
}

int OeasySocket::Setsockopt( int sockfd, int level, int optname, const void *optval, int optlen )
{
	//����˾�����ʹ��SO_REUSEADDR,�ڰ�֮ǰ�����ܵ���setsockopt������SO_REUSEADDR�׽���ѡ���ѡ�����ʹ��server���صȴ�TIME_WAIT״̬��ʧ�Ϳ�������������
	//int on = 1;  
	//if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,  
	//	&on,sizeof(on)) == -1)
	return SOCK_SETOPT(sockfd, level, optname,  (const char*)optval, optlen); 
}

int OeasySocket::Select( int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout )
{
	return SOCK_SELECT(nfds, readfds, writefds, exceptfds, timeout);
}

void OeasySocket::setinnerSocket( int socket )
{
	m_socket = socket;
}






