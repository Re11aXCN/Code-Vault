#pragma once
#include <string>
#include <vector>
#include <functional>
#include <iostream>

namespace test
{
	class Test
	{
	public:
		Test() {}
		virtual ~Test() {}

		// 纯虚函数子类都需要实现，所以这里设置为虚函数选择性实现
		virtual void OnUpdate(float deltaTime) {}
		virtual void OnRender() {}
		virtual void OnImGuiRender() {}
	};

	class TestMenu : public Test
	{
	private:
		Test*& m_CurrentTest;	// 指向当前活动测试对象的指针的引用
		std::vector<std::pair<std::string, std::function<Test* ()>>> m_Tests;
	public:
		TestMenu(Test*& currentTestPtr);

		void OnImGuiRender() override;

		template<typename T>
		void RegisterTest(const std::string& name)
		{
			std::cout << "Register test: " << name << std::endl;

			m_Tests.push_back(std::make_pair(name, []() { return new T(); }));
		}
	};
}