#include <CQSandboxParticleSystem.h>

namespace CQSandbox {

ParticleSystem::
ParticleSystem() :
 CPSysSystem(-1.0, 0.1)
{
}

CPSysParticle *
ParticleSystem::
makeParticle(double mass, double x, double y, double z)
{
  auto *particle = new Particle;

#if 0
  if (particleObj_) {
    particle->setObj(particleObj_);

    particleObj_->setParticle(particle);
  }
#endif

  particle->setMass(mass);

  particle->position()->set(x, y, z);

  addParticle(particle);

  return particle;
}

//---

Particle::
Particle(double mass) :
 CPSysParticle(mass)
{
}

void
Particle::
updateParticle()
{
}

}
