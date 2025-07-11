#ifndef CAMERACONFIG_H
#define CAMERACONFIG_H

#include <string>
#include <optional>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct CameraConfig {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};  // Identity quaternion
    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    
    // Alternative: specify rotation as Euler angles (degrees)
    std::optional<glm::vec3> eulerAngles = std::nullopt;
    
    // Alternative: specify look-at target
    std::optional<glm::vec3> target = std::nullopt;
    std::optional<glm::vec3> up = std::nullopt;
    
    static CameraConfig loadFromFile(const std::string& filePath);
    static bool saveToFile(const CameraConfig& config, const std::string& filePath);
    
private:
    void computeRotationFromEuler();
    void computeRotationFromLookAt();
};

#endif // CAMERACONFIG_H