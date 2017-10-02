//
//  OctreeNode.cpp
//  SphereOctree
//
//  Created by Simon Weigl on 6/1/16.
//
//

#include "OctreeNode.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <stdio.h>

using namespace std;

OctreeNode::OctreeNode(shared_ptr<BoundingBox> _bb,
                       shared_ptr<vector<shared_ptr<Face>>> f, size_t deepness)
    : bb(_bb), children(make_shared<vector<shared_ptr<OctreeNode>>>()),
      faces(f) {
  // After we split a BB, there can be some which have no more faces.
  if (faces->size() == 0)
    return;
  int rIts = 1;

  // Find a sphere which includes all faces.
  // Possible approaches: Ritter's algorithm / midpoint of bounding box /
  // Fischer's exact solver. Using the idea of "Ritter's algorithm":
  // 1. Pick a point x from P, search a point y in P, which has the largest
  // distance from x.
  // 2. Search a point z in P, which has the largest distance from y.
  //    Set up an initial ball B, with its centre as the midpoint of y and z,
  //    the radius as half of the distance between y and z.
  // 3. If all points in P are within ball B, then we get a bounding sphere.
  //    Otherwise, let p be a point outside the ball, construct a new ball
  //    covering both point p and previous ball. Repeat this step until all
  //    points are covered.
  while (!allIn()) {
    srand(time(nullptr));
    shared_ptr<Eigen::Vector3f> x = faces->at(rand() % faces->size())->a;
    shared_ptr<Eigen::Vector3f> y;
    float dMax = 0.0f;
    for (auto it = faces->begin(); it != faces->end(); ++it) {
      float d = eucDistance(x, (*it)->b);
      if (d > dMax) {
        dMax = d;
        y = (*it)->b;
      }
    }
    // Step 2
    shared_ptr<Eigen::Vector3f> z;
    dMax = 0.0f;
    for (auto it = faces->begin(); it != faces->end(); ++it) {
      float d = eucDistance(y, (*it)->a);
      if (d > dMax) {
        dMax = d;
        z = (*it)->a;
      }
    }

    sphereOrigin = make_shared<Eigen::Vector3f>(*y + ((*z - *y) / 2));

    // For performance increase radius with every step
    sphereRadius = (eucDistance(z, y) / 2) * (1.0 + ((rIts / 50) * 0.05));
    rIts++;
  }

  // Create children
  if (--deepness > 0) {
    shared_ptr<vector<shared_ptr<BoundingBox>>> newBBs = bb->split();
    for (auto it = newBBs->begin(); it != newBBs->end(); ++it) {
      auto newChild =
          make_shared<OctreeNode>(*it, (*it)->facesIn(faces), deepness);
      // If child has no faces and therefore no sphere, do not add it
      if (newChild->getOrigin() != nullptr)
        children->push_back(newChild);
    }
  }
}

bool OctreeNode::allIn() const {
  if (sphereRadius <= 0.0)
    return false;
  for (auto it = faces->begin(); it != faces->end(); ++it) {
    if (eucDistance(sphereOrigin, (*it)->a) > sphereRadius)
      return false;
    if (eucDistance(sphereOrigin, (*it)->b) > sphereRadius)
      return false;
    if (eucDistance(sphereOrigin, (*it)->c) > sphereRadius)
      return false;
  }
  return true;
}

float OctreeNode::eucDistance(shared_ptr<Eigen::Vector3f> x,
                              shared_ptr<Eigen::Vector3f> y) const {
  float d = sqrt(pow(x->x() - y->x(), 2) + pow(x->y() - y->y(), 2) +
                 pow(x->z() - y->z(), 2));
  return d;
}

void OctreeNode::drawColliding(shared_ptr<Program> p, shared_ptr<Shape> s,
                               shared_ptr<MatrixStack> MV) {
  // If this node / sphere is colliding, draw it.
  if (isColliding) {
    MV->pushMatrix();
    MV->translate(*getOrigin());
    MV->scale(getScale());
    glUniformMatrix4fv(p->getUniform("MV"), 1, GL_FALSE,
                       MV->topMatrix().data());
    s->draw(p);
    MV->popMatrix();
  }
  // Draw children (if they are colliding).
  for (auto it = children->begin(); it != children->end(); ++it)
    (*it)->drawColliding(p, s, MV);
}

void OctreeNode::drawLevel(shared_ptr<Program> p, shared_ptr<Shape> s,
                           size_t lvl, shared_ptr<MatrixStack> MV) {
  if (--lvl == 0) {
    MV->pushMatrix();
    MV->translate(*getOrigin());
    MV->scale(getScale());
    glUniformMatrix4fv(p->getUniform("MV"), 1, GL_FALSE,
                       MV->topMatrix().data());
    s->draw(p);
    MV->popMatrix();
  } else {
    for (auto it = children->begin(); it != children->end(); ++it)
      (*it)->drawLevel(p, s, lvl, MV);
  }
}

void OctreeNode::resetColliding() {
  isColliding = false;
  for (auto it = children->begin(); it != children->end(); ++it)
    (*it)->resetColliding();
}

size_t OctreeNode::getNumChildren() const {
  if (children->size() == 0)
    return 0;
  else {
    size_t num = children->size();
    for (auto it = children->begin(); it != children->end(); ++it)
      num += (*it)->getNumChildren();
    return num;
  }
}

bool OctreeNode::checkCollision(
    shared_ptr<OctreeNode> n, std::shared_ptr<Eigen::Matrix4f> myPosition,
    std::shared_ptr<Eigen::Matrix4f> otherPosition) {
  // Get the global midpoint of the sphere, by translating the object position
  // with the local sphere position.
  Eigen::Vector4f a =
      (*myPosition * Eigen::Vector4f(sphereOrigin->x(), sphereOrigin->y(),
                                     sphereOrigin->z(), 1.0f));
  Eigen::Vector4f b = (*otherPosition *
                       Eigen::Vector4f(n->getOrigin()->x(), n->getOrigin()->y(),
                                       n->getOrigin()->z(), 1.0f));
  auto myMidpoint = make_shared<Eigen::Vector3f>(a.x(), a.y(), a.z());
  auto otherMidpoint = make_shared<Eigen::Vector3f>(b.x(), b.y(), b.z());

  // Recursively check for colliding spheres
  if (eucDistance(myMidpoint, otherMidpoint) <=
      (sphereRadius + n->getRadius())) {
    if (children->size() == 0 && n->getChildren()->size() == 0) {
      this->collides();
      n->collides();
      return true;
    } else {
      bool childCollision = false;
      for (auto itMe = children->begin(); itMe != children->end(); ++itMe) {
        for (auto itOther = n->getChildren()->begin();
             itOther != n->getChildren()->end(); ++itOther)
          childCollision |=
              (*itMe)->checkCollision(*itOther, myPosition, otherPosition);
      }
      return childCollision;
    }
  }
  return false;
}
