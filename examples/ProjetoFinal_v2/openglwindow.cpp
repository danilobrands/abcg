#include "openglwindow.hpp"

#include <imgui.h>

#include <cppitertools/itertools.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "imfilebrowser.h"

// alterações Danilo
#include <random>

void OpenGLWindow::handleEvent(SDL_Event& event) {
  glm::ivec2 mousePosition;
  SDL_GetMouseState(&mousePosition.x, &mousePosition.y);

  if (event.type == SDL_MOUSEMOTION) {
    m_trackBallModel.mouseMove(mousePosition);
    m_trackBallLight.mouseMove(mousePosition);
  }
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    if (event.button.button == SDL_BUTTON_LEFT) {
      m_trackBallModel.mousePress(mousePosition);
    }
    // Desativando a rotação da luz e da skybox
    // if (event.button.button == SDL_BUTTON_RIGHT) {
    //  m_trackBallLight.mousePress(mousePosition);
    //}
  }
  if (event.type == SDL_MOUSEBUTTONUP) {
    if (event.button.button == SDL_BUTTON_LEFT) {
      m_trackBallModel.mouseRelease(mousePosition);
    }
    // if (event.button.button == SDL_BUTTON_RIGHT) {
    //  m_trackBallLight.mouseRelease(mousePosition);
    //}
  }
  if (event.type == SDL_MOUSEWHEEL) {
    m_zoom += (event.wheel.y > 0 ? 1.0f : -1.0f) / 5.0f;
    m_zoom = glm::clamp(m_zoom, -1.5f, 1.0f);
  }
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
      m_dollySpeed = 1.0f;
    if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
      m_dollySpeed = -1.0f;
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_panSpeed = -1.0f;
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
      m_panSpeed = 1.0f;
    if (event.key.keysym.sym == SDLK_q) m_truckSpeed = -1.0f;
    if (event.key.keysym.sym == SDLK_e) m_truckSpeed = 1.0f;
  }
  if (event.type == SDL_KEYUP) {
    if ((event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) &&
        m_dollySpeed > 0)
      m_dollySpeed = 0.0f;
    if ((event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) &&
        m_dollySpeed < 0)
      m_dollySpeed = 0.0f;
    if ((event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a) &&
        m_panSpeed < 0)
      m_panSpeed = 0.0f;
    if ((event.key.keysym.sym == SDLK_RIGHT ||
         event.key.keysym.sym == SDLK_d) &&
        m_panSpeed > 0)
      m_panSpeed = 0.0f;
    if (event.key.keysym.sym == SDLK_q && m_truckSpeed < 0) m_truckSpeed = 0.0f;
    if (event.key.keysym.sym == SDLK_e && m_truckSpeed > 0) m_truckSpeed = 0.0f;
  }
}

void OpenGLWindow::initializeGL() {
  glClearColor(0, 0, 0, 1);
  glEnable(GL_DEPTH_TEST);

  // Create programs
  for (const auto& name : m_shaderNames) {
    auto path{getAssetsPath() + "shaders/" + name};
    auto program{createProgramFromFile(path + ".vert", path + ".frag")};
    m_programs.push_back(program);
  }

  // Danilo
  // removi o objeto inicial
  // Load default model
  loadModel(getAssetsPath() + "10451_Red_Maple_Tree_v1_Iteration3.obj");
  // Load cubemap
  m_model.loadCubeTexture(getAssetsPath() + "maps/cube/");

  // Carregando o dinossauro
  loadModel3(getAssetsPath() + "Raptor.obj");

  // Carregando árvore para o shader normal
  loadModel4(getAssetsPath() + "Lowpoly_tree_sample.obj");

  // Carregando o elefante para o shader normal
  loadModel5(getAssetsPath() + "Elephant.obj");

  // Carregando as árvores com textura
  loadModel2(getAssetsPath() + "10451_Red_Maple_Tree_v1_Iteration3.obj");
  // m_model2.loadDiffuseTexture(getAssetsPath() +
  //                            "10451_Red_Maple_Tree_v1_Diffuse.jpg");

  // m_model2.loadDiffuseTexture(getAssetsPath() +
  //                            "maps/cube/.jpg");

  // Initial trackball spin
  m_trackBallModel.setAxis(glm::normalize(glm::vec3(1, 1, 1)));
  m_trackBallModel.setVelocity(0.0001f);

  // Iniciando com a textura
  m_currentProgramIndex = 3;

  initializeSkybox();
  atCoordenadas();
}

