#include "Camera.h"
#include "MatrixStack.h"
#include <iostream>

Camera::Camera()
    : aspect(1.0f), fovy(45.0f), znear(0.1f), zfar(1000.0f),
      rotations(0.0, 0.0), translations(0.0f, 0.0f, -5.0f), rfactor(0.2f),
      tfactor(0.001f), sfactor(0.005f) {}

Camera::~Camera() {}

void Camera::mouseClicked(double x, double y, bool shift, bool ctrl, bool alt) {
  mousePrev << x, y;
  if (shift) {
    state = Camera::TRANSLATE;
  } else if (ctrl) {
    state = Camera::SCALE;
  } else {
    state = Camera::ROTATE;
  }
}

void Camera::mouseMoved(double x, double y) {
  Eigen::Vector2f mouseCurr(x, y);
  Eigen::Vector2f dv = mouseCurr - mousePrev;
  switch (state) {
  case Camera::ROTATE:
    rotations += rfactor * dv;
    break;
  case Camera::TRANSLATE:
    translations(0) -= translations(2) * tfactor * dv(0);
    translations(1) += translations(2) * tfactor * dv(1);
    break;
  case Camera::SCALE:
    translations(2) *= (1.0f - sfactor * dv(1));
    break;
  }
  mousePrev = mouseCurr;
}

void Camera::applyProjectionMatrix(std::shared_ptr<MatrixStack> P) const {
  // Modify provided MatrixStack
  P->perspective(fovy, aspect, znear, zfar);
}

void Camera::applyViewMatrix(std::shared_ptr<MatrixStack> MV) const {
  MV->translate(translations);
  MV->rotate(rotations(1), Eigen::Vector3f(1.0f, 0.0f, 0.0f));
  MV->rotate(rotations(0), Eigen::Vector3f(0.0f, 1.0f, 0.0f));
}
