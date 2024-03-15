// ==========================================================================
// AGL3:  GL/GLFW init AGLWindow and AGLDrawable class definitions
//
// Ver.3  14.I.2020 (c) A. Łukaszewski
// ==========================================================================
// AGL3 example usage
//===========================================================================
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include <AGL3Window.hpp>
#include <AGL3Drawable.hpp>

struct Vec2
{
   float x, y;

   Vec2(float x = 0, float y = 0) : x(x), y(y) {}

   Vec2 operator-() const
   {
      return Vec2(-x, -y);
   }

   Vec2 operator+(const Vec2 &other) const
   {
      return Vec2(x + other.x, y + other.y);
   }

   Vec2 operator-(const Vec2 &other) const
   {
      return Vec2(x - other.x, y - other.y);
   }

   float dot(const Vec2 &other) const
   {
      return x * other.x + y * other.y;
   }

   float length() const
   {
      return std::sqrt(x * x + y * y);
   }

   float angle(const Vec2 &other) const
   {
      return std::acos(dot(other) / length() / other.length());
   }

   static bool in_triangle(const Vec2 p, const Vec2 tri[])
   {
      auto v0 = tri[0] - p;
      auto v1 = tri[1] - p;
      auto v2 = tri[2] - p;

      auto sum = v0.angle(v1) + v1.angle(v2) + v2.angle(v0);

      return std::abs(sum - M_PI * 2) < 1e-6;
   }

   static bool collision(const Vec2 tri1[], const Vec2 tri2[])
   {
      for (int i = 0; i < 3; i++)
         if (in_triangle(tri1[i], tri2) || in_triangle(tri2[i], tri1))
            return true;

      return false;
   }
};

std::ostream &operator<<(std::ostream &os, const Vec2 vec)
{
   return os << '(' << vec.x << ", " << vec.y << ')';
}

class Triangle : public AGLDrawable
{
protected:
   GLfloat color[3] = {1.0, 1.0, 1.0};

public:
   const Vec2 verts[3] = {
       {1, 0},
       {-1, 0.5},
       {-1, -0.5}};
   Vec2 pos;
   float angle;
   static GLfloat *collapse_time;

   Triangle() : AGLDrawable(0)
   {
      setShaders();
      setBuffers();

      angle = 0;
      pos = {0, 0};
   }

   void setShaders()
   {
      compileShaders(R"END(

         #version 330 
         #extension GL_ARB_explicit_uniform_location : require
         #extension GL_ARB_shading_language_420pack : require
         layout(location = 0) in vec2 pos;
         layout(location = 0) uniform float scale;
         layout(location = 1) uniform vec2  center;
         layout(location = 2) uniform float angle;
         layout(location = 4) uniform float c_time;
         out vec4 vcolor;


         void main(void) {
            const vec4 colors[]    = vec4[3](vec4(1, 1, 1, 1),
                                             vec4(0, 0, 0, 1),
                                             vec4(0, 0, 0, 1));

            float x = (pos.x * cos(angle) - pos.y * sin(angle)) * scale + center.x;
            float y = (pos.x * sin(angle) + pos.y * cos(angle)) * scale + center.y;
            // float c_fac = pow(5 * c_time + 1, -c_time);
            float c_fac = max(0, log(-c_time + 1) + 1);
            gl_Position = vec4(x * c_fac, y * c_fac, 0.0, 1.0);
            vcolor = colors[gl_VertexID];
         }

      )END",
                     R"END(

         #version 330 
         #extension GL_ARB_explicit_uniform_location : require
         layout(location = 3) uniform vec4  tri_color;
         layout(location = 4) uniform float c_time;
         in vec4 vcolor;
         out vec4 color;

         void main(void) {
            // color = vec4(tri_color, 1.0);
            // color = vcolor;
            if (c_time < 0.001)
            {
               color = vec4(mix(tri_color.rgb, vcolor.rgb, tri_color.a), 1);
            }
            else
            {
               color = vec4(sin(c_time * 3 * gl_FragCoord.x / 200) / 2 + 0.5,
                            sin(c_time * 7 * gl_FragCoord.y / 200) / 2 + 0.5,
                            sin(c_time * 11 * gl_FragCoord.x * gl_FragCoord.y / 40000) / 2 + 0.5, 1);
            }
         } 

      )END");
   }

   void setBuffers()
   {
      bindBuffers();

      glBufferData(GL_ARRAY_BUFFER, 3 * 2 * sizeof(float), verts, GL_STATIC_DRAW);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(
          0,        // attribute 0, must match the layout in the shader.
          2,        // size
          GL_FLOAT, // type
          GL_FALSE, // normalized?
          0,        // 24,             // stride
          (void *)0 // array buffer offset
      );
   }

   void draw()
   {
      bindProgram();
      bindBuffers();
      glUniform1f(0, 1.0 / 16);     // scale  in vertex shader
      glUniform2f(1, pos.x, pos.y); // center in vertex shader
      glUniform1f(2, angle);
      glUniform4f(3, color[0], color[1], color[2], 1);
      glUniform1f(4, *collapse_time);

      glDrawArrays(GL_TRIANGLES, 0, 3);
   }

   void set_color(GLfloat r, GLfloat g, GLfloat b)
   {
      color[0] = r;
      color[1] = g;
      color[2] = b;
   }
};

