#ifndef OPENGLWINDOW_HPP_
#define OPENGLWINDOW_HPP_

#include <string_view>

#include "abcg.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "trackball.hpp"

class OpenGLWindow : public abcg::OpenGLWindow {
 protected:
  void handleEvent(SDL_Event& ev) override;
  void initializeGL() override;
  void paintGL() override;
  void paintUI() override;
  void resizeGL(int width, int height) override;
  void terminateGL() override;

 private:
  int m_viewportWidth{};
  int m_viewportHeight{};

  Model m_model;

  int m_trianglesToDraw{};

  TrackBall m_trackBallModel;
  TrackBall m_trackBallLight;
  float m_zoom{};

  //foi substituido ao ser acoplano no projeto com a trackball
  //glm::vec3 m_eyePosition{};
  glm::mat4 m_modelMatrix{1.0f};

  // Danilo
  //glm::mat4 m_model2Matrix{1.0f};
  glm::mat4 m_viewMatrix{1.0f};
  glm::mat4 m_projMatrix{1.0f};

  // Shaders
  const std::vector<const char*> m_shaderNames{
      "texture", "cubereflect", "cuberefract", "normalmapping", "blinnphong",
      "phong",   "gouraud",     "normal",      "depth"};
  std::vector<GLuint> m_programs;
  int m_currentProgramIndex{};

  // Mapping mode
  // 0: triplanar; 1: cylindrical; 2: spherical; 3: from mesh
  int m_mappingMode{};

  // Light and material properties
  glm::vec4 m_lightDir{-1.0f, -1.0f, -1.0f, 0.0f};
  glm::vec4 m_Ia{1.0f};
  glm::vec4 m_Id{1.0f};
  glm::vec4 m_Is{1.0f};
  glm::vec4 m_Ka;
  glm::vec4 m_Kd;
  glm::vec4 m_Ks;
  float m_shininess{};

  // Skybox
  const std::string m_skyShaderName{"skybox"};
  GLuint m_skyVAO{};
  GLuint m_skyVBO{};
  GLuint m_skyProgram{};

  // clang-format off
  const std::array<glm::vec3, 36>  m_skyPositions{
    // Front
    glm::vec3{-1, -1, +1}, glm::vec3{+1, -1, +1}, glm::vec3{+1, +1, +1},
    glm::vec3{-1, -1, +1}, glm::vec3{+1, +1, +1}, glm::vec3{-1, +1, +1},
    // Back
    glm::vec3{+1, -1, -1}, glm::vec3{-1, -1, -1}, glm::vec3{-1, +1, -1},
    glm::vec3{+1, -1, -1}, glm::vec3{-1, +1, -1}, glm::vec3{+1, +1, -1},
    // Right
    glm::vec3{+1, -1, -1}, glm::vec3{+1, +1, -1}, glm::vec3{+1, +1, +1},
    glm::vec3{+1, -1, -1}, glm::vec3{+1, +1, +1}, glm::vec3{+1, -1, +1},
    // Left
    glm::vec3{-1, -1, +1}, glm::vec3{-1, +1, +1}, glm::vec3{-1, +1, -1},
    glm::vec3{-1, -1, +1}, glm::vec3{-1, +1, -1}, glm::vec3{-1, -1, -1},
    // Top
    glm::vec3{-1, +1, +1}, glm::vec3{+1, +1, +1}, glm::vec3{+1, +1, -1},
    glm::vec3{-1, +1, +1}, glm::vec3{+1, +1, -1}, glm::vec3{-1, +1, -1},
    // Bottom
    glm::vec3{-1, -1, -1}, glm::vec3{+1, -1, -1}, glm::vec3{+1, -1, +1},
    glm::vec3{-1, -1, -1}, glm::vec3{+1, -1, +1}, glm::vec3{-1, -1, +1}
  };
  // clang-format on

  void initializeSkybox();
  void renderSkybox();
  void terminateSkybox();
  void loadModel(std::string_view path);
  // Danilo
  // Model para as árvores com texutra
  void loadModel2(std::string_view path);
  Model m_model2;
  // Model para o Dinossauro com texutra
  void loadModel3(std::string_view path);
  Model m_model3;
  // Model para as árvores com shader normal
  void loadModel4(std::string_view path);
  Model m_model4;
  // Model para o elefante com o shader normal
  void loadModel5(std::string_view path);
  Model m_model5;

  //declaração da variavel para ter um timer
  abcg::ElapsedTimer m_restartWaitTimer;



  void update();

  // Danilo

  //gerar a camera do lookat
  Camera m_camera;
  float m_dollySpeed{0.0f};
  float m_truckSpeed{0.0f};
  float m_panSpeed{0.0f};

  //int para controlar a quantidade de árvores geradas
  static const int arvores = 40;

  // método para atualizar as coordenadas
  void atCoordenadas();
  // variaveis para coordenadas
  float eixo_x[arvores];
  float eixo_y[arvores];
  // variaveis para o tamanho
  float tamanho[arvores];

  //a cor não está sendo utilizada!
  //float cor1[arvores];
  //float cor2[arvores];
  //float cor3[arvores];
};

#endif