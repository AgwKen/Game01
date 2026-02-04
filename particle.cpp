/*==============================================================================

	Particle  [particle.h]
														 Author : PYAE SONE THANT
														 Date   : 2026/02/04
--------------------------------------------------------------------------------

==============================================================================*/
#include "particle.h"

void Emitter::Update(double elapsed_time)
{
	m_accumulated_time += elapsed_time;

	const double sec_per_particle = 1.0 / m_particles_per_second;

	while (m_accumulated_time >= sec_per_particle) {
		if (m_count >= m_capacity)break; //もう作れない !! いっぱい!
		if (m_is_emmit) {
			m_particles[m_count++] = createParticle();
		}
		m_accumulated_time -= sec_per_particle;
	}
	// 更新処理
	for (int i = 0; i < m_count; i++) {
		m_particles[i]->Update(elapsed_time);
	}

	for (int i = m_count - 1; i > 0; i--) {
		if (m_particles[i]->IsDestroy()) {
			delete m_particles[i];
			m_particles[i] = m_particles[m_count - 1];
		}
	}
}

void Emitter::Render() const
{
	for (int i = 0; i < m_count; i++) {
		m_particles[i]->Render();
	}
}

