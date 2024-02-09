#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>
#include "OpenGL/gl3.h"
#include <glm/glm.hpp>

/* ************************************************************************************************
 * **
 * **   Global State
 * **
 * ************************************************************************************************/
struct State {
};

/* ************************************************************************************************
 * **
 * **   Initialise GLFW
 * **
 * ************************************************************************************************/
#include <GLFW/glfw3.h>

GLFWwindow *init_glfw(const std::string &window_title, std::string &glsl_version) {
  if (!glfwInit()) {
    spdlog::error("Failed to init GLFW");
    throw std::runtime_error("Failed to init GLFW");
  }

  glsl_version = "#version 410";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  // Create window with graphics context
  auto window = glfwCreateWindow(1280, 720, window_title.c_str(), nullptr, nullptr);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  return window;
}

/* ************************************************************************************************
 * **
 * **   Initialise Dear ImGui
 * **
 * ************************************************************************************************/
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

void init_dear_imgui(GLFWwindow *window, const std::string &glsl_version) {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void) io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version.c_str());
}

/* ************************************************************************************************
 * **
 * **   Initalise shaders
 * **
 * ************************************************************************************************/
#include <GLHelpers/shader.h>

std::shared_ptr<Shader> init_shader() {
  return Shader::from_files("shaders/ex01.vert", "shaders/ex01.frag");
}

/* ************************************************************************************************
 * **
 * **   Geometry handling
 * **
 * ************************************************************************************************/
void init_vbo(GLuint vbo, const glm::vec3 &colour) {
  float wall_vertices[] = {
    -2.0f, -1.0f, -0.5f, 0, 0, 0,
    2.0f, -1.0f, -0.5f, 0, 0, 0,
    2.0f, -1.0f, 0.5f, 0, 0, 0,
    -2.0f, -1.0f, 0.5f, 0, 0, 0,
    -2.0f, 1.0f, -0.5f, 0, 0, 0,
    2.0f, 1.0f, -0.5f, 0, 0, 0,
    2.0f, 1.0f, 0.5f, 0, 0, 0,
    -2.0f, 1.0f, 0.5f, 0, 0, 0,
  };
  for (auto i = 0; i < 8; ++i) {
    wall_vertices[i * 6 + 3] = colour.r;
    wall_vertices[i * 6 + 4] = colour.g;
    wall_vertices[i * 6 + 5] = colour.b;
  }
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 8 * 6 * sizeof(float), wall_vertices, GL_STATIC_DRAW);
}

void init_ebo(GLuint ebo) {
  uint32_t cube_face_indices[] = {
    0, 4, 5, 0, 5, 1,
    1, 5, 6, 1, 6, 2,
    2, 6, 7, 2, 7, 3,
    3, 7, 4, 3, 4, 0,
    3, 0, 1, 3, 1, 2,
    4, 7, 6, 4, 6, 5
  };
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, (6 * 6) * sizeof(uint32_t), cube_face_indices, GL_STATIC_DRAW);
}

void init_geometry(GLuint &vao, GLuint &vbo, GLuint &ebo) {
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint buffs[2];
  glGenBuffers(2, buffs);

  vbo = buffs[0];
  init_vbo(vbo, glm::vec3{1.0f, 0.0f, 0.0f});

  ebo = buffs[1];
  init_ebo(ebo);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3,GL_FLOAT,GL_FALSE, 24, (GLvoid *) nullptr);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3,GL_FLOAT,GL_FALSE, 24, (GLvoid *) 12);
}

/* ************************************************************************************************
 * **
 * **   M A I N
 * **
 * ************************************************************************************************/
int main() {
  spdlog::cfg::load_env_levels();

  std::string glsl_version;
  auto window = init_glfw("Absorption Demo", glsl_version);
  if (window == nullptr) {
    return EXIT_FAILURE;
  }

  State s{};
  glfwSetWindowUserPointer(window, &s);

  // Init ImGui
  init_dear_imgui(window, glsl_version);

  // Init Geometry
  GLuint vao, vbo, ebo;
  init_geometry(vao, vbo, ebo);

  auto shader = init_shader();

  /* ************************************************************************************************
   * **
   * **   Main render loop
   * **
   * ************************************************************************************************/
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Start ImGUI Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Controls", nullptr, 0);
    ImGui::End();

    /* ********************************************************************************
     * *
     * *  Do the actual rendering
     * *
     * ********************************************************************************/
    ImGui::Render();

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.2f, 0.2f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vao);
    shader->use();
    shader->set_uniform("aspect_ratio", static_cast<float>(display_w) / static_cast<float>(display_h));
    glDrawElements(GL_TRIANGLES, 36,GL_UNSIGNED_INT, 0);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }
  return EXIT_SUCCESS;
}
