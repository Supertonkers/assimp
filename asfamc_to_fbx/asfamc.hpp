//
//  asfamc.hpp
//  asfamc_to_fbx
//
//  Created by Tom on 9/14/24.
//

#ifndef asfamc_hpp
#define asfamc_hpp

#include <stdio.h>
#include <nlohmann/json.hpp>
#include "Importer.hpp"

namespace mesh {

struct MeshData {
    std::string name;
    float distance;
    aiVector3D startPoint;
    aiVector3D endPoint;
    aiVector3D refPoint;
    bool useFaceNormals;
    bool useTangentBasis;
    std::vector<aiVector3D> vertices;
    std::vector<aiVector3D> normals;
    std::vector<aiVector3D> uvs;
    std::vector<unsigned int> indices;
    /**
     Field is overwritten, not privded in actual data.*/
    std::string texturePath;  // Add texture path here
};

MeshData ParseMeshData(const nlohmann::json& mesh_json);
}

namespace asf {
    struct Bone {
        std::string name;
        aiVector3D direction;
        float length;
        aiVector3D axis;
        std::string dof;
        aiVector3D order;
        aiVector3D position;
        aiQuaternion orientation;
        bool u0;
        bool u1;
        std::vector<Bone> children;
    };

Bone ParseBone(const nlohmann::json& bone_json);

}

namespace amc {
    struct MotionData {
        std::vector<std::string> parts;
        std::vector<std::vector<double>> smoothing;
    };
MotionData ParseMotionData(const nlohmann::json& motion_json);
}



#endif /* asfamc_hpp */
