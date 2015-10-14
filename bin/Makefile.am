VPATH=../src
bin_PROGRAMS = mttserver tcpclient tcpserver udpclient udpserver 
utilSFiles = util.c httputil.c
utilHFiles = util.h httputil.h
tcpclient_SOURCES = tcpclient.h tcpclient.c ${utilSFiles} ${utilHFiles}
tcpserver_SOURCES = tcpserver.h tcpserver.c ${utilSFiles} ${utilHFiles}
udpclient_SOURCES = udpclient.h udpclient.c ${utilSFiles} ${utilHFiles}
udpserver_SOURCES = udpserver.h udpserver.c ${utilSFiles} ${utilHFiles}
mttserver_SOURCES = mthread-tcp-server.h mthread-tcp-server.c ${utilSFiles} ${utilHFiles}

