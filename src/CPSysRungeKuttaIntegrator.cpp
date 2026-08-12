#include <CPSysRungeKuttaIntegrator.h>
#include <CPSysSystem.h>

CPSysRungeKuttaIntegrator::
CPSysRungeKuttaIntegrator(CPSysSystem *s) :
 s_(s)
{
}

void
CPSysRungeKuttaIntegrator::
allocateParticles()
{
  uint numParticles = s_->numberOfParticles();

  while (numParticles > originalPositions_.size()) {
    originalPositions_ .add(new CPSysVector3D());
    originalVelocities_.add(new CPSysVector3D());
    k1Forces_          .add(new CPSysVector3D());
    k1Velocities_      .add(new CPSysVector3D());
    k2Forces_          .add(new CPSysVector3D());
    k2Velocities_      .add(new CPSysVector3D());
    k3Forces_          .add(new CPSysVector3D());
    k3Velocities_      .add(new CPSysVector3D());
    k4Forces_          .add(new CPSysVector3D());
    k4Velocities_      .add(new CPSysVector3D());
  }
}

void
CPSysRungeKuttaIntegrator::
step(double deltaT)
{
  allocateParticles();

  /////////////////////////////////////////////////////////
  // save original position and velocities

  uint numParticles = s_->numberOfParticles();

  for (uint i = 0; i < numParticles; ++i) {
    auto p = s_->getParticle(i);

    if (p->isFree()) {
      originalPositions_ .get(int(i))->set(p->position());
      originalVelocities_.get(int(i))->set(p->velocity());
    }

    p->force()->clear();  // and clear the forces
  }

  ////////////////////////////////////////////////////////
  // get all the k1 values

  s_->applyForces();

  // save the intermediate forces
  for (uint i = 0; i < numParticles; ++i) {
    auto *p = s_->getParticle(i);

    if (p->isFree()) {
      k1Forces_    .get(int(i))->set(p->force   ());
      k1Velocities_.get(int(i))->set(p->velocity());
    }

    p->force()->clear();
  }

  ////////////////////////////////////////////////////////////////
  // get k2 values

  for (uint i = 0; i < numParticles; ++i) {
    auto *p = s_->getParticle(i);

    if (p->isFree()) {
      auto *originalPosition = originalPositions_.get(int(i));
      auto *k1Velocity       = k1Velocities_     .get(int(i));

      p->setPosition(originalPosition->x() + k1Velocity->x()*0.5*deltaT,
                     originalPosition->y() + k1Velocity->y()*0.5*deltaT,
                     originalPosition->z() + k1Velocity->z()*0.5*deltaT);

      auto *originalVelocity = originalVelocities_.get(int(i));
      auto *k1Force          = k1Forces_          .get(int(i));

      p->setVelocity(originalVelocity->x() + k1Force->x()*0.5*deltaT/p->mass(),
                     originalVelocity->y() + k1Force->y()*0.5*deltaT/p->mass(),
                     originalVelocity->z() + k1Force->z()*0.5*deltaT/p->mass());
    }
  }

  s_->applyForces();

  // save the intermediate forces
  for (uint i = 0; i < numParticles; ++i) {
    auto *p = s_->getParticle(i);

    if ( p->isFree()) {
      k2Forces_    .get(int(i))->set(p->force   ());
      k2Velocities_.get(int(i))->set(p->velocity());
    }

    p->force()->clear();  // and clear the forces now that we are done with them
  }


  /////////////////////////////////////////////////////
  // get k3 values

  for (uint i = 0; i < numParticles; ++i) {
    auto *p = s_->getParticle(i);

    if (p->isFree()) {
      auto *originalPosition = originalPositions_.get(int(i));
      auto *k2Velocity       = k2Velocities_     .get(int(i));

      p->setPosition(originalPosition->x() + k2Velocity->x()*0.5*deltaT,
                     originalPosition->y() + k2Velocity->y()*0.5*deltaT,
                     originalPosition->z() + k2Velocity->z()*0.5*deltaT);

      auto *originalVelocity = originalVelocities_.get(int(i));
      auto *k2Force          = k2Forces_          .get(int(i));

      p->setVelocity(originalVelocity->x() + k2Force->x()*0.5*deltaT/p->mass(),
                     originalVelocity->y() + k2Force->y()*0.5*deltaT/p->mass(),
                     originalVelocity->z() + k2Force->z()*0.5*deltaT/p->mass());
    }
  }

  s_->applyForces();

  // save the intermediate forces
  for (uint i = 0; i < numParticles; ++i) {
    auto *p = s_->getParticle(i);

    if (p->isFree()) {
      k3Forces_    .get(int(i))->set(p->force   ());
      k3Velocities_.get(int(i))->set(p->velocity());
    }

    p->force()->clear();  // and clear the forces now that we are done with them
  }

  //////////////////////////////////////////////////
  // get k4 values

  for (uint i = 0; i < numParticles; ++i) {
    auto *p = s_->getParticle(i);

    if (p->isFree()) {
      auto *originalPosition = originalPositions_.get(int(i));
      auto *k3Velocity       = k3Velocities_     .get(int(i));

      p->setPosition(originalPosition->x() + k3Velocity->x()*deltaT,
                     originalPosition->y() + k3Velocity->y()*deltaT,
                     originalPosition->z() + k3Velocity->z()*deltaT);

      auto *originalVelocity = originalVelocities_.get(int(i));
      auto *k3Force          = k3Forces_          .get(int(i));

      p->setVelocity(originalVelocity->x() + k3Force->x()*deltaT/p->mass(),
                     originalVelocity->y() + k3Force->y()*deltaT/p->mass(),
                     originalVelocity->z() + k3Force->z()*deltaT/p->mass());
    }
  }

  s_->applyForces();

  // save the intermediate forces
  for (uint i = 0; i < numParticles; ++i) {
    auto *p = s_->getParticle(i);

    if (p->isFree()) {
      k4Forces_    .get(int(i))->set(p->force   ());
      k4Velocities_.get(int(i))->set(p->velocity());
    }
  }

  /////////////////////////////////////////////////////////////
  // put them all together and what do you get?

  for (uint i = 0; i < numParticles; ++i) {
    auto *p = s_->getParticle(i);

    //p->setStepAge(p->stepAge() + deltaT);

    if (p->isFree()) {
      // update position

      auto *originalPosition = originalPositions_.get(int(i));
      auto *k1Velocity       = k1Velocities_     .get(int(i));
      auto *k2Velocity       = k2Velocities_     .get(int(i));
      auto *k3Velocity       = k3Velocities_     .get(int(i));
      auto *k4Velocity       = k4Velocities_     .get(int(i));

      p->setPosition(originalPosition->x() + deltaT/6.0*(k1Velocity->x() + 2.0*k2Velocity->x() +
                       2.0*k3Velocity->x() + k4Velocity->x()),
                     originalPosition->y() + deltaT/6.0*(k1Velocity->y() + 2.0*k2Velocity->y() +
                       2.0*k3Velocity->y() + k4Velocity->y()),
                     originalPosition->z() + deltaT/6.0*(k1Velocity->z() + 2.0*k2Velocity->z() +
                       2.0*k3Velocity->z() + k4Velocity->z()));

      // update velocity

      auto *originalVelocity = originalVelocities_.get(int(i));
      auto *k1Force          = k1Forces_          .get(int(i));
      auto *k2Force          = k2Forces_          .get(int(i));
      auto *k3Force          = k3Forces_          .get(int(i));
      auto *k4Force          = k4Forces_          .get(int(i));

      p->setVelocity(originalVelocity->x() + deltaT/(6.0*p->mass())*(k1Force->x() +
                       2.0*k2Force->x() + 2.0*k3Force->x() + k4Force->x()),
                     originalVelocity->y() + deltaT/(6.0*p->mass())*(k1Force->y() +
                       2.0*k2Force->y() + 2.0*k3Force->y() + k4Force->y()),
                     originalVelocity->z() + deltaT/(6.0*p->mass())*(k1Force->z() +
                       2.0*k2Force->z() + 2.0*k3Force->z() + k4Force->z()));
    }
  }
}