void OpenGLWindow::initializeSkybox() {
  // Create skybox program
  auto path{getAssetsPath() + "shaders/" + m_skyShaderName};
  m_skyProgram = createProgramFromFile(path + ".vert", path + ".frag");

  // Generate VBO
  glGenBuffers(1, &m_skyVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_skyVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(m_skyPositions), m_skyPositions.data(),
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  GLint positionAttribute{glGetAttribLocation(m_skyProgram, "inPosition")};

  // Create VAO
  glGenVertexArrays(1, &m_skyVAO);

  // Bind vertex attributes to current VAO
  glBindVertexArray(m_skyVAO);

  glBindBuffer(GL_ARRAY_BUFFER, m_skyVBO);
  glEnableVertexAttribArray(positionAttribute);
  glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // End of binding to current VAO
  glBindVertexArray(0);
}

// Carrega do modelo para a trackball
void OpenGLWindow::loadModel(std::string_view path) {
  m_model.loadDiffuseTexture(getAssetsPath() +
                             "10451_Red_Maple_Tree_v1_Diffuse.jpg");
  m_model.loadNormalTexture(getAssetsPath() + "maps/pattern_normal.png");
  m_model.loadFromFile(path);
  m_model.setupVAO(m_programs.at(m_currentProgramIndex));
  m_trianglesToDraw = m_model.getNumTriangles();

  // Use material properties from the loaded model
  m_Ka = m_model.getKa();
  m_Kd = m_model.getKd();
  m_Ks = m_model.getKs();
  m_shininess = m_model.getShininess();
}

// carrega as árvores com textura
void OpenGLWindow::loadModel2(std::string_view path) {
  m_model2.loadDiffuseTexture(getAssetsPath() +
                              "maps/10451_Red_Maple_Tree_v1_Diffuse.jpg");
  m_model2.loadNormalTexture(getAssetsPath() +
                             "maps/10451_Red_Maple_Tree_v1_Diffuse.jpg");
  m_model2.loadFromFile(path);
  // 3 é o program que gera textura
  m_model2.setupVAO(m_programs.at(3));
  m_trianglesToDraw = m_model2.getNumTriangles();

  if (m_model2.isUVMapped()) {
    // Use mesh texture coordinates if available...
    m_mappingMode = 3;
  } else {
    // ...or triplanar mapping otherwise
    m_mappingMode = 0;
  }
  // Use material properties from the loaded model
  m_Ka = m_model2.getKa();
  m_Kd = m_model2.getKd();
  m_Ks = m_model2.getKs();
  m_shininess = m_model2.getShininess();
}
// carrega o dino
void OpenGLWindow::loadModel3(std::string_view path) {
  m_model3.loadDiffuseTexture(getAssetsPath() + "maps/raptor.jpg");
  m_model3.loadNormalTexture(getAssetsPath() + "maps/raptor_normal.jpg");
  m_model3.loadFromFile(path);
  // 3 é o program que gera textura
  m_model3.setupVAO(m_programs.at(3));
  m_trianglesToDraw = m_model3.getNumTriangles();

  if (m_model3.isUVMapped()) {
    // Use mesh texture coordinates if available...
    m_mappingMode = 3;
  } else {
    // ...or triplanar mapping otherwise
    m_mappingMode = 0;
  }

  // Use material properties from the loaded model
  m_Ka = m_model3.getKa();
  m_Kd = m_model3.getKd();
  m_Ks = m_model3.getKs();
  m_shininess = m_model3.getShininess();
}

// carrega as árvores para o shader normal
void OpenGLWindow::loadModel4(std::string_view path) {
  //as texturas foram incluidas para não bugar no web asm
  m_model4.loadDiffuseTexture(getAssetsPath() + "maps/raptor.jpg");
  m_model4.loadNormalTexture(getAssetsPath() + "maps/raptor_normal.jpg");
  m_model4.loadFromFile(path);
  // 7 é o program que usa o shader normal
  m_model4.setupVAO(m_programs.at(7));
  m_trianglesToDraw = m_model4.getNumTriangles();

  // Use material properties from the loaded model
  m_Ka = m_model4.getKa();
  m_Kd = m_model4.getKd();
  m_Ks = m_model4.getKs();
  m_shininess = m_model4.getShininess();
}

// carrega elefante para o shader normal
void OpenGLWindow::loadModel5(std::string_view path) {
  //as texturas foram incluidas para não bugar no web asm
  m_model5.loadDiffuseTexture(getAssetsPath() + "maps/raptor.jpg");
  m_model5.loadNormalTexture(getAssetsPath() + "maps/raptor_normal.jpg");
  m_model5.loadFromFile(path);
  // 7 é o program que usa o shader normal
  m_model5.setupVAO(m_programs.at(7));
  m_trianglesToDraw = m_model5.getNumTriangles();

  // Use material properties from the loaded model
  m_Ka = m_model5.getKa();
  m_Kd = m_model5.getKd();
  m_Ks = m_model5.getKs();
  m_shininess = m_model5.getShininess();
}

void OpenGLWindow::paintGL() {
  update();

  // m_restartWaitTimer.elapsed();

  if (m_restartWaitTimer.elapsed() < 3.0f) {
    m_currentProgramIndex = 0;
  } else {
    m_currentProgramIndex = 7;
    if (m_restartWaitTimer.elapsed() > 6.0f) {
      atCoordenadas();
      m_restartWaitTimer.restart();
    }
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  // Use currently selected program
  const auto program{m_programs.at(m_currentProgramIndex)};
  glUseProgram(program);

  // Get location of uniform variables
  GLint viewMatrixLoc{glGetUniformLocation(program, "viewMatrix")};
  GLint projMatrixLoc{glGetUniformLocation(program, "projMatrix")};
  GLint modelMatrixLoc{glGetUniformLocation(program, "modelMatrix")};
  GLint normalMatrixLoc{glGetUniformLocation(program, "normalMatrix")};
  GLint lightDirLoc{glGetUniformLocation(program, "lightDirWorldSpace")};
  GLint shininessLoc{glGetUniformLocation(program, "shininess")};
  GLint IaLoc{glGetUniformLocation(program, "Ia")};
  GLint IdLoc{glGetUniformLocation(program, "Id")};
  GLint IsLoc{glGetUniformLocation(program, "Is")};
  GLint KaLoc{glGetUniformLocation(program, "Ka")};
  GLint KdLoc{glGetUniformLocation(program, "Kd")};
  GLint KsLoc{glGetUniformLocation(program, "Ks")};
  GLint diffuseTexLoc{glGetUniformLocation(program, "diffuseTex")};
  GLint normalTexLoc{glGetUniformLocation(program, "normalTex")};
  GLint cubeTexLoc{glGetUniformLocation(program, "cubeTex")};
  GLint mappingModeLoc{glGetUniformLocation(program, "mappingMode")};
  GLint texMatrixLoc{glGetUniformLocation(program, "texMatrix")};

  // Set uniform variables used by every scene object
  glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_viewMatrix[0][0]);
  glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_projMatrix[0][0]);
  glUniform1i(diffuseTexLoc, 0);
  glUniform1i(normalTexLoc, 1);
  glUniform1i(cubeTexLoc, 2);
  glUniform1i(mappingModeLoc, m_mappingMode);

  glm::mat3 texMatrix{m_trackBallLight.getRotation()};
  glUniformMatrix3fv(texMatrixLoc, 1, GL_TRUE, &texMatrix[0][0]);

  auto lightDirRotated{m_trackBallLight.getRotation() * m_lightDir};
  glUniform4fv(lightDirLoc, 1, &lightDirRotated.x);
  glUniform4fv(IaLoc, 1, &m_Ia.x);
  glUniform4fv(IdLoc, 1, &m_Id.x);
  glUniform4fv(IsLoc, 1, &m_Is.x);

  // Set uniform variables of the current object
  glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &m_modelMatrix[0][0]);
  // glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &m_modelMatrix[0][0]);

  auto modelViewMatrix{glm::mat3(m_viewMatrix * m_modelMatrix)};
  glm::mat3 normalMatrix{glm::inverseTranspose(modelViewMatrix)};
  glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);

  glUniform1f(shininessLoc, m_shininess);
  glUniform4fv(KaLoc, 1, &m_Ka.x);
  glUniform4fv(KdLoc, 1, &m_Kd.x);
  glUniform4fv(KsLoc, 1, &m_Ks.x);

  // Danilo
  // não vou mais desenhar o objeto na trackball
  // m_model.render(m_trianglesToDraw);

  // --------------------------------------------------
  // Vamos tentar desenhar a arvore com a matriz modelo

  // Desenhando as árvores com textura
  if (m_currentProgramIndex == 0) {
    for (int i = 0; i < arvores; i++) {
      glm::mat4 m_model2Matrix{1.0f};
      m_model2Matrix =
          glm::translate(m_model2Matrix, glm::vec3(eixo_x[i], 1.5f, eixo_y[i]));
      m_model2Matrix = glm::rotate(m_model2Matrix, glm::radians(20.0f * i),
                                   glm::vec3(0, 1, 0));

      // rotação para endireitar o objeto
      m_model2Matrix =
          glm::rotate(m_model2Matrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));
      m_model2Matrix = glm::scale(m_model2Matrix, glm::vec3(tamanho[i]));

      glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &m_model2Matrix[0][0]);
      m_model2.render(m_trianglesToDraw);
    }

    // Desenhando o Dinossauro
    glm::mat4 m_model3Matrix{1.0f};
    m_model3Matrix = glm::translate(m_model3Matrix, glm::vec3(0, 0.5f, -7));
    // usei o valor gerado na posição eixo_x[10] para aleatorizar a rotação do
    // dino
    m_model3Matrix = glm::rotate(
        m_model3Matrix, glm::radians(45.0f * eixo_x[10]), glm::vec3(0, 1, 0));
    // rotação para endireitar o objeto
    // m_model3Matrix =
    //    glm::rotate(m_model3Matrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    m_model3Matrix = glm::scale(m_model3Matrix, glm::vec3(1.5f));

    glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &m_model3Matrix[0][0]);
    m_model3.render(m_trianglesToDraw);
  }

  // Desenhando árvores com shader normal

  if (m_currentProgramIndex >= 7) {
    for (int i = 0; i < arvores; i++) {
      glm::mat4 m_model4Matrix{1.0f};
      m_model4Matrix =
          glm::translate(m_model4Matrix, glm::vec3(eixo_x[i], 1.5f, eixo_y[i]));
      m_model4Matrix = glm::rotate(m_model4Matrix, glm::radians(20.0f * i),
                                   glm::vec3(0, 1, 0));
      m_model4Matrix = glm::scale(m_model4Matrix, glm::vec3(2));

      glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &m_model4Matrix[0][0]);
      m_model4.render(m_trianglesToDraw);
    }

    // Desenhando o Elefante
    glm::mat4 m_model5Matrix{1.0f};
    m_model5Matrix = glm::translate(m_model5Matrix, glm::vec3(0, 0.8f, -7));
    // usei o valor gerado na posição eixo_x[10] para aleatorizar a rotação do
    // elefante
    m_model5Matrix = glm::rotate(
        m_model5Matrix, glm::radians(45.0f * eixo_x[10]), glm::vec3(0, 1, 0));
    // rotação para endireitar o objeto
    // m_model5Matrix =
    //    glm::rotate(m_model3Matrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    m_model5Matrix = glm::scale(m_model5Matrix, glm::vec3(1.1f));

    glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &m_model5Matrix[0][0]);
    m_model5.render(m_trianglesToDraw);
  }

  // glUseProgram(m_programs.at(7));

  // Desenhar skybox
  if (m_currentProgramIndex < 7) {
    renderSkybox();
  }
}

