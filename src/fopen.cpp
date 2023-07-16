#include<stdio.h>
#include<iostream>
#include<fstream>
#include <istream>
#include <ostream>
#include<jsoncpp/json/json.h>
using namespace std;
using namespace Json;

int main()
{
    Reader reader;
    Value root;
    char buf[] = "123456";
    char buf1[] = "abcdef";
    ofstream outfile;
    outfile.open("file.dat", ios::out | ios::trunc | ios::in);
    outfile << buf;
    outfile << buf1;
    //reader.parse(outfile, root);
    //char buffer[5] = {0};
    //FILE *acceptMesg = NULL;
	//acceptMesg = fopen("/home/linux/Project/acceptMesg.txt","a+");
    
    //fwrite(buf, 1, 7, acceptMesg);
    //fwrite(buf, 1, 5, acceptMesg);
    //fclose(acceptMesg);
    outfile.close();
    return 0;
}