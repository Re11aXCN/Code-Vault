#ifndef SNOWFLAKEGENERATOR_HPP
#define SNOWFLAKEGENERATOR_HPP

#include <atomic>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <bit>
#include <concepts>
#include <cstdint>
#include <limits>
#include <type_traits>

// ѩ���㷨ģ����
template <
    typename T = uint64_t,
    std::size_t TimestampBits = 41, // ʱ���λ����Ĭ��41λ��Լ69�꣩
    std::size_t MachineIdBits = 10, // ����IDλ����Ĭ��10λ��1024̨������
    std::size_t SequenceBits = 12,  // ���к�λ����Ĭ��12λ��ÿ����4096��ID��
    uint64_t Epoch = 1609459200000ULL // ��ʼʱ��� 2021-01-01 00:00:00 UTC
>
    requires std::unsigned_integral<T> &&
(TimestampBits + MachineIdBits + SequenceBits + 1 == sizeof(T) * 8)
class SnowflakeGenerator {
public:
    // ��̬������֤������Ч��
    static_assert(TimestampBits > 0 && MachineIdBits > 0 && SequenceBits > 0,
        "Bits must be positive");
    static_assert(sizeof(T) * 8 == TimestampBits + MachineIdBits + SequenceBits + 1,
        "Total bits must match type size");
    static_assert(TimestampBits <= 42, "Timestamp too large for millisecond precision");
    static_assert(SequenceBits > 1, "Sequence bits too small");

    // ����������ֵ
    static constexpr T kMaxMachineId = (T{ 1 } << MachineIdBits) - 1;
    static constexpr T kMaxSequence = (T{ 1 } << SequenceBits) - 1;
    static constexpr T kMaxTimestamp = (T{ 1 } << TimestampBits) - 1;

    // ���캯��
    explicit SnowflakeGenerator(T machine_id)
        : machine_id_(machine_id),
        last_timestamp_(0),
        sequence_(0) {
        if (machine_id > kMaxMachineId) {
            throw std::invalid_argument("Machine ID exceeds maximum value");
        }
    }

    // ����ΨһID
    T generate() {
        T current_timestamp = current_time();
        T old_timestamp = last_timestamp_.load(std::memory_order_relaxed);

        // ����ʱ�����
        if (current_timestamp < old_timestamp) {
            handle_clock_backwards(old_timestamp, current_timestamp);
        }

        // ͬһ�����ڵ����д���
        if (current_timestamp == old_timestamp) {
            T seq = sequence_.fetch_add(1, std::memory_order_relaxed) & kMaxSequence;

            // ���к�������ȴ���һ����
            if (seq == 0) {
                current_timestamp = wait_next_millis(old_timestamp);
            }
        }
        else {
            sequence_.store(0, std::memory_order_relaxed);
        }

        // �������ʱ���
        last_timestamp_.store(current_timestamp, std::memory_order_relaxed);

        // �������ID
        return ((current_timestamp << (MachineIdBits + SequenceBits)) |
            (machine_id_ << SequenceBits) |
            (sequence_.fetch_add(1, std::memory_order_relaxed) & kMaxSequence);
    }

private:
    // ��ȡ��ǰʱ���
    T current_time() const {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto ms = duration_cast<milliseconds>(now.time_since_epoch());
        return static_cast<T>(ms.count() - Epoch);
    }

    // ����ʱ�ӻ���
    void handle_clock_backwards(T old_timestamp, T current_timestamp) {
        auto diff = old_timestamp - current_timestamp;
        if (diff > max_clock_backward_ms) {
            throw std::runtime_error("Clock moved backwards more than allowed");
        }
        std::this_thread::sleep_for(
            std::chrono::milliseconds(diff + 1));
    }

    // �ȴ���һ����
    T wait_next_millis(T last_timestamp) const {
        T timestamp = current_time();
        while (timestamp <= last_timestamp) {
            std::this_thread::yield();
            timestamp = current_time();
        }
        return timestamp;
    }

    // ��Ա����
    const T machine_id_;
    std::atomic<T> last_timestamp_;
    std::atomic<T> sequence_;

    // ��������ʱ�ӻ��ˣ����룩
    static constexpr T max_clock_backward_ms = 100;
};

// ����ģ���ʹ��
using StandardSnowflake = SnowflakeGenerator<>;

// ʾ���÷�
int main() {
    // ����ѩ��������������IDΪ123��
    StandardSnowflake generator(123);

    // ����10��ID
    for (int i = 0; i < 10; ++i) {
        auto id = generator.generate();
        // ��ʵ��Ӧ����ʹ�����ɵ�ID
        // std::cout << "Generated ID: " << id << "\n";
    }
    using CustomSnowflake = SnowflakeGenerator<
        uint64_t,   // ID����
        39,         // ʱ���λ����Լ17�꣩
        12,         // ����IDλ����4096̨��
        12,         // ���к�λ��
        1672531200000ULL // 2023-01-01��ʼʱ��
    >;
    CustomSnowflake generator(2049); // ����ID

    // ���ɷֲ�ʽID
    auto order_id = generator.generate();
    auto user_id = generator.generate();
    return 0;
}#endif // !SNOWFLAKEGENERATOR_HPP
