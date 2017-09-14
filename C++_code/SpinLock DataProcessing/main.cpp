// DataProcessing.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <io.h>

using namespace std;
int proportion = 40;
int core_num =62;
int p_to_s_num = 31;

//主要是用compute_ctime_new来处理自旋锁抢占耗时测试数据。
//用dataprocessing3来处理多组自旋锁测试曲线数据（5组），其逻辑是先将文档切割成5份，
//然后用dataprocessing4（）来对每一份进行处理。
//其它函数可以不看，是之前的旧数据处理函数。
//每次处理数据需要注意几个参数，一个是core_num是测试核数。
int main(int argc, char* argv[])
{
	void dataprocessing2();
	void dataprocessing3();
	void compute_ctime_old();
	void compute_ctime_new();
	//dataprocessing3();
	compute_ctime_new();
	return 0;
}

//重输出自旋锁抢占耗时测试数据，用于matlab代码绘制，其中‘test_ctime.txt’是输入原始数据
//‘ctime.txt’是输出结果数据，‘mid_result.txt’是中间结果
//每次处理数据需要注意几个参数，一个是ctime_core_num是测试核数。
void compute_ctime_new() {
	ifstream in;
	in.open("C:\\Users\\y00428318\\Downloads\\testcase-lock-test-c-s-p\\test_ctime.txt");
	int ctime_core_num = 47;

	const string outputPath = "C:\\Users\\y00428318\\Downloads\\testcase-lock-test-c-s-p\\ctime_outputFolder\\ctime.txt";
	const string midResutlFile = "C:\\Users\\y00428318\\Downloads\\testcase-lock-test-c-s-p\\ctime_outputFolder\\mid_result.txt";
	ofstream ctime_out;
	ctime_out.open(outputPath.c_str());
	ofstream cores_out;
	cores_out.open("C:\\Users\\y00428318\\Downloads\\testcase-lock-test-c-s-p\\ctime_outputFolder\\cores.txt");
	ofstream midResult_out;
	midResult_out.open(midResutlFile.c_str());


	string temp = "";
	int test_threads = 0;
	long long ts_1_us = 0;
	long long ts_2_us = 0;
	long long success_count;

	for (int i = 0; i<ctime_core_num; i++) {
		in >> temp;
		if (temp.length()>2) {

		}
		else {
			in >> temp;
		}
		//	printk("cores: %d start: %ld %ld end: %ld %ld count: %ld", test_threads, ts_1.tv_sec, ts_1.tv_usec, ts_2.tv_sec, ts_2.tv_usec, success_count);
		in >> temp;
		in >> test_threads;
		in >> temp;
		in >> ts_1_us;
		in >> temp;
		in >> ts_2_us;
		in >> temp;
		in >> success_count;

		long long diff = ts_2_us - ts_1_us;
		float ctime = diff * 1000.0f / success_count;
		midResult_out << diff<<" "<< success_count << endl;
		ctime_out << ctime << endl;
		cores_out << test_threads << endl;
	}

	in.close();
	ctime_out.close();
	cores_out.close();

	return;
}

