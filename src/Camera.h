#pragma once
#ifndef __Camera__
#define __Camera__

#include <memory>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>

class MatrixStack;

class Camera {
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  enum { ROTATE = 0, TRANSLATE, SCALE };

  Camera();
  virtual ~Camera();
  void setInitDistance(float z) { translations(2) = -std::abs(z); }
  void setAspect(float a) { aspect = a; };
  void setRotationFactor(float f) { rfactor = f; };
  void setTranslationFactor(float f) { tfactor = f; };
  void setScaleFactor(float f) { sfactor = f; };
  void mouseClicked(double x, double y, bool shift, bool ctrl, bool alt);
  void mouseMoved(double x, double y);
  void applyProjectionMatrix(std::shared_ptr<MatrixStack> P) const;
  void applyViewMatrix(std::shared_ptr<MatrixStack> MV) const;

private:
  float aspect;
  float fovy;
  float znear;
  float zfar;
  Eigen::Vector2f rotations;
  Eigen::Vector3f translations;
  Eigen::Vector2f mousePrev;
  int state;
  float rfactor;
  float tfactor;
  float sfactor;
};

#endif