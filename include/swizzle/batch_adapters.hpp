// CxxSwizzle
// Copyright (c) 2013-2015, Piotr Gwiazdowski <gwiazdorrr+github at gmail.com>
#pragma once

#include <cmath>
#include <cstdint>
#include <swizzle/detail/utils.h>
#include <array>

#define CXXSWIZZLE_FORCE_INLINE

namespace swizzle
{
    struct construct_tag {};

    template <typename DataType, typename PrimitiveType, size_t... Index>
    struct batch_base
    {
        static const size_t size = sizeof...(Index);
        using data_type = DataType;
        using primitive_type = PrimitiveType;

        using storage_type = std::conditional_t< size == 1, data_type, std::array<DataType, size> >;

        storage_type storage;

        CXXSWIZZLE_FORCE_INLINE batch_base()
        {}

        CXXSWIZZLE_FORCE_INLINE batch_base(const typename detail::only_if<size != 1, storage_type&> storage)
            : storage(storage)
        {}

        CXXSWIZZLE_FORCE_INLINE batch_base(const typename data_type& d)
        {
            ((at<Index>() = d), ...);
        }

        CXXSWIZZLE_FORCE_INLINE batch_base(const batch_base& other)
            : storage(other.storage)
        {}

        CXXSWIZZLE_FORCE_INLINE batch_base(detail::only_if<!std::is_same_v<primitive_type, data_type>, primitive_type> value)
            : batch_base(batch_scalar_cast(value))
        {}

        template <typename OtherBatchType, typename OtherPrimitiveType>
        CXXSWIZZLE_FORCE_INLINE explicit batch_base(const batch_base<OtherBatchType, OtherPrimitiveType, Index...>& other)
        {
            ((at<Index>() = batch_cast<primitive_type>(other.at<Index>())), ...);
        }

        template <typename... Types>
        CXXSWIZZLE_FORCE_INLINE explicit batch_base(construct_tag, Types&& ... values)
        {
            static_assert(sizeof...(Types) == sizeof...(Index));
            construct<0>(std::forward<Types>(values)...);
        }

        friend CXXSWIZZLE_FORCE_INLINE void load_aligned(batch_base& target, const PrimitiveType* ptr)
        {
            target.load_aligned_internal(ptr);
        }

        friend CXXSWIZZLE_FORCE_INLINE void store_aligned(const batch_base& target, PrimitiveType* ptr)
        {
            target.store_aligned_internal(ptr);
        }

        template <size_t Index, size_t Size = size>
        CXXSWIZZLE_FORCE_INLINE std::enable_if_t< (Size > 1), data_type>& at()
        {
            return storage[Index];
        }

        template <size_t Index, size_t Size = size>
        CXXSWIZZLE_FORCE_INLINE std::enable_if_t< (Size == 1), data_type>& at()
        {
            return storage;
        }

        template <size_t Index, size_t Size = size>
        CXXSWIZZLE_FORCE_INLINE const std::enable_if_t< (Size > 1), data_type>& at() const
        {
            return storage[Index];
        }

        template <size_t Index, size_t Size = size>
        CXXSWIZZLE_FORCE_INLINE const std::enable_if_t< (Size == 1), data_type>& at() const
        {
            return storage;
        }

        CXXSWIZZLE_FORCE_INLINE void assign(const batch_base& other)
        {
            (batch_assign(at<Index>(), other.at<Index>()),...);
        }

        CXXSWIZZLE_FORCE_INLINE void assign_fast(const batch_base& other)
        {
            ((at<Index>() = other.at<Index>()), ...);
        }

        template <size_t Index>
        CXXSWIZZLE_FORCE_INLINE void assign_at(const data_type& other)
        {
            batch_assign(at<Index>(), other);
        }

        template <size_t Index>
        CXXSWIZZLE_FORCE_INLINE void assign_at_fast(const data_type& other)
        {
            at<Index>() = other;
        }

    private:
        template <size_t Index, typename... Types>
        CXXSWIZZLE_FORCE_INLINE void construct(const data_type& first, Types&& ... others)
        {
            at<Index>() = first;
            construct<Index + 1>(std::forward<Types>(others)...);
        }