class Obstacle : public Triangle
{
public:
   Obstacle(float x = 0.0, float y = 0.0) : Triangle()
   {
      setShaders();
      setBuffers();

      angle = std::rand() / (float)RAND_MAX * M_PI * 2;
      pos = {x, y};
      set_color(1, 0, 0);
   }

   void draw()
   {
      bindProgram();
      bindBuffers();
      glUniform1f(0, 1.0 / 16);     // scale  in vertex shader
      glUniform2f(1, pos.x, pos.y); // center in vertex shader
      glUniform1f(2, angle);
      glUniform4f(3, color[0], color[1], color[2], 0.5);
      glUniform1f(4, *collapse_time);

      glDrawArrays(GL_TRIANGLES, 0, 3);
   }
};

class Goal : public Triangle
{
public:
   Goal() : Triangle()
   {
      pos = {0.9, 0.9};
      set_color(0, 0.5, 1);
   }

   void draw()
   {
      bindProgram();
      bindBuffers();
      glUniform1f(0, 1.0 / 16);     // scale  in vertex shader
      glUniform2f(1, pos.x, pos.y); // center in vertex shader
      glUniform1f(2, angle);
      glUniform4f(3, color[0], color[1], color[2], 0.7);
      glUniform1f(4, *collapse_time);

      glDrawArrays(GL_TRIANGLES, 0, 3);
   }
};

class Player : public Triangle
{
private:
   const float ROT_SPEED = 1;
   const float MOVE_SPEED = 0.5;

public:
   Player() : Triangle()
   {
      setShaders();
      setBuffers();

      pos = {-0.9, -0.9};
      set_color(0, 1, 0);
   }

   void draw()
   {
      bindProgram();
      bindBuffers();
      glUniform1f(0, 1.0 / 16);     // scale  in vertex shader
      glUniform2f(1, pos.x, pos.y); // center in vertex shader
      glUniform1f(2, angle);
      glUniform4f(3, color[0], color[1], color[2], 0.3);
      glUniform1f(4, *collapse_time);

      glDrawArrays(GL_TRIANGLES, 0, 3);
   }

   void move(int8_t dir, Obstacle obstacles[], int N, const Goal &goal, bool *finished, double time_delta)
   {
      float delta_x = std::cos(angle) * time_delta * MOVE_SPEED * (dir < 0 ? -1 : 1);
      float delta_y = std::sin(angle) * time_delta * MOVE_SPEED * (dir < 0 ? -1 : 1);

      Vec2 real_verts[3];
      for (int i = 0; i < 3; i++)
      {
         const Vec2 &v = verts[i];

         real_verts[i] = {
             v.x * std::cos(angle) - v.y * std::sin(angle) + (pos.x + delta_x) * 16,
             v.x * std::sin(angle) + v.y * std::cos(angle) + (pos.y + delta_y) * 16};

         if (real_verts[i].x < -1 * 16 || real_verts[i].x > 1 * 16 || real_verts[i].y < -1 * 16 || real_verts[i].y > 1 * 16)
            return;
      }


      Vec2 real_obs_verts[3];
      for (int j = 0; j < N; j++)
      {
         const Obstacle &obs = obstacles[j];

         for (int k = 0; k < 3; k++)
         {
            const Vec2 &v = obs.verts[k];

            real_obs_verts[k] = {
                v.x * std::cos(obs.angle) - v.y * std::sin(obs.angle) + obs.pos.x * 16,
                v.x * std::sin(obs.angle) + v.y * std::cos(obs.angle) + obs.pos.y * 16};
         }

         if (Vec2::collision(real_verts, real_obs_verts))
            return;
      }

      Vec2 real_goal_verts[3];
      for (int k = 0; k < 3; k++)
      {
         const Vec2 &v = goal.verts[k];

         real_goal_verts[k] = {
             v.x * std::cos(goal.angle) - v.y * std::sin(goal.angle) + goal.pos.x * 16,
             v.x * std::sin(goal.angle) + v.y * std::cos(goal.angle) + goal.pos.y * 16};
      }

      if (Vec2::collision(real_verts, real_goal_verts))
         *finished = true;

      pos.x += delta_x;
      pos.y += delta_y;
   }

