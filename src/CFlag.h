#ifndef CFLAG_H
#define CFLAG_H

#include <CParticle3D.h>
#include <CVector3D.h>

class CFlagParticle {
 public:
  CFlagParticle();
 ~CFlagParticle();

  double getMass() const;
  void setMass(double mass);

  const CVector3D &getPosition() const;
  void setPosition(double x, double y, double z);

  const CVector3D &getVelocity() const;
  void setVelocity(const CVector3D &velocity);

  const CVector3D &getAcceleration() const;
  void setAcceleration(const CVector3D &acceleration);

  const CVector3D &getForce() const { return force_; }
  void setForce(const CVector3D &force);

  bool getLocked() const { return locked_; }
  void setLocked(bool locked);

  void incPosition(const CVector3D &position);
  void incVelocity(const CVector3D &velocity);

  void zeroForce();
  void incForceY(double y);
  void incForce(const CVector3D &force);

 private:
  CParticle3D particle_;
  CVector3D   force_;
  bool        locked_ { false };
};

//---

class CFlagParticleSpring {
 public:
  CFlagParticleSpring();
 ~CFlagParticleSpring();

  CFlagParticle *getParticle1() const { return particle1_; }
  CFlagParticle *getParticle2() const { return particle2_; }
  void setParticles(CFlagParticle *particle1, CFlagParticle *particle2);

  double getConstant() const { return constant_; }
  void setConstant(double constant);

  double getDamping() const { return damping_; }
  void setDamping(double damping);

  double getDefLength() const { return def_length_; }
  void setDefLength(double def_length);

 private:
  CFlagParticle *particle1_  { nullptr };
  CFlagParticle *particle2_  { nullptr };
  double         constant_   { 0.0 };
  double         damping_    { 0.0 };
  double         def_length_ { 0.0 };
};

//---

class CFlag {
 public:
  CFlag(double x, double y, double width, double height,
        int num_rows=20, int num_cols=20, double mass=100);
 ~CFlag();

  void step(double dt);

  void getGridDimensions(int *num_rows, int *num_cols) const {
    *num_rows = num_rows_; *num_cols = num_cols_;
  }

  CFlagParticle *getParticle(uint ix, uint iy) const { return &particles_[ix][iy]; }
  CFlagParticle **getParticles() const { return particles_; }

  CFlagParticleSpring *getSpring(uint i) const { return &springs_[i]; }
  CFlagParticleSpring *getSprings() const { return springs_; }

  int getNumSprings() const { return num_springs_; }

  void setWind(bool b) { has_wind_ = b; }

  void setWindForce(double f) { wind_force_factor_ = f; }

 private:
  void createParticles();
  void deleteParticles();

  void createSprings();
  void deleteSprings();

  void calcForces();
  void applyForces(double dt);
  void correctSprings();

 private:
  double x_      { 0.0 };
  double y_      { 0.0 };
  double width_  { 1.0 };
  double height_ { 1.0 };

  int num_rows_ { 1 };
  int num_cols_ { 1 };

  double mass_ { 1.0 };

  bool has_wind_ { false };
  bool has_drag_ { false };
  bool has_gravity_ { true };

  CFlagParticle **particles_ { nullptr };

  int                  num_springs_ { 0 };
  CFlagParticleSpring *springs_ { nullptr };

  CVector3D wind_vector_ { 10.0, 0.0, 0.0 };
  double    wind_force_factor_ { 100.0 };

  double gravity_ { -32.174 }; // ft/s/s

  double drag_coefficient_ { 0.01 };

  double spring_tension_ { 500.0 };
  double spring_shear_   { 600.0 };
  double spring_damping_ { 2.0 };
};

#endif
