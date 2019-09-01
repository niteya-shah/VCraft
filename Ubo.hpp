#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/rotate_vector.hpp>

#ifndef Vkx_ubo
#define Vkx_ubo

struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
  glm::vec3 camera;
  glm::vec3 objectView;
  const glm::vec3 axis = glm::vec3(0.0f, 0.0f, 1.0f);
  glm::vec3 axisX;
  glm::vec3 axisY;
  glm::vec3 axisZ;

  void setAxis(){
    axisX = glm::normalize(camera - objectView);
    axisY = glm::normalize(glm::vec3(axisX[1], - axisX[0], 0));//glm::vec3(axisX[1], - axisX[0], camera[0]) for absolute movement
    axisZ = glm::normalize(glm::cross(axisX,axisY));
  }

  UniformBufferObject() {
    model = glm::mat4(1.0f);
    camera = glm::vec3(3.0f, 0.0f, 1.0f);
    objectView = glm::vec3(0.0f, 0.0f, 1.0f);
    view = glm::lookAt(camera, objectView, axis);
    proj = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f,
                            200.0f);
    proj[1][1] *= -1;

    setAxis();
  }
  // Set for absolute 3D movement;
  // void UpdateUbo(SDL_Event event) {
  //
  //   switch (event.key.keysym.sym) {
  //     //Create Relative Movement
  //   case SDLK_w:
  //     camera = camera - axisX;//glm::vec3(axisX[0],axisX[1], 0.0f);
  //     objectView = objectView - axisX;//glm::vec3(axisX[0],axisX[1], 0.0f);
  //     break;
  //   case SDLK_s:
  //     camera = camera + axisX;//glm::vec3(axisX[0],axisX[1], 0.0f);
  //     objectView = objectView + axisX;//glm::vec3(axisX[0],axisX[1], 0.0f);
  //     break;
  //   case SDLK_a:
  //     camera = camera + axisY;//glm::vec3(axisY[0],axisY[1], 0.0f);
  //     objectView = objectView + axisY;//glm::vec3(axisY[0],axisY[1], 0.0f);
  //     break;
  //   case SDLK_d:
  //     camera = camera - axisY;//glm::vec3(axisY[0],axisY[1], 0.0f);
  //     objectView = objectView - axisY;//glm::vec3(axisY[0],axisY[1], 0.0f);
  //     break;
  //   case SDLK_KP_2:
  //     objectView =  objectView + axisZ;
  //     break;
  //   case SDLK_KP_6:
  //     objectView = objectView - axisY;
  //     break;
  //   case SDLK_KP_4:
  //     objectView = objectView + axisY;
  //     break;
  //   case SDLK_KP_8:
  //     objectView = objectView - axisZ;
  //     break;
  //   default:
  //     break;
  //   }

  void UpdateUbo(SDL_Event event) {

    switch (event.key.keysym.sym) {
      //Create Relative Movement
    case SDLK_w:
      camera = camera - glm::vec3(axisX[0],axisX[1], 0.0f);
      objectView = objectView - glm::vec3(axisX[0],axisX[1], 0.0f);
      break;
    case SDLK_s:
      camera = camera + glm::vec3(axisX[0],axisX[1], 0.0f);
      objectView = objectView + glm::vec3(axisX[0],axisX[1], 0.0f);
      break;
    case SDLK_a:
      camera = camera + glm::vec3(axisY[0],axisY[1], 0.0f);
      objectView = objectView + glm::vec3(axisY[0],axisY[1], 0.0f);
      break;
    case SDLK_d:
      camera = camera - glm::vec3(axisY[0],axisY[1], 0.0f);
      objectView = objectView - glm::vec3(axisY[0],axisY[1], 0.0f);
      break;
    case SDLK_KP_2:
      objectView =  objectView + axisZ;
      break;
    case SDLK_KP_6:
      objectView = objectView - axisY;
      break;
    case SDLK_KP_4:
      objectView = objectView + axisY;
      break;
    case SDLK_KP_8:
      objectView = objectView - axisZ;
      break;
    default:
      break;
    }

    setAxis();
    //std::cout<<camera<<"---"<<axisY<< std::endl;

    view = glm::lookAt(camera, objectView, axis);
  }
};
#endif
