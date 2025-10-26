#include <vector>
#include <atomic>
#include <cmath>
#include <tbb/parallel_for.h>
#include "no_initialized_pod.h"
#include "utils/ticktock.h"

static auto filter_sin_greater_than_zero() -> void
{
    std::size_t n = 1 << 27;
    std::vector<NoInitializedPod<float>> filter_res;
    std::atomic<std::size_t> filter_size;
    TICK(filter_sin_greater_than_zero)
    filter_res.resize(n);
    tbb::parallel_for(tbb::blocked_range<std::size_t>(0, n),
        [&](tbb::blocked_range<std::size_t> r) {
            std::size_t local_size{ 0 };
            std::vector<NoInitializedPod<float>> local_v(r.size());
            for (std::size_t i = r.begin(); i != r.end(); ++i) {
                if (float val = std::sin(i); val > 0) {
                    local_v[local_size++] = NoInitializedPod<float>(val);
                }
            }
            std::size_t old_size = filter_size.fetch_add(local_size);
            for (std::size_t i = 0; i != local_size; ++i) {
                filter_res[old_size + i] = std::move(local_v[i]);
            }
        });
    filter_res.resize(filter_size);
    TOCK(filter_sin_greater_than_zero)
}