void compute_ctime_old() {
	ifstream in;
	in.open("C:\\Users\\y00428318\\Downloads\\testcase-lock-test-c-s-p\\test_ctime.txt");
	int ctime_core_num = 47;

	const string outputPath = "C:\\Users\\y00428318\\Downloads\\testcase-lock-test-c-s-p\\ctime_outputFolder\\ctime.txt";
	ofstream ctime_out;
	ctime_out.open(outputPath.c_str());
	ofstream cores_out;
	cores_out.open("C:\\Users\\y00428318\\Downloads\\testcase-lock-test-c-s-p\\ctime_outputFolder\\cores.txt");


	string temp = "";
	int test_threads = 0;
	long int ts_1_sec = 0;
	long int ts_1_usec = 0;
	long int ts_2_sec = 0;
	long int ts_2_usec = 0;
	long int success_count;

	for (int i = 0; i<ctime_core_num; i++) {
		in >> temp;
		if (temp.length()>2) {

		}
		else {
			in >> temp;
		}
		//	printk("cores: %d start: %ld %ld end: %ld %ld count: %ld", test_threads, ts_1.tv_sec, ts_1.tv_usec, ts_2.tv_sec, ts_2.tv_usec, success_count);
		in >> temp;
		in >> test_threads;
		in >> temp;
		in >> ts_1_sec;
		in >> ts_1_usec;
		in >> temp;
		in >> ts_2_sec;
		in >> ts_2_usec;
		in >> temp;
		in >> success_count;

		float diff = (ts_2_sec - ts_1_sec) * 1000000.0f + (ts_2_usec - ts_1_usec);
		float ctime = diff * 1000 / success_count;
		ctime_out << ctime << endl;
		cores_out << test_threads << endl;
	}

	in.close();
	ctime_out.close();
	cores_out.close();

	return;
}
//deal 5 set of output
void dataprocessing3() {
	void dataprocessing4(string filePath, string speedupPath, string corePath, string ratePath);

	ifstream in;
	in.open("C:\\Users\\y00428318\\Downloads\\testcase-lock-test-c-s-p\\test.txt");

	int tests = 5;

	const string outputPath = "C:\\Users\\y00428318\\Downloads\\testcase-lock-test-c-s-p\\outputFolder\\";
	int i = 0;

	ofstream *ofstream_arr = new ofstream[tests];

	//vector<ofstream*> ofstream_arr(tests);
	//cout << "a" << endl;
	for (i = 0; i<tests; i++) {
		std::stringstream sstr;
		sstr << i;
		string index = "";
		sstr >> index;
		string fileName = outputPath + index + ".txt";
		ofstream_arr[i].open(fileName.c_str());
		//ofstream newStream;
		//newStream.open(fileName.c_str());
		//ofstream_arr.push_back(&newStream);
	}

	for (i = 0; i<tests; i++)
	{
		string temp = "";
		int test_thread = 0;
		unsigned long s_time = 0;
		unsigned long p_time = 0;
		int test_num = 0;
		int p_to_s = 0;
		unsigned int all_num = 0;
		unsigned int all_times = 0;
		unsigned int avg_cpu = 0;
		//ofstream_arr[i]<<"abc"<<endl;
		for (int j = 0; j<core_num; j++) {
			for (int k = 0; k<p_to_s_num; k++) {
				getline(in, temp);
				ofstream_arr[i] << temp << " " << endl;
			}
		}
	}
	for (i = 0; i<tests; i++)
	{
		ofstream_arr[i].close();
	}
	in.close();

	for (i = 0; i<tests; i++) {
		std::stringstream sstr;
		sstr << i;
		string index = "";
		sstr >> index;
		string fileName = outputPath + index + ".txt";
		string speedupPath = outputPath + index + "speedup.txt";
		string corePath = outputPath + index + "core.txt";
		string ratePath = outputPath + index + "rate.txt";
		dataprocessing4(fileName, speedupPath, corePath, ratePath);
	}

	//vector<ofstream>().swap(ofstream_arr);
	//lete(ofstream_arr);
	return;
}

void dataprocessing4(string filePath, string speedupPath, string corePath, string ratePath) {
	ifstream in;
	in.open(filePath.c_str());
	if (!in) {
		cerr << "file open error" << endl;
		return;
	}

	//ofstream out;
	//out.open("C:\\Users\\y00428318\\Downloads\\testcase-lock-test-c-s-p\\test_output.txt");
	ofstream oSpeedup;
	oSpeedup.open(speedupPath.c_str());
	ofstream oCores;
	oCores.open(corePath.c_str());
	ofstream oRate;
	oRate.open(ratePath.c_str());

	string temp = "";
	int test_thread = 0;
	string test_work_num = "";
	string fac_all_work = "";
	unsigned long p_time = 0;
	unsigned long s_time = 0;
	string unkown = "";
	int lock_hold = 0;
	int unlock_hold = 0;
	string cur_proportion = "";
	int s_tests = 0;
	int p_to_s = 0;
	unsigned int all_num = 0;
	unsigned int all_times = 0;
	unsigned int avg_cpu = 0;

	//unsigned int oneThreadTime = 0;
	for (int i = 0; i<core_num; i++) {
		for (int j = 0; j<p_to_s_num; j++) {
			in >> temp;
			if (temp.length()>2)
			{

			}
			else {
				in >> temp;
			}
			in >> temp;
			in >> test_thread;
			in >> s_tests;
			in >> p_to_s;
			in >> all_num;
			in >> all_times;
			in >> avg_cpu;

			if (oCores.is_open()) {
				oCores << j + 1 << endl;
			}
			/*
			out<<test_thread<<" ";
			out<<test_work_num<<" ";
			out<<fac_all_work<<" ";
			out<<duration_1<<" ";
			out<<duration_2<<" ";
			out<<unkown<<" ";
			out<<lock_hold<<" ";
			out<<unlock_hold<<" ";
			out<<cur_proportion<<endl;
			*/
			float avg_cpu_f = avg_cpu / 10.0f;
			oSpeedup << avg_cpu_f << " ";


		}
		if (oCores.is_open()) {
			oCores.close();
		}
		oSpeedup << endl;
		oRate << i + 1 << endl;
	}

	in.close();
	//out.close();
	oSpeedup.close();
	oRate.close();

	return;
}

