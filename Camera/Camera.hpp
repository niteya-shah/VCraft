#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "./ubo.hpp"

#ifndef VCraft_camera
#define VCraft_camera

struct Camera {

  UniformBufferObject ubo;
  glm::vec3 camera_location;
  glm::vec3 objectView_location;
  const glm::vec3 axis = glm::vec3(0.0f, 0.0f, 1.0f);
  glm::vec3 axisX;
  glm::vec3 axisY;
  glm::vec3 axisZ;

  void setAxis() {
    axisX = glm::normalize(camera_location - objectView_location);
    axisY = glm::normalize(glm::vec3(
        axisX[1], -axisX[0],
        0)); // glm::vec3(axisX[1], - axisX[0], camera_location[0]) for absolute movement
    axisZ = glm::normalize(glm::cross(axisX, axisY));
  }

  Camera() {
    ubo.model = glm::mat4(1.0f);
    camera_location = glm::vec3(3.0f, 0.0f, 1.0f);
    objectView_location = glm::vec3(0.0f, 0.0f, 1.0f);
    ubo.view = glm::lookAt(camera_location, objectView_location, axis);
    ubo.proj = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f,
                            200.0f);
    ubo.proj[1][1] *= -1;

    setAxis();
  }
  // Set for absolute 3D movement;
  // void UpdateUbo(SDL_Event event) {
  //
  //   switch (event.key.keysym.sym) {
  //     //Create Relative Movement
  //   case SDLK_w:
  //     camera_location = camera_location - axisX;//glm::vec3(axisX[0],axisX[1], 0.0f);
  //     objectView_location = objectView_location - axisX;//glm::vec3(axisX[0],axisX[1], 0.0f);
  //     break;
  //   case SDLK_s:
  //     camera_location = camera_location + axisX;//glm::vec3(axisX[0],axisX[1], 0.0f);
  //     objectView_location = objectView_location + axisX;//glm::vec3(axisX[0],axisX[1], 0.0f);
  //     break;
  //   case SDLK_a:
  //     camera_location = camera_location + axisY;//glm::vec3(axisY[0],axisY[1], 0.0f);
  //     objectView_location = objectView_location + axisY;//glm::vec3(axisY[0],axisY[1], 0.0f);
  //     break;
  //   case SDLK_d:
  //     camera_location = camera_location - axisY;//glm::vec3(axisY[0],axisY[1], 0.0f);
  //     objectView_location = objectView_location - axisY;//glm::vec3(axisY[0],axisY[1], 0.0f);
  //     break;
  //   case SDLK_KP_2:
  //     objectView_location =  objectView_location + axisZ;
  //     break;
  //   case SDLK_KP_6:
  //     objectView_location = objectView_location - axisY;
  //     break;
  //   case SDLK_KP_4:
  //     objectView_location = objectView_location + axisY;
  //     break;
  //   case SDLK_KP_8:
  //     objectView_location = objectView_location - axisZ;
  //     break;
  //   default:
  //     break;
  //   }

  void UpdateUbo(SDL_Event event) {

    switch (event.key.keysym.sym) {
      // Create Relative Movement
    case SDLK_w:
      camera_location = camera_location - glm::vec3(axisX[0], axisX[1], 0.0f);
      objectView_location = objectView_location - glm::vec3(axisX[0], axisX[1], 0.0f);
      break;
    case SDLK_s:
      camera_location = camera_location + glm::vec3(axisX[0], axisX[1], 0.0f);
      objectView_location = objectView_location + glm::vec3(axisX[0], axisX[1], 0.0f);
      break;
    case SDLK_a:
      camera_location = camera_location + glm::vec3(axisY[0], axisY[1], 0.0f);
      objectView_location = objectView_location + glm::vec3(axisY[0], axisY[1], 0.0f);
      break;
    case SDLK_d:
      camera_location = camera_location - glm::vec3(axisY[0], axisY[1], 0.0f);
      objectView_location = objectView_location - glm::vec3(axisY[0], axisY[1], 0.0f);
      break;
    case SDLK_KP_2:
      objectView_location = objectView_location + axisZ;
      break;
    case SDLK_KP_6:
      objectView_location = objectView_location - axisY;
      break;
    case SDLK_KP_4:
      objectView_location = objectView_location + axisY;
      break;
    case SDLK_KP_8:
      objectView_location = objectView_location - axisZ;
      break;
    default:
      break;
    }

    setAxis();
    // std::cout<<camera_location<<"---"<<axisY<< std::endl;

    ubo.view = glm::lookAt(camera_location, objectView_location, axis);
  }
};
#endif
