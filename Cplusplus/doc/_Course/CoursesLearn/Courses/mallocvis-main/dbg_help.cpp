#include <windows.h>  
#include <DbgHelp.h>  
#include <iostream>  

#pragma comment(lib, "DbgHelp.lib")  

void InitializeDbgHelp() {
	SymInitialize(GetCurrentProcess(), NULL, TRUE);
}

void PrintStackTrace(unsigned int skipFrames = 0) {
	HANDLE process = GetCurrentProcess();
	HANDLE thread = GetCurrentThread();
	CONTEXT context = {};
	context.ContextFlags = CONTEXT_FULL;

	// 初始化栈帧  
	STACKFRAME64 stack = {};
	DWORD imageType = IMAGE_FILE_MACHINE_I386;

	// 获取当前线程的栈顶地址  
	RtlCaptureContext(&context);
#ifdef _M_IX86  
	stack.AddrPC.Offset = context.Eip;
	stack.AddrPC.Mode = AddrModeFlat;
	stack.AddrFrame.Offset = context.Ebp;
	stack.AddrFrame.Mode = AddrModeFlat;
	stack.AddrStack.Offset = context.Esp;
	stack.AddrStack.Mode = AddrModeFlat;
#elif _M_X64  
	stack.AddrPC.Offset = context.Rip;
	stack.AddrPC.Mode = AddrModeFlat;
	stack.AddrFrame.Offset = context.Rsp;
	stack.AddrFrame.Mode = AddrModeFlat;
	stack.AddrStack.Offset = context.Rsp;
	stack.AddrStack.Mode = AddrModeFlat;
#endif  

	// 跳过一定数量的帧  
	for (unsigned int i = 0; i < skipFrames; ++i) {
		if (!StackWalk64(imageType, process, thread, &stack, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
			break;
		}
	}

	// 打印调用栈  
	while (StackWalk64(imageType, process, thread, &stack, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
		DWORD64 address = stack.AddrPC.Offset;
		std::cout << "Address: " << std::hex << address << std::endl;

		// 这里可以添加额外的代码来解析地址，获取函数名等信息  
	}
}

int main() {
	InitializeDbgHelp();
	PrintStackTrace(1); // 跳过顶层帧（通常是PrintStackTrace函数本身）  
	return 0;
}