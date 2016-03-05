#include <iostream>
#include <math.h>

#include "utils.h"

using namespace std; 

int main() {
	vector<double> log_pr;
	log_pr.push_back(log(1));
	log_pr.push_back(log(0));
	

	long rand_seed = 1234l;
	hdp::Utils::InitRandomNumberGen(rand_seed);

	/*
	for (int i = 0; i < 100; i++) {
		cout << hdp::Utils::SampleFromLogPr(log_pr) << endl;	
	}
	*/
	cout << hdp::Utils::LogSum(log(0), log(2)) << endl;
	cout << log(2) << endl;

	cout << 1 / (1 * 2.0) << endl;

	return 0;
}