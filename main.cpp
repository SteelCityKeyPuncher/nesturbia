#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#include "GLFW/glfw3.h"
#include "glad/glad.h"

#include "nesturbia/nesturbia.hpp"

namespace {

// Constants
constexpr auto kWindowTitle = "NESturbia";
constexpr double kFrameTime = 1.0 / 60.0;

constexpr auto kVertexShader = "#version 330 core\n"
                               "layout(location = 0) in vec3 aPos;\n"
                               "layout(location = 1) in vec2 aTexCoord;\n"
                               "out vec2 TexCoord;\n"
                               "void main() {\n"
                               "gl_Position = vec4(aPos, 1.0f);\n"
                               "TexCoord = aTexCoord;\n"
                               "}";

constexpr auto kFragmentShader = "#version 330 core\n"
                                 "in vec2 TexCoord;\n"
                                 "out vec4 FragColor;\n"
                                 "uniform sampler2D tex;\n"
                                 "void main() { FragColor = texture(tex, TexCoord); }";

// Local types
// This allows the std::unique_ptr<GLFWwindow> to properly free the window upon destruction
struct glfwDeleter {
  void operator()(GLFWwindow *window) { glfwDestroyWindow(window); }
};

// Local variables
std::unique_ptr<GLFWwindow, glfwDeleter> glfwWindow;
nesturbia::Nesturbia emulator;
GLuint shader = -1;
GLuint texture = -1;
GLuint VAO = -1;
GLuint VBO = -1;
GLuint EBO = -1;

// Local functions
bool parseArguments(int argc, char **argv);
bool initializeGraphics();
void runLoop();
void glfwErrorCallback(int error, const char *description);
void glfwWindowSizeCallback(GLFWwindow *window, int, int);

} // namespace

int main(int argc, char **argv) {
  if (!parseArguments(argc, argv)) {
    return EXIT_FAILURE;
  }

  if (!initializeGraphics()) {
    return EXIT_FAILURE;
  }

  runLoop();

  return EXIT_SUCCESS;
}

namespace {
bool parseArguments(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Expected one argument (ROM path)." << std::endl;
    return false;
  }

  // Open the given file
  auto file = std::ifstream(argv[1], std::ios::binary);
  if (!file) {
    std::cerr << "Could not open ROM '" << argv[1] << "'." << std::endl;
    return false;
  }

  // Read the file into a vector
  const auto rom = std::vector<uint8_t>(std::istreambuf_iterator<char>(file), {});
  if (!emulator.LoadRom(rom.data(), rom.size())) {
    std::cerr << "Could not load ROM '" << argv[1] << "'." << std::endl;
    return false;
  }

  // Success
  return true;
}

bool initializeGraphics() {
  if (!glfwInit()) {
    std::cerr << "Could not initialize GLFW." << std::endl;
    return false;
  }

  glfwSetErrorCallback(glfwErrorCallback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  glfwWindow.reset(glfwCreateWindow(256, 240, kWindowTitle, nullptr, nullptr));
  if (!glfwWindow) {
    std::cerr << "Could not create a window." << std::endl;
    return false;
  }

  // Update the framebuffer when the window is resized
  // This is necessary because, even in fullscreen mode, the screen may be
  // resized a few times (at least in X11/Ubuntu)
  glfwSetWindowSizeCallback(glfwWindow.get(), glfwWindowSizeCallback);
  glfwMakeContextCurrent(glfwWindow.get());

  if (!gladLoadGL()) {
    std::cerr << "Could not load OpenGL." << std::endl;
    return false;
  }

  // Clear the screen to black
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  // 0 = could tear, but swapping buffers doesn't block
  // 1 = no tearing, blocked at vsync rate
  glfwSwapInterval(1);

  // Compile and link the shaders
  GLint success;

  // Vertex shader
  const auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &kVertexShader, nullptr);
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    std::cerr << "Failed to compile the vertex shader." << std::endl;
    return false;
  }

  // Fragment shader
  const auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &kFragmentShader, nullptr);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    std::cerr << "Failed to compile the fragment shader." << std::endl;
    return false;
  }

  // Link the two shaders
  shader = glCreateProgram();
  glAttachShader(shader, vertexShader);
  glAttachShader(shader, fragmentShader);
  glLinkProgram(shader);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  glGetProgramiv(shader, GL_LINK_STATUS, &success);
  if (!success) {
    std::cerr << "Failed to link the vertex and fragment shaders." << std::endl;
    return false;
  }

  constexpr std::array<float, 6 * 5> kVertices = {
      // Top left
      -1.f,
      1.f,
      0.0f,
      0.f,
      0.f,
      // Bottom left
      -1.f,
      -1.f,
      0.0f,
      0.f,
      1.f,
      // Bottom Right
      1.0f,
      -1.f,
      0.0f,
      1.f,
      1.f,
      // Top left
      -1.f,
      1.f,
      0.0f,
      0.f,
      0.f,
      // Bottom Right
      1.0f,
      -1.f,
      0.0f,
      1.f,
      1.f,
      // Top Right
      1.0f,
      1.f,
      0.0f,
      1.f,
      0.f,
  };

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kVertices), kVertices.data(), GL_STATIC_DRAW);

  // Position attribute (x, y, z)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)nullptr);
  glEnableVertexAttribArray(0);

  // Texture coordinates attribute (u, v)
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  // Wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 240, 0, GL_RGB, GL_UNSIGNED_BYTE,
               emulator.ppu.pixels.data());

  // Success
  return true;
}

void runLoop() {
  // Used to determine the duration since the previous game logic update
  double lastUpdateTime = 0.0;

  // Used to determine the duration since the previous render
  // Set to a negative value to guarantee that rendering occurs in the first iteration
  double lastFrameTime = -1.0;

  glUseProgram(shader);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glBindVertexArray(VAO);

  // Fixed time step rendering logic
  // Run the update logic as fast as possible
  // If enough time has elapsed to actually render, then do so
  while (!glfwWindowShouldClose(glfwWindow.get())) {
    const auto currentTime = glfwGetTime();
    // TODO read inputs - here after glfwPollEvents()?

    // See if we can render in this loop
    if ((currentTime - lastFrameTime) >= kFrameTime) {
      // It's time to render
      // Prepare the window for rendering (clear color buffer)
      glClear(GL_COLOR_BUFFER_BIT);

      emulator.RunFrame();

      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 240, GL_RGB, GL_UNSIGNED_BYTE,
                      emulator.ppu.pixels.data());
      glDrawArrays(GL_TRIANGLES, 0, 6);

      // Finish up window rendering (swap buffers)
      glfwSwapBuffers(glfwWindow.get());

      // TODO input polling
      glfwPollEvents();

      // TODO: Temporary, close the window if the ESC key is pressed
      if (glfwGetKey(glfwWindow.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(glfwWindow.get(), true);
      }
    }
  }
}

void glfwErrorCallback(int error, const char *description) {
  std::cerr << "GLFW error " << error << ": " << description << std::endl;
}

void glfwWindowSizeCallback(GLFWwindow *window, int, int) {
  int frameBufferWidth;
  int frameBufferHeight;

  glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
  glViewport(0, 0, frameBufferWidth, frameBufferHeight);
}

} // namespace
