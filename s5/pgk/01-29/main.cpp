#include <iostream>

#include "AGL3Window.hpp"
#include "Camera.hpp"
#include "Model.hpp"
#include "Util.hpp"

// ==========================================================================
// Window Main Loop Inits ...................................................
// ==========================================================================
class MyWin : public AGLWindow {
public:
  MyWin(){};
  MyWin(int _wd, int _ht, const char *name, int vers, int fullscr = 0)
      : AGLWindow(_wd, _ht, name, vers, fullscr){};
  virtual void KeyCB(int key, int scancode, int action, int mods);
  void MainLoop(const std::string &path);
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

Camera main_cam;

void processInputs(GLFWwindow *window, float deltaTime) {
  static bool tabPressed = false;
  static bool mPressed = false;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    main_cam.processKeyboard(Key::W, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    main_cam.processKeyboard(Key::S, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    main_cam.processKeyboard(Key::A, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    main_cam.processKeyboard(Key::D, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    main_cam.processKeyboard(Key::W, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    main_cam.processKeyboard(Key::S, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    main_cam.processKeyboard(Key::A, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    main_cam.processKeyboard(Key::D, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    main_cam.processKeyboard(Key::SPACE, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    main_cam.processKeyboard(Key::LEFT_CONTROL, deltaTime);

  if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
    if (!tabPressed) {
      tabPressed = true;
    }
  } else {
    if (tabPressed) {
      tabPressed = false;
    }
  }

  if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
    if (!mPressed) {
      mPressed = true;
    }
  } else {
    if (mPressed) {
      mPressed = false;
    }
  }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  main_cam.processScroll(xoffset, yoffset);
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

  main_cam.processMouseMovement(xoffset, yoffset);
}

void MyWin::MainLoop(const std::string &path) {
  width = &wd;
  height = &ht;
  ViewportOne(0, 0, wd, ht);
  glfwSetWindowSizeCallback(win(), callback);
  glfwSetInputMode(win(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  if (glfwRawMouseMotionSupported())
    glfwSetInputMode(win(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  glClearColor(0.4f, 0.3f, 0.3f, 1.0f);
  glfwSetCursorPosCallback(win(), mouse_callback);
  glfwSetScrollCallback(win(), scroll_callback);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_PRIMITIVE_RESTART);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  AGLErrors("main-init");

  main_cam = Camera(glm::vec3(-2.0f, 2.0f, 2.0f), -45.0f, -30.0f);

  Model model = Model(path);

  AGLErrors("main-instancing");

  float deltaTime;
  float currentTime = 0.0f;
  float lastTime = glfwGetTime();

  do {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    processInputs(win(), deltaTime);

    glm::mat4 viewMatrix = main_cam.getViewMatrix();
    glm::mat4 perspProjectionMatrix = glm::perspective(
        glm::radians(main_cam.fov), (float)wd / (float)ht, 0.001f, 50.0f);

    AGLErrors("main-loopbegin");

    // =====================================================        Drawing
    // Main view -------------------------------------------

    glViewport(0, 0, wd, ht);

    model.draw(viewMatrix, perspProjectionMatrix);


    AGLErrors("main-afterdraw");
    glfwSwapBuffers(win()); // =============================   Swap buffers
    glfwPollEvents();
    // glfwWaitEvents();

  } while (glfwGetKey(win(), GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(win()) == 0);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Brak podanej ścieżki do pliku .obj." << std::endl;
    return 1;
  }

  const std::string path = argv[1];
  std::string windowName = "Wyświetlacz .obj - " + path;

  MyWin win;
  win.Init(1280, 720, windowName.c_str(), 0, 33);
  win.MainLoop(path);
  return 0;
}
