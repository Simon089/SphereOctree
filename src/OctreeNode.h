//
//  OctreeNode.h
//  SphereOctree
//
//  Created by Simon Weigl on 6/1/16.
//
//

#ifndef OctreeNode_h
#define OctreeNode_h

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>

#include "BoundingBox.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include <memory>
#include <vector>

// A Octree-Node has a bounding-box, 0 to 8 child-nodes, faces which are located
// in the bounding-box and in the sphere, the sphere-origin and the
// sphere-radius.
class OctreeNode : public std::enable_shared_from_this<OctreeNode> {
  std::shared_ptr<BoundingBox> bb;
  std::shared_ptr<std::vector<std::shared_ptr<OctreeNode>>> children;
  std::shared_ptr<std::vector<std::shared_ptr<Face>>> faces;
  std::shared_ptr<Eigen::Vector3f> sphereOrigin;
  float sphereRadius = 0.0f;
  bool isColliding = false;

  // Returns true, if all faces are in the sphere, false otherwise.
  bool allIn() const;

  // Returns euclidean distance between 2 vectors.
  float eucDistance(std::shared_ptr<Eigen::Vector3f> x,
                    std::shared_ptr<Eigen::Vector3f> y) const;

public:
  // Creates the bounding-sphere of this node and adds child nodes if the
  // deepness is greater than 1.
  OctreeNode(std::shared_ptr<BoundingBox> boundingBox,
             std::shared_ptr<std::vector<std::shared_ptr<Face>>> faces,
             size_t deepness);

  inline std::shared_ptr<Eigen::Vector3f> getOrigin() const {
    return sphereOrigin;
  }
  inline float getRadius() const { return sphereRadius; }
  inline float getScale() const { return sphereRadius * 2; }
  inline void collides() { isColliding = true; }
  inline std::shared_ptr<std::vector<std::shared_ptr<OctreeNode>>>
  getChildren() const {
    return children;
  }

  // Sets 'isColliding' of node all child nodes to false.
  void resetColliding();

  // Returns number of child nodes.
  size_t getNumChildren() const;

  // Draw all colliding spheres of this node and all child nodes
  // @arg program: Program to draw the colliding sphers
  // @arg shapeSphere: Sphere shape
  // @arg MV: Matrix stack with the transitions of the object
  void drawColliding(std::shared_ptr<Program> program,
                     std::shared_ptr<Shape> shapeSphere,
                     std::shared_ptr<MatrixStack> MV);

  // Draw all spheres on the given tree-level
  // @arg program: Program to draw the colliding sphers
  // @arg level: level of the tree to draw (starts with 1)
  // @arg shapeSphere: Sphere shape
  // @arg MV: Matrix stack with the transitions of the object
  void drawLevel(std::shared_ptr<Program> program,
                 std::shared_ptr<Shape> shapeSphere, size_t level,
                 std::shared_ptr<MatrixStack> MV);

  // Returnes true, if a leaf of this node collides with a leaf of the other
  // node.
  // @arg otherNode: Node to check for a collision
  // @arg myPosition: Transition matrix of this node
  // @arg otherPosition: Transition matrix of the other node
  bool checkCollision(std::shared_ptr<OctreeNode> otherNode,
                      std::shared_ptr<Eigen::Matrix4f> myPosition,
                      std::shared_ptr<Eigen::Matrix4f> otherPosition);
};

#endif /* OctreeNode_h */
