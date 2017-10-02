//
//  BoundingBox.h
//  SphereOctree
//
//  Created by Simon Weigl on 6/1/16.
//
//

#ifndef BoundingBox_h
#define BoundingBox_h

#include "Shape.h"
#include <memory>
#include <vector>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>

/*
 * A bounding box is defines by its bottom-front-left point and top-back-right
 * point.
 */
class BoundingBox {
  Eigen::Vector3f p1, p2;

public:
  // Creates a bounding box, specified by the front-left-bottom point and the
  // back-right-top point.
  BoundingBox(Eigen::Vector3f leftFrontBottom, Eigen::Vector3f rightBackTop);

  // Returnes true, if the point is in the bounding box, false otherwise.
  bool isIn(std::shared_ptr<Eigen::Vector3f> point) const;

  // Returns a vector of shared-pointers to the faces in 'faces' where at least
  // on point of the face is in the bounding box. todo: this won't add a face,
  // where all three points are outside the BB, but the face is still inside it.
  // is this a problem?
  inline bool isIn(std::shared_ptr<Face> face) const {
    return (isIn(face->a) || isIn(face->b) || isIn(face->c));
  }

  // Returns all faces from the input list which are located (or partwise
  // located) in the bounding box.
  std::shared_ptr<std::vector<std::shared_ptr<Face>>>
  facesIn(std::shared_ptr<std::vector<std::shared_ptr<Face>>> faces) const;

  // Splits the bounding box in 8 equal sized bounding boxes.
  std::shared_ptr<std::vector<std::shared_ptr<BoundingBox>>> split() const;
};

#endif /* BoundingBox_h */
