#include "ScopeProfiler.h"
#include "print.h"
#include<vector>

void fake(int i); //GCC可以，MSVC有链接时优化不行

int main()
{
	/* debug	doNotOptimize(sum);
	   avg   |   min   |   max   |  total  | cnt | tag
   154981|   154981|   154981|   154981|    1| 迭代器遍历
	 7689|     7689|     7689|     7689|    1| 下标遍历
	 1944|     1944|     1944|     1944|    1| range遍历
	 1716|     1716|     1716|     1716|    1| 指针遍历
	*/

	/* release	doNotOptimize(sum);
	   avg   |   min   |   max   |  total  | cnt | tag
	  947|      947|      947|      947|    1| 指针遍历
	  707|      707|      707|      707|    1| 迭代器遍历
	  678|      678|      678|      678|    1| range遍历
	  412|      412|      412|      412|    1| 下标遍历
	*/
	int sum = 0;
	std::vector<int> arr;
	arr.resize(1'000'000);
	for (size_t i = 0; i < arr.size(); i++)
	{
		arr[i] = i;
	}
	// 以下为性能测试
	{
		sum = 0;
		ScopeProfiler _("下标遍历");
		for (size_t i = 0; i < arr.size(); i++)
		{
			sum += arr[i];
		}
		//print(sum);
		//fake(sum);
		doNotOptimize(sum);
	}
	{
		sum = 0;
		ScopeProfiler _("range遍历");
		for (auto const& a : arr)
		{
			sum += a;
		}
		//print(sum);
		//fake(sum);
		doNotOptimize(sum);
	}
	{
		sum = 0;
		ScopeProfiler _("迭代器遍历");
		for (auto it = arr.begin(); it != arr.end(); ++it)
		{
			sum += *it;
		}
		//print(sum);
		//fake(sum);
		doNotOptimize(sum);
	}
	{
		sum = 0;
		ScopeProfiler _("指针遍历");
		int* p = arr.data(), * endp = p + arr.size();
		while (p != endp)
		{
			sum += *p;
			++p;
		}
		//print(sum);
		//fake(sum);
		doNotOptimize(sum);
	}
	printScopeProfiler();
}