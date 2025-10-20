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

	// ��ʼ��ջ֡  
	STACKFRAME64 stack = {};
	DWORD imageType = IMAGE_FILE_MACHINE_I386;

	// ��ȡ��ǰ�̵߳�ջ����ַ  
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

	// ����һ��������֡  
	for (unsigned int i = 0; i < skipFrames; ++i) {
		if (!StackWalk64(imageType, process, thread, &stack, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
			break;
		}
	}

	// ��ӡ����ջ  
	while (StackWalk64(imageType, process, thread, &stack, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
		DWORD64 address = stack.AddrPC.Offset;
		std::cout << "Address: " << std::hex << address << std::endl;

		// ���������Ӷ���Ĵ�����������ַ����ȡ����������Ϣ  
	}
}

int main() {
	InitializeDbgHelp();
	PrintStackTrace(1); // ��������֡��ͨ����PrintStackTrace��������  
	return 0;
}