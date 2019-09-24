// CxxSwizzle
// Copyright (c) 2013-2015, Piotr Gwiazdowski <gwiazdorrr+github at gmail.com>
#pragma once

// VC needs to come first or else it's going to complain (damn I hate these)
#include <Vc/vector.h>
#include <Vc/global.h>
#include <type_traits>
#include <swizzle/detail/primitive_adapters.h>
#include <swizzle/glsl/vector_helper.h>
#include <swizzle/detail/fwd.h>
#include <swizzle/detail/simd_mask.h>

namespace swizzle
{
    namespace detail
    {
        template <typename AssignPolicy = default_assign_policy>
        using vc_bool = bool_adapter<::Vc::float_m, AssignPolicy>;

        template <typename AssignPolicy = default_assign_policy>
        using vc_int = int_adapter<::Vc::int32_v, vc_bool<AssignPolicy>, AssignPolicy>;

        template <typename AssignPolicy = default_assign_policy>
        using vc_uint = uint_adapter<::Vc::uint32_v, vc_bool<AssignPolicy>, AssignPolicy>;

        template <typename AssignPolicy = default_assign_policy>
        using vc_float = float_adapter<::Vc::float_v, vc_bool<AssignPolicy>, AssignPolicy>;
        
        // conversions for adapters to work

        inline ::Vc::float_v simd_bool_to_simd_float(::Vc::float_m value)
        {
            Vc::float_v result(Vc::Zero);
            result(value) = 1.0f;
            return result;
        }

        inline ::Vc::int32_m bool_to_simd(bool value)
        {
            return ::Vc::int32_m(value);
        }

        inline ::Vc::int32_v int_to_simd(int value)
        {
            return value;
        }

        inline ::Vc::uint32_v uint_to_simd(unsigned value)
        {
            return value;
        }

        inline ::Vc::float_v float_to_simd(float value)
        {
            return value;
        }

        inline bool simd_to_bool(::Vc::int32_m value)
        {
            return value.isNotEmpty();
        }

        inline bool simd_to_bool(::Vc::float_m value)
        {
            return value.isNotEmpty();
        }

        // meta for cxx swizzle magic

        template <typename AssignPolicy>
        struct get_vector_type_impl< vc_float<AssignPolicy> >
        {
            typedef ::swizzle::vector<vc_float<AssignPolicy>, 1> type;
        };

        template <typename AssignPolicy>
        struct get_vector_type_impl< vc_int<AssignPolicy> >
        {
            typedef ::swizzle::vector<vc_int<AssignPolicy>, 1> type;
        };

        template <typename AssignPolicy>
        struct get_vector_type_impl< vc_uint<AssignPolicy> >
        {
            typedef ::swizzle::vector<vc_uint<AssignPolicy>, 1> type;
        };

        template <typename AssignPolicy>
        struct get_vector_type_impl< vc_bool<AssignPolicy> >
        {
            typedef ::swizzle::vector<vc_bool<AssignPolicy>, 1> type;
        };

        template <size_t Size, typename AssignPolicy>
        struct vector_build_info<vc_float<AssignPolicy>, Size> : vector_build_info_base<vc_float<AssignPolicy>, Size, std::array<::Vc::float_v, Size>, vc_bool<AssignPolicy>>
        {};

        template <size_t Size, typename AssignPolicy>
        struct vector_build_info<vc_int<AssignPolicy>, Size> : vector_build_info_base<vc_int<AssignPolicy>, Size, std::array<::Vc::int32_v, Size>, vc_bool<AssignPolicy>>
        {};

        template <size_t Size, typename AssignPolicy>
        struct vector_build_info<vc_uint<AssignPolicy>, Size> : vector_build_info_base<vc_uint<AssignPolicy>, Size, std::array<::Vc::uint32_v, Size>, vc_bool<AssignPolicy>>
        {};

        template <size_t Size, typename AssignPolicy>
        struct vector_build_info<vc_bool<AssignPolicy>, Size> : vector_build_info_base<vc_bool<AssignPolicy>, Size, std::array<::Vc::float_m, Size>, vc_bool<AssignPolicy>>
        {};

        template <typename AssignPolicy>
        constexpr bool is_scalar_floating_point_v<vc_float<AssignPolicy>> = true;

        template <typename AssignPolicy>
        constexpr bool is_scalar_integral_v<vc_int<AssignPolicy>> = true;

        template <typename AssignPolicy>
        constexpr bool is_scalar_integral_v<vc_uint<AssignPolicy>> = true;

        template <typename AssignPolicy>
        constexpr bool is_scalar_bool_v<vc_bool<AssignPolicy>> = true;
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

    template <typename T>
    inline Vector<T> pow(const Vector<T>& x, const Vector<T>& n)
    {
        //! Vc doesn't come with pow function, so we're gonna go
        //! with the poor man's version of it.
        return exp(n * log(x));
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
        return rsqrt(x);
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

#if Vc_IMPL_AVX 
    template <typename T>
    inline Vector<T> dFdx(const Vector<T>& x)
    {
        // this assumes vectors are row major and there are 2 rows
        auto data = x.data();
        float_v low =  Mem::shuffle<X0, X0, X2, X2, Y4, Y4, Y6, Y6>(data, data);
        float_v high = Mem::shuffle<X1, X1, X3, X3, Y5, Y5, Y7, Y7>(data, data);
        return high - low;
    }

    template <typename T>
    inline Vector<T> dFdy(const Vector<T>& x)
    {
        // this assumes vectors are row major and there are 2 rows
        auto data = x.data();
        float_v low =  Mem::shuffle<X0, X1, X2, X3, Y0, Y1, Y2, Y3>(data, data);
        float_v high = Mem::shuffle<X4, X5, X6, X7, Y4, Y5, Y6, Y7>(data, data);
        return high - low;
    }
#elif Vc_IMPL_SSE
    template <typename T>
    inline Vector<T> dFdx(const Vector<T>& x)
    {
        // this assumes vectors are row major and there are 2 rows
        auto data = x.data();
        float_v low =  Mem::shuffle<X0, X0, Y2, Y2>(data, data);
        float_v high = Mem::shuffle<X1, X1, Y3, Y3>(data, data);
        return high - low;
    }

    template <typename T>
    inline Vector<T> dFdy(const Vector<T>& x)
    {
        // this assumes vectors are row major and there are 2 rows
        auto data = x.data();
        float_v low =  Mem::shuffle<X0, X1, Y0, Y1>(data, data);
        float_v high = Mem::shuffle<X2, X3, Y2, Y3>(data, data);
        return high - low;
    }
#endif
}
