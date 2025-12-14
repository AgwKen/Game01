/*========================================================================================


  Bullet Cpp[bullet.cpp]										        PYAE SONE THANT
                                                                        DATE:12/11/2025

------------------------------------------------------------------------------------------

=========================================================================================*/
#include "bullet.h"
using namespace DirectX;
#include "model.h"
#include "trajetory3d.h"


class Bullet
{
private:
    XMFLOAT3 m_position{};
    XMFLOAT3 m_velocity{};
    double m_accumulated_time{ 0.0 };
	static constexpr double TIME_LIMIT = 3.0; // seconds

public:
    Bullet(const XMFLOAT3& position, const XMFLOAT3& velocity)
        : m_position(position), m_velocity(velocity)
    {
    }

    void Update(double elapsed_time)
    {
        m_accumulated_time += static_cast<float>(elapsed_time);
        XMStoreFloat3(&m_position, XMLoadFloat3(&m_position) + XMLoadFloat3(&m_velocity) * static_cast<float>(elapsed_time));
        Trajectory3d_Create(m_position, { 0.8f, 0.4f, 0.4f, 1.0f }, 1.0f, 1.0);
    }

    const XMFLOAT3& GetPosition() const {
        return m_position;
    }

    XMFLOAT3 GetFront() const {
		XMFLOAT3 front;
        XMStoreFloat3(&front,XMVector3Normalize(XMLoadFloat3(&m_velocity)));
		return front;
	}

    bool IsDestroy() const {
        return m_accumulated_time > TIME_LIMIT;
    }
};

static constexpr int MAX_BULLETS = 1028;
static Bullet* g_pBullets[MAX_BULLETS]{};
static int g_BulletCount{ 0 };
static MODEL* g_pBulletModel{ nullptr };
void Bullet_Initialize()
{
    g_pBulletModel = ModelLoad("Resources/Model/house01.fbx",0.001f);
}
void Bullet_Finalize()
{
	ModelRelease(g_pBulletModel);

    for (int i = 0; i < g_BulletCount; ++i) {
        delete g_pBullets[i];
	}
	g_BulletCount = 0;
}
void Bullet_Update(double elapsed_time)
{
	for (int i = 0; i < g_BulletCount; ) {
        if (g_pBullets[i]->IsDestroy()) {
            delete g_pBullets[i];
            g_pBullets[i] = g_pBullets[g_BulletCount - 1];
            g_BulletCount--;
        }
        else {
            ++i; //  prevents infinite loop
        }
    }

    for (int i = 0; i < g_BulletCount; ++i) {
        g_pBullets[i]->Update(elapsed_time);
    }
}

void Bullet_Draw()
{
	XMMATRIX mtxWorld{};

    for (int i = 0; i < g_BulletCount; ++i) {   
		XMVECTOR position = XMLoadFloat3(&g_pBullets[i]->GetPosition());
		mtxWorld = XMMatrixTranslationFromVector(position);
        ModelDraw(g_pBulletModel,mtxWorld);
    }
}

void Bullet_Create(const XMFLOAT3& position, const XMFLOAT3& velocity)
{
    if (g_BulletCount >= MAX_BULLETS)
        return;

    g_pBullets[g_BulletCount] = new Bullet(position, velocity);
    ++g_BulletCount;
}

void Bullet_Destroy(int index)
{
	delete g_pBullets[index];
	g_pBullets[index] = g_pBullets[g_BulletCount - 1];
	g_BulletCount--;
}

int Bullet_GetBulletsCount()
{
    return g_BulletCount;
}

AABB Bullet_GetAABB(int index)
{
    XMFLOAT3 pos = Bullet_GetPosition(index);

    // SMALL HITBOX (prevents disappearing from floor collision)
    return {
        { pos.x - 0.1f, pos.y - 0.1f, pos.z - 0.1f },
        { pos.x + 0.1f, pos.y + 0.1f, pos.z + 0.1f }
    };
}

Sphere Bullet_GetSphere(int index)
{
    return { g_pBullets[index]->GetPosition(), g_pBulletModel->local_aabb.GetHalf().x };
}

const DirectX::XMFLOAT3& Bullet_GetPosition(int index)
{
	return g_pBullets[index]->GetPosition();
}
