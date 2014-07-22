/* RequiredLibraries: cryptopp */
/* RequiredWindowsLibraries: libcryptopp */
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
using namespace std;
#include "cryptopp/sha.h"
#include "cryptopp/base64.h"

void doWork()
{
	for(long i = 0; i < 10000000; i++)
	{
		CryptoPP::SHA512 hash;
		string digest;
		ostringstream oss;
		oss << "Testing" << i;
		string msg = oss.str();
		CryptoPP::StringSource s(msg, true, new CryptoPP::HashFilter(hash, new CryptoPP::Base64Encoder( new CryptoPP::StringSink(digest))));
	}
}
int main( int, char** ) {
	chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
	doWork();
	chrono::steady_clock::time_point t2 = chrono::steady_clock::now();
	chrono::duration<double> elapsed = chrono::duration_cast<chrono::duration<double>>(t2-t1);
	cout << elapsed.count() << endl;
	return 0;
}
