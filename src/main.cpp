#include <cassert>
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>

#include "Camera.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "OctreeNode.h"
#include "Program.h"
#include "Shape.h"
#include "Texture.h"
#include "WorldObject.h"

using namespace std;
using namespace Eigen;

GLFWwindow *window;         // Main application window
string RESOURCE_DIR = "./"; // Where the resources are loaded from

shared_ptr<Camera> camera;
shared_ptr<Program> prog;
shared_ptr<Program> silProg;
shared_ptr<Program> transProg;
shared_ptr<Shape> bunny;
shared_ptr<Shape> sphere;
shared_ptr<Shape> teapot;
vector<shared_ptr<WorldObject>> objs;
shared_ptr<Texture> gridTex;

bool keyToggles[256] = {false}; // only for English keyboards!

bool collision = false;

// This function is called when a GLFW error occurs.
static void error_callback(int error, const char *description) {
  cerr << description << endl;
}

// This function is called when a key is pressed.
static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }
}

// This function is called when a character key on the keyboard is pressed.
static void char_callback(GLFWwindow *window, unsigned int key) {
  /* Let only one of the number keys (0 - 9) be active */
  if (key >= (unsigned)'0' && key <= (unsigned)'9') {
    for (int i = 0; i < 10; ++i)
      keyToggles[(unsigned)'0' + i] = false;
  }
  keyToggles[key] = !keyToggles[key];
}

// This function is called when the mouse is clicked.
static void mouse_button_callback(GLFWwindow *window, int button, int action,
                                  int mods) {
  // Get the current mouse position.
  double xmouse, ymouse;
  glfwGetCursorPos(window, &xmouse, &ymouse);
  // Get current window size.
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  if (action == GLFW_PRESS) {
    bool shift = mods & GLFW_MOD_SHIFT;
    bool ctrl = mods & GLFW_MOD_CONTROL;
    bool alt = mods & GLFW_MOD_ALT;
    camera->mouseClicked(xmouse, ymouse, shift, ctrl, alt);
  }
}

// This function is called when the mouse moves.
static void cursor_position_callback(GLFWwindow *window, double xmouse,
                                     double ymouse) {
  int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
  if (state == GLFW_PRESS) {
    camera->mouseMoved(xmouse, ymouse);
  }
}

// If the window is resized, capture the new size and reset the viewport.
static void resize_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

// This function is called once to initialize the scene and OpenGL.
static void init() {
  // Initialize time.
  glfwSetTime(0.0);

  // Set background color.
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  // Enable z-buffer test.
  glEnable(GL_DEPTH_TEST);

  // Program for Objects
  prog = make_shared<Program>();
  prog->setShaderNames(RESOURCE_DIR + "vert.glsl", RESOURCE_DIR + "frag.glsl");
  prog->setVerbose(true);
  prog->init();
  prog->addAttribute("aPos");
  prog->addAttribute("aNor");
  prog->addAttribute("aTex");
  prog->addUniform("MV");
  prog->addUniform("P");

  silProg = make_shared<Program>();
  silProg->setShaderNames(RESOURCE_DIR + "vert.glsl",
                          RESOURCE_DIR + "fragGrid.glsl");
  silProg->setVerbose(true);
  silProg->init();
  silProg->addAttribute("aPos");
  silProg->addAttribute("aNor");
  silProg->addAttribute("aTex");
  silProg->addUniform("MV");
  silProg->addUniform("P");
  silProg->addUniform("T");
  silProg->addUniform("texture");

  transProg = make_shared<Program>();
  transProg->setShaderNames(RESOURCE_DIR + "vert.glsl",
                            RESOURCE_DIR + "fragTrans.glsl");
  transProg->setVerbose(true);
  transProg->init();
  transProg->addAttribute("aPos");
  transProg->addAttribute("aNor");
  transProg->addAttribute("aTex");
  transProg->addUniform("MV");
  transProg->addUniform("P");

  // Set camera
  camera = make_shared<Camera>();

  // Load meshes
  bunny = make_shared<Shape>();
  bunny->loadMesh(RESOURCE_DIR + "bunny.obj");
  bunny->fitToUnitBox();
  bunny->init();

  sphere = make_shared<Shape>();
  sphere->loadMesh(RESOURCE_DIR + "sphere.obj");
  sphere->fitToUnitBox();
  sphere->init();

  teapot = make_shared<Shape>();
  teapot->loadMesh(RESOURCE_DIR + "teapot.obj");
  teapot->fitToUnitBox();
  teapot->init();

  // Texture
  gridTex = make_shared<Texture>();
  gridTex->setFilename(RESOURCE_DIR + "grid.jpg");
  gridTex->init();
  gridTex->setUnit(1);
  gridTex->setWrapModes(GL_REPEAT, GL_REPEAT);

  // Create our three world objects (including octrees)
  // todo: parallize this
  auto o = make_shared<WorldObject>(bunny, sphere, prog, silProg, transProg,
                                    Vector3f(-2.0f, -1.0f, 0.0f) // Position
                                    ,
                                    Vector3f(0.01f, 0.001f, 0.0f) // Velocity
                                    ,
                                    keyToggles);
  o->init();
  objs.push_back(o);
  o = make_shared<WorldObject>(teapot, sphere, prog, silProg, transProg,
                               Vector3f(2.0f, -0.8f, 0.0f) // Position
                               ,
                               Vector3f(-0.01f, 0.0005f, 0.0f) // Velocity
                               ,
                               keyToggles);
  o->init();
  objs.push_back(o);
  o = make_shared<WorldObject>(bunny, sphere, prog, silProg, transProg,
                               Vector3f(0.0f, 1.0f, 0.0f) // Position
                               ,
                               Vector3f(0.0f, -0.005f, 0.0f) // Velocity
                               ,
                               keyToggles);
  o->init();
  objs.push_back(o);

  GLSL::checkError(GET_FILE_LINE);
}

