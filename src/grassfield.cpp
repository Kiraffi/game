#include "grassfield.h"

#include <memory>
#include <vector>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <external/glad.h> // raylib

static const int32_t MAX_GRASS_AMOUNT = 64 * 1024 * 2;
static const int32_t GRASS_VERTICES_PER_INSTANCE = 16;

struct Grass
{
    Vector3 pos;
    uint32_t col;

    float width;
    float height;
    float padding2;
    float padding3;
};

struct GrassVertex
{
    Vector3 pos;
    uint32_t col;
    Vector3 norm;
    uint32_t padding;
};

class GrassFieldInstance : public GrassField
{
public:
    virtual ~GrassFieldInstance()
    {
        rlUnloadShaderBuffer(m_ssboGrassAtomic);
        rlUnloadShaderBuffer(m_ssboGrassData);
        rlUnloadShaderBuffer(m_ssboGrassVertices);
        rlUnloadShaderBuffer(m_uniformBuffer);

        // For some reason this is the program not shader?
        rlUnloadShaderProgram(m_computeResetAtomicProgram);
        rlUnloadShaderProgram(m_computeGrassProgram);

        UnloadShader(m_render);
    }

    std::vector<Grass> m_grass;
    Shader m_render = {};
    uint32_t m_ssboGrassAtomic = {};
    uint32_t m_ssboGrassData = {};
    uint32_t m_ssboGrassVertices = {};
    uint32_t m_uniformBuffer = {};
    uint32_t m_computeResetAtomicProgram = {};
    uint32_t m_computeGrassProgram = {};

    uint32_t m_indexBuffer = {};
    uint32_t m_vao = {};

    Camera m_camera = {
        .position = {0, 0, 10},
        .up = {0, 1, 0},
        .fovy = DEG2RAD * 90.0f,
        .projection = CAMERA_PERSPECTIVE };

};


static std::unique_ptr<GrassFieldInstance> s_grassFieldInstance;

GrassField* GrassField::getInstance()
{
    if(s_grassFieldInstance == nullptr)
    {
        s_grassFieldInstance.reset(new GrassFieldInstance());
    }
    return s_grassFieldInstance.get();
}

void GrassField::init()
{
    s_grassFieldInstance->m_camera.position = {0, 0, 10};
    {
        char *atomicShaderCode = LoadFileText("assets/shaders/atomic_reset.comp");
        uint32_t atomicShader = rlCompileShader(atomicShaderCode, RL_COMPUTE_SHADER);
        s_grassFieldInstance->m_computeResetAtomicProgram = rlLoadComputeShaderProgram(atomicShader);
        UnloadFileText(atomicShaderCode);

        char *computeGrassShaderCode = LoadFileText("assets/shaders/grass_generate.comp");
        uint32_t computeGrassShader = rlCompileShader(computeGrassShaderCode, RL_COMPUTE_SHADER);
        s_grassFieldInstance->m_computeGrassProgram = rlLoadComputeShaderProgram(computeGrassShader);
        UnloadFileText(computeGrassShaderCode);

        s_grassFieldInstance->m_render = LoadShader("assets/shaders/grass_render.vert", "assets/shaders/grass_render.frag");
    }
    {
        std::vector<int32_t> indices;
        indices.reserve(MAX_GRASS_AMOUNT * GRASS_VERTICES_PER_INSTANCE * 6 / 4);
        for(int i = 0; i < MAX_GRASS_AMOUNT * GRASS_VERTICES_PER_INSTANCE / 4; ++i)
        {
            indices.emplace_back(i * 4 + 2);
            indices.emplace_back(i * 4 + 1);
            indices.emplace_back(i * 4 + 0);

            indices.emplace_back(i * 4 + 2);
            indices.emplace_back(i * 4 + 3);
            indices.emplace_back(i * 4 + 1);
        }
        s_grassFieldInstance->m_indexBuffer = rlLoadVertexBufferElement(
            indices.data(),
            sizeof(int32_t) * indices.size(),
            false);
    }

    {
        for(int i = 0; i < MAX_GRASS_AMOUNT; ++i)
        {
            s_grassFieldInstance->m_grass.emplace_back(Grass{
                .pos{.x = float(i % 256), .z = -float(i / 256)},
                .col = ~0u
            });
        }
    s_grassFieldInstance->m_ssboGrassData = rlLoadShaderBuffer(
        sizeof(Grass) * MAX_GRASS_AMOUNT,
        s_grassFieldInstance->m_grass.data(),
        RL_DYNAMIC_COPY);
    }

    s_grassFieldInstance->m_ssboGrassVertices = rlLoadShaderBuffer(
        sizeof(GrassVertex) * MAX_GRASS_AMOUNT * GRASS_VERTICES_PER_INSTANCE,
        NULL,
        RL_DYNAMIC_COPY);

    s_grassFieldInstance->m_ssboGrassAtomic = rlLoadShaderBuffer(
        sizeof(uint32_t) * 256,
        NULL,
        RL_DYNAMIC_COPY);

    {
        glGenBuffers(1, &s_grassFieldInstance->m_uniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, s_grassFieldInstance->m_uniformBuffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(uint32_t) * 256, nullptr, RL_STREAM_COPY);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

    }
    {
        glGenVertexArrays(1, &s_grassFieldInstance->m_vao);
        glBindVertexArray(s_grassFieldInstance->m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_grassFieldInstance->m_indexBuffer);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }
}

