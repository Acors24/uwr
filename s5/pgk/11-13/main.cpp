#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <vector>
#include <iostream>

#include <AGL3Window.hpp>

#include "Player.hpp"
#include "Obstacle.hpp"
#include "Camera.hpp"
#include "Box.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ==========================================================================
// Window Main Loop Inits ...................................................
// ==========================================================================
class MyWin : public AGLWindow
{
public:
   MyWin(){};
   MyWin(int _wd, int _ht, const char *name, int vers, int fullscr = 0)
       : AGLWindow(_wd, _ht, name, vers, fullscr){};
   virtual void KeyCB(int key, int scancode, int action, int mods);
   void MainLoop(const int N);
};

// ==========================================================================
void MyWin::KeyCB(int key, int scancode, int action, int mods)
{
   AGLWindow::KeyCB(key, scancode, action, mods); // f-key full screen switch
   if ((key == GLFW_KEY_SPACE) && action == GLFW_PRESS)
   {
      ; // do something
   }
   if (key == GLFW_KEY_HOME && (action == GLFW_PRESS))
   {
      ; // do something
   }
}

int *width, *height;

void callback(GLFWwindow *window, int width, int height)
{
   *::width = width;
   *::height = height;
}

Camera main_cam = Camera(glm::vec3(0.0f), 45.0f, 35.0f);

void processInputs(GLFWwindow *window, float deltaTime, std::vector<Obstacle> &obstacles, Box &box, Goal &goal, const Player &player, bool &finished)
{
   if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      main_cam.processKeyboard(FORWARD, deltaTime, obstacles, box, goal, player, finished);
   if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      main_cam.processKeyboard(BACKWARD, deltaTime, obstacles, box, goal, player, finished);
   if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      main_cam.processKeyboard(LEFT, deltaTime, obstacles, box, goal, player, finished);
   if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      main_cam.processKeyboard(RIGHT, deltaTime, obstacles, box, goal, player, finished);
   if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
      main_cam.processKeyboard(UP, deltaTime, obstacles, box, goal, player, finished);
   if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
      main_cam.processKeyboard(DOWN, deltaTime, obstacles, box, goal, player, finished);

   if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
      main_cam.processKeyboard(LOOK_UP, deltaTime, obstacles, box, goal, player, finished);
   if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
      main_cam.processKeyboard(LOOK_DOWN, deltaTime, obstacles, box, goal, player, finished);
   if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
      main_cam.processKeyboard(LOOK_LEFT, deltaTime, obstacles, box, goal, player, finished);
   if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
      main_cam.processKeyboard(LOOK_RIGHT, deltaTime, obstacles, box, goal, player, finished);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
   main_cam.processScroll(xoffset, yoffset);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
   static bool first = true;
   static float lastX = 0, lastY = 0;

   if (first)
   {
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

void MyWin::MainLoop(const int N)
{
   width = &wd;
   height = &ht;
   ViewportOne(0, 0, wd, ht);
   glfwSetWindowSizeCallback(win(), callback);
   glfwSetInputMode(win(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
   if (glfwRawMouseMotionSupported())
      glfwSetInputMode(win(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
   glClearColor(0.15, 0.15, 0.15, 1.0);
   glfwSetCursorPosCallback(win(), mouse_callback);
   glfwSetScrollCallback(win(), scroll_callback);
   glEnable(GL_DEPTH_TEST);

   AGLErrors("main-init");

   Box box(0.53f);
   Camera mini_cam = Camera(glm::vec3(0.5f), -135.0f, -35.0f);
   float radius = 1.0 / N / 5.0;
   Player player = Player({radius, radius, radius}, radius);
   main_cam.position = player.position;

   std::vector<Obstacle> obstacles;
   obstacles.reserve(N * N * N);

   Goal goal = Goal(1, 1, 1);

   if (N != 1)
   {
      for (float x = 0; x < N; x++)
      {
         for (float y = 0; y < N; y++)
         {
            for (float z = 0; z < N; z++)
            {
               obstacles.emplace_back(x / (N - 1), y / (N - 1), z / (N - 1));
            }
         }
      }
      obstacles[0].setPosition(0.5, -1, 0.5);
      obstacles[obstacles.size() - 1].setPosition(0.5, -1, 0.5);
   }
   else
   {
      goal.setPosition(0.5, 0.5, 0.5);
   }

   AGLErrors("main-instancing");

   const float scale = 1.0 / (2 * N);
   Obstacle::scale = scale;

   float deltaTime;
   float currentTime = 0.0f;
   float lastTime = glfwGetTime();
   float collapse_time = 0.0f;
   bool finished = false;

   do
   {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      currentTime = glfwGetTime();
      deltaTime = currentTime - lastTime;
      lastTime = currentTime;
      // finished = true;
      if (finished)
         collapse_time += deltaTime;

      mini_cam.yaw += 10 * deltaTime;
      if (finished)
      {
         mini_cam.yaw += 10 * collapse_time;
      }      
      mini_cam.updateCameraVectors();

      processInputs(win(), deltaTime, obstacles, box, goal, player, finished);
      player.position = main_cam.position;

      glm::mat4 viewMatrix = main_cam.getViewMatrix();
      glm::mat4 perspProjectionMatrix = glm::perspective(glm::radians(main_cam.fov), (float)wd / (float)ht, 0.001f, 3.0f);
      glm::mat4 miniViewMatrix = mini_cam.getViewMatrix();
      glm::mat4 orthoProjectionMatrix = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

      AGLErrors("main-loopbegin");

      // =====================================================        Drawing

      glViewport(0, 0, wd, ht);

      box.draw(viewMatrix, perspProjectionMatrix, collapse_time, true);

      for (int i = 0; i < obstacles.size(); i++)
         obstacles[i].draw(viewMatrix, perspProjectionMatrix, collapse_time);

      goal.draw(viewMatrix, perspProjectionMatrix, collapse_time, false, true);
       
      /* MINI */
      
      glClear(GL_DEPTH_BUFFER_BIT);
      const float miniScale = 1.0f / 2;
      int minLength = wd < ht ? wd : ht;
      glViewport(wd - minLength * miniScale, 0, minLength * miniScale, minLength * miniScale);

      box.draw(miniViewMatrix, orthoProjectionMatrix, collapse_time, false);

      for (int i = 0; i < obstacles.size(); i++)
         obstacles[i].draw(miniViewMatrix, orthoProjectionMatrix, collapse_time, true, false);

      goal.draw(miniViewMatrix, orthoProjectionMatrix, collapse_time, true, false);

      player.draw(miniViewMatrix, orthoProjectionMatrix);

      AGLErrors("main-afterdraw");
      glfwSwapBuffers(win()); // =============================   Swap buffers
      glfwPollEvents();
      // glfwWaitEvents();

   } while (glfwGetKey(win(), GLFW_KEY_ESCAPE) != GLFW_PRESS &&
            glfwWindowShouldClose(win()) == 0 && collapse_time < 1.2f);
}

int main(int argc, char *argv[])
{
   MyWin win;
   win.Init(1280, 720, "Labirynt 3D", 0, 33);

   int seed = time(0);
   int N = 10;

   if (argc > 1)
      seed = std::stoi(argv[1]);

   srand(seed);

   if (argc > 2)
      N = std::stoi(argv[2]);

   win.MainLoop(N);
   return 0;
}
