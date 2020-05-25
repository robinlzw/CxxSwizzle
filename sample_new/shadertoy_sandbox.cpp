#include "shadertoy_sandbox.hpp"

using namespace swizzle;

#include <swizzle/define_vector_functions.hpp>

// add custom types that are likely to be passed as out/inout here
struct Ray;
struct ray;

namespace inout
{
    using ray = ray&;
    using Ray = Ray&;

    // proxies are needed for vectors, as they need to work with swizzles
    using vec2 = swizzle::inout_wrapper<vec2>;
    using vec3 = swizzle::inout_wrapper<vec3>;
    using vec4 = swizzle::inout_wrapper<vec4>;
    using ivec2 = swizzle::inout_wrapper<ivec2>;
    using ivec3 = swizzle::inout_wrapper<ivec3>;
    using ivec4 = swizzle::inout_wrapper<ivec4>;
    using uvec2 = swizzle::inout_wrapper<uvec2>;
    using uvec3 = swizzle::inout_wrapper<uvec3>;
    using uvec4 = swizzle::inout_wrapper<uvec4>;
    using bvec2 = swizzle::inout_wrapper<bvec2>;
    using bvec3 = swizzle::inout_wrapper<bvec3>;
    using bvec4 = swizzle::inout_wrapper<bvec4>;

    // regular types don't need proxies
    using float_type = float_type&;
    using int_type = int_type&;
    using uint_type = uint_type&;
    using bool_type = bool_type&;
}

// change meaning of glsl keywords to match sandbox
#define uniform extern
#define in
#define out inout
#define inout inout::
#define float float_type
#define int  int_type
#define uint uint_type
#define bool bool_type
#define lowp
#define highp
// char is not a type in glsl so can be used freely
#define char definitely_not_a_char

// cmath constants need to be undefined, as sometimes shaders define their own
#undef M_E       
#undef M_LOG2E   
#undef M_LOG10E  
#undef M_LN2     
#undef M_LN10    
#undef M_PI      
#undef M_PI_2    
#undef M_PI_4    
#undef M_1_PI    
#undef M_2_PI    
#undef M_2_SQRTPI
#undef M_SQRT2   
#undef M_SQRT1_2 

#ifdef CXXSWIZZLE_OVERRIDE_IF
#define if(x) CXXSWIZZLE_OVERRIDE_IF(x)
#endif
#ifdef CXXSWIZZLE_OVERRIDE_ELSE
#define else CXXSWIZZLE_OVERRIDE_ELSE
#endif
#ifdef CXXSWIZZLE_OVERRIDE_WHILE
#define while(x) CXXSWIZZLE_OVERRIDE_WHILE(x)
#endif
#ifdef CXXSWIZZLE_OVERRIDE_CONDITION
#define condition(x) CXXSWIZZLE_OVERRIDE_CONDITION(x)
#endif

#pragma warning(push)
#pragma warning(disable: 4244) // disable return implicit conversion warning
#pragma warning(disable: 4305) // disable truncation warning

struct _cxxswizzle_fragment_shader : shadertoy::shader_inputs
{
    vec2 gl_FragCoord;
    vec4 gl_FragColor;

    // need a function that's not likely to be defined in the shader itself; operator seems like a good choice
    vec4 operator()(const shader_inputs& uniforms, vec2 fragCoord)
    {
        static_cast<shader_inputs&>(*this) = uniforms;
        gl_FragCoord = fragCoord;

        // vvvvvvvvvvvvvvvvvvvvvvvvvv
        // THE SHADER IS INVOKED HERE
        mainImage(gl_FragColor, gl_FragCoord);

        return clamp(gl_FragColor, vec4(0), vec4(1));
    }

    #include "main_include.hpp"
};

// need to undef any stuff that might possibly have been defined in the shader...
#undef namespace
#undef shadertoy
#undef vec4
#undef shade
#undef const
#undef shader_inputs
#undef uniforms
#undef vec2
#undef fragCoord
#undef return
#undef _cxxswizzle_fragment_shader

namespace shadertoy
{
    vec4 image(const shader_inputs& uniforms, vec2 fragCoord)
    {
        return _cxxswizzle_fragment_shader()(uniforms, fragCoord);
    }
}