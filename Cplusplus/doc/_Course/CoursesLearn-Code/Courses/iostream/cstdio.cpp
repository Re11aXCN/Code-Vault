#include <cstdio>
#include <thread>

using namespace std;

// Linux:
// stdout: _IOLBF 行缓冲   \n就会刷新
// stderr: _IONBF 无缓冲
// fopen: _IOFBF 完全缓冲
// 
// cout:-I0LBF 行缓冲 1
// cerr:-IONBF 无缓冲 2
// clog:-I0LBF 行缓冲 2
// fstream:-IOFBF 完全缓冲

// MSVC:
// stdout: _IONBF 无缓冲
// stderr: _IONBF 无缓冲
// fopen: _IOFBF 完全缓冲
// 
// cout:-I0LBF 无缓冲 1
// cerr:-IONBF 无缓冲 2
// clog:-I0LBF 无缓冲 2
// fstream:-IOFBF 完全缓冲
// 
// 
// C语言标准要求stdout可以行缓冲，stderr必须无缓冲(因为错误信息必须立马看到)
static char buf[BUFSIZ];

int main() {
	setvbuf(stdout, buf, _IOFBF, sizeof buf);

	for (int i = 0; i < 65536; i += 8) {
		fprintf(stdout, "[%5d]\n", i);
		this_thread::sleep_for(1ms);
	}

	return 0;
}
