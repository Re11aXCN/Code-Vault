#include "ScopeProfiler.h"
#include "print.h"
#include<vector>

void fake(int i); //GCC���ԣ�MSVC������ʱ�Ż�����

int main()
{
	/* debug	doNotOptimize(sum);
	   avg   |   min   |   max   |  total  | cnt | tag
   154981|   154981|   154981|   154981|    1| ����������
	 7689|     7689|     7689|     7689|    1| �±����
	 1944|     1944|     1944|     1944|    1| range����
	 1716|     1716|     1716|     1716|    1| ָ�����
	*/

	/* release	doNotOptimize(sum);
	   avg   |   min   |   max   |  total  | cnt | tag
	  947|      947|      947|      947|    1| ָ�����
	  707|      707|      707|      707|    1| ����������
	  678|      678|      678|      678|    1| range����
	  412|      412|      412|      412|    1| �±����
	*/
	int sum = 0;
	std::vector<int> arr;
	arr.resize(1'000'000);
	for (size_t i = 0; i < arr.size(); i++)
	{
		arr[i] = i;
	}
	// ����Ϊ���ܲ���
	{
		sum = 0;
		ScopeProfiler _("�±����");
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
		ScopeProfiler _("range����");
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
		ScopeProfiler _("����������");
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
		ScopeProfiler _("ָ�����");
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