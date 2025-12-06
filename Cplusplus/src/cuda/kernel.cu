#include <cstdio>
#include <cuda_runtime.h>

__host__ __device__ void say_hello() {
#if defined(__CUDA_ARCH__) && (__CUDA_ARCH__ > 0)
    printf("Hello from kernel!\n");
#else
    printf("Hello from host!\n");
#endif
}

__global__ void kernel() {
    say_hello();
}

int main() {
    kernel<<<1, 1>>>();
    cudaDeviceSynchronize();
    say_hello();
    return 0;
}