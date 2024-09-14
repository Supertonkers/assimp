//
//  asfamc.cpp
//  asfamc_to_fbx
//
//  Created by Tom on 9/14/24.
//

#include "asfamc.hpp"

namespace mesh {

MeshData ParseMeshData(const nlohmann::json& mesh_json) {
    MeshData meshData;
    meshData.name = mesh_json["mesh_name"];
    meshData.distance = mesh_json["mesh_distance"];
    meshData.startPoint = aiVector3D(mesh_json["mesh_start_point"][0], mesh_json["mesh_start_point"][1], mesh_json["mesh_start_point"][2]);
    meshData.endPoint = aiVector3D(mesh_json["mesh_end_point"][0], mesh_json["mesh_end_point"][1], mesh_json["mesh_end_point"][2]);
    meshData.refPoint = aiVector3D(mesh_json["mesh_ref_point"][0], mesh_json["mesh_ref_point"][1], mesh_json["mesh_ref_point"][2]);
    meshData.useFaceNormals = mesh_json["mesh_use_face_normals"];
    meshData.useTangentBasis = mesh_json["mesh_use_tangent_basis"];

    for (const auto& vertex : mesh_json["mesh_vertices"]) {
        meshData.vertices.push_back(aiVector3D(vertex[0], vertex[1], vertex[2]));
    }

    for (const auto& normal : mesh_json["mesh_normals"]) {
        meshData.normals.push_back(aiVector3D(normal[0], normal[1], normal[2]));
    }

    for (const auto& uv : mesh_json["mesh_uv"]) {
        meshData.uvs.push_back(aiVector3D(uv[0], uv[1], 0.0f)); // UVs are 2D, but aiVector3D expects 3D
    }

    meshData.indices = mesh_json["mesh_indices"].get<std::vector<unsigned int>>();

    return meshData;
}
}

namespace asf {

Bone ParseBone(const nlohmann::json& bone_json) {
    Bone bone;
    bone.name = bone_json["bone_name"];
    bone.direction = aiVector3D(bone_json["bone_direction"][0], bone_json["bone_direction"][1], bone_json["bone_direction"][2]);
    bone.length = bone_json["bone_length"];
    bone.axis = aiVector3D(bone_json["bone_axis"][0], bone_json["bone_axis"][1], bone_json["bone_axis"][2]);
    bone.dof = bone_json["bone_dof"];
    bone.order = aiVector3D(bone_json["bone_order"][0], bone_json["bone_order"][1], bone_json["bone_order"][2]);
    bone.position = aiVector3D(bone_json["bone_position"][0], bone_json["bone_position"][1], bone_json["bone_position"][2]);
    bone.orientation = aiQuaternion(bone_json["bone_orientation"][0], bone_json["bone_orientation"][1], bone_json["bone_orientation"][2], 1.0f); // Assuming orientation is a quaternion without w component
    bone.u0 = bone_json["bone_u0"];
    bone.u1 = bone_json["bone_u1"];

    for (const auto& child : bone_json["bone_hierarchy"]) {
        bone.children.push_back(ParseBone(child));
    }

    return bone;
}

}

namespace amc {

MotionData ParseMotionData(const nlohmann::json& motion_json) {
    MotionData motionData;
    motionData.parts = motion_json["motion_parts"].get<std::vector<std::string>>();
    motionData.smoothing = motion_json["motion_smoothing"].get<std::vector<std::vector<double>>>();
    return motionData;
}

}
