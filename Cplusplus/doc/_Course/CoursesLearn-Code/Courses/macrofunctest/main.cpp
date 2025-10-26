#include <cstdio>
#include <string>
#include <iostream>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#if defined(_MSC_VER) // 等价于 #ifdef _MSC_VER
#define NOINLINE __declspec(noinline)
#define LIKELY(x)
#define UNLIKELY(x)
#elif defined(__GNUC__) || defined(__clang__) //任意两个定义
#define NOINLINE __attribute__((noinline))		// 不内联，有些需求是不需要优化的
#define LIKELY(x) (__builtin_expect(!!(x), 1)) // 等于c++20 [[likely]]优化的命令
#define UNLIKELY(x) (__builtin_expect(!!(x), 0)) // 两次！！是为了将处理指针，使得指针变为了bool类型
#else
#define NOINLINE
#define LIKELY(x)
#define UNLIKELY(x)
#endif

#ifdef _WIN32
#define PATHSEP "\\"
#else
#define PATHSEP "/"
#endif

NOINLINE std::string opengl_errno_name(int err) {
	switch (err) {
#define PER_GL_ERROR(x) case GL_##x: return #x;
		PER_GL_ERROR(NO_ERROR)
			PER_GL_ERROR(INVALID_ENUM)
			PER_GL_ERROR(INVALID_VALUE)
			PER_GL_ERROR(INVALID_OPERATION)
			PER_GL_ERROR(STACK_OVERFLOW)
			PER_GL_ERROR(STACK_UNDERFLOW)
			PER_GL_ERROR(OUT_OF_MEMORY)
	}
	return "unknown error: " + std::to_string(err);
}

NOINLINE void check_gl_error(const char* filename, int lineno, const char* expr) {
	int err = glGetError();
	if (UNLIKELY(err != 0)) {
		std::cerr << filename << ":" << lineno << ": " << expr << " failed: " << opengl_errno_name(err) << '\n';
		std::terminate();
	}
}

#ifdef NDEBUG
#define ASSERT_GT(x, y)
#else
// 当值用的时候永远给宏参数包括号(__x > __y)，要不然可能会出现运算符优先级错误（与理想的不匹配）
// auto __x = (x); 防止宏多次展开，如果i++，使用了两次(x)，那么i++两次与实际不符合，我们用变量让宏只展开一次并存储
#define ASSERT_GT(x, y) do { \
    auto __x = (x); \
    auto __y = (y); \
    if (!(__x > __y)) { \
        std::cerr << "Assert failed: " << #x << " (" << __x << ")" << " > " << #y << '\n'; \
        std::terminate(); \
    } \
} while (0)
#endif

#if NDEBUG  // 内置宏debug 和 release切换用的是否进行日志调试
#define CHECK_GL(x) do { \
    (x); \
} while (0)
#else /* Not NDEBUG */
#define CHECK_GL(x) do { \
    (x); \
    check_gl_error(__FILE__, __LINE__, #x); \
} while (0) // 加do while(0)之执行一次，如果不加在else分支会出现一些问题
#endif

#define MIN(x, y) ({ \
    typeof(x) __x = (x); \
    typeof(y) __y = (y); \
    __x < __y ? __x : __y; \
})

#define STR2(x) #x		//第二波将数字变为了带引号的数字——字符串
#define STR(x) STR2(x) //第一波将__LINE__变为了数字
// __VA_OPT__的作用是当__VA_ARGS__不为空的时候，才显示"," 处理PRINT("hello, world\n");用
#define PRINT(x, ...) do { \
    printf(__FILE_NAME__ ":" STR(__LINE__) ": " x __VA_OPT__(,) __VA_ARGS__); \
} while (0)

#define SHOW(x) PRINT(#x " = %d\n", x)

int main() {
	int i = 1;
	SHOW(i);
	PRINT("hello, world\n");
	PRINT("answer is %d\n", 42);
	PRINT("diploma is %d + %d\n", 985, 211);
	return 0;

	if (!glfwInit()) {
		return -1;
	}
	GLFWwindow* window = glfwCreateWindow(640, 480, "Triangle", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	while (!glfwWindowShouldClose(window)) {
		CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));

		CHECK_GL(glBegin(GL_TRIANGLES));
		CHECK_GL(glVertex2f(-0.5f, -0.5f));
		CHECK_GL(glVertex2f(0.5f, -0.5f));
		CHECK_GL(glVertex2f(0.0f, 0.5f));
		CHECK_GL(glEnd());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
