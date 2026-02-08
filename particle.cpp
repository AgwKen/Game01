/*==============================================================================

    Particle  [particle.cpp]
                                                         Author : PYAE SONE THANT
                                                         Date   : 2026/02/04
--------------------------------------------------------------------------------

==============================================================================*/
#include "particle.h"
#include "direct3d.h"

void Emitter::Update(double elapsed_time)
{
    m_accumulated_time += elapsed_time;

    const double sec_per_particle = 1.0 / m_particles_per_second;

    // Particle creation loop
    while (m_accumulated_time >= sec_per_particle)
    {
        if (m_count >= m_capacity)
			break; //will stop if too much particles

        if (m_is_emmit)
        {
            m_particles[m_count++] = createParticle();
        }

        m_accumulated_time -= sec_per_particle;
    }

    for (size_t i = 0; i < m_count; i++)
    {
        m_particles[i]->Update(elapsed_time);
    }

    for (int i = static_cast<int>(m_count) - 1; i >= 0; i--)
    {
        if (m_particles[i]->IsDestroy())
        {
            delete m_particles[i];
            m_particles[i] = m_particles[m_count - 1];

            // Reduce active count
            m_count--;
        }
    }
}

void Emitter::Render() const
{
    Direct3D_SetAdditiveBlendState();

    Direct3D_SetDepthReadOnly(true);

    for (size_t i = 0; i < m_count; i++)
    {
        m_particles[i]->Render();
    }

    Direct3D_SetDepthReadOnly(false);
    Direct3D_SetDefaultBlendState();
}
