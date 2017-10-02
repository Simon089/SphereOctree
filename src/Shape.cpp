#include "Shape.h"
#include <iostream>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>

#include "GLSL.h"
#include "Program.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

using namespace std;

Shape::Shape() : eleBufID(0), posBufID(0), norBufID(0), texBufID(0) {}

Shape::~Shape() {}

void Shape::loadMesh(const string &meshName) {
  // Load geometry
  // Some obj files contain material information.
  // We'll ignore them for this assignment.
  vector<tinyobj::shape_t> shapes;
  vector<tinyobj::material_t> objMaterials;
  string errStr;
  bool rc = tinyobj::LoadObj(shapes, objMaterials, errStr, meshName.c_str());
  if (!rc) {
    cerr << errStr << endl;
  } else {
    posBuf = shapes[0].mesh.positions;
    norBuf = shapes[0].mesh.normals;
    texBuf = shapes[0].mesh.texcoords;
    eleBuf = shapes[0].mesh.indices;
  }
}

void Shape::fitToUnitBox() {
  // Scale the vertex positions so that they fit within [-1, +1] in all three
  // dimensions.
  Eigen::Vector3f vmin(posBuf[0], posBuf[1], posBuf[2]);
  Eigen::Vector3f vmax(posBuf[0], posBuf[1], posBuf[2]);
  for (int i = 0; i < (int)posBuf.size(); i += 3) {
    Eigen::Vector3f v(posBuf[i], posBuf[i + 1], posBuf[i + 2]);
    vmin(0) = min(vmin(0), v(0));
    vmin(1) = min(vmin(1), v(1));
    vmin(2) = min(vmin(2), v(2));
    vmax(0) = max(vmax(0), v(0));
    vmax(1) = max(vmax(1), v(1));
    vmax(2) = max(vmax(2), v(2));
  }
  Eigen::Vector3f center = 0.5f * (vmin + vmax);
  Eigen::Vector3f diff = vmax - vmin;
  float diffmax = diff(0);
  diffmax = max(diffmax, diff(1));
  diffmax = max(diffmax, diff(2));
  float scale = 1.0f / diffmax;
  for (int i = 0; i < (int)posBuf.size(); i += 3) {
    posBuf[i] = (posBuf[i] - center(0)) * scale;
    posBuf[i + 1] = (posBuf[i + 1] - center(1)) * scale;
    posBuf[i + 2] = (posBuf[i + 2] - center(2)) * scale;
  }
}

void Shape::init() {
  // Send the position array to the GPU
  glGenBuffers(1, &posBufID);
  glBindBuffer(GL_ARRAY_BUFFER, posBufID);
  glBufferData(GL_ARRAY_BUFFER, posBuf.size() * sizeof(float), &posBuf[0],
               GL_STATIC_DRAW);

  // Send the normal array to the GPU
  if (!norBuf.empty()) {
    glGenBuffers(1, &norBufID);
    glBindBuffer(GL_ARRAY_BUFFER, norBufID);
    glBufferData(GL_ARRAY_BUFFER, norBuf.size() * sizeof(float), &norBuf[0],
                 GL_STATIC_DRAW);
  }

  // Send the texture array to the GPU
  if (!texBuf.empty()) {
    glGenBuffers(1, &texBufID);
    glBindBuffer(GL_ARRAY_BUFFER, texBufID);
    glBufferData(GL_ARRAY_BUFFER, texBuf.size() * sizeof(float), &texBuf[0],
                 GL_STATIC_DRAW);
  }

  // Send the element array to the GPU
  glGenBuffers(1, &eleBufID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size() * sizeof(unsigned int),
               &eleBuf[0], GL_STATIC_DRAW);

  // Unbind the arrays
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  GLSL::checkError(GET_FILE_LINE);
}

void Shape::draw(const shared_ptr<Program> prog) const {
  // Bind position buffer
  int h_pos = prog->getAttribute("aPos");
  glEnableVertexAttribArray(h_pos);
  glBindBuffer(GL_ARRAY_BUFFER, posBufID);
  glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

  // Bind normal buffer
  int h_nor = prog->getAttribute("aNor");
  if (h_nor != -1 && norBufID != 0) {
    glEnableVertexAttribArray(h_nor);
    glBindBuffer(GL_ARRAY_BUFFER, norBufID);
    glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
  }

  // Bind texcoords buffer
  int h_tex = prog->getAttribute("aTex");
  if (h_tex != -1 && texBufID != 0) {
    glEnableVertexAttribArray(h_tex);
    glBindBuffer(GL_ARRAY_BUFFER, texBufID);
    glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
  }

  // Bind element buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);

  // Draw
  glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT,
                 (const void *)0);

  // Disable and unbind
  if (h_tex != -1) {
    glDisableVertexAttribArray(h_tex);
  }
  if (h_nor != -1) {
    glDisableVertexAttribArray(h_nor);
  }
  glDisableVertexAttribArray(h_pos);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  GLSL::checkError(GET_FILE_LINE);
}

shared_ptr<vector<shared_ptr<Eigen::Vector3f>>> Shape::getPositions() const {
  shared_ptr<vector<shared_ptr<Eigen::Vector3f>>> positions =
      make_shared<vector<shared_ptr<Eigen::Vector3f>>>();
  for (size_t i = 0; i < posBuf.size(); i += 3) {
    positions->push_back(make_shared<Eigen::Vector3f>(
        posBuf.at(i), posBuf.at(i + 1), posBuf.at(i + 2)));
  }
  return positions;
}

shared_ptr<vector<shared_ptr<Face>>> Shape::getFaces() const {
  shared_ptr<vector<shared_ptr<Eigen::Vector3f>>> positions = getPositions();
  shared_ptr<vector<shared_ptr<Face>>> faces =
      make_shared<vector<shared_ptr<Face>>>();
  for (size_t i = 0; i < eleBuf.size(); i += 3) {
    faces->push_back(make_shared<Face>(positions->at(eleBuf[i]),
                                       positions->at(eleBuf[i + 1]),
                                       positions->at(eleBuf[i + 2])));
  }
  return faces;
}