        template <size_t Index>
        CXXSWIZZLE_FORCE_INLINE void construct()
        {}

        CXXSWIZZLE_FORCE_INLINE void load_aligned_internal(const PrimitiveType* ptr)
        {
            (batch_load_aligned(at<Index>(), ptr + Index * sizeof(data_type) / sizeof(primitive_type)), ...);
        }

        CXXSWIZZLE_FORCE_INLINE void store_aligned_internal(PrimitiveType* ptr) const
        {
            (batch_store_aligned(at<Index>(), ptr + Index * sizeof(data_type) / sizeof(primitive_type)), ...);
        }
    };


    template <typename DataType, size_t... Index>
    struct bool_batch : batch_base<DataType, bool, Index...>
    {
        using batch_base::batch_base;
        using this_type = bool_batch;
        using this_arg = const this_type&;
        using primitive_type = typename batch_base::primitive_type;

        bool_batch(bool b) : bool_batch(batch_scalar_cast(b)) {}

        CXXSWIZZLE_FORCE_INLINE this_type& operator=(this_arg other) &
        {
            assign(other);
            return *this;
        }

        CXXSWIZZLE_FORCE_INLINE this_type& operator=(this_arg other) &&
        {
            assign_fast(other);
            return *this;
        }

        // for CxxSwizzle ADL-magic

        CXXSWIZZLE_FORCE_INLINE this_type decay() const
        {
            return *this;
        }

        // implement if you really need to.
        CXXSWIZZLE_FORCE_INLINE explicit operator bool() const { return (batch_collapse(at<Index>()) || ...); }