   void rotate(int8_t dir, Obstacle obstacles[], int N, const Goal &goal, bool *finished, double time_delta)
   {
      float delta_angle = ROT_SPEED * time_delta * (dir < 0 ? -1 : 1);

      Vec2 real_verts[3];
      for (int i = 0; i < 3; i++)
      {
         const Vec2 &v = verts[i];

         real_verts[i] = {
             v.x * std::cos(angle + delta_angle) - v.y * std::sin(angle + delta_angle) + pos.x * 16,
             v.x * std::sin(angle + delta_angle) + v.y * std::cos(angle + delta_angle) + pos.y * 16};

         if (real_verts[i].x < -1 * 16 || real_verts[i].x > 1 * 16 || real_verts[i].y < -1 * 16 || real_verts[i].y > 1 * 16)
            return;
      }

      Vec2 real_obs_verts[3];
      for (int j = 0; j < N; j++)
      {
         const Obstacle &obs = obstacles[j];

         for (int k = 0; k < 3; k++)
         {
            const Vec2 &v = obs.verts[k];

            real_obs_verts[k] = {
                v.x * std::cos(obs.angle) - v.y * std::sin(obs.angle) + obs.pos.x * 16,
                v.x * std::sin(obs.angle) + v.y * std::cos(obs.angle) + obs.pos.y * 16};
         }

         if (Vec2::collision(real_verts, real_obs_verts))
            return;
      }

      Vec2 real_goal_verts[3];
      for (int k = 0; k < 3; k++)
      {
         const Vec2 &v = goal.verts[k];

         real_goal_verts[k] = {
             v.x * std::cos(goal.angle) - v.y * std::sin(goal.angle) + goal.pos.x * 16,
             v.x * std::sin(goal.angle) + v.y * std::cos(goal.angle) + goal.pos.y * 16};
      }

      if (Vec2::collision(real_verts, real_goal_verts))
         *finished = true;

      angle += delta_angle;
   }
};

class Background : public AGLDrawable
{
public:
   static GLfloat *collapse_time;
   GLfloat bg_time;

   Background() : AGLDrawable(0)
   {
      setShaders();
      setBuffers();

      bg_time = 0;
   }

   void setShaders()
   {
      compileShaders(R"END(

         #version 330
         #extension GL_ARB_explicit_uniform_location : require
         // out vec4 vcolor;
         layout(location = 2) uniform float c_time;
         out vec2 pos;
         void main(void) {
            const vec2 vertices[4] = vec2[4](vec2(-1, -1),
                                             vec2(-1,  1),
                                             vec2( 1, -1),
                                             vec2( 1,  1));
            // const vec4 colors[]    = vec4[3](vec4(1.0, 0.0, 0.0, 1.0),
            //                                  vec4(0.0, 1.0, 0.0, 1.0),
            //                                  vec4(0.0, 0.0, 1.0, 1.0));

            // vcolor      = colors[gl_VertexID];
            pos = vertices[gl_VertexID];
            // float c_fac = pow(5 * c_time + 1, -c_time);
            float c_fac = max(0, log(-c_time + 1) + 1);
            gl_Position = vec4(pos * c_fac, 0.5, 0.5);
         }

      )END",
                     R"END(

         #version 330
         #extension GL_ARB_explicit_uniform_location : require
         // in  vec4 vcolor;
         in  vec2 pos;
         out vec4 color;
         layout(location = 1) uniform float time;

         void main(void) {
            // color = vec4((1 + pos.x) / 10, (1 + pos.y) / 10, 0.1, 0.0);
            float co = (1 + pow(sin(2 * distance(vec2(0, 0), pos.xy) - time), 0.5)) / 10;
            color = vec4(co, 0, 0, 0);
         } 

      )END");
   }

   void setBuffers()
   {
      bindBuffers();
      GLfloat verts[4][2] = {
          {-1, -1},
          {-1, 1},
          {1, -1},
          {1, 1}};

      glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), verts, GL_STATIC_DRAW);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(
          0,        // attribute 0, must match the layout in the shader.
          2,        // size
          GL_FLOAT, // type
          GL_FALSE, // normalized?
          0,        // 24,             // stride
          (void *)0 // array buffer offset
      );
   }

   void draw()
   {
      bindProgram();
      glUniform1f(1, bg_time);
      glUniform1f(2, *collapse_time);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
   }
};

