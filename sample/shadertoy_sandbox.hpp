#include "config.hpp"

#include <swizzle/vector.hpp>
#include <swizzle/matrix.hpp>
#include <swizzle/sampler_generic.hpp>
#include <swizzle/detail/simd_mask.h>
#include <swizzle/inout_wrapper.hpp>
#include <swizzle/detail/batch_write_mask.hpp>
#include <type_traits>

namespace shadertoy
{
    const int num_samplers = 4;
    const int num_buffers = 4;

    struct shader_inputs
    {
        swizzle::vec3 iResolution;               // viewport resolution (in pixels)
        swizzle::float_type iTime;               // shader playback time (in seconds)
        swizzle::float_type iTimeDelta;          // render time (in seconds)
        swizzle::int_type iFrame;                // shader playback frame
        swizzle::float_type iChannelTime[4];     // channel playback time (in seconds) NOTE: not supported yet
        swizzle::vec3 iChannelResolution[4];     // channel resolution (in pixels)
        swizzle::vec4 iMouse;                    // mouse pixel coords. xy: current (if MLB down), zw: click
        swizzle::sampler2D iChannel0;            // input channel. XX = 2D/Cube
        swizzle::sampler2D iChannel1;
        swizzle::sampler2D iChannel2;
        swizzle::sampler2D iChannel3;
        swizzle::vec4 iDate;                     // (year, month, day, time in seconds)
        swizzle::float_type iSampleRate;         // sound sample rate (i.e., 44100)
        swizzle::int_type iFrameRate;            // NOTE: undocummented
    };

    swizzle::vec4 image(const shader_inputs& uniforms, swizzle::vec2 fragCoord, swizzle::vec4 prevFragColor, swizzle::bool_type* discarded);
    swizzle::vec4 buffer_a(const shader_inputs& input, swizzle::vec2 fragCoord, swizzle::vec4 prevFragColor, swizzle::bool_type* discarded);
    swizzle::vec4 buffer_b(const shader_inputs& input, swizzle::vec2 fragCoord, swizzle::vec4 prevFragColor, swizzle::bool_type* discarded);
    swizzle::vec4 buffer_c(const shader_inputs& input, swizzle::vec2 fragCoord, swizzle::vec4 prevFragColor, swizzle::bool_type* discarded);
    swizzle::vec4 buffer_d(const shader_inputs& input, swizzle::vec2 fragCoord, swizzle::vec4 prevFragColor, swizzle::bool_type* discarded);
}