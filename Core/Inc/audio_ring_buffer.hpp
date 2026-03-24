#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <cstring>

template <typename T, size_t Size>
class AudioRingBuffer {
	static_assert((Size & (Size - 1)) == 0, "Size must be a power of 2");

public:
	AudioRingBuffer() : pWrite_(0), pRead_(0), isReady_(false) {}

	void write(const T* input, size_t length) {
		size_t current_write = pWrite_.load(std::memory_order_relaxed);

		for (size_t i = 0; i < length; ++i) {
				data_[(current_write + i) & (Size - 1)] = input[i];
		}

		pWrite_.store(current_write + length, std::memory_order_release);
}

	void read(T* output, size_t length) {
		size_t current_read = pRead_.load(std::memory_order_relaxed);
		size_t current_write = pWrite_.load(std::memory_order_acquire);
		size_t available = current_write - current_read;

		if (!isReady_) {
			if (available >= 1024) {
				isReady_ = true;
			}
		}

		if (isReady_) {
			if (available >= length) {
				for (size_t i = 0; i < length; ++i) {
					output[i] = data_[(current_read + i) & (Size - 1)];
				}
				pRead_.store(current_read + length, std::memory_order_release);
			} else {
				std::memset(output, 0, length * sizeof(T));
				isReady_ = false;
			}
		} else {
			std::memset(output, 0, length * sizeof(T));
		}
	}

	void reset() {
		pWrite_.store(0, std::memory_order_release);
		pRead_.store(0, std::memory_order_release);
		isReady_ = false;
	}

	size_t getAvailableSamples() const {
		return pWrite_.load(std::memory_order_acquire) - pRead_.load(std::memory_order_relaxed);
	}

	size_t getAvailableFrames() const {
		return getAvailableSamples() / 2;
	}

	bool getIsReady() const {
		return isReady_;
	}

private:
	std::array<T, Size> data_;
	std::atomic<size_t> pWrite_{};
	std::atomic<size_t> pRead_{};
	std::atomic<bool> isReady_{};
};
