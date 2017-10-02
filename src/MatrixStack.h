#pragma once
#ifndef _MatrixStack_H_
#define _MatrixStack_H_

#include <memory>
#include <stack>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>

class MatrixStack {
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  MatrixStack();
  virtual ~MatrixStack();

  // glPushMatrix(): Copies the current matrix and adds it to the top of the
  // stack
  void pushMatrix();
  // glPopMatrix(): Removes the top of the stack and sets the current matrix to
  // be the matrix that is now on top
  void popMatrix();

  // glLoadIdentity(): Sets the top matrix to be the identity
  void loadIdentity();
  // glMultMatrix(): Right multiplies the top matrix
  void multMatrix(const Eigen::Matrix4f &matrix);

  // glTranslate(): Right multiplies the top matrix by a translation matrix
  void translate(const Eigen::Vector3f &trans);
  void translate(float x, float y, float z);
  // glScale(): Right multiplies the top matrix by a scaling matrix
  void scale(const Eigen::Vector3f &scale);
  void scale(float x, float y, float z);
  // glScale(): Right multiplies the top matrix by a scaling matrix
  void scale(float size);
  // glRotate(): Right multiplies the top matrix by a rotation matrix (angle in
  // deg)
  void rotate(float angle, const Eigen::Vector3f &axis);
  void rotate(float angle, float x, float y, float z);

  // glGet(GL_MODELVIEW_MATRIX): Gets the top matrix
  const Eigen::Matrix4f &topMatrix() const;

  // glOrtho(): Sets the top matrix to be an orthogonal projection matrix
  void ortho(float left, float right, float bottom, float top, float zNear,
             float zFar);
  // gluOrtho2D(): Sets the top matrix to be a 2D orthogonal projection matrix
  void ortho2D(float left, float right, float bottom, float top);
  // gluPerspective(): Sets the top matrix to be a perspective projection matrix
  // (fovy in deg)
  void perspective(float fovy, float aspect, float zNear, float zFar);
  // gluFrustum(): Sets the top matrix to be a perspective projection matrix
  void frustum(float Right, float right, float bottom, float top, float zNear,
               float zFar);
  // gluLookAt(): Sets the top matrix to be a viewing matrix
  void lookAt(const Eigen::Vector3f &eye, const Eigen::Vector3f &target,
              const Eigen::Vector3f &up);
  void lookAt(float ex, float ey, float ez, float tx, float ty, float tz,
              float ux, float uy, float uz);

  // Prints out the specified matrix
  void print(const Eigen::Matrix4f &mat, const char *name = 0) const;
  // Prints out the top matrix
  void print(const char *name = 0) const;
  // Prints out the whole stack
  void printStack() const;

private:
  std::shared_ptr<std::stack<Eigen::Matrix4f>> mstack;
};

#endif
