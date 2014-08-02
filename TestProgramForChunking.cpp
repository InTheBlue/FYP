/* RequiredLibraries: cryptopp */
/* RequiredWindowsLibraries: libcryptopp */
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <vector>
using namespace std;
#include "cryptopp/sha.h"
#include "cryptopp/base64.h"

#include "Lib/Parallel.h"

#ifdef DEBUG
#define DEBUG_MSG(str) do { std::cout << str << std::endl; } while( false )
#else
#define DEBUG_MSG(str) do { } while ( false )
#endif

string doWork(string count)
{
	CryptoPP::SHA512 hash;
	string digest;
	ostringstream oss;
	oss << "Testing" << count;
	string msg = oss.str();
	CryptoPP::StringSource s(msg, true, new CryptoPP::HashFilter(hash, new CryptoPP::Base64Encoder( new CryptoPP::StringSink(digest))));
	return digest;
}
int main( int, char** ) {
	DEBUG_MSG("Setup");
	vector<string> work;
	for(long i = 0; i < 10000000; i++)
	{
		ostringstream oss;
		oss << "Testing" << i;
		work.push_back(oss.str());
	}
	auto start = work.begin();
	auto end = work.end();
	chrono::steady_clock::time_point t0 = chrono::steady_clock::now();
	DEBUG_MSG("Foreach with no chunking");
	Parallel::Foreach(start, end, doWork);
	chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
	DEBUG_MSG("ForeachChunking with grainSize 1");
	Parallel::ForeachChunking(start, end, doWork, 1);
	chrono::steady_clock::time_point t2 = chrono::steady_clock::now();
	DEBUG_MSG("ForeachChunking with grainSize 2");
	Parallel::ForeachChunking(start, end, doWork, 2);
	chrono::steady_clock::time_point t3 = chrono::steady_clock::now();
	DEBUG_MSG("ForeachChunking with grainSize 3");
	Parallel::ForeachChunking(start, end, doWork, 3);
	chrono::steady_clock::time_point t4 = chrono::steady_clock::now();
	DEBUG_MSG("ForeachChunking with grainSize 4");
	Parallel::ForeachChunking(start, end, doWork, 4);
	chrono::steady_clock::time_point t5 = chrono::steady_clock::now();
	DEBUG_MSG("ForeachChunking with grainSize 5");
	Parallel::ForeachChunking(start, end, doWork, 5);
	chrono::steady_clock::time_point t6 = chrono::steady_clock::now();
	DEBUG_MSG("ForeachChunking with grainSize 6");
	Parallel::ForeachChunking(start, end, doWork, 6);
	chrono::steady_clock::time_point t7 = chrono::steady_clock::now();
	DEBUG_MSG("ForeachChunking with grainSize 7");
	Parallel::ForeachChunking(start, end, doWork, 7);
	chrono::steady_clock::time_point t8 = chrono::steady_clock::now();
	DEBUG_MSG("ForeachChunking with grainSize 8");
	Parallel::ForeachChunking(start, end, doWork, 8);
	chrono::steady_clock::time_point t9 = chrono::steady_clock::now();
	vector<chrono::duration<double>> Times;
	Times.push_back(chrono::duration_cast<chrono::duration<double>>(t1-t0));
	Times.push_back(chrono::duration_cast<chrono::duration<double>>(t2-t1));	
	Times.push_back(chrono::duration_cast<chrono::duration<double>>(t3-t2));
	Times.push_back(chrono::duration_cast<chrono::duration<double>>(t4-t3));
	Times.push_back(chrono::duration_cast<chrono::duration<double>>(t5-t4));
	Times.push_back(chrono::duration_cast<chrono::duration<double>>(t6-t5));
	Times.push_back(chrono::duration_cast<chrono::duration<double>>(t7-t6));
	Times.push_back(chrono::duration_cast<chrono::duration<double>>(t8-t7));
	Times.push_back(chrono::duration_cast<chrono::duration<double>>(t9-t8));
	for(auto it = Times.begin(); it != Times.end(); it++)
	{
		cout << (*it).count() << "\t";
	}
	cout << endl;
	return 0;
}
