#ifndef _SAMPLERS_HLSLH_
#define _SAMPLERS_HLSLH_

SamplerState samp_point_wrap : register(s0);
SamplerState samp_point_clamp : register(s1);
SamplerState samp_linear_wrap : register(s2);
SamplerState samp_linear_clamp : register(s3);
SamplerState samp_anisotropic_wrap : register(s4);
SamplerState samp_anisotropic_clamp : register(s5);

#endif