void OpenGLWindow::renderSkybox() {
  glUseProgram(m_skyProgram);

  // Get location of uniform variables
  GLint viewMatrixLoc{glGetUniformLocation(m_skyProgram, "viewMatrix")};
  GLint projMatrixLoc{glGetUniformLocation(m_skyProgram, "projMatrix")};
  GLint skyTexLoc{glGetUniformLocation(m_skyProgram, "skyTex")};

  // Set uniform variables
  auto viewMatrix{m_trackBallLight.getRotation()};
  glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &viewMatrix[0][0]);
  glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_projMatrix[0][0]);
  glUniform1i(skyTexLoc, 0);

  glBindVertexArray(m_skyVAO);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_model.getCubeTexture());

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CW);
  glDepthFunc(GL_LEQUAL);
  glDrawArrays(GL_TRIANGLES, 0, m_skyPositions.size());
  glDepthFunc(GL_LESS);

  glBindVertexArray(0);
  glUseProgram(0);
}

void OpenGLWindow::paintUI() {
  abcg::OpenGLWindow::paintUI();

  // desativando o menu
  // if (false) {
  // File browser for models
  static ImGui::FileBrowser fileDialogModel;
  fileDialogModel.SetTitle("Load 3D Model");
  fileDialogModel.SetTypeFilters({".obj"});
  fileDialogModel.SetWindowSize(m_viewportWidth * 0.8f,
                                m_viewportHeight * 0.8f);

  // File browser for textures
  static ImGui::FileBrowser fileDialogDiffuseMap;
  fileDialogDiffuseMap.SetTitle("Load Diffuse Map");
  fileDialogDiffuseMap.SetTypeFilters({".jpg", ".png"});
  fileDialogDiffuseMap.SetWindowSize(m_viewportWidth * 0.8f,
                                     m_viewportHeight * 0.8f);

  // File browser for normal maps
  static ImGui::FileBrowser fileDialogNormalMap;
  fileDialogNormalMap.SetTitle("Load Normal Map");
  fileDialogNormalMap.SetTypeFilters({".jpg", ".png"});
  fileDialogNormalMap.SetWindowSize(m_viewportWidth * 0.8f,
                                    m_viewportHeight * 0.8f);

// Only in WebGL
#if defined(__EMSCRIPTEN__)
  fileDialogModel.SetPwd(getAssetsPath());
  fileDialogDiffuseMap.SetPwd(getAssetsPath() + "/maps");
  fileDialogNormalMap.SetPwd(getAssetsPath() + "/maps");
#endif

  // Create main window widget
  {
    // Escondendo o a janela de configurações e o menu
    auto widgetSize{ImVec2(0, 0)};

    if (!m_model.isUVMapped()) {
      // Add extra space for static text
      widgetSize.y += 26;
    }

    ImGui::SetNextWindowPos(ImVec2(m_viewportWidth - widgetSize.x - 5, 5));
    ImGui::SetNextWindowSize(widgetSize);
    auto flags{ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDecoration};
    ImGui::Begin("Widget window", nullptr, flags);

    // Menu
    {
      bool loadModel{};
      bool loadDiffMap{};
      bool loadNormalMap{};
      if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
          ImGui::MenuItem("Load 3D Model...", nullptr, &loadModel);
          ImGui::MenuItem("Load Diffuse Map...", nullptr, &loadDiffMap);
          ImGui::MenuItem("Load Normal Map...", nullptr, &loadNormalMap);
          ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
      }
      if (loadModel) fileDialogModel.Open();
      if (loadDiffMap) fileDialogDiffuseMap.Open();
      if (loadNormalMap) fileDialogNormalMap.Open();
    }

    // Slider will be stretched horizontally
    ImGui::PushItemWidth(widgetSize.x - 16);
    ImGui::SliderInt("", &m_trianglesToDraw, 0, m_model.getNumTriangles(),
                     "%d triangles");
    ImGui::PopItemWidth();

    static bool faceCulling{};
    ImGui::Checkbox("Back-face culling", &faceCulling);

    if (faceCulling) {
      glEnable(GL_CULL_FACE);
    } else {
      glDisable(GL_CULL_FACE);
    }

    // CW/CCW combo box
    {
      static std::size_t currentIndex{};
      std::vector<std::string> comboItems{"CCW", "CW"};

      ImGui::PushItemWidth(120);
      if (ImGui::BeginCombo("Front face",
                            comboItems.at(currentIndex).c_str())) {
        for (auto index : iter::range(comboItems.size())) {
          const bool isSelected{currentIndex == index};
          if (ImGui::Selectable(comboItems.at(index).c_str(), isSelected))
            currentIndex = index;
          if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      ImGui::PopItemWidth();

      if (currentIndex == 0) {
        glFrontFace(GL_CCW);
      } else {
        glFrontFace(GL_CW);
      }
    }

    // Projection combo box
    {
      static std::size_t currentIndex{};
      std::vector<std::string> comboItems{"Perspective", "Orthographic"};

      ImGui::PushItemWidth(120);
      if (ImGui::BeginCombo("Projection",
                            comboItems.at(currentIndex).c_str())) {
        for (auto index : iter::range(comboItems.size())) {
          const bool isSelected{currentIndex == index};
          if (ImGui::Selectable(comboItems.at(index).c_str(), isSelected))
            currentIndex = index;
          if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      ImGui::PopItemWidth();

      auto aspect{static_cast<float>(m_viewportWidth) /
                  static_cast<float>(m_viewportHeight)};
      if (currentIndex == 0) {
        m_projMatrix =
            glm::perspective(glm::radians(45.0f), aspect, 0.1f, 50.0f);

      } else {
        m_projMatrix =
            glm::ortho(-1.0f * aspect, 1.0f * aspect, -1.0f, 1.0f, 0.1f, 5.0f);
      }
    }

    // Shader combo box
    {
      static std::size_t currentIndex{};

      ImGui::PushItemWidth(120);
      if (ImGui::BeginCombo("Shader", m_shaderNames.at(currentIndex))) {
        for (auto index : iter::range(m_shaderNames.size())) {
          const bool isSelected{currentIndex == index};
          if (ImGui::Selectable(m_shaderNames.at(index), isSelected))
            currentIndex = index;
          if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      ImGui::PopItemWidth();

      // Set up VAO if shader program has changed
      if (static_cast<int>(currentIndex) != m_currentProgramIndex) {
        m_currentProgramIndex = currentIndex;
        m_model.setupVAO(m_programs.at(m_currentProgramIndex));
      }
    }

    if (!m_model.isUVMapped()) {
      ImGui::TextColored(ImVec4(1, 1, 0, 1), "Mesh has no UV coords.");
    }

    // UV mapping box
    {
      std::vector<std::string> comboItems{"Triplanar", "Cylindrical",
                                          "Spherical"};

      if (m_model.isUVMapped()) comboItems.emplace_back("From mesh");

      ImGui::PushItemWidth(120);
      if (ImGui::BeginCombo("UV mapping",
                            comboItems.at(m_mappingMode).c_str())) {
        for (auto index : iter::range(comboItems.size())) {
          const bool isSelected{m_mappingMode == static_cast<int>(index)};
          if (ImGui::Selectable(comboItems.at(index).c_str(), isSelected))
            m_mappingMode = index;
          if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      ImGui::PopItemWidth();
    }

    ImGui::End();

    // Create window for light sources
    if (m_currentProgramIndex > 1 && m_currentProgramIndex < 6) {
      auto widgetSize{ImVec2(222, 244)};
      ImGui::SetNextWindowPos(ImVec2(m_viewportWidth - widgetSize.x - 5,
                                     m_viewportHeight - widgetSize.y - 5));
      ImGui::SetNextWindowSize(widgetSize);
      ImGui::Begin(" ", nullptr, ImGuiWindowFlags_NoDecoration);

      ImGui::Text("Light properties");

      // Slider to control light properties
      ImGui::PushItemWidth(widgetSize.x - 36);
      ImGui::ColorEdit3("Ia", &m_Ia.x, ImGuiColorEditFlags_Float);
      ImGui::ColorEdit3("Id", &m_Id.x, ImGuiColorEditFlags_Float);
      ImGui::ColorEdit3("Is", &m_Is.x, ImGuiColorEditFlags_Float);
      ImGui::PopItemWidth();

      ImGui::Spacing();

      ImGui::Text("Material properties");

      // Slider to control material properties
      ImGui::PushItemWidth(widgetSize.x - 36);
      ImGui::ColorEdit3("Ka", &m_Ka.x, ImGuiColorEditFlags_Float);
      ImGui::ColorEdit3("Kd", &m_Kd.x, ImGuiColorEditFlags_Float);
      ImGui::ColorEdit3("Ks", &m_Ks.x, ImGuiColorEditFlags_Float);
      ImGui::PopItemWidth();

      // Slider to control the specular shininess
      ImGui::PushItemWidth(widgetSize.x - 16);
      ImGui::SliderFloat("", &m_shininess, 0.0f, 500.0f, "shininess: %.1f");
      ImGui::PopItemWidth();

      ImGui::End();
    }
  }

  fileDialogModel.Display();
  if (fileDialogModel.HasSelected()) {
    loadModel(fileDialogModel.GetSelected().string());
    fileDialogModel.ClearSelected();

    if (m_model.isUVMapped()) {
      // Use mesh texture coordinates if available...
      m_mappingMode = 3;
    } else {
      // ...or triplanar mapping otherwise
      m_mappingMode = 0;
    }
  }

  fileDialogDiffuseMap.Display();
  if (fileDialogDiffuseMap.HasSelected()) {
    m_model.loadDiffuseTexture(fileDialogDiffuseMap.GetSelected().string());
    fileDialogDiffuseMap.ClearSelected();
  }

  fileDialogNormalMap.Display();
  if (fileDialogNormalMap.HasSelected()) {
    m_model.loadNormalTexture(fileDialogNormalMap.GetSelected().string());
    fileDialogNormalMap.ClearSelected();
  }
}

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;

  m_trackBallModel.resizeViewport(width, height);
  m_trackBallLight.resizeViewport(width, height);
}

void OpenGLWindow::terminateGL() {
  for (const auto& program : m_programs) {
    glDeleteProgram(program);
  }
  terminateSkybox();
}

void OpenGLWindow::terminateSkybox() {
  glDeleteProgram(m_skyProgram);
  glDeleteBuffers(1, &m_skyVBO);
  glDeleteVertexArrays(1, &m_skyVAO);
}

void OpenGLWindow::update() {
  m_modelMatrix = m_trackBallModel.getRotation();

  m_viewMatrix = glm::lookAt(m_camera.m_eye, m_camera.m_at, m_camera.m_up);
  // m_viewMatrix =
  //    glm::lookAt(glm::vec3(m_dollySpeed, 0.0f, 2.0f + m_zoom),
  //                glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

  // m_eyePosition = glm::vec3(0.0f, 0.0f, 2.0f + m_zoom);
  // m_viewMatrix = glm::lookAt(m_eyePosition, glm::vec3(0.0f, 0.0f, 0.0f),
  //                           glm::vec3(0.0f, 1.0f, 0.0f));
  float deltaTime{static_cast<float>(getDeltaTime())};
  // Update LookAt camera
  m_camera.dolly(m_dollySpeed * deltaTime);
  m_camera.truck(m_truckSpeed * deltaTime);
  m_camera.pan(m_panSpeed * deltaTime);
}

void OpenGLWindow::atCoordenadas() {
  int temp;

  for (int i = 0; i < arvores; i++) {
    // coordenadas
    temp = rand() % 14;
    eixo_x[i] = temp - 5;
    temp = rand() % 14;
    eixo_y[i] = temp - 7 - 7;

    // tamanho
    temp = rand() % 6;
    tamanho[i] = 1.7 + temp / 10.0f;

    // cor
    // temp = rand() % 5;
    // cor1[i] = (temp / 10) + 0.5f;
    // temp = rand() % 5;
    // cor2[i] = (temp / 10) + 0.5f;
    // temp = rand() % 5;
    // cor3[i] = (temp / 10) + 0.5f;
    // printf("ultima cor: %d", cor[2][i]);
  }
}