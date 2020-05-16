﻿// CxxSwizzle
// Copyright (c) 2013-2015, Piotr Gwiazdowski <gwiazdorrr+github at gmail.com>
#pragma once

// VC needs to come first or else it's going to complain (damn I hate these)
#include <Vc/vector.h>
#include <Vc/global.h>
#include <type_traits>
#include <swizzle/batch_adapters.hpp>
#include <swizzle/detail/fwd.hpp>
#include <swizzle/detail/simd_mask.h>
#include <swizzle/detail/vector_traits.h>
#include <swizzle/detail/batch_write_mask.hpp>

namespace swizzle
{
    using float_type = float_batch<::Vc::float_v, ::Vc::float_m, 0>;
    using int_type   = int_batch<::Vc::int32_v, ::Vc::float_m, 0>;
    using uint_type  = uint_batch<::Vc::uint32_v, ::Vc::float_m, 0>;
    using bool_type  = bool_batch<::Vc::float_m, 0>;

    // batch types traits definitions
    namespace detail
    {
        template <>
        struct batch_traits<float_type> : batch_traits_builder<
            ::Vc::float_v,
            float,
            ::Vc::VectorAlignment, ::Vc::float_v::Size, float_type::size,
            bool_type,
            false, false, true>
        {};

        template <>
        struct batch_traits<int_type> : batch_traits_builder<
            ::Vc::int32_v,
            int32_t,
            ::Vc::VectorAlignment, ::Vc::int32_v::Size, int_type::size,
            bool_type,
            false, true, false>
        {};

        template <>
        struct batch_traits<uint_type> : batch_traits_builder<
            ::Vc::uint32_v,
            uint32_t,
            ::Vc::VectorAlignment, ::Vc::uint32_v::Size, uint_type::size,
            bool_type,
            false, true, false>
        {};

        template <>
        struct batch_traits<bool_type> : batch_traits_builder<
            ::Vc::float_m,
            bool,
            ::Vc::VectorAlignment, ::Vc::float_m::Size, bool_type::size,
            bool_type,
            true, false, false>
        {};
    }

    // free functions needed for wrappers to work

    template <typename T>
    inline ::Vc::Vector<T> batch_cast(const ::Vc::float_m& value)
    {
        ::Vc::Vector<T> result(::Vc::Zero);
        result(value) = 1.0f;
        return result;
    }

    template <typename To, typename From>
    inline ::Vc::Vector<To> batch_cast(const ::Vc::Vector<From>& value)
    {
        return ::Vc::Vector<To>(value);
    }

    template <typename T>
    inline ::Vc::Vector<T> batch_scalar_cast(T value)
    {
        return ::Vc::Vector<T>(value);
    }

    inline ::Vc::float_m batch_scalar_cast(bool value)
    {
        return ::Vc::float_m(value);
    }

    inline bool batch_collapse(const ::Vc::float_m& value)
    {
        return value.isNotEmpty();
    }

    template <typename T>
    inline void batch_load_aligned(::Vc::Vector<T>& value, const T* data)
    {
        value.load(data, Vc::Aligned);
    }

    template <typename T>
    inline void batch_store_aligned(const ::Vc::Vector<T>& value, T* data)
    {
        value.store(data, Vc::Aligned);
    }

    template <size_t Index, typename ResultType>
    inline void batch_masked_read_internal(const ::Vc::Vector<ResultType>& mask, ::Vc::Vector<ResultType>& result)
    {}

    template <size_t Index, typename ResultType, typename... Types>
    inline void batch_masked_read_internal(const ::Vc::Vector<ResultType>& mask, ::Vc::Vector<ResultType>& result, const ::Vc::Vector<ResultType>& vec, Types&&... types)
    {
        result.assign(vec, (mask == Index));
        batch_masked_read_internal<Index + 1>(mask, result, std::forward<Types>(types)...);
    }

    template <typename MaskType, typename ResultType, typename... Types>
    inline void batch_masked_read(const ::Vc::Vector<MaskType>& mask, ::Vc::Vector<ResultType>& result, Types&&... types)
    {
        batch_masked_read_internal<0, ResultType>(simd_cast<::Vc::Vector<ResultType>>(mask), result, std::forward<Types>(types)...);
    }

