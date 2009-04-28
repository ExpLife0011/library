/**
 * Copyright (C) 2008 Meteor Liu
 *
 * This code has been released into the Public Domain.
 * You may do whatever you like with it.
 *
 * @file
 * @author Meteor Liu <meteor1113@gmail.com>
 * @date 2009-01-01
 */


// #include <standard library headers>
#include <iostream>

// #include <other library headers>
#include <errno.h>
#include <fcntl.h>

// #include "customer headers"
#include "socket.hpp"


namespace sock
{

#ifdef _WIN32

    class WSAInitial
    {
    public:
        WSAInitial()
            {
                WSADATA wsaData;
                if (WSAStartup(0x101, &wsaData))
                {
                    exit(-1);
                }
            }

        virtual ~WSAInitial() { WSACleanup(); }

    };

    static WSAInitial sWinsockInit;

#endif


    // ctor && dtor

    Socket::Socket(int type)
        :mSock(INVALID_SOCKET)
    {
        memset(&mAddr, 0, sizeof(mAddr));
        Create(type);
    }


    Socket::~Socket()
    {
        Close();
    }


    bool Socket::Bind(int port)
    {
        if (!IsValid())
        {
            return false;
        }

        mAddr.sin_family = AF_INET;
        mAddr.sin_addr.s_addr = INADDR_ANY;
        mAddr.sin_port = htons(port);

        int ret = ::bind(mSock,
                         (struct sockaddr*)&mAddr,
                         sizeof(mAddr));

        return (ret != SOCKET_ERROR);
    }


    bool Socket::Listen()
    {
        if (!IsValid())
        {
            return false;
        }

        int ret = ::listen(mSock, SOMAXCONN);
        return (ret != SOCKET_ERROR);
    }


    bool Socket::Accept(Socket& ns)
    {
        int addr_length = sizeof(sockaddr_in);
        ns.mSock = ::accept(mSock, (sockaddr*)&ns.mAddr,
                            (::socklen_t*)&addr_length);
#ifdef WIN32
        return (ns.mSock != INVALID_SOCKET);
#else
        return (ns.mSock > 0);
#endif
    }


    bool Socket::Connect(const std::string& host, int port)
    {
        if (!IsValid())
        {
            return false;
        }

        if (!MakeSockaddr(&mAddr, host, port))
        {
            return false;
        }

        int status = ::connect(mSock, (sockaddr*)&mAddr, sizeof(mAddr));

#ifdef _DEBUG
        if (status == SOCKET_ERROR)
        {
            PrintError();
        }
#endif
/*
  if (status != SOCKET_ERROR)
  {
  linger m_sLinger;
  m_sLinger.l_onoff = 1;
  m_sLinger.l_linger = 0;
  status = ::setsockopt(mSock, SOL_SOCKET, SO_LINGER,
  (const char*)&m_sLinger, sizeof(linger));
  }
//*/
        mHost = host;
        mPort = port;
        return (status != SOCKET_ERROR);
    }


    bool Socket::Reconnect()
    {
        Close();
        Create(SOCK_STREAM);
        return Connect(mHost, mPort);
    }


    int Socket::SelectRead(long sec, long usec)
    {
        return Select(0, sec, usec);
    }


    int Socket::SelectWrite(long sec, long usec)
    {
        return Select(1, sec, usec);
    }


    bool Socket::Send(const std::string& s)
    {
        return Send(s.c_str(), static_cast<int>(s.size()));
    }


    bool Socket::Send(const char* buf, int len)
    {
        if (buf == NULL)
        {
            return false;
        }

        int totalSendLen = 0;
        while (totalSendLen < len)
        {
            int leaveLen = len - totalSendLen;
            int s = (leaveLen < MAXSEND) ? leaveLen : MAXSEND;
            int sendLen = ::send(mSock, &buf[totalSendLen], s, 0);
            if (sendLen == SOCKET_ERROR)
            {
#ifdef _DEBUG
                PrintError();
#endif
                return false;
            }

            totalSendLen += sendLen;
            //int sel = SelectWrite(sec, usec);
            //if (sel <= 0)
            //{
            //    return false;
            //}
        }

        return (totalSendLen == len);
    }


    int Socket::Recv(std::string& s)
    {
        char buf[MAXRECV + 1];
        memset(buf, 0, MAXRECV + 1);

        int status = Recv(buf, MAXRECV);
        if (status > 0)
        {
            s = buf;
        }
        else
        {
            return status;
        }

        return static_cast<int>(s.size());
    }


    int Socket::Recv(char* buf, int len)
    {
        if (buf == NULL)
        {
            return -1;
        }

        int status = ::recv(mSock, buf, len, 0);
#ifdef _DEBUG
        if (status == SOCKET_ERROR)
        {
            PrintError();
        }
#endif
        return status;
    }


    bool Socket::SendTo(const std::string& s, const std::string& host,
                        int port)
    {
        return SendTo(s.c_str(), static_cast<int>(s.size()), host, port);
    }


