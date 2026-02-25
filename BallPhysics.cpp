#include "BallPhysics.h"
#include "terrain.h"
#include <algorithm>
#include <cmath>

static constexpr float BALL_RADIUS = 0.3f;
static constexpr float GRAVITY = -25.0f;
static constexpr float BOUNCE_RESTITUTION = 0.35f;
static constexpr float SPIN_LOSS_ON_BOUNCE = 0.7f;
static constexpr float GROUND_FRICTION = 3.5f;
static constexpr float STOP_SPEED = 0.15f;

// ============================================================================
// TERRAIN NORMAL
// ============================================================================
static XMFLOAT3 GetTerrainNormal(float x, float z)
{
    const float eps = 0.5f;

    float hL = Mesh_GetHeightAt(x - eps, z);
    float hR = Mesh_GetHeightAt(x + eps, z);
    float hD = Mesh_GetHeightAt(x, z - eps);
    float hU = Mesh_GetHeightAt(x, z + eps);

    XMFLOAT3 n = { hL - hR, 2.0f, hD - hU };
    XMStoreFloat3(&n, XMVector3Normalize(XMLoadFloat3(&n)));
    return n;
}

void BallPhysics_Initialize(BallState& state, const DirectX::XMFLOAT3& startPos)
{
    state.position = startPos;
    state.prevPosition = startPos;
    state.velocity = { 0,0,0 };
    state.angularVelocity = DirectX::XMVectorZero();
    state.onGround = false;
}

void BallPhysics_Update(BallState& state, float dt)
{
    using namespace DirectX;

    // GRAVITY
    state.velocity.y += GRAVITY * dt;

    // MOVE
    state.prevPosition = state.position;

    state.position.x += state.velocity.x * dt;
    state.position.y += state.velocity.y * dt;
    state.position.z += state.velocity.z * dt;

    state.onGround = false;

    // TERRAIN COLLISION
    float ground = Mesh_GetHeightAt(state.position.x, state.position.z);

    if (state.position.y - BALL_RADIUS < ground)
    {
        state.position.y = ground + BALL_RADIUS;

        XMFLOAT3 n = GetTerrainNormal(state.position.x, state.position.z);
        XMVECTOR normal = XMLoadFloat3(&n);
        XMVECTOR vel = XMLoadFloat3(&state.velocity);

        float vn = XMVectorGetX(XMVector3Dot(vel, normal));
        if (vn < 0.0f)
        {
            vel -= normal * (1.0f + BOUNCE_RESTITUTION) * vn;
            state.angularVelocity *= SPIN_LOSS_ON_BOUNCE;
        }

        XMStoreFloat3(&state.velocity, vel);

        if (n.y > 0.4f)
            state.onGround = true;
    }

    // GROUND FRICTION
    if (state.onGround)
    {
        XMVECTOR velXZ = XMVectorSet(state.velocity.x, 0, state.velocity.z, 0);
        float speed = XMVectorGetX(XMVector3Length(velXZ));

        if (speed > 0.0f)
        {
            speed = std::max(0.0f, speed - GROUND_FRICTION * dt);

            if (speed > 0.01f)
            {
                velXZ = XMVector3Normalize(velXZ) * speed;
                state.velocity.x = XMVectorGetX(velXZ);
                state.velocity.z = XMVectorGetZ(velXZ);
            }
            else
            {
                state.velocity.x = 0;
                state.velocity.z = 0;
            }
        }
    }

    // MAGNUS EFFECT
    XMVECTOR velXZ = XMVectorSet(state.velocity.x, 0, state.velocity.z, 0);

    if (XMVectorGetX(XMVector3Length(state.angularVelocity)) > 0.01f)
    {
        XMVECTOR mag = XMVector3Cross(state.angularVelocity, velXZ) * 0.015f;

        state.velocity.x += XMVectorGetX(mag);
        state.velocity.z += XMVectorGetZ(mag);

        state.angularVelocity *= 0.99f;
    }
}