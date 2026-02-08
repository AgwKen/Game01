#ifndef PARTICLE_H
#define PARTICLE_H

#include <DirectXMath.h>

class Particle
{
private:
    DirectX::XMVECTOR m_position{};
    DirectX::XMVECTOR m_velocity{};

    double m_life_time{};
    double m_accumulate_time{};

    DirectX::XMFLOAT4 m_color{ 1,1,1,1 };

public:
    Particle(const DirectX::XMVECTOR& position,
        const DirectX::XMVECTOR& velocity,
        double life_time)
        : m_position(position)
        , m_velocity(velocity)
        , m_life_time(life_time)
    {
    }

    virtual ~Particle() = default;

    virtual bool IsDestroy() const {
        return m_life_time <= 0.0;
    }

    virtual void Update(double elapsed_time) {
        m_accumulate_time += elapsed_time;

        if (m_accumulate_time > m_life_time) {
            Destroy();
        }
    }

    virtual void Render() const = 0;

protected:
    virtual void Destroy() {
        m_life_time = 0.0;
    }

    void SetPosition(const DirectX::XMVECTOR& position) {
        m_position = position;
    }

    void SetVelocity(const DirectX::XMVECTOR& velocity) {
        m_velocity = velocity;
    }

    const DirectX::XMVECTOR& GetPosition() const {
        return m_position;
    }

    const DirectX::XMVECTOR& GetVelocity() const {
        return m_velocity;
    }

    void AddPosition(const DirectX::XMVECTOR& v) {
        m_position = DirectX::XMVectorAdd(m_position, v);
    }

    void AddVelocity(const DirectX::XMVECTOR& v) {
        m_velocity = DirectX::XMVectorAdd(m_velocity, v);
    }


    // ===== COLOR SUPPORT =====

    void SetColor(const DirectX::XMFLOAT4& color) {
        m_color = color;
    }

    const DirectX::XMFLOAT4& GetColor() const {
        return m_color;
    }

    double GetLifeTime() const {
        return m_life_time;
    }

    double GetAccumulatedTime() const {
        return m_accumulate_time;
    }
};

class Emitter
{
private:
    DirectX::XMVECTOR m_position{};

    double m_particles_per_second{};
    double m_accumulated_time = 0.0;

    bool m_is_emmit{};

    size_t m_capacity{};
    size_t m_count{};

    Particle** m_particles{};

protected:
    virtual Particle* createParticle() = 0;

    const DirectX::XMVECTOR& GetPosition() const {
        return m_position;
    }

    double GetParticlesPerSecond() const {
        return m_particles_per_second;
    }

public:
    Emitter(size_t capacity,
        const DirectX::XMVECTOR& position,
        double particles_per_second,
        bool is_emmit = false)

        : m_capacity(capacity)
        , m_position(position)
        , m_particles_per_second(particles_per_second)
        , m_is_emmit(is_emmit)
        , m_count(0)
    {
        m_particles = new Particle * [m_capacity];

        for (size_t i = 0; i < m_capacity; i++) {
            m_particles[i] = nullptr;
        }
    }

    virtual ~Emitter() {
        for (size_t i = 0; i < m_count; i++) {
            delete m_particles[i];
        }

        delete[] m_particles;
    }

    void SetPosition(const DirectX::XMVECTOR& pos)
    {
        m_position = pos;
    }

    virtual void Update(double elapsed_time);
    virtual void Render() const;

    void Emmit(bool isEmmit) { m_is_emmit = isEmmit; }
    bool IsEmmit() const { return m_is_emmit; }
};

#endif
