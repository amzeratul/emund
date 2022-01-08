#pragma once

#include <halley.hpp>


template <typename T, size_t FirstBit = 0, size_t BitLen = sizeof(T) * 8>
class ConstBitView
{
    static_assert(FirstBit + BitLen <= sizeof(T) * 8);
    static_assert(BitLen > 0);

public:
    constexpr static size_t firstBit = FirstBit;
    constexpr static size_t lastBit = FirstBit + BitLen - 1;
	constexpr static size_t bitLen = BitLen;
    using ValueType = T;

    constexpr ConstBitView(const T& value)
        : value(value)
    {}

    constexpr T getMaskedValue() const
    {
        return value & makeMask();
    }

    constexpr static T makeMask()
    {
        const auto low = (1 << firstBit) - 1;
        const auto high = (1 << (lastBit + 1)) - 1;
        return high & ~low;
    }

private:
    const T& value;
};


template <typename T, size_t FirstBit = 0, size_t BitLen = sizeof(T) * 8>
class BitView
{
    static_assert(FirstBit + BitLen <= sizeof(T) * 8);
    static_assert(BitLen > 0);

public:
    constexpr static size_t firstBit = FirstBit;
    constexpr static size_t lastBit = FirstBit + BitLen - 1;
	constexpr static size_t bitLen = BitLen;
    using ValueType = T;

    constexpr BitView(T& value)
        : baseValue(value)
    {}

	constexpr static BitView of(T& value)
    {
        return BitView(value);
    }

	constexpr static ConstBitView<T, FirstBit, BitLen> of(const T& value)
    {
        return ConstBitView<T, FirstBit, BitLen>(value);
    }

    template <typename OtherType>
    constexpr void set(const OtherType& other)
    {
        if constexpr (std::is_integral_v<OtherType>) {
            set(ConstBitView<OtherType, 0, sizeof(OtherType) * 8>(other));
        } else {
            using H = std::conditional_t<sizeof(T) >= sizeof(typename OtherType::ValueType), T, typename OtherType::ValueType>;
            setMaskedValue(shift(static_cast<H>(other.getMaskedValue()), static_cast<int>(firstBit) - static_cast<int>(OtherType::firstBit)));
        }
    }

	template <typename OtherType>
	constexpr BitView& operator=(const OtherType& other)
    {
        set(other);
        return *this;
    }

	BitView& operator+=(T value)
    {
        set(T(getValue() + value));
        return *this;
    }

	BitView& operator-=(T value)
    {
        set(T(getValue() + value));
        return *this;
    }

    constexpr static T makeMask()
    {
        const auto low = (1 << firstBit) - 1;
        const auto high = (1 << (lastBit + 1)) - 1;
        return high & ~low;
    }

    constexpr T getMaskedValue() const
    {
        return baseValue & makeMask();
    }

	constexpr void setMaskedValue(T value)
    {
        baseValue = (baseValue & ~makeMask()) | (value & makeMask());
    }

	constexpr T getValue() const
    {
        return shift(getMaskedValue(), -static_cast<int>(FirstBit));
    }

private:
    T& baseValue;

	template <typename V>
    V constexpr static shift(V v, int amount)
    {
	    if (amount >= 0) {
            return v << amount;
	    } else {
            return v >> -amount;
	    }
    }
};
