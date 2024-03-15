#include <cstring>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/trigonometric.hpp>
#include <iomanip>
#include <ios>
#include <iostream>
#include <string>

#include "AGL3Window.hpp"

#include "Camera.hpp"
#include "EarthGrid.hpp"
#include "Map.hpp"
#include "Crosshair.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ==========================================================================
// Window Main Loop Inits ...................................................
// ==========================================================================
class MyWin : public AGLWindow {
public:
  MyWin(){};
  MyWin(int _wd, int _ht, const char *name, int vers, int fullscr = 0)
      : AGLWindow(_wd, _ht, name, vers, fullscr){};
  virtual void KeyCB(int key, int scancode, int action, int mods);
  void MainLoop(const std::string &dirName, int x1, int x2, int y1, int y2,
                float startLon, float startLat, float startAlt);
};

// ==========================================================================
void MyWin::KeyCB(int key, int scancode, int action, int mods) {
  AGLWindow::KeyCB(key, scancode, action, mods); // f-key full screen switch
  if ((key == GLFW_KEY_SPACE) && action == GLFW_PRESS) {
    ; // do something
  }
  if (key == GLFW_KEY_HOME && (action == GLFW_PRESS)) {
    ; // do something
  }
}

int *width, *height;

void callback(GLFWwindow *window, int width, int height) {
  *::width = width;
  *::height = height;
}

Camera camera;
int appLOD = LOD::AUTO;

void processInputs(GLFWwindow *window, float deltaTime, bool flat) {
  static int tab_held = 0;

  bool shiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.processKeyboard(Key::W, deltaTime, flat, shiftPressed);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.processKeyboard(Key::S, deltaTime, flat, shiftPressed);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.processKeyboard(Key::A, deltaTime, flat, shiftPressed);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.processKeyboard(Key::D, deltaTime, flat, shiftPressed);
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    camera.processKeyboard(Key::W, deltaTime, flat, shiftPressed);
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    camera.processKeyboard(Key::S, deltaTime, flat, shiftPressed);
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    camera.processKeyboard(Key::A, deltaTime, flat, shiftPressed);
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    camera.processKeyboard(Key::D, deltaTime, flat, shiftPressed);
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    camera.processKeyboard(Key::SPACE, deltaTime, flat, shiftPressed);
  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    camera.processKeyboard(Key::LEFT_CONTROL, deltaTime, flat, shiftPressed);

  if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    appLOD = LOD::NATIVE;
  else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    appLOD = LOD::HIGH;
  else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    appLOD = LOD::MEDIUM;
  else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
    appLOD = LOD::LOW;
  else if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
    appLOD = LOD::AWFUL;
  else if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
    appLOD = LOD::POTATO;
  else if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
    appLOD = LOD::NONE;
  else if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
    appLOD = LOD::USOS;
  else if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
    appLOD = LOD::AUTO;

  if (appLOD != LOD::AUTO)
    Map::lod = appLOD;

  if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
    if (!tab_held) {
      Map::toggleFlat();
      camera.setMode(Map::isFlat());
      tab_held = 1;
    }
  } else {
    tab_held = 0;
  }
}

float mapScale = 10.0f;

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  camera.processScroll(xoffset, yoffset, Map::isFlat());
  if (Map::isFlat()) {
    mapScale += yoffset * 0.2f;
    if (mapScale < 0.1f)
      mapScale = 0.1f;
    if (mapScale > 40.0f)
      mapScale = 40.0f;
  }
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  static bool first = true;
  static float lastX = 0, lastY = 0;

  if (first) {
    lastX = xpos;
    lastY = ypos;
    first = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = ypos - lastY;

  lastX = xpos;
  lastY = ypos;

  bool shiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
  camera.processMouseMovement(xoffset, yoffset, Map::isFlat(), shiftPressed,
                              true);
}

