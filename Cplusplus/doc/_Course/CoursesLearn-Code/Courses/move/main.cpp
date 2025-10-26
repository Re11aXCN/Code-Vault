#include <cstdio>
#include <iostream>
#include <string>

using namespace std;

struct IndentGuard {
	IndentGuard(std::string& indent_) : indent(indent_) { // ���ñ�����:���ʼ����{}�ڳ�ʼ������
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

// ����������
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
			IndentGuard guard(indent);  // ������������ʲôʱ���ܹ����õ�
			emit_variable("i");
			// throw std::runtime_error("error"); // �׳��쳣Ҳ�ܹ��������������IndentGuard�Ͳ���
			emit_variable("j");

		}// ��һ��{},��ǰ�ͷţ���һ�в���Ҫtab����
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
