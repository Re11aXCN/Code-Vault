https://www.bilibili.com/video/BV1gu411m7kN

pmr

execute

tbb
C#并行

reduce


parallel_reduce

parallel_deterministic_reduce

tbb::blocked_range

并行缩并 O(n/c+c) 工作复杂度O(n)，n是元素个数
改进的并行缩并适合GPU O(logn)工作复杂度O(n)


浮点数很大加很小，等于没加，精度丢失，并行处理浮点数（每次处理小部分）比串行好


scan（前缀和）

parallel_scan

并行扫描 O(n/c+c) 工作复杂度O(n+c)，n是元素个数
改进的并行缩并适合GPU O(logn)工作复杂度O(nlogn)

tbb::tick_count::now() 和 chrono类似，不过更精准



google benchmark

set(BENCHMARK_ENABLE_TESING OFF CHCHE BOOL "Turn off the fking test!")
使用时不需要googletest依赖模块

tbb两次并行嵌套并上锁，因为并行如果自己处理完之后会抢别人工作做，加快效率，但是容易造成死锁，建议使用recursive_mutex递归锁
更推荐内层for再lock之后，
locck_guard
tbb::this_task_arena::isolate([&]{
tbb::parallel_for

tbb::simple_partitioner [Z字形读取（见第14讲  第八章莫顿码）](./第14讲：内存对齐页访存优化 内容要点.md)，为了缓存高命中，临近数据不会那么快读取失效

并发vector
tbb::concurrent_vector 保证push之后不会因为扩容导致原来数据失效，但是不保证元素在内存是连续的
vector、deque的结合体？
tbb也提供了其他的并发数据结构
boost也提供，msvc也提供

sobol采样学习