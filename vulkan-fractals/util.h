#pragma once

#include <iostream>
#include <tuple>
#include <algorithm>
#include <iterator>
#include <array>
#include <utility>
#include <fstream>
#include <cstddef>
#include <chrono>

#include <vulkan/vulkan.h>

#include <fmt/printf.h>
#include <fmt/format.h>
#include <fmt/string.h>
#include <fmt/ostream.h>
#include <fmt/time.h>
#include <fmt/container.h>
#include <glm/glm.hpp>

extern const std::array<std::pair<VkResult, const char *>, 18> VULKAN_ERROR_STRINGS;

namespace details {

    template<class T>
    static void check(T &&message)
    {
        std::cout << std::forward<T>(message);
    }

    template<class T, class... TRest>
    void check(T &&message, TRest &&...rest)
    {
        std::cout << std::forward<T>(message);
        check(std::forward<TRest>(rest)...);
    }

} // namespace details

/**
 * Checks that the given value evaluates to true, otherwise the given message is printed and a runtime exception is thrown
 * @tparam T
 * @tparam TRest
 * @param condition
 * @param message
 * @param rest
 */
template<class T, class... TRest>
void check(bool condition, T &&message, TRest &&...rest)
{
    if (condition) { return; }
    std::cout << "ERROR: ";
    details::check(std::forward<T>(message), std::forward<TRest>(rest)...);
    throw std::runtime_error("check condition failed");
}

/**
 * Return the given Vulkan error constant as a string
 * @param result Vulkan error code
 * @return String representation of error code
 */
inline const char *vulkanErrorString(VkResult result)
{
    auto iter = std::find_if(std::begin(VULKAN_ERROR_STRINGS), std::end(VULKAN_ERROR_STRINGS), [&](const auto &map) {
        return map.first == result;
    });
    if (iter == std::end(VULKAN_ERROR_STRINGS)) {
        return "unknown-error";
    }
    return iter->second;
}

/**
 * Eliminate the need to call a Vulkan listing function twice to allocate a vector by hand.
 * @tparam T Vulkan type to be stored
 * @tparam TPfn Type of function to be called
 * @tparam TParams Types of custom parameters
 * @param pfn Vulkan listing function to be called
 * @param params Params passed to the Vulkan function. Excludes the allocator, count and storing vector as parameters.
 * @return Vector with stored Vulkan objects
 */
template<class T, class TPfn, class... TParams>
std::vector<T> listVulkan(TPfn &&pfn, TParams &&...params)
{
    uint32_t count;
	std::vector<T> vector;
    (*std::forward<TPfn>(pfn))(std::forward<TParams>(params)..., &count, nullptr);
	vector.resize(count);
    (*std::forward<TPfn>(pfn))(std::forward<TParams>(params)..., &count, vector.data());
    return std::move(vector);
};

/**
 * Check the result to be {@code VK_TRUE}, otherwise print the given error message
 * together with the Vulkan error code string and throws a runtime exception.
 * @tparam TParams Types of the message parameters
 * @param result Vulkan result to be checked
 * @param message Message to be printed
 * @param params Message parameters to be printed
 */
template<class... TParams>
void checkVk(VkResult result, const std::string &message, TParams &&...params)
{
    check(result == VK_SUCCESS, '[', vulkanErrorString(result), "] ", message, std::forward<TParams>(params)...);
}

/**
 * Clamps the given value between the given bounds, inclusively
 * @tparam T Type of value
 * @tparam TMin Type of minimum
 * @tparam TMax Type of maximum
 * @param value Value to clamped
 * @param min Lower bound
 * @param max Upper bound
 * @return Clamped value
 */
template<class T, class TMin, class TMax>
T clamp(T &&value, TMin &&min, TMax &&max)
{
    return std::max(std::min(std::forward<T>(static_cast<T>(max)), std::forward<T>(value)),
                    std::forward<T>(static_cast<T>(min)));
}

namespace details {

