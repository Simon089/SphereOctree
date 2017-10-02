//
//  WorldObject.cpp
//  SphereOctree
//
//  Created by Simon Weigl on 6/4/16.
//
//

#include "WorldObject.h"
#include <chrono>
#include <cstdlib>
#include <iostream>

using namespace std;

WorldObject::WorldObject(shared_ptr<Shape> objShape,
                         shared_ptr<Shape> sphereShape,
                         shared_ptr<Program> sProg, shared_ptr<Program> oProg,
                         std::shared_ptr<Program> tProg, Eigen::Vector3f iPos,
                         Eigen::Vector3f v, bool *keyToo)
    : shape(objShape), sphere(sphereShape), shapeProg(sProg), octreeProg(oProg),
      transProg(tProg), initialPosition(iPos), velocity(v), keyToogles(keyToo) {
  auto start = std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
                   .count();
  octree = make_shared<OctreeNode>(
      make_shared<BoundingBox>(Eigen::Vector3f(-1.0f, -1.0, 1.0f),
                               Eigen::Vector3f(1.0f, 1.0f, -1.0f)),
      shape->getFaces(), TREE_DEEPNESS);
  auto end = std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::system_clock::now().time_since_epoch())
                 .count();
  cout << "Octree created in " << (end - start)
       << "ms. (Faces: " << shape->getFaces()->size()
       << ", Deepness: " << TREE_DEEPNESS
       << ", Child nodes: " << octree->getNumChildren() << ")" << endl;
}

void WorldObject::init() {
  octree->resetColliding();
  isColliding = false;
  position = initialPosition;
  int r = rand() % 3;
  if (r == 0)
    rotationVec = Eigen::Vector3f(1.0f, 0.0f, 0.0f);
  else if (r == 1)
    rotationVec = Eigen::Vector3f(0.0f, 1.0f, 0.0f);
  else
    rotationVec = Eigen::Vector3f(0.0f, 0.0f, 1.0f);

  angle = rand() % 360;
  rotation = 2 - (rand() % 5);
}

void WorldObject::move() {
  if (!isColliding) {
    position += velocity;
    angle += rotation;
    angle = angle % 360;
  }
}

void WorldObject::collisionDetection(shared_ptr<WorldObject> obj) {
  shared_ptr<MatrixStack> myStack = make_shared<MatrixStack>();
  shared_ptr<MatrixStack> otherStack = make_shared<MatrixStack>();
  addTransitionMatrix(myStack);
  obj->addTransitionMatrix(otherStack);
  bool curCollision = octree->checkCollision(
      obj->getOctree(), make_shared<Eigen::Matrix4f>(myStack->topMatrix()),
      make_shared<Eigen::Matrix4f>(otherStack->topMatrix()));
  isColliding |= curCollision;
  obj->isColliding |= curCollision;
}

void WorldObject::draw(std::shared_ptr<Camera> camera) const {
  auto MV = make_shared<MatrixStack>();
  auto P = make_shared<MatrixStack>();
  camera->applyProjectionMatrix(P);
  camera->applyViewMatrix(MV);

  MV->pushMatrix();
  addTransitionMatrix(MV);

  shapeProg->bind();
  glUniformMatrix4fv(shapeProg->getUniform("P"), 1, GL_FALSE,
                     P->topMatrix().data());
  glUniformMatrix4fv(shapeProg->getUniform("MV"), 1, GL_FALSE,
                     MV->topMatrix().data());
  shape->draw(shapeProg);
  shapeProg->unbind();

  // Draw all spheres on level
  if (keyToogles[(unsigned)'s']) {
    int level = 1;
    for (int i = 0; i < 9; ++i) {
      if (keyToogles[(unsigned)'1' + i])
        level += i;
    }
    if (keyToogles[(unsigned)'0'])
      level = TREE_DEEPNESS;
    transProg->bind();
    glUniformMatrix4fv(transProg->getUniform("P"), 1, GL_FALSE,
                       P->topMatrix().data());
    glUniformMatrix4fv(transProg->getUniform("MV"), 1, GL_FALSE,
                       MV->topMatrix().data());
    octree->drawLevel(transProg, sphere, level, MV);
    transProg->unbind();
  }
  // Draw colliding spheres only
  else {
    octreeProg->bind();
    glUniformMatrix4fv(octreeProg->getUniform("P"), 1, GL_FALSE,
                       P->topMatrix().data());
    Eigen::Matrix3f T;
    T(0) = TREE_DEEPNESS;
    T(4) = TREE_DEEPNESS;
    glUniformMatrix3fv(octreeProg->getUniform("T"), 1, GL_FALSE, T.data());
    octree->drawColliding(octreeProg, sphere, MV);
    octreeProg->unbind();
  }

  MV->popMatrix();
}

void WorldObject::addTransitionMatrix(shared_ptr<MatrixStack> m) const {
  m->translate(position);
  m->rotate(angle, rotationVec);
}
