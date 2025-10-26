#if defined(_WIN64)
#include <Windows.h>
#endif
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <map>
#include <new>
#include <iostream>


#if defined(_MSC_VER)
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
/*
	当level为0时，返回当前函数的返回地址。
	当level为1时，返回当前函数的调用者的返回地址。
*/
#if _MSC_VER < 1700 	// VS2012及以后不再支持内联汇编
__declspec (naked) void* __builtin_return_address(int iLevel)
{
	__asm
	{
		push ebx;

		mov eax, ebp;
		mov ebx, DWORD PTR[esp + 8]; // iLevel

	__Next:
		test ebx, ebx;
		je  __break;
		dec ebx;
		mov eax, DWORD PTR[eax];
		jmp __Next;
	__break:

		mov eax, DWORD PTR[eax + 4];
		pop ebx;
		ret;
	}
}
#else
void* __builtin_return_address(unsigned int iLevel)
{
	void* addresses[1];
	USHORT framesToSkip = 1; // Skip the GetReturnAddress function itself
	USHORT framesToCapture = iLevel + 1; // Include the caller of GetReturnAddress

	DWORD captured = CaptureStackBackTrace(framesToSkip, framesToCapture, addresses, nullptr);

	if (captured > iLevel)
	{
		return addresses[iLevel];
	}

	return nullptr;
}
std::string addr_to_symbol(void* addr) {
	char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)] = { 0 };
	DWORD64 displacement = 0;

	HANDLE process = GetCurrentProcess();
	if (!SymInitialize(process, nullptr, TRUE)) {
		return "";
	}

	DWORD64 moduleBase = SymGetModuleBase64(process, reinterpret_cast<DWORD64>(addr));
	if (moduleBase == 0) {
		SymCleanup(process);
		return "";
	}

	SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(buffer);
	symbol->MaxNameLen = MAX_SYM_NAME;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	if (!SymFromAddr(process, reinterpret_cast<DWORD64>(addr), &displacement, symbol)) {
		SymCleanup(process);
		return "";
	}

	std::string ret = symbol->Name;
	ret += "+" + std::to_string(displacement);

	SymCleanup(process);
	return ret;
}

#endif // _MSC_VER
#elif defined(__GNUC__)
#include <execinfo.h>
#include <cxxabi.h>
std::string addr_to_symbol(void* addr) {
	char** strings = backtrace_symbols(&addr, 1);
	if (strings == nullptr)
		return "";
	std::string ret = strings[0];
	free(strings);
	//return ret;

	size_t pos = ret.find('(');
	if (pos != std::string::npos) {
		auto pos2 = ret.find('+', pos);
		if (pos2 != std::string::npos) {
			ret = ret.substr(pos + 1, pos2 - pos - 1);
			char* demangled = abi::__cxa_demangle(ret.data(), nullptr, nullptr, nullptr);
			if (demangled) {
				ret = demangled;
				free(demangled);
			}
		}
		else {

		}
	}
	return ret;
}
#endif



struct ALlocInfo {
	void* ptr;
	size_t size;
	void* caller;
	std::string callerName;
};

/*
void before_main() {
	enable = true;
}

void after_main() {
	enable = false;
	for (auto [ptr, info] : allocated)
		printf("内存泄漏了! ptr = %p, size = %zd, caller = %s \n", ptr, info.size, info.callerName.c_str());
}

int main() {
	// new char[100000000000000];
	before_main();
	int* p1 = new int;
	dummy(3);
	delete p1;
	after_main();

}
*/

//更好的写法
struct GlobalData {
	std::map<void*, ALlocInfo> allocated;	// 存储 指针地址 ，用于判断哪个指针没有delete进而判断内存泄漏位置
	bool enable = false;

	GlobalData() {
		enable = true;
	}
	~GlobalData() {
		enable = false;
		for (auto [ptr, info] : allocated)
			printf("内存泄漏了! ptr = %p, size = %zd, caller = %s \n", ptr, info.size, info.callerName.c_str());
	}
} global;

struct EnableGuard {
	bool was_enable;
	EnableGuard() {
		was_enable = global.enable;
		global.enable = false;
	}
	explicit operator bool() {
		return was_enable;
	}
	~EnableGuard() {
		global.enable = was_enable;
	}
};
void* operator new(size_t size) {
	EnableGuard ena;
	void* ptr = malloc(size);
	if (ena) {	// 调用explicit operator bool()
		void* caller = __builtin_return_address(0);
		std::string callerName = addr_to_symbol(caller);
		if (ptr) {
			printf("new: ptr = %p, size = %zd, caller = %p, calerName = %s\n", ptr, size, caller, callerName.c_str());
			global.allocated[ptr] = ALlocInfo{ ptr, size, caller, callerName };
		}
	}
	if (ptr == nullptr)
		throw std::bad_alloc();

	return ptr;
}

void operator delete(void* ptr) noexcept {
	EnableGuard ena;
	if (ena) {
		void* caller = __builtin_return_address(0);
		std::string callerName = addr_to_symbol(caller);
		if (ptr) {
			printf("delete: ptr = %p, caller = %p, callerName = %s\n", ptr, caller, callerName.c_str());
			global.allocated.erase(ptr);
		}
	}

	free(ptr);
}
/*
void* operator new(size_t size) {
	bool was_enable = global.enable;

	global.enable = false;
	void* ptr = malloc(size);	// malloc一个超级大的C数组会返回空，所以要判断抛异常


	// allocated[ptr] = size;	// 死循环，allocated[ptr]本身操作也是调用new，使用enable 标记解决
	if (was_enable) {
		void* caller = __builtin_return_address(0);
		std::string callerName = addr_to_symbol(caller);
		if (ptr) {
			printf("new: ptr = %p, size = %zd, caller = %p, calerName = %s\n", ptr, size, caller, callerName.c_str());
			global.allocated[ptr] = ALlocInfo{ ptr, size, caller, callerName };
		}
	}
	global.enable = was_enable;

	if (ptr == nullptr)
		throw std::bad_alloc();

	return ptr;
}

void operator delete(void* ptr) noexcept {
	bool was_enable = global.enable;
	global.enable = false;
	if (was_enable) {
		void* caller = __builtin_return_address(0);
		std::string callerName = addr_to_symbol(caller);
		if (ptr) {
			printf("delete: ptr = %p, caller = %p, callerName = %s\n", ptr, caller, callerName.c_str());
			global.allocated.erase(ptr);
		}
	}

	free(ptr);
	global.enable = was_enable;
}
*/
short* dummy(int x) {
	short* p2 = new short;
	return p2;
}



int main() {
	// new char[100000000000000];
	int* p1 = new int;
	dummy(3);
	delete p1;
}