        CXXSWIZZLE_FORCE_INLINE friend this_type operator||(this_arg a, this_arg b) { return this_type(construct_tag{}, a.at<Index>() || b.at<Index>()...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type operator&&(this_arg a, this_arg b) { return this_type(construct_tag{}, a.at<Index>() && b.at<Index>()...); }
        CXXSWIZZLE_FORCE_INLINE this_type operator!() const { return this_type(construct_tag{}, !at<Index>()...); }
    };


    template <typename DataType, typename BoolType, size_t... Index>
    struct int_batch : batch_base<DataType, int, Index...>
    {
        using batch_base::batch_base;
        using this_type = int_batch;
        using this_arg = const this_type&;
        using primitive_type = typename batch_base::primitive_type;
        using bool_type = bool_batch<BoolType, Index...>;

        explicit int_batch(double value) : int_batch(static_cast<int>(value)) {}

        CXXSWIZZLE_FORCE_INLINE this_type decay() const
        {
            return *this;
        }

#include <swizzle/detail/common_batch_operators.hpp>

        // this type specific operators
        CXXSWIZZLE_FORCE_INLINE this_type& operator%=(this_arg other) { return *this = *this % other; }

        // functions: 8.3
        CXXSWIZZLE_FORCE_INLINE friend this_type min(this_arg x, this_arg y) { return this_type(construct_tag{}, min(x.at<Index>(), y.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type max(this_arg x, this_arg y) { return this_type(construct_tag{}, max(x.at<Index>(), y.at<Index>())...); }


        template <typename OtherBatchType, typename OtherPrimitiveType, typename... OtherTypes>
        CXXSWIZZLE_FORCE_INLINE friend void masked_read(const this_type& mask, batch_base<OtherBatchType, OtherPrimitiveType, Index...>& result, OtherTypes&&... others)
        {
            batch_masked_read(mask.storage, result.storage, others.storage...);
        }
    };

    template <typename DataType, typename BoolType, size_t... Index>
    struct uint_batch : batch_base<DataType, uint32_t, Index...>
    {
        using batch_base::batch_base;
        using this_type = uint_batch;
        using this_arg = const this_type&;
        using primitive_type = typename batch_base::primitive_type;
        using bool_type = bool_batch<BoolType, Index...>;

        // for CxxSwizzle ADL-magic

        CXXSWIZZLE_FORCE_INLINE this_type decay() const
        {
            return *this;
        }

#include <swizzle/detail/common_batch_operators.hpp>

        // this type specific operators
        CXXSWIZZLE_FORCE_INLINE this_type& operator%=(this_arg other) { return *this = *this % other; }

        // functions: 8.3
        CXXSWIZZLE_FORCE_INLINE friend this_type min(this_arg x, this_arg y) { return this_type(construct_tag{}, min(x.at<Index>(), y.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type max(this_arg x, this_arg y) { return this_type(construct_tag{}, max(x.at<Index>(), y.at<Index>())...); }

        template <typename OtherBatchType, typename OtherPrimitiveType, typename... OtherTypes>
        CXXSWIZZLE_FORCE_INLINE friend void masked_read(const this_type& mask, batch_base<OtherBatchType, OtherPrimitiveType, Index...>& result, OtherTypes&&... others)
        {
            batch_masked_read(mask.storage, result.storage, others.storage...);
        }
    };


    template <typename DataType, typename BoolType, size_t... Index>
    struct float_batch : batch_base<DataType, float, Index...>
    {
        using batch_base::batch_base;
        using this_type = float_batch;
        using this_arg = const this_type&;
        using primitive_type = typename batch_base::primitive_type;
        using bool_type = bool_batch<BoolType, Index...>;


        CXXSWIZZLE_FORCE_INLINE float_batch(double value) : float_batch(batch_scalar_cast(static_cast<float>(value))) {}
        CXXSWIZZLE_FORCE_INLINE float_batch(const bool_type& value) : batch_base(value) {}
        CXXSWIZZLE_FORCE_INLINE explicit float_batch(int32_t value) : float_batch(static_cast<float>(value)) {}
        CXXSWIZZLE_FORCE_INLINE explicit float_batch(uint32_t value) : float_batch(static_cast<float>(value)) {}

        // for CxxSwizzle ADL-magic

        CXXSWIZZLE_FORCE_INLINE this_type decay() const
        {
            return *this;
        }

#include <swizzle/detail/common_batch_operators.hpp>

        // functions
        // 8.1

        CXXSWIZZLE_FORCE_INLINE friend this_type radians(this_arg degrees) { return this_type(construct_tag{}, radians(degrees.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type degrees(this_arg radians) { return this_type(construct_tag{}, degrees(radians.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type cos(this_arg angle) { return this_type(construct_tag{}, cos(angle.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type tan(this_arg angle) { return this_type(construct_tag{}, tan(angle.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type sin(this_arg angle) { return this_type(construct_tag{}, sin(angle.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type asin(this_arg x) { return this_type(construct_tag{}, asin(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type acos(this_arg x) { return this_type(construct_tag{}, acos(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type atan(this_arg y_over_x) { return this_type(construct_tag{}, atan(y_over_x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type atan(this_arg y, this_arg x) { return this_type(construct_tag{}, atan(y.at<Index>(), x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type sinh(this_arg x) { return this_type(construct_tag{}, sinh(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type cosh(this_arg x) { return this_type(construct_tag{}, cosh(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type tanh(this_arg x) { return this_type(construct_tag{}, tanh(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type asinh(this_arg x) { return this_type(construct_tag{}, asinh(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type acosh(this_arg x) { return this_type(construct_tag{}, acosh(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type atanh(this_arg x) { return this_type(construct_tag{}, atanh(x.at<Index>())...); }

        // 8.2

        CXXSWIZZLE_FORCE_INLINE friend this_type pow(this_arg x, this_arg y) { return this_type(construct_tag{}, pow(x.at<Index>(), y.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type exp(this_arg x) { return this_type(construct_tag{}, exp(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type log(this_arg x) { return this_type(construct_tag{}, log(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type exp2(this_arg x) { return this_type(construct_tag{}, exp2(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type log2(this_arg x) { return this_type(construct_tag{}, log2(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type sqrt(this_arg x) { return this_type(construct_tag{}, sqrt(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type inversesqrt(this_arg x) { return this_type(construct_tag{}, inversesqrt(x.at<Index>())...); }

        // 8.3

        CXXSWIZZLE_FORCE_INLINE friend this_type abs(this_arg x) { return this_type(construct_tag{}, abs(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type sign(this_arg x) { return this_type(construct_tag{}, sign(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type floor(this_arg x) { return this_type(construct_tag{}, floor(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type trunc(this_arg x) { return this_type(construct_tag{}, trunc(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type round(this_arg x) { return this_type(construct_tag{}, round(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type roundEven(this_arg x) { return this_type(construct_tag{}, roundEven(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type ceil(this_arg x) { return this_type(construct_tag{}, ceil(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type fract(this_arg x) { return this_type(construct_tag{}, fract(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type mod(this_arg x, this_arg y) { return this_type(construct_tag{}, mod(x.at<Index>(), y.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type modf(this_arg x, this_type& i) { return this_type(construct_tag{}, modf(x.at<Index>(), i.at<Index>())...); }

        CXXSWIZZLE_FORCE_INLINE friend this_type min(this_arg x, this_arg y) { return this_type(construct_tag{}, min(x.at<Index>(), y.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type max(this_arg x, this_arg y) { return this_type(construct_tag{}, max(x.at<Index>(), y.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type mix(this_arg x, this_arg y, const bool_type& a) { return this_type(construct_tag{}, mix(x.at<Index>(), y.at<Index>(), a.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type step(this_arg edge, this_arg x) { return this_type(construct_tag{}, step(edge.at<Index>(), x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type smoothstep(this_arg edge0, this_arg edge1, this_arg x) { return smoothstep_helper(edge0, edge1, x); }

        CXXSWIZZLE_FORCE_INLINE friend bool_type isnan(this_arg x) { return this_type(construct_tag{}, isnan(x.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend bool_type isinf(this_arg x) { return this_type(construct_tag{}, isinf(x.at<Index>())...); }

        //genIType floatBitsToInt(genType value)
        //genUType floatBitsToUint(genType value)

        // 8.4
        CXXSWIZZLE_FORCE_INLINE friend this_type length(this_arg x) { return x; }
        CXXSWIZZLE_FORCE_INLINE friend this_type distance(this_arg p0, this_arg p1) { return abs(p0 - p1); }
        CXXSWIZZLE_FORCE_INLINE friend this_type dot(this_arg x, this_arg y) { return x * y; }
        CXXSWIZZLE_FORCE_INLINE friend this_type normalize(this_arg x) { return step(this_type(construct_tag{}, 0.0f), x) * 2.0f - 1.0f; }
        CXXSWIZZLE_FORCE_INLINE friend this_type faceforward(this_arg N, this_arg I, this_arg Nref) { return (step(this_type(construct_tag{}, 0.0f), dot(Nref, I)) * (-2.0f) + 1.0f) * N; }
        CXXSWIZZLE_FORCE_INLINE friend this_type reflect(this_arg N, this_arg I) { return (I - 2.0f * dot(I, N) * N); }
        CXXSWIZZLE_FORCE_INLINE friend this_type refract(this_arg I, this_arg N, this_arg eta)
        {
            this_type k = 1.0 - eta * eta * (1.0 - dot(N, I) * dot(N, I));
            this_type mask = step(this_type(0), k);
            return mask * (eta * I - (eta * dot(N, I) + sqrt(k)) * N);
        }

        // 8.8
        CXXSWIZZLE_FORCE_INLINE friend this_type dFdx(this_arg p) { return this_type(construct_tag{}, dFdx(p.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type dFdy(this_arg p) { return this_type(construct_tag{}, dFdy(p.at<Index>())...); }
        CXXSWIZZLE_FORCE_INLINE friend this_type fwidth(this_arg p)
        {
            return abs(dFdx(p)) + abs(dFdy(p));
        }

        static this_type smoothstep_helper(this_arg edge0, this_arg edge1, this_arg x)
        {
            auto t = (x - edge0) / (edge1 - edge0);
            t = min(max(t, this_type(0.0)), this_type(1.0));
            return t * t * (this_type(3.0) - this_type(2.0) * t);
        }

    };
}