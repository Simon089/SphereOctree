//
//  WorldObject.h
//  SphereOctree
//
//  Created by Simon Weigl on 6/4/16.
//
//

#ifndef WorldObject_h
#define WorldObject_h

/* Defines how many levels the octree has */
#define TREE_DEEPNESS 9

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>

#include "Camera.h"
#include "MatrixStack.h"
#include "OctreeNode.h"
#include "Program.h"
#include "Shape.h"
#include <memory>

// A world object contains its shape, the sphere shape, its position, the octree
// node, and attributes and programs for drawing.
class WorldObject {
  std::shared_ptr<Shape> shape;
  std::shared_ptr<Shape> sphere;
  Eigen::Vector3f position;
  std::shared_ptr<OctreeNode> octree;
  std::shared_ptr<Program> shapeProg;
  std::shared_ptr<Program> octreeProg;
  std::shared_ptr<Program> transProg;
  Eigen::Vector3f rotationVec;
  Eigen::Vector3f initialPosition;
  Eigen::Vector3f velocity;
  int rotation;
  int angle;
  bool *keyToogles;
  bool isColliding = false;

public:
  // Creates an object of a given shape and inits it's sphere-octree.
  // @arg shape: Shape of the object
  // @arg sphere: Sphere shape
  // @arg shapeProg: Program for drawing the object shape
  // @arg octreeProg: Program for drawing the colliding spheres in the octree
  // @arg transProg: Program for drawing all spheres on a level in the octree
  // @arg initPosition: Initial position of the object
  // @arg velocity: Velocity of the object per move() call
  // @arg keyToogles: Pointer to [bool] key toogles
  WorldObject(std::shared_ptr<Shape> shape, std::shared_ptr<Shape> sphere,
              std::shared_ptr<Program> shapeProg,
              std::shared_ptr<Program> octreeProg,
              std::shared_ptr<Program> transProg, Eigen::Vector3f initPosition,
              Eigen::Vector3f velocity, bool *keyToogles);

  // Inits the object. Is used to set it back to the start position.
  void init();

  // Moves and rotates the object.
  void move();

  // Check if the object is colliding with the other object.
  void collisionDetection(std::shared_ptr<WorldObject> obj);

  // Draws the colliding spheres or all spheres on a level, depending on the
  // keyToogles.
  void draw(std::shared_ptr<Camera> camera) const;

  // Adds the translation and rotation of the object to the matrix stack.
  void addTransitionMatrix(std::shared_ptr<MatrixStack> m) const;

  inline std::shared_ptr<OctreeNode> getOctree() const { return octree; }
  inline Eigen::Vector3f getPosition() const { return position; }
  inline bool getColliding() const { return isColliding; }
};

#endif /* WorldObject_h */