void GrassField::update(double dt)
{
    /*
    // Send SSBO buffer to GPU
    rlUpdateShaderBuffer(s_grassFieldInstance->m_ssboGrassData,
        s_grassFieldInstance->m_grass.data(),
        sizeof(Grass) * s_grassFieldInstance->m_grass.size(),
        0);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
*/
    float screenWidth = GetScreenWidth();
    float screenHeight = GetScreenHeight();

    struct UniformBuf
    {
        Matrix m_vp;
        Matrix m_view;
        Matrix m_proj;
        Matrix m_padding;
    };

    UniformBuf uni = {};

    static const float SPEED = 10.0f;

    Vector3 movement = {};

    if(IsKeyDown(KEY_W)) movement -= Vector3{0, 0, 1} * dt * SPEED;
    if(IsKeyDown(KEY_S)) movement += Vector3{0, 0, 1} * dt * SPEED;
    if(IsKeyDown(KEY_A)) movement -= Vector3{1, 0, 0} * dt * SPEED;
    if(IsKeyDown(KEY_D)) movement += Vector3{1, 0, 0} * dt * SPEED;
    if(IsKeyDown(KEY_Q)) movement += Vector3{0, 1, 0} * dt * SPEED;
    if(IsKeyDown(KEY_E)) movement -= Vector3{0, 1, 0} * dt * SPEED;

    s_grassFieldInstance->m_camera.position += movement;
    s_grassFieldInstance->m_camera.target += movement;

    uni.m_proj = MatrixPerspective(DEG2RAD * 90.0f,
        screenWidth / screenHeight, 0.1f, 10000.0f);
    uni.m_view = GetCameraMatrix(s_grassFieldInstance->m_camera);

    //uni.m_vp = uni.m_proj * uni.m_view;
    uni.m_vp = uni.m_view * uni.m_proj;
    //uni.m_vp = uni.m_proj * uni.m_view;


    glBindBuffer(GL_UNIFORM_BUFFER, s_grassFieldInstance->m_uniformBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UniformBuf), &uni);
}

void GrassField::computeGrassField()
{
    {
        rlEnableShader(s_grassFieldInstance->m_computeResetAtomicProgram);
        rlBindShaderBuffer(s_grassFieldInstance->m_ssboGrassAtomic, 1);
        rlComputeShaderDispatch(1, 1, 1);
        rlDisableShader();
    }
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    {
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, s_grassFieldInstance->m_uniformBuffer);

        rlEnableShader(s_grassFieldInstance->m_computeGrassProgram);
        rlBindShaderBuffer(s_grassFieldInstance->m_ssboGrassData, 1);
        rlBindShaderBuffer(s_grassFieldInstance->m_ssboGrassVertices, 2);
        rlBindShaderBuffer(s_grassFieldInstance->m_ssboGrassAtomic, 3);
        rlComputeShaderDispatch(s_grassFieldInstance->m_grass.size(), 1, 1);
        rlDisableShader();
    }
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

// Probably only works if before any raylib stuff....
void GrassField::drawGrassField()
{
    glUseProgram(s_grassFieldInstance->m_render.id);
    glBindVertexArray(s_grassFieldInstance->m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_grassFieldInstance->m_indexBuffer);
    rlBindShaderBuffer(s_grassFieldInstance->m_ssboGrassVertices, 1);

    //glDrawElements(GL_TRIANGLES, s_grassFieldInstance->m_grass.size() * 6 * GRASS_VERTICES_PER_INSTANCE / 4, GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, s_grassFieldInstance->m_ssboGrassAtomic);
    glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}