void MyWin::MainLoop(const std::string &dirName, int x1, int x2, int y1, int y2,
                     float startLon, float startLat, float startAlt) {
  width = &wd;
  height = &ht;
  ViewportOne(0, 0, wd, ht);
  glfwSetWindowSizeCallback(win(), callback);
  glfwSetInputMode(win(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  if (glfwRawMouseMotionSupported())
    glfwSetInputMode(win(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
  glfwSetCursorPosCallback(win(), mouse_callback);
  glfwSetScrollCallback(win(), scroll_callback);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_PRIMITIVE_RESTART);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  AGLErrors("main-init");

  Map::create(dirName, x1, x2, y1, y2);

  float mapWidth = Map::max_lon - Map::min_lon;
  float mapHeight = Map::max_lat - Map::min_lat;

  float cam_x, cam_y, cam_z;
  EarthGrid::sphericalToCartesian(
      glm::radians(startLon), glm::radians(startLat),
      EarthGrid::EARTH_RADIUS + startAlt, cam_x, cam_y, cam_z);

  camera = Camera(glm::vec3(cam_x, cam_y, cam_z));
  camera.longitude = startLon;
  camera.latitude = startLat;
  camera.altitude = startAlt;
  camera.setMode(Map::isFlat());

  EarthGrid grid;
  Crosshair crosshair;

  AGLErrors("main-instancing");

  float deltaTime;
  float currentTime = glfwGetTime();
  float lastTime = currentTime - 1;

  do {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    processInputs(win(), deltaTime, Map::isFlat());

    glm::mat4 viewMatrix = camera.getViewMatrix();

    AGLErrors("main-loopbegin");

    // =====================================================        Drawing
    glViewport(0, 0, wd, ht);

    float aspect = (float)wd / (float)ht;

    int rendered = 0;

    if (Map::isFlat()) {
      glm::mat4 orthoProjectionMatrix =
          glm::ortho(-mapScale * aspect, mapScale * aspect, -mapScale, mapScale,
                     -10000.0f, 10000.0f);
      Map::drawAll(viewMatrix, orthoProjectionMatrix, rendered);
      glClear(GL_DEPTH_BUFFER_BIT);
      crosshair.draw();
    } else {
      glm::mat4 perspProjectionMatrix =
          glm::perspective(glm::radians(camera.fov), aspect, 0.001f, 100000.0f);
      Map::drawAll(viewMatrix, perspProjectionMatrix, rendered);
      grid.draw(viewMatrix, perspProjectionMatrix);
    }

    AGLErrors("main-afterdraw");

    int fps = 1 / deltaTime;
    if (appLOD == LOD::AUTO) {
      if (fps < 10 && Map::lod < LOD::USOS)
        Map::lod++;
      if (fps > 15 && Map::lod > LOD::NATIVE)
        Map::lod--;
    }

    if (true) {
      std::cout << std::setw(10) << std::right << std::scientific
                << std::setprecision(3) << rendered / deltaTime
                << " trójkątów/s. " << std::setw(4) << std::right << fps
                << " FPS";
      std::cout << " | " << std::fixed << camera.longitude << " "
                << camera.latitude;
      std::cout << "      \r" << std::flush;
    }

    glfwSwapBuffers(win()); // =============================   Swap buffers
    glfwPollEvents();
    // glfwWaitEvents();

  } while (glfwGetKey(win(), GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(win()) == 0);

  Map::destroy();
}

int main(int argc, char *argv[]) {
  MyWin win;
  win.Init(1280, 720, "Teren", 0, 33);

  if (argc < 2) {
    std::cout << "Użycie:\n\t./main <katalog/z/plikami/hgt> [-lon <x1> <x2>] "
                 "[-lat <y1> <y2>] [-start <lon> <lat> <alt>]\n";
    std::cout << "Współrzędne geograficzne podawane są w stopniach.\n";
    std::cout << "Wysokość podawana jest w kilometrach n.p.m..\n";
    std::cout << "Kolejność argumentów nie ma znaczenia.\n";

    return 1;
  }

  std::string dirName = argv[1];
  int x1 = -180, x2 = 180;
  int y1 = -90, y2 = 90;
  float startLon = 0.0f, startLat = 0.0f, startAlt = 1.0f;

  if (argc > 2) {
    char **lonIndex = argv + 2;
    while (lonIndex != argv + argc && std::strcmp(*lonIndex, "-lon"))
      lonIndex++;
    if (lonIndex != argv + argc) {
      x1 = std::stoi(lonIndex[1]);
      x2 = std::stoi(lonIndex[2]);
    }

    char **latIndex = argv + 2;
    while (latIndex != argv + argc && std::strcmp(*latIndex, "-lat"))
      latIndex++;
    if (latIndex != argv + argc) {
      y1 = std::stoi(latIndex[1]);
      y2 = std::stoi(latIndex[2]);
    }

    char **startIndex = argv + 2;
    while (startIndex != argv + argc && std::strcmp(*startIndex, "-start"))
      startIndex++;
    if (startIndex != argv + argc) {
      startLon = std::stof(startIndex[1]);
      startLat = std::stof(startIndex[2]);
      startAlt = std::stof(startIndex[3]);
    }
  }

  win.MainLoop(dirName, x1, x2, y1, y2, startLon, startLat, startAlt);
  return 0;
}