GLfloat *Triangle::collapse_time;
GLfloat *Background::collapse_time;

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
   void MainLoop(unsigned int GRID_SIDE_LENGTH);
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

void callback(GLFWwindow *window, int width, int height)
{
   int size = (width < height ? width : height);
   glViewport((width - size) / 2, (height - size) / 2, size, size);
}

// ==========================================================================
void MyWin::MainLoop(unsigned int GRID_SIDE_LENGTH)
{
   ViewportOne(0, 0, wd, ht);
   glfwSetWindowSizeCallback(win(), callback);

   const uint32_t OBSTACLE_AMOUNT = GRID_SIDE_LENGTH * GRID_SIDE_LENGTH;

   Player player;
   Obstacle obstacles[OBSTACLE_AMOUNT];
   for (uint32_t i = 0; i < GRID_SIDE_LENGTH; i++)
   {
      float y = 1.8 / (GRID_SIDE_LENGTH - 1) * i - 0.9;
      for (uint32_t j = 0; j < GRID_SIDE_LENGTH; j++)
      {
         float x = 1.8 / (GRID_SIDE_LENGTH - 1) * j - 0.9;
         obstacles[i * GRID_SIDE_LENGTH + j].pos = {x, y};
      }
   }
   Goal goal;
   Background bg;

   obstacles[0].pos.x -= 100;
   obstacles[OBSTACLE_AMOUNT - 1].pos.x += 100;

   double time_delta, current_time, previous_time;

   bool finished = false;
   GLfloat collapse_time = 0;
   Player::collapse_time = &collapse_time;
   Obstacle::collapse_time = &collapse_time;
   Goal::collapse_time = &collapse_time;
   Background::collapse_time = &collapse_time;

   do
   {
      current_time = glfwGetTime();
      time_delta = current_time - previous_time;
      previous_time = current_time;
      bg.bg_time += time_delta * 3;
      if (collapse_time > 1)
         break;
      if (finished)
      {
         collapse_time += time_delta;
      }
      glClear(GL_COLOR_BUFFER_BIT);

      AGLErrors("main-loopbegin");
      // =====================================================        Drawing
      bg.draw();
      player.draw();
      for (uint32_t i = 0; i < OBSTACLE_AMOUNT; i++)
      {
         obstacles[i].draw();
      }
      goal.draw();

      AGLErrors("main-afterdraw");

      glfwSwapBuffers(win()); // =============================   Swap buffers
      glfwPollEvents();
      // glfwWaitEvents();

      if (glfwGetKey(win(), GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(win(), GLFW_KEY_LEFT) == GLFW_PRESS)
      {
         player.rotate(1, obstacles, OBSTACLE_AMOUNT, goal, &finished, time_delta);
      }
      if (glfwGetKey(win(), GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(win(), GLFW_KEY_RIGHT) == GLFW_PRESS)
      {
         player.rotate(-1, obstacles, OBSTACLE_AMOUNT, goal, &finished, time_delta);
      }

      if (glfwGetKey(win(), GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(win(), GLFW_KEY_UP) == GLFW_PRESS)
      {
         player.move(1, obstacles, OBSTACLE_AMOUNT, goal, &finished, time_delta);
      }
      if (glfwGetKey(win(), GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(win(), GLFW_KEY_DOWN) == GLFW_PRESS)
      {
         player.move(-1, obstacles, OBSTACLE_AMOUNT, goal, &finished, time_delta);
      }
      
      // for (int i = 0; i < 100000000; i++); // frame limiter 😎

   } while (glfwGetKey(win(), GLFW_KEY_ESCAPE) != GLFW_PRESS &&
            glfwWindowShouldClose(win()) == 0);
}

int main(int argc, char *argv[])
{
   unsigned int seed = time(0);
   if (argc > 1)
      seed = std::stoi(argv[1]);
   srand(seed);

   unsigned int GRID_SIDE_LENGTH = 10;
   if (argc > 2)
   {
      GRID_SIDE_LENGTH = std::stoi(argv[2]);
   }

   MyWin win;
   win.Init(600, 600, "Labirynt", 0, 33);
   win.MainLoop(GRID_SIDE_LENGTH);

   std::cout << glfwGetTime() << "s" << std::endl;
   return 0;
}
