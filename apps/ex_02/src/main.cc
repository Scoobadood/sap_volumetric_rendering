#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>
#include "OpenGL/gl3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
  return Shader::from_files("shaders/ex02.vert", "shaders/ex02.frag");
}

/* ************************************************************************************************
 * **
 * **   Geometry handling
 * **
 * ************************************************************************************************/
void init_vbo(GLuint vbo) {
  float wall_vertices[] = {
    -1.5f, -1.0f, 2.0f,
    1.5f, -1.0f, 2.0f,
    1.5f, -1.0f, 2.5f,
    -1.5f, -1.0f, 2.5f,
    -1.5f, 1.0f, 2.0f,
    1.5f, 1.0f, 2.0f,
    1.5f, 1.0f, 2.5f,
    -1.5f, 1.0f, 2.5f,
  };
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(float), wall_vertices, GL_STATIC_DRAW);
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
  init_vbo(vbo);

  ebo = buffs[1];
  init_ebo(ebo);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3,GL_FLOAT,GL_FALSE, 12, (GLvoid *) nullptr);
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

  float sigma_a = 0.1f;
  float sigma_s = 0.1f;
  int32_t light_angle = 90;
  float light_z = 0.0f;
  glm::vec3 wall_colour(0.572f, 0.772f, 0.921f);
  glm::vec3 light_colour{1.0f, 0.3 / 1.3, 0.9 / 1.3};
  bool march_forwards = true;

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

    ImGui::SliderFloat("sigma_a", &sigma_a, 0, 1.0f, "%0.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("sigma_s", &sigma_s, 0, 1.0f, "%0.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderInt("Light Pos (𝜃)", &light_angle, 0, 359, "%d", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("Light Pos (Z)", &light_z, -20, 20, "%0.1f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::ColorEdit3("Light colour", &light_colour[0], ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3("Wall colour", &wall_colour[0], ImGuiColorEditFlags_NoInputs);
    ImGui::Checkbox("March forwards", &march_forwards);
    ImGui::End();

    auto light_position = glm::vec3(10 * cosf((light_angle / 180.0f) * (float) M_PI),
                                    10 * sinf((light_angle / 180.0f) * (float) M_PI),
                                    light_z);

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

    auto aspect_ratio = static_cast<float>(display_w) / static_cast<float>(display_h);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect_ratio, 0.01f, 100.0f);
    auto view = glm::translate(glm::mat4{1}, glm::vec3{0, 0, -5.0f});
    glm::vec3 cam_pos = glm::inverse(view) * glm::vec4(0, 0, 0, 1);

    auto model = glm::mat4{1};

    glBindVertexArray(vao);
    shader->use();
    shader->set_uniform("wall_colour", wall_colour);
    shader->set_uniform("sigma_a", sigma_a);
    shader->set_uniform("sigma_s", sigma_s);
    shader->set_uniform("projection", projection);
    shader->set_uniform("view", view);
    shader->set_uniform("model", model);
    shader->set_uniform("cam_pos", cam_pos);
    shader->set_uniform("sphere_pos", glm::vec3(0,0,-20));
    shader->set_uniform("sphere_radius", 5.0f);
    shader->set_uniform("step_size", 0.2f);
    shader->set_uniform("light_colour", light_colour);
    shader->set_uniform("light_position",light_position);
    shader->set_uniform("do_march_forwards",march_forwards);


    glDrawElements(GL_TRIANGLES, 36,GL_UNSIGNED_INT, 0);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }
  return EXIT_SUCCESS;
}
