#version 450

layout(location = 0) in  vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PC {
    float time;
    float resX;
    float resY;
    float frame;
    vec4  camRight;
    vec4  camUp;
    vec4  camForward;
} pc;

layout(set = 0, binding = 0) uniform sampler2D cloudTex;

layout(set = 1, binding = 0) uniform CloudConfig {
    float cloudBase;
    float cloudTop;
    float fbmFrequency;
    float warpStrength;
    float densityMultiplier;
    float densityThreshold;
    float absorption;
    float scatteringG;
    vec4  cloudLitColor;
    vec4  cloudShadowColor;
    vec4  sunColorCloud;
    vec4  ambientColor;
    vec4  zenithColor;
    vec4  upperMid;
    vec4  lowerMid;
    vec4  horizonColor;
    vec4  groundColor;
    vec4  sunColorSky;
    float sunAngularRadius;
    float glowIntensity;
    float horizonBloom;
    float _pad;
} cfg;

vec3 skyGradient(float altitude)
{
    if (altitude < 0.0)
        return mix(cfg.horizonColor.rgb, cfg.groundColor.rgb, clamp(-altitude * 6.0, 0.0, 1.0));

    float h    = clamp(altitude, 0.0, 1.0);
    vec3  low  = mix(cfg.horizonColor.rgb, cfg.lowerMid.rgb,  smoothstep(0.0, 0.18, h));
    vec3  high = mix(cfg.lowerMid.rgb,     cfg.upperMid.rgb,  smoothstep(0.1, 0.45, h));
    vec3  top  = mix(cfg.upperMid.rgb,     cfg.zenithColor.rgb, smoothstep(0.3, 1.0,  h));
    return mix(mix(low, high, smoothstep(0.0, 0.3, h)), top, smoothstep(0.2, 0.8, h));
}

void main()
{
    float aspect = pc.resX / pc.resY;
    vec2  uv     = (inUV * 2.0 - 1.0) * vec2(aspect, -1.0);

    vec3 ray = normalize(uv.x * pc.camRight.xyz
                       + uv.y * pc.camUp.xyz
                       + 1.8 * pc.camForward.xyz);

    float a      = pc.time * 0.04;
    vec3  sunDir = normalize(vec3(sin(a), 0.4 + sin(a * 0.4) * 0.2, -cos(a)));

    vec3 color = skyGradient(ray.y);

    float glow = exp(-(1.0 - dot(ray, sunDir)) * 10.0);
    color += vec3(1.0, 0.70, 0.40) * glow * clamp(sunDir.y + 0.4, 0.0, 1.0) * cfg.glowIntensity;

    float horizonBloomVal = pow(1.0 - abs(ray.y), 5.0);
    color += cfg.horizonColor.rgb * horizonBloomVal * cfg.horizonBloom;

    vec4 clouds = texture(cloudTex, inUV);
    color = color * (1.0 - clouds.a) + clouds.rgb;

    float cosA = dot(ray, sunDir);
    float sunM = smoothstep(cos(cfg.sunAngularRadius) - 0.0005,
                            cos(cfg.sunAngularRadius) + 0.0005, cosA);
    float diskV = clamp(sunDir.y + 0.1, 0.0, 1.0) * (1.0 - clouds.a * 0.85);
    color = mix(color, cfg.sunColorSky.rgb * 2.2, sunM * diskV);

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, 1.0);
}
