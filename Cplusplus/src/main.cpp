 #include "header.h"

 ///*
 // #include <boost/container/small_vector.hpp>
 // #ifdef _WIN32
 // #include <windows.h>
 // #else
 // #include <fcntl.h>
 // #include <sys/mman.h>
 // #include <sys/stat.h>
 // #include <unistd.h>
 // #endif
 // #include <experimental/generator>
 // #include <immintrin.h>
 // #include <tbb/concurrent_vector.h>
 // #include <tbb/enumerable_thread_specific.h>
 // #include <tbb/parallel_for.h>
 // #include <tbb/parallel_invoke.h>
 // #include <tbb/parallel_sort.h>
 //*/

 template<typename Type>
 concept Align32 = (sizeof(Type) & 31) == 0;
 #include "algorithm/radix_sort.hpp"
 #include "profiling/ticktock.hpp"

 int main()
 {
   static_assert(std::is_trivially_destructible_v<int*>);
   using namespace std::literals;
   std::ios_base::sync_with_stdio(false);
   std::cout.setf(std::ios_base::boolalpha);

 #pragma pack(push, 16)

   struct A
   {
     char   a;
     int    c;
     double b;
   };

 #pragma pack(pop)

   sizeof(A);
   alignof(A);

   struct alignas(32) MyStruct
   {
     int    a;
     double b;
   };

   void* raw_mem = _aligned_malloc(alignof(MyStruct), sizeof(MyStruct));

   // 只析构对象，因为内存是placement  new分配的
   Align32 auto* obj =
       new (raw_mem) MyStruct();  

   obj->~MyStruct();
   _aligned_free(raw_mem);

   // 或者使用operator new的重载版本
   void* raw_mem2 =
       ::operator new (sizeof(MyStruct), std::align_val_t(alignof(MyStruct)));
   MyStruct* obj2 = new (raw_mem2) MyStruct();
   obj2->~MyStruct();  // 记得使用对应的operator delete释放
   ::operator delete (raw_mem2, std::align_val_t(alignof(MyStruct)));

   // https://cppreference.cn/w/cpp/thread/hardware_destructive_interference_size

   // std::hardware_destructive_interference_size // 避免伪共享的对齐常量

   return 0;
 }
