g++ epollserver.cpp sqlite.cpp epollmain.cpp TaskQueue.cpp XthreadPool.cpp -o epoll -levent -ljsoncpp -lsqlite3 -lpthread
./epoll
rm epoll
