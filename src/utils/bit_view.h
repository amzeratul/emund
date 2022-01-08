#pragma once

#include <halley.hpp>


template <typename T, size_t FirstBit = 0, size_t LastBit = sizeof(T) * 8 - 1>
class ConstBitView
{
    static_assert(FirstBit <= LastBit);
    static_assert(LastBit < sizeof(T) * 8);

public:
    constexpr static size_t firstBit = FirstBit;
    constexpr static size_t lastBit = LastBit;
	constexpr static size_t bitLen = LastBit - FirstBit - 1;
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
        const auto low = (1 << FirstBit) - 1;
        const auto high = (1 << (LastBit + 1)) - 1;
        return high & ~low;
    }

private:
    const T& value;
};


template <typename T, size_t FirstBit = 0, size_t LastBit = sizeof(T) * 8 - 1>
class BitView
{
    static_assert(FirstBit <= LastBit);
    static_assert(LastBit < sizeof(T) * 8);

public:
    constexpr static size_t firstBit = FirstBit;
    constexpr static size_t lastBit = LastBit;
    constexpr static size_t bitLen = LastBit - FirstBit - 1;
    using ValueType = T;

    constexpr BitView(T& value)
        : baseValue(value)
    {}

	constexpr static BitView of(T& value)
    {
        return BitView(value);
    }

	constexpr static ConstBitView<T, FirstBit, LastBit> of(const T& value)
    {
        return ConstBitView(value);
    }

    template <typename OtherType>
    constexpr void set(const OtherType& other)
    {
        if constexpr (std::is_integral_v<OtherType>) {
            set(ConstBitView<OtherType, 0, sizeof(OtherType) * 8 - 1>(other));
        } else {
            //static_assert(bitLen == OtherType::bitLen);
            using H = std::conditional_t<sizeof(T) >= sizeof(OtherType::ValueType), T, typename OtherType::ValueType>;
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
        setMaskedValue(getMaskedValue() + value);
        return *this;
    }

	BitView& operator-=(T value)
    {
        setMaskedValue(getMaskedValue() + value);
        return *this;
    }

    constexpr static T makeMask()
    {
        const auto low = (1 << FirstBit) - 1;
        const auto high = (1 << (LastBit + 1)) - 1;
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
