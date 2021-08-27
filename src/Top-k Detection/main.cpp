#include <iostream>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <numeric>

#include "include/Choose_Topk.h"


#define x (1.05)
double F = 0.15, S = 4.4;
using namespace std;
int topk=2000;
vector<int> items;
unordered_map<int, int>fre,freq;
unordered_map<string,int>id;int idsm;//!
int memaccess_i = 0, memaccess_q = 0, memaccess_d = 0;

int thres_mid = 16, thres_large = 1024;

double item_num = 0, flow_num = 0;

vector<double> experiment_F1,experiment_ARE, experiment_AAE,experiment_CON, experiment_through_insert, experiment_through_query/*, experiment_sum, exper_max*/;
int D;

void readFile_CAIDA(const char* filename, int length, int MAX_ITEM = INT32_MAX){
	// 177335flows, 2472727 items read
	// max freq = 16889
	ifstream inFile(filename, ios::binary);

	if (!inFile.is_open())
		cout << "File fail." << endl;

	int max_freq = 0;
	char key[length];
	int true_key;
	for (int i = 0; i < MAX_ITEM; ++i)
	{
		inFile.read(key, length);
		if (inFile.gcount() < length)
			break;
		if(length<=4){
			true_key=atoi(key);
		}else{
			if(!id.count(string(key, length)))
			id[string(key, length)]=++idsm;
			true_key=id[string(key, length)];
		}
		// printf("%d\n",true_key);
		items.push_back(true_key);
		fre[true_key]++;
	}
	inFile.close();
	for (auto pr : fre)
		max_freq = max(max_freq, pr.second);

	uint32_t size = fre.size();
	uint32_t *num = new uint32_t[size];
	uint32_t pos = 0;
	for (auto it : fre) {
		num[pos++] = it.second;
	}
	nth_element(num, num + size - topk, num + size);
	thres_large = num[size - topk];
	
	for (auto it : fre) {
		if(it.second>=thres_large){
			freq[it.first]=it.second;
		}
	}

	item_num = items.size();
	flow_num = freq.size();
	cout << "dataset name: " << filename << endl;
	cout << freq.size() << "flows, " << items.size() << " items read" << endl;;
	cout << "max freq = " << max_freq << endl;
	cout << "large flow thres = " << thres_large << ", middle flow thres = " << thres_mid << endl << endl;
}
void calcF1(const vector<element>* ret_f)
{
	double _F1 = 0, _AAE = 0, _CON = 0;
	for (int i = 0; i < testcycles; ++i)
	{
		for(auto pr:*ret_f){
			if(freq.count(pr.item))_F1+=1;
		}
	}
	_F1 /= freq.size(), _AAE /= freq.size(), _CON /= freq.size();
	_F1 /= testcycles, _AAE /= testcycles, _CON /= testcycles;
	experiment_F1.push_back(_F1);
	cout << "F1 = " << _F1;/*<< ", insert = " << throughput_i << ", query = " << throughput_o << endl;*/
}
void test_F1(int mem_in_byte)
{
	int d = D;  //counts of hash function
	int w = mem_in_byte;  //   bits/counter_size/hash_counts

	vector<element> ret_f[testcycles];
	double throughput_i = 0, throughput_o = 0;
	long long estimate_sum = 0;
	for (int i = 0; i < testcycles; ++i)
	{
		timespec time1, time2, time3, time4;
		long long resns;


		Abstract* bcm;
		bcm=Choose_Topk(w,d, topk,i * 100,1);
                clock_gettime(CLOCK_MONOTONIC, &time1);
		for (auto key : items){
			bcm->Insert(key);
		}
                 clock_gettime(CLOCK_MONOTONIC, &time2);

                 clock_gettime(CLOCK_MONOTONIC, &time3);
		for (auto pr : freq)
			estimate_sum += bcm->Query(pr.first);
                 clock_gettime(CLOCK_MONOTONIC, &time4);

		resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
		throughput_i += (double)1000.0 * item_num / resns;
		resns = (long long)(time4.tv_sec - time3.tv_sec) * 1000000000LL + (time4.tv_nsec - time3.tv_nsec);
		throughput_o += (double)1000.0 * flow_num / resns;
		bcm->Query_topk(ret_f[i],topk);
	}

	cout << "sum: " << estimate_sum/testcycles << "\tBCM" << endl;
	calcF1(ret_f);
}
void calcAcc(const vector<int>* ret_f)
{
	double _ARE = 0, _AAE = 0, _CON = 0;
	for (int i = 0; i < testcycles; ++i)
	{
		
		int z = 0;
		for (auto pr : freq)
		{
			int est_val = ret_f[i][z++];
			int real_val = pr.second;
			int dist = abs(est_val - real_val);
			int conflict = est_val != real_val ? 1 : 0;
			_ARE += (double)dist / real_val, _AAE += dist, _CON += conflict;
			
			//cout<<est_val<<','<<real_val<<endl;
		}
	}
	_ARE /= freq.size(), _AAE /= freq.size(), _CON /= freq.size();
	_ARE /= testcycles, _AAE /= testcycles, _CON /= testcycles;
	experiment_ARE.push_back(_ARE);
	experiment_AAE.push_back(_AAE);
	experiment_CON.push_back(_CON);
	cout << ", ARE = " << _ARE << ", AAE = " << _AAE<< ",CON = "<< _CON << endl;/*<< ", insert = " << throughput_i << ", query = " << throughput_o << endl;*/
}
void test_Topk(int mem_in_byte)
{
	int d = D;  //counts of hash function
	int w = mem_in_byte;  //   bits/counter_size/hash_counts

	vector<int> ret_f[testcycles];
	double throughput_i = 0, throughput_o = 0;
	long long estimate_sum = 0;
	for (int i = 0; i < testcycles; ++i)
	{
		timespec time1, time2, time3, time4;
		long long resns;


		Abstract* bcm;
		bcm=Choose_Topk(w,d, topk,i * 100,1);
                clock_gettime(CLOCK_MONOTONIC, &time1);
		for (auto key : items){
			bcm->Insert(key);
		}
                 clock_gettime(CLOCK_MONOTONIC, &time2);

                 clock_gettime(CLOCK_MONOTONIC, &time3);
		for (auto pr : freq)
			estimate_sum += bcm->Query(pr.first);
                 clock_gettime(CLOCK_MONOTONIC, &time4);

		resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
		throughput_i += (double)1000.0 * item_num / resns;
		resns = (long long)(time4.tv_sec - time3.tv_sec) * 1000000000LL + (time4.tv_nsec - time3.tv_nsec);
		throughput_o += (double)1000.0 * flow_num / resns;
		for (auto pr : freq)
			ret_f[i].push_back(bcm->Query(pr.first));
	}

	// cout << "sum: " << estimate_sum/testcycles << "\tBCM" << endl;
	experiment_through_insert.push_back(throughput_i / testcycles);
	experiment_through_query.push_back(throughput_o / testcycles);
	calcAcc(ret_f);
}
int main(){
	char* filename = "CAIDA";
	readFile_CAIDA("0.dat", 13);
	//readFile_CAIDA("012.dat",4);
	int mem_in_byte_start = 1 << 20;
	int mem_in_byte_end = 1 << 21;
	int mem_in_byte;
	for(D=2;D<=4;D++){
		for (mem_in_byte = mem_in_byte_start; mem_in_byte <= mem_in_byte_end; mem_in_byte<<=1){
			test_F1(mem_in_byte);
			test_Topk(mem_in_byte);
		}
	}
	ofstream oFile;
	oFile.open("sheet.csv", ios::app);
	if (!oFile) return 0;
	oFile << "F1," <<"AAE," << "ARE," << "insert," << "query," <<"CON,"<< endl;
	for (int o = 0, j = 0, kb = mem_in_byte; o < experiment_AAE.size(); o++){
		oFile << experiment_F1.at(o) << "," << experiment_AAE.at(o) << "," << experiment_ARE.at(o) << "," << experiment_through_insert.at(o) << ","
			<< experiment_through_query.at(o)<<"," <<experiment_CON.at(o)/*<< "," << fixed << experiment_sum.at(o) << "," << fixed << exper_max.at(o)*/ << endl;
	}
	oFile.close();

	return 0;
}