    /**
     * Loads the given Vulkan instance-level function and calls it with the given parameters.
     * @tparam TPfn Type of Vulkan function
     * @tparam TParams Type of additional parameters
     * @param name Name of Vulkan function
     * @param instance Vulkan instance
     * @param params Additional parameters
     * @return Return value of Vulkan function or nothing if void function
     */
    template<class TPfn, class... TParams>
    auto invokeVk(const std::string &name, VkInstance instance, TParams &&...params)
    {
        auto func = reinterpret_cast<TPfn>(vkGetInstanceProcAddr(instance, name.c_str()));
        ::check(func != nullptr, "Cannot load Vulkan instance-level function ", name);
        return func(instance, std::forward<TParams>(params)...);
    };

    /**
     * Loads the given Vulkan device-level function and calls it with the given parameters.
     * @tparam TPfn Type of Vulkan function
     * @tparam TParams Type of additional parameters
     * @param name Name of Vulkan function
     * @param device Vulkan logical device
     * @param params Additional parameters
     * @return Return value of Vulkan function or nothing if void function
     */
    template<class TPfn, class... TParams>
    auto invokeVk(const std::string &name, VkDevice device, TParams &&...params)
    {
        auto func = reinterpret_cast<TPfn>(vkGetDeviceProcAddr(device, name.c_str()));
        ::check(func != nullptr, "Cannot load Vulkan device-level function ", name);
        return func(device, std::forward<TParams>(params)...);
    };

} // namespace details

#define invokeVk(name, ...) details::invokeVk<PFN_##name>(#name, __VA_ARGS__)

std::vector<uint8_t> readRawFile(const std::string &filename);

/**
 * Memory size of container in bytes.
 * Assumes the container to provide its value type and a {@code size()} method.
 * @tparam T Type of container
 * @param container Container to be queried
 * @return Size of container in bytes
 */
template<class T>
auto byteSize(const T &container)
{
    return container.size() * sizeof(typename T::value_type);
}

namespace details {

	using namespace std::chrono;

	template<class... TParams>
	void log(
		const char *func,
		const char *file,
		int line,
		const char *logMessageFormat,
		TParams &&...params
	)
	{
#ifndef NDEBUG
		auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        auto millis = static_cast<int>(now.count() % 1000);
        auto seconds = static_cast<int>(now.count() / 1000 % 60);
        auto minutes = static_cast<int>(now.count() / 1000 / 60 % 60);
        auto hours = static_cast<int>(now.count() / 1000 / 60 / 60 % 24 + 1);

        fmt::printf("%d:%d:%d:%d [%s:%d]  %s: ", hours, minutes, seconds, millis, file, line, func);
		fmt::printf(logMessageFormat, std::forward<TParams>(params)...);
		fmt::print("\n");
#endif // NDEBUG
	}

} // namespace details

#ifdef _WIN32
#define FUNCTION_NAME __FUNCSIG__
#else
#define FUNCTION_NAME __PRETTY_FUNCTION__
#endif

#define log(...) details::log(FUNCTION_NAME, __FILE__, __LINE__, __VA_ARGS__)

inline glm::vec3 hsvToRgb(float h, float s, float v)
{
    float c = v * s;
    auto hPrime = static_cast<float>(fmod(h / 60.0, 6));
    auto x = static_cast<float>(c * (1 - fabs(fmod(hPrime, 2) - 1)));
    float m = v - c;
    float r, g, b;

    if (0 <= hPrime && hPrime < 1) {
        r = c;
        g = x;
        b = 0;
    }
    else if (1 <= hPrime && hPrime < 2) {
        r = x;
        g = c;
        b = 0;
    }
    else if (2 <= hPrime && hPrime < 3) {
        r = 0;
        g = c;
        b = x;
    }
    else if (3 <= hPrime && hPrime < 4) {
        r = 0;
        g = x;
        b = c;
    }
    else if (4 <= hPrime && hPrime < 5) {
        r = x;
        g = 0;
        b = c;
    }
    else if (5 <= hPrime && hPrime < 6) {
        r = c;
        g = 0;
        b = x;
    }
    else {
        r = 0;
        g = 0;
        b = 0;
    }

    r += m;
    g += m;
    b += m;

    glm::vec3 ret;
    ret.r = r;
    ret.g = g;
    ret.b = b;
    return ret;
}
