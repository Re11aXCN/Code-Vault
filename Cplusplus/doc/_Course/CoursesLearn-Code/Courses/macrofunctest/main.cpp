#include <cstdio>
#include <string>
#include <iostream>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#if defined(_MSC_VER) // �ȼ��� #ifdef _MSC_VER
#define NOINLINE __declspec(noinline)
#define LIKELY(x)
#define UNLIKELY(x)
#elif defined(__GNUC__) || defined(__clang__) //������������
#define NOINLINE __attribute__((noinline))		// ����������Щ�����ǲ���Ҫ�Ż���
#define LIKELY(x) (__builtin_expect(!!(x), 1)) // ����c++20 [[likely]]�Ż�������
#define UNLIKELY(x) (__builtin_expect(!!(x), 0)) // ���Σ�����Ϊ�˽�����ָ�룬ʹ��ָ���Ϊ��bool����
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
// ��ֵ�õ�ʱ����Զ�������������(__x > __y)��Ҫ��Ȼ���ܻ������������ȼ�����������Ĳ�ƥ�䣩
// auto __x = (x); ��ֹ����չ�������i++��ʹ��������(x)����ôi++������ʵ�ʲ����ϣ������ñ����ú�ֻչ��һ�β��洢
#define ASSERT_GT(x, y) do { \
    auto __x = (x); \
    auto __y = (y); \
    if (!(__x > __y)) { \
        std::cerr << "Assert failed: " << #x << " (" << __x << ")" << " > " << #y << '\n'; \
        std::terminate(); \
    } \
} while (0)
#endif

#if NDEBUG  // ���ú�debug �� release�л��õ��Ƿ������־����
#define CHECK_GL(x) do { \
    (x); \
} while (0)
#else /* Not NDEBUG */
#define CHECK_GL(x) do { \
    (x); \
    check_gl_error(__FILE__, __LINE__, #x); \
} while (0) // ��do while(0)ִ֮��һ�Σ����������else��֧�����һЩ����
#endif

#define MIN(x, y) ({ \
    typeof(x) __x = (x); \
    typeof(y) __y = (y); \
    __x < __y ? __x : __y; \
})

#define STR2(x) #x		//�ڶ��������ֱ�Ϊ�˴����ŵ����֡����ַ���
#define STR(x) STR2(x) //��һ����__LINE__��Ϊ������
// __VA_OPT__�������ǵ�__VA_ARGS__��Ϊ�յ�ʱ�򣬲���ʾ"," ����PRINT("hello, world\n");��
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
