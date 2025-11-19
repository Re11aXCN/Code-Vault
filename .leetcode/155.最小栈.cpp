/*
 * @lc app=leetcode.cn id=155 lang=cpp
 *
 * [155] 最小栈
 */

// @lc code=start
class MinStack {
private:
    std::vector<int> main_stack;   // 主栈，存储所有元素
    std::vector<int> min_stack;    // 辅助栈，存储当前最小值

public:
    MinStack() {}
    
    void push(int val) {
        main_stack.push_back(val);
        // 如果辅助栈为空或新元素小于等于当前最小值，压入辅助栈
        if (min_stack.empty() || val <= min_stack.back()) {
            min_stack.push_back(val);
        }
    }
    
    void pop() {
        // 如果主栈栈顶元素等于当前最小值，弹出辅助栈栈顶
        if (main_stack.back() == min_stack.back()) {
            min_stack.pop_back();
        }
        main_stack.pop_back();
    }
    
    int top() {
        return main_stack.back();
    }
    
    int getMin() {
        return min_stack.back();
    }
};

/**
 * Your MinStack object will be instantiated and called as such:
 * MinStack* obj = new MinStack();
 * obj->push(val);
 * obj->pop();
 * int param_3 = obj->top();
 * int param_4 = obj->getMin();
 */
// @lc code=end

