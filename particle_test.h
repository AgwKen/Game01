    #pragma once
    #ifndef PARTICLE_TEST_H
    #define PARTICLE_TEST_H

    #include "particle.h"
    #include "texture.h"
    #include <random>

    class NormalParticle : public Particle
    {
    private:
        int m_texId{ -1 };

    public:
        NormalParticle(
            int texture_id,
            const DirectX::XMVECTOR& position,
            const DirectX::XMVECTOR& velocity,
            double life_time,
            const DirectX::XMFLOAT4& color
        )
            : m_texId(texture_id),
            Particle(position, velocity, life_time)
        {
            SetColor(color);
        }

        void Update(double elapsed_time) override;
        void Render() const override;
    };

    class NormalEmitter : public Emitter
    {
    private:
        int m_texture_id{ -1 };

        std::mt19937 m_mt;

        std::uniform_real_distribution<float> m_dir_dist{ -DirectX::XM_PI, DirectX::XM_PI };
        std::uniform_real_distribution<float> m_speed_dist{ 2.0f, 4.0f };
        std::uniform_real_distribution<float> m_lifetime_dist{ 0.6f, 1.0f };


        // NEW: color randomizer
        std::uniform_real_distribution<float> m_color_dist{ 0.0f, 1.0f };

    public:
        NormalEmitter(
            size_t capacity,
            const DirectX::XMVECTOR& position,
            double particles_per_second,
            bool is_emmit = false
        )
            : Emitter(capacity, position, particles_per_second, is_emmit)
            , m_texture_id(Texture_Load(L"Texture/effect000.jpg"))
        {
            std::random_device rd;
            m_mt.seed(rd());
        }

    protected:
        Particle* createParticle() override;
    };

    #endif
