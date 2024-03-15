#include <glm/geometric.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/trigonometric.hpp>
#include <stdlib.h>
#include <time.h>

#include "AGL3Window.hpp"

#include "Bubble.hpp"
#include "Camera.hpp"
#include "Player.hpp"
#include "Sun.hpp"
#include "Volcano.hpp"

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
  void MainLoop();
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

Player *player;
Camera *main_cam;
bool miniMapVisible = true;

void processInputs(GLFWwindow *window, float deltaTime) {
  static bool tabPressed = false;
  static bool mPressed = false;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    player->processKeyboard(Key::W, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    player->processKeyboard(Key::S, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    player->processKeyboard(Key::A, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    player->processKeyboard(Key::D, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    player->processKeyboard(Key::W, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    player->processKeyboard(Key::S, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    player->processKeyboard(Key::A, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    player->processKeyboard(Key::D, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    player->processKeyboard(Key::SPACE, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    player->processKeyboard(Key::LEFT_CONTROL, deltaTime);

  if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
    if (!tabPressed) {
      player->processKeyboard(Key::TAB, deltaTime);
      tabPressed = true;
    }
  } else {
    if (tabPressed) {
      tabPressed = false;
    }
  }

  if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
    if (!mPressed) {
      miniMapVisible = !miniMapVisible;
      mPressed = true;
    }
  } else {
    if (mPressed) {
      mPressed = false;
    }
  }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  main_cam->processScroll(xoffset, yoffset);
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

  main_cam->processMouseMovement(xoffset, yoffset);
}

void spawnPlayer() {
  float distanceAwayFromCenter = 19.0f;
  float angle = 2.0f * M_PI * rand() / RAND_MAX;
  player->position = glm::vec3(std::cos(angle) * distanceAwayFromCenter, 1.0f,
                               std::sin(angle) * distanceAwayFromCenter);
  player->yaw = main_cam->yaw = glm::degrees(angle + M_PI);
  main_cam->pitch = 0.0f;
  player->tpCamera = true;
  player->updateCameraPosition();
  main_cam->processMouseMovement(0, 0);
}

void MyWin::MainLoop() {
  width = &wd;
  height = &ht;
  ViewportOne(0, 0, wd, ht);
  glfwSetWindowSizeCallback(win(), callback);
  glfwSetInputMode(win(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  if (glfwRawMouseMotionSupported())
    glfwSetInputMode(win(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  //   glClearColor(0.3f, 0.4f, 0.8f, 1.0);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glfwSetCursorPosCallback(win(), mouse_callback);
  glfwSetScrollCallback(win(), scroll_callback);
  glEnable(GL_DEPTH_TEST);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  AGLErrors("main-init");

  player = new Player(glm::vec3(0.0f), 0.0f);
  main_cam = new Camera(player);
  Volcano volcano = Volcano();
  Sun sun = Sun();

  Camera mini_cam = Camera(glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, -90.0f);

  Bubble::init();

  glm::mat4 orthoProjectionMatrix =
      glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, -20.0f, 2.0f);

  AGLErrors("main-instancing");

  float deltaTime;
  float currentTime = 0.0f;
  float lastTime = glfwGetTime();

  for (int i = 0; i < 1000; i++)
      Bubble::updateAll(0.1f, main_cam->position);

  spawnPlayer();
  int finishes = 0;

  do {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    processInputs(win(), deltaTime);
    player->update(deltaTime, Bubble::positions, volcano);
    {
      glm::vec3 distanceVec = player->position;
      if (glm::length2(distanceVec) > 19.0f * 19.0f)
        player->position = glm::normalize(distanceVec) * 19.0f;

      const float altitude_limit = 10.0f;
      if (player->position.y > altitude_limit)
        player->position.y = altitude_limit;
    }
    sun.position = main_cam->position + 1.0f;

    if (glm::distance2(player->position, glm::vec3(0.0f, 1.0f, 0.0f)) < 2.0f ||
        glfwGetKey(win(), GLFW_KEY_F) == GLFW_PRESS) {
      spawnPlayer();
      finishes++;
      player->points++;
      Bubble::difficulty++;
    }

    Bubble::updateAll(deltaTime, main_cam->position);

    glm::mat4 viewMatrix = main_cam->getViewMatrix();
    glm::mat4 perspProjectionMatrix = glm::perspective(
        glm::radians(main_cam->fov), (float)wd / (float)ht, 0.001f, 50.0f);
    glm::mat4 miniViewMatrix = mini_cam.getViewMatrix();

    AGLErrors("main-loopbegin");

    // =====================================================        Drawing
    // Main view -------------------------------------------

    glViewport(0, 0, wd, ht);

    sun.draw(viewMatrix, perspProjectionMatrix);

    glClear(GL_DEPTH_BUFFER_BIT);

    glm::vec3 lightPositions[10];
    for (int i = 0; i < 10; i++)
      lightPositions[i] = Bubble::positions[i];

    volcano.draw(viewMatrix, perspProjectionMatrix, main_cam->position,
                 lightPositions, player->position, true);

    if (player->tpCamera)
      player->draw(viewMatrix, perspProjectionMatrix, false);

    Bubble::drawAll(viewMatrix, perspProjectionMatrix, main_cam->position,
                    player->position, true);

    // Mini view ---------------------------------------------

    if (miniMapVisible) {
      glClear(GL_DEPTH_BUFFER_BIT);
      const float miniScale = 1.0f / 2;
      int minLength = wd < ht ? wd : ht;
      glViewport(wd - minLength * miniScale, 0, minLength * miniScale,
                 minLength * miniScale);

      volcano.draw(miniViewMatrix, orthoProjectionMatrix, mini_cam.position,
                   lightPositions, player->position, false);

      player->draw(miniViewMatrix, orthoProjectionMatrix, true);

      Bubble::drawAll(miniViewMatrix, orthoProjectionMatrix, mini_cam.position,
                      player->position, false);
    }

    AGLErrors("main-afterdraw");
    glfwSwapBuffers(win()); // =============================   Swap buffers
    glfwPollEvents();
    // glfwWaitEvents();

  } while (glfwGetKey(win(), GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(win()) == 0 && !player->dead);

  delete player;
  delete main_cam;

  std::cout << "Ukończono " << finishes << " razy w " << glfwGetTime()
            << " sekund." << std::endl;
  std::cout << "Zdobyto " << player->points << " punktów." << std::endl;
}

int main(int argc, char *argv[]) {
  MyWin win;
  win.Init(1280, 720, "Wulkan", 0, 33);

  int seed = time(0);
  srand(seed);

  win.MainLoop();
  return 0;
}