    bool Socket::SendTo(const char* buf, int len, const std::string& host,
                        int port)
    {
        if (buf == NULL)
        {
            return false;
        }

        sockaddr_in addr;
        if (!MakeSockaddr(&addr, host, port))
        {
            return false;
        }

        int totalSendLen = 0;
        while (totalSendLen < len)
        {
            int leaveLen = len - totalSendLen;
            int s = (leaveLen < MAXSEND) ? leaveLen : MAXSEND;
            int sendLen = ::sendto(mSock, &buf[totalSendLen], s, 0,
                                   (sockaddr*)&addr, sizeof(addr));
//            if (sendLen == SOCKET_ERROR)
            if (sendLen <= 0)
            {
#ifdef _DEBUG
                PrintError();
#endif
                return false;
            }
            totalSendLen += sendLen;
        }

        return (totalSendLen == len);
    }


    int Socket::RecvFrom(std::string& s, std::string& host, int& port)
    {
        char buf[MAXRECV + 1];
        memset(buf, 0, MAXRECV + 1);

        int status = RecvFrom(buf, MAXRECV, host, port);
        if (status > 0)
        {
            s = buf;
        }
        else
        {
            return status;
        }

        return static_cast<int>(s.size());
    }


    int Socket::RecvFrom(char* buf, int len, std::string& host, int& port)
    {
        if (buf == NULL)
        {
            return -1;
        }

        sockaddr_in addr;
        int addrSize = sizeof(addr);
        int status = ::recvfrom(mSock, buf, len, 0, (sockaddr*)&addr,
                                (socklen_t*)&addrSize);
#ifdef _DEBUG
        if (status == SOCKET_ERROR)
        {
            PrintError();
        }
#endif
        if (status != SOCKET_ERROR)
        {
            ParseSockAddr(addr, host, port);
        }
        return status;
    }


    void Socket::SetNonBlocking(const bool b)
    {
#ifdef _WIN32
        u_long opts = b ? 1 : 0;
        ioctlsocket(mSock, FIONBIO, &opts);
#else
        int opts = fcntl(mSock, F_GETFL);
        if (opts < 0)
        {
            return;
        }

        opts = b ? (opts | O_NONBLOCK) : (opts & ~O_NONBLOCK);
        fcntl(mSock, F_SETFL,opts);
#endif
    }


    bool Socket::IsValid() const
    {
        return (mSock != INVALID_SOCKET);
    }


    bool Socket::Create(int type)
    {
        mSock = socket(AF_INET, type, 0);
        if (!IsValid())
        {
            return false;
        }

        // TIME_WAIT - argh
        int on = 1;
        int ret = setsockopt(mSock, SOL_SOCKET, SO_REUSEADDR,
                             (const char*)&on, sizeof(on));

        return (ret != SOCKET_ERROR);
    }


    void Socket::Close()
    {
        if (IsValid())
        {
#ifdef WIN32
            ::closesocket(mSock);
#else
            ::close (mSock);
#endif
            mSock = INVALID_SOCKET;
        }
    }


    /**
     *
     *
     * @param type 0-read, 1-write, 2-except
     * @param sec
     * @param usec
     *
     * @return
     */
    int Socket::Select(int type, long sec, long usec)
    {
        timeval timeout;
        timeout.tv_sec = sec;
        timeout.tv_usec = usec;
        timeval* tv = ((sec < 0) || (usec < 0)) ? NULL : &timeout;

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(mSock, &fds);

        fd_set* readfds = (type == 0) ? &fds : NULL;
        fd_set* writefds = (type == 1) ? &fds : NULL;
        fd_set* exceptfds = (type == 2) ? &fds : NULL;

        return ::select(mSock + 1, readfds, writefds, exceptfds, tv);
    }


    void Socket::PrintError()
    {
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4996)
        int err = ::WSAGetLastError();
        std::cout << "error code:" << err << std::endl;
#pragma warning(pop)
#else
        std::cout << "error code:" << errno << std::endl;
#endif
    }


    bool Socket::MakeSockaddr(sockaddr_in* addr, const std::string& host,
                              int port)
    {
        if (addr == NULL)
        {
            return false;
        }

        addr->sin_family = AF_INET;
        addr->sin_port = htons(port);
#ifdef WIN32
        addr->sin_addr.s_addr = inet_addr(host.c_str());
        if (addr->sin_addr.s_addr == INADDR_NONE)
        {
            return false;
        }
#else
        int changestatus = inet_pton(AF_INET, host.c_str(), &(addr->sin_addr));
        if (changestatus < 0 && errno == EAFNOSUPPORT)
        {
            return false;
        }
#endif
        return true;
    }


    bool Socket::ParseSockAddr(const sockaddr_in& addr, std::string& host,
                               int& port)
    {
        host = inet_ntoa(addr.sin_addr);
        port = ntohs(addr.sin_port);
        return true;
    }

}