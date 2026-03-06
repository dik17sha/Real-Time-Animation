#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <iostream>

class IKChain {
public: 
    std::vector<glm::vec3> joints;
    std::vector<float> boneLengths;
    float totalLength;
    glm::vec3 initialRootPosition;

    // CONSTRUCTOR
    IKChain() {
        joints = {
            glm::vec3(0.0f, 0.0f, 0.0f),   // Root
            glm::vec3(0.0f, -2.0f, 0.0f),  // Distance = 2.0
            glm::vec3(0.0f, -4.0f, 0.0f),  // Distance = 2.0
            glm::vec3(0.0f, -5.0f, 0.0f)   // Distance = 1.0
        };

        boneLengths = {2.0f, 2.0f, 1.0f};

        totalLength = 0.0f;
        for (float len : boneLengths) {
            totalLength += len;
        }

        initialRootPosition = joints[0];
    } 

    
    void solveFABRIK(glm::vec3 target, int maxIters = 15, float tolerance = 0.001f) {
        float distToTarget = glm::distance(initialRootPosition, target);

        if (distToTarget >= totalLength) {
            for (size_t i = 0; i < boneLengths.size(); ++i) {

                float r = glm::distance(target, joints[i]);
                float lambda = boneLengths[i] / r;

                joints[i + 1] = (1.0f - lambda) * joints[i] + lambda * target;
            }

            return; 
        }

        
        for (int iter = 0; iter < maxIters; iter++) {
            joints.back() = target;

            for (int i = joints.size() - 2; i >= 0; i--) {

                glm::vec3 dir = glm::normalize(joints[i] - joints[i + 1]);
                joints[i] = joints[i + 1] + dir * boneLengths[i];
            }

            joints[0] = initialRootPosition;

            for(size_t i = 1; i < joints.size(); ++i) {
                glm::vec3 dir = glm::normalize(joints[i] - joints[i - 1]);
                joints[i] = joints[i - 1] + dir * boneLengths[i - 1];
            }

            if (glm::distance(joints.back(), target) < tolerance) {
                break;
            }
        }
    }
};