void dataprocessing2() {
	ifstream in;
	in.open("C:\\Users\\y00428318\\Downloads\\testcase-lock-test-c-s-p\\test.txt");
	if (!in) {
		cerr << "file open error" << endl;
		return;
	}

	ofstream out;
	out.open("C:\\Users\\y00428318\\Downloads\\testcase-lock-test-c-s-p\\test_output.txt");
	ofstream oSpeedup;
	oSpeedup.open("C:\\Users\\y00428318\\Downloads\\testcase-lock-test-c-s-p\\speedup.txt");
	ofstream oCores;
	oCores.open("C:\\Users\\y00428318\\Downloads\\testcase-lock-test-c-s-p\\cores.txt");
	ofstream oRate;
	oRate.open("C:\\Users\\y00428318\\Downloads\\testcase-lock-test-c-s-p\\rate.txt");

	string temp = "";
	int test_thread = 0;
	string test_work_num = "";
	string fac_all_work = "";
	unsigned long duration_1 = 0;
	unsigned long duration_2 = 0;
	string unkown = "";
	int lock_hold = 0;
	int unlock_hold = 0;
	string cur_proportion = "";
	int s_tests = 0;
	int p_to_s = 0;
	unsigned int all_num = 0;
	unsigned int all_times = 0;
	unsigned int avg_cpu = 0;

	//unsigned int oneThreadTime = 0;
	for (int i = 0; i<p_to_s_num; i++) {
		for (int j = 0; j<core_num; j++) {
			in >> temp;
			in >> temp;
			in >> test_thread;
			in >> test_work_num;
			in >> fac_all_work;
			in >> temp;
			in >> s_tests;
			in >> temp;
			in >> p_to_s;
			in >> temp;
			in >> all_num;
			in >> all_times;
			in >> avg_cpu;

			if (oCores.is_open()) {
				oCores << j + 1 << endl;
			}

			/*
			out<<test_thread<<" ";
			out<<test_work_num<<" ";
			out<<fac_all_work<<" ";
			out<<duration_1<<" ";
			out<<duration_2<<" ";
			out<<unkown<<" ";
			out<<lock_hold<<" ";
			out<<unlock_hold<<" ";
			out<<cur_proportion<<endl;
			*/
			oSpeedup << avg_cpu << " ";


		}
		if (oCores.is_open()) {
			oCores.close();
		}
		oSpeedup << endl;
		oRate << i + 1 << endl;
	}

	in.close();
	out.close();
	oSpeedup.close();
	oRate.close();

	return;
}

void getAverageData() {
	void getFiles(const std::string & path, std::vector<std::string> & files);

	const string path = "C:\\Users\\y00428318\\Downloads\\testcase-lock-test-c-s-p\\computeAverage";
	vector<string> files;
	getFiles(path, files);
	int fileNum = files.size();
	if (fileNum == 0)
	{
		return;
	}



	string header_1 = "";
	string header_2 = "";
	string header_3 = "";
	string header_4 = "";
	string header_5 = "";
	int test_thread = 0;
	string test_work_num = "";
	string fac_all_work = "";
	unsigned long duration_1 = 0;
	unsigned long duration_2 = 0;
	string unkown = "";
	int lock_hold = 0;
	int unlock_hold = 0;
	string cur_proportion = "";
	int s_tests = 0;
	int p_to_s = 0;
	unsigned int all_num = 0;
	unsigned int all_times = 0;
	unsigned int avg_cpu = 0;

	ifstream test;
	test.open("C:\\Users\\y00428318\\Downloads\\testcase-lock-test-c-s-p\\computeAverage\\test 62_31_1_1.txt");
	test >> header_1;
	test.close();

	vector<ifstream> in(fileNum);
	for (int k = 0; k<fileNum; k++) {
		cout << files[k].c_str() << endl;
		in[k].open(files[k].c_str());
		if (!in[k])
			cout << "not open" << endl;
	}

	in[0] >> header_1;
	cout << header_1 << endl;



	ofstream outputFile;
	outputFile.open("C:\\Users\\y00428318\\Downloads\\testcase-lock-test-c-s-p\\test.txt");
	vector<unsigned int> avg_cpu_arr(fileNum);
	for (int m = 0; m<p_to_s_num; m++) {
		for (int j = 0; j<core_num; j++) {
			int i = 0;
			for (; i<fileNum; i++) {
				in[i] >> header_1;
				in[i] >> header_2;
				in[i] >> test_thread;
				in[i] >> test_work_num;
				in[i] >> fac_all_work;
				in[i] >> header_3;
				in[i] >> s_tests;
				in[i] >> header_4;
				in[i] >> p_to_s;
				in[i] >> header_5;
				in[i] >> all_num;
				in[i] >> all_times;
				in[i] >> avg_cpu_arr[i];
			}
			outputFile << header_1 << " ";
			outputFile << header_2 << " ";
			outputFile << test_thread << " ";
			outputFile << test_work_num << " ";
			outputFile << fac_all_work << " ";
			outputFile << header_3 << " ";
			outputFile << s_tests << " ";
			outputFile << header_4 << " ";
			outputFile << p_to_s << " ";
			outputFile << header_5 << " ";
			outputFile << all_num << " ";
			outputFile << all_times << " ";

			unsigned int all_cpu = 0;
			for (i = 0; i<fileNum; i++) {
				all_cpu += avg_cpu_arr[i];
			}
			avg_cpu = all_cpu / fileNum;
			outputFile << avg_cpu << endl;
			/*
			out<<test_thread<<" ";
			out<<test_work_num<<" ";
			out<<fac_all_work<<" ";
			out<<duration_1<<" ";
			out<<duration_2<<" ";
			out<<unkown<<" ";
			out<<lock_hold<<" ";
			out<<unlock_hold<<" ";
			out<<cur_proportion<<endl;
			*/


		}

	}
	for (int n = 0; n<fileNum; n++) {
		in[n].close();
	}
	return;
}



