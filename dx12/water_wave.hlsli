#ifndef _WATER_WAVE_HLSLH_
#define _WATER_WAVE_HLSLH_

struct PlaneWave
{
    float2 dir;
    float wavelength;
    float speed;
    float amplitude;
    float phase0;
};

float plane_wave(float2 xz, PlaneWave wave, out float2 partial_xz)
{
    float2 dir = normalize(wave.dir);
    float k = 2 * 3.1416f / wave.wavelength;
    float2 k_vector = dir * k;
    float frequency = wave.speed * k;
    float phase = dot(k_vector, xz) + frequency * time + wave.phase0;
    float offset = wave.amplitude * sin(phase);
    partial_xz = wave.amplitude * cos(phase) * k_vector;
    return offset;
}

float get_water_offset(float2 xz, float t, out float2 partial_xz)
{
    const int pwaves_count = 3;
    PlaneWave pwaves[pwaves_count] = { float2(1, 3), 5, 3, 1, 0,
                                       float2(3, 1), 5, 3, 1, 0.3f ,
                                       float2(2, 2), 5, 2, 1, 0.6f };
    float offset = 0;
    float offset_max = 0;
    partial_xz = float2(0, 0);
    for (int i = 0; i < pwaves_count; i++)
    {
        float2 partial;
        offset += plane_wave(xz, pwaves[i], partial);
        offset_max += pwaves[i].amplitude;
        partial_xz += partial;
    }

    partial_xz /= offset_max;
    return offset / offset_max;
}

#endif
