#include <cstdio>
#include <iostream>
#include <string>

using namespace std;

struct IndentGuard {
	IndentGuard(std::string& indent_) : indent(indent_) { // 引用必须在:后初始化，{}内初始化不行
		oldIndent = indent;
		indent += "  ";
	}

	IndentGuard(IndentGuard&&) = delete;

	~IndentGuard() {
		indent = oldIndent;
	}

	std::string oldIndent;
	std::string& indent;
};

// 代码生成器
struct Codegen {
	std::string code;
	std::string indent;

	void emit(std::string text) {
		code += indent + text + "\n";
	}

	void emit_variable(std::string name) {
		code += indent + "int " + name + ";\n";
	}

	void codegen() {
		emit("int main() {");
		{
			IndentGuard guard(indent);  // 析构函数不管什么时候都能够调用到
			emit_variable("i");
			// throw std::runtime_error("error"); // 抛出异常也能够析构，如果不加IndentGuard就不行
			emit_variable("j");

		}// 套一层{},提前释放，下一行不需要tab缩进
		emit("}");
		emit_variable("g");
	}
};

int main() {
	Codegen cg;
	cg.codegen();
	std::cout << cg.code;
	return 0;
}