// This function is called every frame to draw the scene.
static void render() {
  // Clear framebuffer.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (keyToggles[(unsigned)'c']) {
    glDisable(GL_CULL_FACE);
  } else {
    glEnable(GL_CULL_FACE);
  }
  if (keyToggles[(unsigned)'g']) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  // Get current frame buffer size.
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  camera->setAspect((float)width / (float)height);

  // Draw our objects
  silProg->bind();
  gridTex->bind(silProg->getUniform("texture"));
  silProg->unbind();
  for (auto it = objs.begin(); it != objs.end(); ++it)
    (*it)->draw(camera);

  GLSL::checkError(GET_FILE_LINE);
}

// Execute a movement step for the objects, if the user did not pause and there
// is no collision
void step() {
  // Move every object and check for collisions
  if (keyToggles[(unsigned)' '] && !collision) {
    for (size_t current = 0; current < objs.size(); ++current) {
      objs.at(current)->move();
      for (size_t next = current + 1; next < objs.size(); ++next)
        objs.at(current)->collisionDetection(objs.at(next));
    }

    collision = true;
    for (size_t current = 0; current < objs.size(); ++current)
      collision &= objs.at(current)->getColliding();
    keyToggles[(unsigned)' '] = !collision;
  }

  // If the current scene stopped, because every object was colliding, and the
  // user presses space again, then reset the scene.
  if (keyToggles[(unsigned)' '] && collision) {
    keyToggles[(unsigned)' '] = false;
    for (auto it = objs.begin(); it != objs.end(); ++it)
      (*it)->init();
    collision = false;
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    cout << "Please specify the resource directory." << endl;
    return 0;
  }
  RESOURCE_DIR = argv[1] + string("/");

  // Set error callback.
  glfwSetErrorCallback(error_callback);
  // Initialize the library.
  if (!glfwInit()) {
    return -1;
  }
  // Create a windowed mode window and its OpenGL context.
  window = glfwCreateWindow(640, 480, "Sphere Octree", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }
  // Make the window's context current.
  glfwMakeContextCurrent(window);
  // Initialize GLEW.
  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    cerr << "Failed to initialize GLEW" << endl;
    return -1;
  }
  glGetError(); // A bug in glewInit() causes an error that we can safely
                // ignore.
  cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
  cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
  GLSL::checkVersion();
  // Set vsync.
  glfwSwapInterval(1);
  // Set keyboard callback.
  glfwSetKeyCallback(window, key_callback);
  // Set char callback.
  glfwSetCharCallback(window, char_callback);
  // Set cursor position callback.
  glfwSetCursorPosCallback(window, cursor_position_callback);
  // Set mouse button callback.
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  // Set the window resize call back.
  glfwSetFramebufferSizeCallback(window, resize_callback);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Initialize scene.
  cout << "Creating scene and objects with sphere-octrees." << endl;
  init();
  // Loop until the user closes the window.
  while (!glfwWindowShouldClose(window)) {
    step();
    // Render scene.
    render();
    // Swap front and back buffers.
    glfwSwapBuffers(window);
    // Poll for and process events.
    glfwPollEvents();
  }
  // Quit program.
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
