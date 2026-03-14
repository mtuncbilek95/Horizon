#version 450

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D u_SceneTexture;

layout(push_constant) uniform PC 
{
    float resX;
    float resY;
} pc;

void main() 
{
    vec2 invRes = 1.0 / vec2(pc.resX, pc.resY);
    
    const float FXAA_SPAN_MAX = 8.0;
    const float FXAA_REDUCE_MUL = 1.0 / 32.0;
    const float FXAA_REDUCE_MIN = 1.0 / 128.0;
    const vec3 luma = vec3(0.299, 0.587, 0.114);

    vec3 rgbM  = texture(u_SceneTexture, inUV).rgb;
    vec3 rgbNW = textureOffset(u_SceneTexture, inUV, ivec2(-1, -1)).rgb;
    vec3 rgbNE = textureOffset(u_SceneTexture, inUV, ivec2(1, -1)).rgb;
    vec3 rgbSW = textureOffset(u_SceneTexture, inUV, ivec2(-1, 1)).rgb;
    vec3 rgbSE = textureOffset(u_SceneTexture, inUV, ivec2(1, 1)).rgb;

    float lM  = dot(rgbM,  luma);
    float lNW = dot(rgbNW, luma);
    float lNE = dot(rgbNE, luma);
    float lSW = dot(rgbSW, luma);
    float lSE = dot(rgbSE, luma);

    float lMin = min(lM, min(min(lNW, lNE), min(lSW, lSE)));
    float lMax = max(lM, max(max(lNW, lNE), max(lSW, lSE)));

    if ((lMax - lMin) < max(0.0312, lMax * 0.125)) 
	{
        outColor = vec4(rgbM, 1.0);
        return;
    }

    vec2 dir;
    dir.x = -((lNW + lNE) - (lSW + lSE));
    dir.y =  ((lNW + lSW) - (lNE + lSE));

    float dirReduce = max((lNW + lNE + lSW + lSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    
    dir = min(vec2(FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX), dir * rcpDirMin)) * invRes;

    vec3 rgbA = 0.5 * (
        texture(u_SceneTexture, inUV + dir * (1.0/3.0 - 0.5)).rgb +
        texture(u_SceneTexture, inUV + dir * (2.0/3.0 - 0.5)).rgb);
        
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(u_SceneTexture, inUV + dir * -0.5).rgb +
        texture(u_SceneTexture, inUV + dir * 0.5).rgb);

    float lB = dot(rgbB, luma);

    if ((lB < lMin) || (lB > lMax)) 
	{
        outColor = vec4(rgbA, 1.0);
    } else 
	{
        outColor = vec4(rgbB, 1.0);
    }
}