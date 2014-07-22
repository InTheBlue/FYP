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
#define DEBUG

#ifdef DEBUG
#define DEBUG_MSG(str) do { std::cout << str << std::endl; } while( false )
#else
#define DEBUG_MSG(str) do { } while ( false )
#endif

void doWork(string count)
{
	CryptoPP::SHA512 hash;
	string digest;
	ostringstream oss;
	oss << "Testing" << count;
	string msg = oss.str();
	CryptoPP::StringSource s(msg, true, new CryptoPP::HashFilter(hash, new CryptoPP::Base64Encoder( new CryptoPP::StringSink(digest))));
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
	DEBUG_MSG("Starting Serial Work");
	chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
	for(auto it = work.begin(); it != work.end(); it++)
	{
		doWork(*(it));
	}
	chrono::steady_clock::time_point t2 = chrono::steady_clock::now();
	DEBUG_MSG("Starting parallel work");
	Parallel::Foreach(work.begin(), work.end(), doWork);
	chrono::steady_clock::time_point t3 = chrono::steady_clock::now();	
	DEBUG_MSG("Finishing parallel work");
	chrono::duration<double> ElapsedSerial = chrono::duration_cast<chrono::duration<double>>(t2-t1);
	chrono::duration<double> ElapsedParallel = chrono::duration_cast<chrono::duration<double>>(t3-t2);
	cout << ElapsedSerial.count() << "\t" << ElapsedParallel.count() << endl;
	return 0;
}
