g++ HttpsServer.cpp sqlite.cpp httpsmain.cpp -o Http -levent -ljsoncpp -lsqlite3 -lssl -lcrypto -levent_openssl
./Http
