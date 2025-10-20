```
-Wall -Wextra -Weffc++
-Werror=uninitialized
-Werror=return-type
-Wconversion -Wsign-compare
-Werror=unused-result
-Werror=suggest-override
-Wzero-as-null-pointer-constant
-Wmissing-declarations
-Wold-style-cast -Werror=vla
-Wnon-virtual-dtor
```

GCC提供了大量编译期警告选项，在运行程序前就帮你发现错误，可以大幅提升代码质量！强烈推荐开启-Wall -Wextra -Werror，将这些常见有问题的写法，从警告转化为编译期错误，迫使你经过一次理智检查（sanity-check）后，如果确认的确有需要，使用(void)x，[[fallthrough]]等特殊写法，阻止警告。如果需要关闭一部分不喜欢的警告，比如-Wall自带的-Wunused警告可以使用-Wno-unused关闭，如果只想把一部分警告转化为错误，可以-Werror=uninitialized。
省流：
-Wall -Wextra -Weffc++
-Werror=uninitialized
-Werror=return-type
-Wconversion -Wsign-compare
-Werror=unused-result
-Werror=suggest-override
-Wzero-as-null-pointer-constant
-Wmissing-declarations
-Wold-style-cast
-Wnon-virtual-dtor
-Werror（对自己狠一点）
在你的项目开启他们，暂时会产生一系列错误，这些都是常见的错误写法，或是有安全隐患的写法。需要你去修改，但改完后，你的代码质量将得到大幅提升。如果是一个新项目，那更好了，你可以从一开始就开启这些警告，有效保证代码的可维护性。你可能无法接受其中的全部，可以选择只开启一部分，也可以先开一部分（比如-Wall和-Wnon-virtual-dtor）试试，感到确实对代码可维护性有帮助后，再逐渐接受其他更严格的警告选项（比如-Wconversion和-Wsign-compare）。