    template <typename T>
    inline void batch_assign(::Vc::Vector<T>& target, const ::Vc::Vector<T>& src)
    {
        target = src;
    }
}

// Vc generally supports it all, but it lacks some crucial functions.
namespace Vc_VERSIONED_NAMESPACE
{
    template <typename T>
    inline Vector<T> step(const Vector<T>& edge, const Vector<T>& x)
    {
        auto result = Vector<T>::One();
        result.setZero(x <= edge);
        return result;
    }

    inline float_v mix(const float_v& x, const float_v& y, const float_m& a)
    {
        float_v result = x;
        result(a) = y;
        return result;
    }

    template <typename T>
    inline Vector<T> pow(const Vector<T>& x, const Vector<T>& n)
    {
        //! Vc doesn't come with pow function, so we're gonna go
        //! with the poor man's version of it.
        Vector<T> result = exp(n * log(x));
        return max(result, Vector<T>::Zero());
    }

    template <typename T>
    inline Vector<T> exp2(const Vector<T>& x)
    {
        //! Vc doesn't come with pow function, so we're gonna go
        //! with the poor man's version of it.
        return exp(2 * log(x));
    }

    template <typename T>
    inline Vector<T> fract(const Vector<T>& x)
    {
        return x - floor(x);
    }

    template <typename T>
    inline Vector<T> inversesqrt(const Vector<T>& x)
    {
        return 1 / sqrt(x);
    }

    template <typename T>
    inline Vector<T> mod(const Vector<T>& x, const Vector<T>& y)
    {
        return x - y * floor(x / y);
    }

    template <typename T>
    inline Vector<T> sign(const Vector<T>& x)
    {
        auto m1 = x > 0;
        auto m2 = x < 0;
        Vector<T> result(::Vc::Zero);
        result(m1) = 1;
        result(m2) = -1;
        return result;
    }

    template <typename T>
    inline Vector<T> atan(const Vector<T>& y, const Vector<T>& x)
    {
        return atan2(y, x);
    }

    template <typename T>
    inline Vector<T> tan(const Vector<T>& x)
    {
        return sin(x) / cos(x);
    }

    //! https://developer.download.nvidia.com/cg/acos.html
    template <typename T>
    inline Vector<T> acos(const Vector<T>& _x)
    {
        Vector<T> negate = Vector<T>::Zero();
        negate(_x < 0) = 1;
        auto x = abs(_x);
        Vector<T> ret = -0.0187293;
        ret = ret * x;
        ret = ret + 0.0742610f;
        ret = ret * x;
        ret = ret - 0.2121144f;
        ret = ret * x;
        ret = ret + 1.5707288f;
        ret = ret * sqrt(1.0f - x);
        ret = ret - 2 * negate * ret;
        return negate * 3.14159265358979f + ret;
    }


    template <typename T>
    inline Vector<T> radians(const Vector<T>& x)
    {
        return  x * (180.0f / 3.14159265358979323846f);
    }

    template <typename T>
    inline Vector<T> degrees(const Vector<T>& x)
    {
        return x * (3.14159265358979323846f / 180.0f);
    }

    template <typename T>
    inline Vector<T> dFdx(const Vector<T>& x)
    {
        // this assumes vectors are row major and there are 2 rows
        auto data = x.data();
        float_v low = Mem::shuffle<X0, X0, Y2, Y2>(data, data);
        float_v high = Mem::shuffle<X1, X1, Y3, Y3>(data, data);
        return high - low;
    }

    template <typename T>
    inline Vector<T> dFdy(const Vector<T>& x)
    {
        // this assumes vectors are row major and there are 2 rows
        auto data = x.data();
#if Vc_IMPL_AVX 
        float_v high = ::Vc::Mem::permute128<X0, X0>(data);
        float_v low = ::Vc::Mem::permute128<X1, X1>(data);
#else
        float_v high = Mem::shuffle<X0, X1, Y0, Y1>(data, data);
        float_v low = Mem::shuffle<X2, X3, Y2, Y3>(data, data);
#endif
        return high - low;
    }
}

#include <swizzle/detail/setup_common.hpp>