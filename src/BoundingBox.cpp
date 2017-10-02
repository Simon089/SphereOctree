//
//  BoundingBox.cpp
//  SphereOctree
//
//  Created by Simon Weigl on 6/1/16.
//
//

#include "BoundingBox.h"
#include <cassert>
#include <iostream>
#include <stdio.h>

using namespace std;

BoundingBox::BoundingBox(Eigen::Vector3f leftFrontBottom,
                         Eigen::Vector3f rightBackTop)
    : p1(leftFrontBottom), p2(rightBackTop) {
  assert(p1.x() < p2.x());
  assert(p1.y() < p2.y());
  assert(p1.z() > p2.z());
}

bool BoundingBox::isIn(shared_ptr<Eigen::Vector3f> point) const {
  if (point->x() >= p1.x() && point->x() <= p2.x()) {
    if (point->y() >= p1.y() && point->y() <= p2.y()) {
      if (point->z() <= p1.z() && point->z() >= p2.z()) {
        return true;
      }
    }
  }
  return false;
}

shared_ptr<vector<shared_ptr<Face>>>
BoundingBox::facesIn(shared_ptr<vector<shared_ptr<Face>>> faces) const {
  auto ins = make_shared<vector<shared_ptr<Face>>>();
  for (auto it = faces->begin(); it != faces->end(); ++it) {
    if (isIn((*it)))
      ins->push_back(*it);
  }
  return ins;
}

shared_ptr<vector<shared_ptr<BoundingBox>>> BoundingBox::split() const {
  float splitX = (p2.x() + p1.x()) / 2;
  float splitY = (p2.y() + p1.y()) / 2;
  float splitZ = (p2.z() + p1.z()) / 2;

  auto bbs = make_shared<vector<shared_ptr<BoundingBox>>>();
  bbs->push_back(make_shared<BoundingBox>(
      p1, Eigen::Vector3f(splitX, splitY, splitZ))); // bottom front left

  bbs->push_back(make_shared<BoundingBox>(
      Eigen::Vector3f(splitX, p1.y(), p1.z()),
      Eigen::Vector3f(p2.x(), splitY, splitZ))); // bottom front right

  bbs->push_back(make_shared<BoundingBox>(
      Eigen::Vector3f(p1.x(), splitY, p1.z()),
      Eigen::Vector3f(splitX, p2.y(), splitZ))); // top front left

  bbs->push_back(make_shared<BoundingBox>(
      Eigen::Vector3f(splitX, splitY, p1.z()),
      Eigen::Vector3f(p2.x(), p2.y(), splitZ))); // top front right

  bbs->push_back(make_shared<BoundingBox>(
      Eigen::Vector3f(p1.x(), p1.y(), splitZ),
      Eigen::Vector3f(splitX, splitY, p2.z()))); // bottom back left

  bbs->push_back(make_shared<BoundingBox>(
      Eigen::Vector3f(splitX, p1.y(), splitZ),
      Eigen::Vector3f(p2.x(), splitY, p2.z()))); // bottom back right

  bbs->push_back(make_shared<BoundingBox>(
      Eigen::Vector3f(p1.x(), splitY, splitZ),
      Eigen::Vector3f(splitX, p2.y(), p2.z()))); // top back left

  bbs->push_back(
      make_shared<BoundingBox>(Eigen::Vector3f(splitX, splitY, splitZ),
                               p2)); // top back right

  return bbs;
}
