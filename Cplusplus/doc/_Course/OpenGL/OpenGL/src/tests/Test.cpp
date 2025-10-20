#include "Test.h"

#include "imgui/imgui.h"

namespace test
{
	TestMenu::TestMenu(Test*& currentTestPtr)
		:m_CurrentTest(currentTestPtr)
	{
	}
	/*
	* 重写虚函数 OnImGuiRender()，用于在ImGui界面中显示按钮，每个按钮对应一个测试，
	* 点击按钮会切换当前活动的测试对象。
	*/
	void TestMenu::OnImGuiRender()
	{
		for (auto& test : m_Tests) {
			if (ImGui::Button(test.first.c_str())) {
				m_CurrentTest = test.second();
			}
		}
	}
}
