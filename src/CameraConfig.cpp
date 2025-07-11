#include "CameraConfig.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <spdlog/spdlog.h>

CameraConfig CameraConfig::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open camera file: " + filePath);
    }
    
    CameraConfig config;
    std::string line;
    
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == '/') {
            continue;
        }
        
        std::istringstream iss(line);
        std::string key;
        if (!(iss >> key)) {
            continue;
        }
        
        if (key == "position") {
            iss >> config.position.x >> config.position.y >> config.position.z;
            spdlog::debug("Camera position: ({}, {}, {})", 
                         config.position.x, config.position.y, config.position.z);
        }
        else if (key == "rotation") {
            // Quaternion format: w x y z
            iss >> config.rotation.w >> config.rotation.x >> config.rotation.y >> config.rotation.z;
            spdlog::debug("Camera rotation (quat): ({}, {}, {}, {})", 
                         config.rotation.w, config.rotation.x, config.rotation.y, config.rotation.z);
        }
        else if (key == "euler") {
            // Euler angles in degrees: pitch yaw roll
            glm::vec3 euler;
            iss >> euler.x >> euler.y >> euler.z;
            config.eulerAngles = euler;
            spdlog::debug("Camera euler angles: ({}, {}, {})", euler.x, euler.y, euler.z);
        }
        else if (key == "target") {
            glm::vec3 target;
            iss >> target.x >> target.y >> target.z;
            config.target = target;
            spdlog::debug("Camera target: ({}, {}, {})", target.x, target.y, target.z);
        }
        else if (key == "up") {
            glm::vec3 up;
            iss >> up.x >> up.y >> up.z;
            config.up = up;
            spdlog::debug("Camera up: ({}, {}, {})", up.x, up.y, up.z);
        }
        else if (key == "fov") {
            iss >> config.fov;
            spdlog::debug("Camera FOV: {}", config.fov);
        }
        else if (key == "near") {
            iss >> config.nearPlane;
            spdlog::debug("Camera near plane: {}", config.nearPlane);
        }
        else if (key == "far") {
            iss >> config.farPlane;
            spdlog::debug("Camera far plane: {}", config.farPlane);
        }
        else {
            spdlog::warn("Unknown camera config key: {}", key);
        }
    }
    
    // Compute rotation if specified via alternative methods
    if (config.eulerAngles.has_value()) {
        config.computeRotationFromEuler();
    } else if (config.target.has_value()) {
        config.computeRotationFromLookAt();
    }
    
    return config;
}

bool CameraConfig::saveToFile(const CameraConfig& config, const std::string& filePath) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    file << "# Camera Configuration File\n";
    file << "# Format: key value1 value2 value3 ...\n\n";
    
    file << "position " << config.position.x << " " << config.position.y << " " << config.position.z << "\n";
    file << "rotation " << config.rotation.w << " " << config.rotation.x << " " 
         << config.rotation.y << " " << config.rotation.z << "\n";
    file << "fov " << config.fov << "\n";
    file << "near " << config.nearPlane << "\n";
    file << "far " << config.farPlane << "\n";
    
    return true;
}

void CameraConfig::computeRotationFromEuler() {
    if (!eulerAngles.has_value()) return;
    
    // Convert degrees to radians
    glm::vec3 radians = glm::radians(eulerAngles.value());
    
    // Create rotation from Euler angles (pitch, yaw, roll)
    rotation = glm::quat(radians);
    
    spdlog::debug("Computed rotation from Euler: ({}, {}, {}, {})", 
                 rotation.w, rotation.x, rotation.y, rotation.z);
}

void CameraConfig::computeRotationFromLookAt() {
    if (!target.has_value()) return;
    
    glm::vec3 upVec = up.value_or(glm::vec3(0.0f, 1.0f, 0.0f));
    
    // Create look-at matrix
    glm::mat4 lookAt = glm::lookAt(position, target.value(), upVec);
    
    // Extract rotation quaternion from look-at matrix
    rotation = glm::quat_cast(lookAt);
    
    spdlog::debug("Computed rotation from look-at: ({}, {}, {}, {})", 
                 rotation.w, rotation.x, rotation.y, rotation.z);
}