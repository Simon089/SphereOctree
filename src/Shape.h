#pragma once
#ifndef _SHAPE_H_
#define _SHAPE_H_

#include <memory>
#include <string>
#include <vector>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>

class Program;

struct Face {
  std::shared_ptr<Eigen::Vector3f> a, b, c;

  Face(std::shared_ptr<Eigen::Vector3f> _a, std::shared_ptr<Eigen::Vector3f> _b,
       std::shared_ptr<Eigen::Vector3f> _c)
      : a(_a), b(_b), c(_c) {}
};

class Shape {
public:
  Shape();
  virtual ~Shape();
  void loadMesh(const std::string &meshName);
  void fitToUnitBox();
  void init();
  void draw(const std::shared_ptr<Program> prog) const;
  std::shared_ptr<std::vector<std::shared_ptr<Eigen::Vector3f>>>
  getPositions() const;
  std::shared_ptr<std::vector<std::shared_ptr<Face>>> getFaces() const;

private:
  std::vector<unsigned int> eleBuf;
  std::vector<float> posBuf;
  std::vector<float> norBuf;
  std::vector<float> texBuf;
  unsigned eleBufID;
  unsigned posBufID;
  unsigned norBufID;
  unsigned texBufID;
};

#endif