void getFiles(const std::string & path, std::vector<std::string> & files)
{
	//文件句柄
	long hFile = 0;
	//文件信息，_finddata_t需要io.h头文件
	struct _finddata_t fileinfo;
	std::string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之
			//如果不是,加入列表
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}


void dataprocessing1() {
	float computeSpeedup(int thread_num, unsigned int oneThreadTime, unsigned int multiThreadTime);
	ifstream in;
	in.open("C:\\Users\\y00428318\\Downloads\\testcase-lockbench\\test.txt");
	if (!in) {
		cerr << "file open error" << endl;
		return;
	}
	ofstream out;
	out.open("C:\\Users\\y00428318\\Downloads\\testcase-lockbench\\test_output.txt");
	ofstream oSpeedup;
	oSpeedup.open("C:\\Users\\y00428318\\Downloads\\testcase-lockbench\\speedup.txt");
	ofstream oCores;
	oCores.open("C:\\Users\\y00428318\\Downloads\\testcase-lockbench\\cores.txt");
	ofstream oRate;
	oRate.open("C:\\Users\\y00428318\\Downloads\\testcase-lockbench\\rate.txt");

	string temp = "";
	int test_thread = 0;
	string test_work_num = "";
	string fac_all_work = "";
	unsigned long duration_1 = 0;
	unsigned long duration_2 = 0;
	string unkown = "";
	int lock_hold = 0;
	int unlock_hold = 0;
	string cur_proportion = "";

	unsigned int oneThreadTime = 0;
	for (int i = 0; i<core_num; i++) {

		for (int j = 0; j<proportion; j++) {
			in >> temp;
			in >> temp;
			in >> test_thread;
			in >> test_work_num;
			in >> fac_all_work;
			in >> duration_1;
			in >> duration_2;
			in >> unkown;
			in >> temp;
			in >> lock_hold;
			in >> temp;
			in >> unlock_hold;
			in >> temp;
			in >> cur_proportion;

			out << test_thread << " ";
			out << test_work_num << " ";
			out << fac_all_work << " ";
			out << duration_1 << " ";
			out << duration_2 << " ";
			out << unkown << " ";
			out << lock_hold << " ";
			out << unlock_hold << " ";
			out << cur_proportion << endl;

			if (i == 0)
				oneThreadTime = duration_2;

			float speedup = computeSpeedup(test_thread, oneThreadTime, duration_2);
			//need convert this matrix while drawing
			oSpeedup << speedup << " ";
			//oSpeedup<<test_thread<<" "<<cur_proportion<<" "<< speedup<<endl;

			if (oRate.is_open()) {
				float rate = 100 * lock_hold / (float)unlock_hold;
				oRate << rate << endl;
			}

		}
		oSpeedup << endl;


		if (oRate.is_open())
			oRate.close();



		oCores << i + 1 << endl;
	}

	in.close();
	out.close();
	oSpeedup.close();
	oCores.close();

	return;
	//in>>temp;
	//cout<<temp;
	//printf("Hello World!\n");
}

float computeSpeedup(int thread_num, unsigned int oneThreadTime, unsigned int multiThreadTime) {
	float theoryTime = oneThreadTime / (float)thread_num;
	float rate = theoryTime / multiThreadTime;
	float speedup = rate * thread_num;
	return speedup;
}

