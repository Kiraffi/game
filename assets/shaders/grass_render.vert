
#version 450 core

//layout (std140, binding=0) uniform frame_data
layout (std140, binding=0, row_major) uniform frame_data
{
    mat4 vpMat;
    mat4 viewMat;
    mat4 projMat;

    vec2 screenSizes;
    vec2 shadowTexSize;
    vec4 camRight;
    vec4 camUp;
    vec4 camForward;

    mat4 sunCameraMat;
    mat4 sunViewProjMat;
    mat4 sunVpMat;
    mat4 cameraViewToSunView;

    vec4 sunPos;
    vec4 sunDir;
    vec4 sunTarget;
    vec4 padding3;

    mat4 padding4;
    mat4 padding5;
    mat4 padding6;
};

struct GrassVertex
{
    vec3 pos;
    uint col;
    vec3 norm;
    uint padding;
};

layout (location = 0) out vec4 vColor;

layout (std430, binding=1) restrict readonly buffer shader_data
{
    GrassVertex values[];
} vData;


void main()
{
    uint vertId = gl_VertexID;
    GrassVertex v = vData.values[vertId];
    vec4 outPos = vpMat * vec4(v.pos, 1.0);
    uint col = v.col;
    vColor.r = ((col >> 24) & 255) / 255.0f;
    vColor.g = ((col >> 16) & 255) / 255.0f;
    vColor.b = ((col >> 8) & 255) / 255.0f;
    vColor.a = ((col >> 0) & 255) / 255.0f;

    gl_Position = outPos;
}
