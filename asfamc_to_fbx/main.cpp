//
//  main.cpp
//  asfamc_to_fbx
//
//  Created by Tom on 9/9/24.
//
#include <iostream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include <fstream>
#include <nlohmann/json.hpp>
#include <filesystem>

#include "scene.h"
#include "Importer.hpp"
#include "Exporter.hpp"
#include "postprocess.h"

// Namespace for filesystem (requires C++17 or later)
namespace fs = std::filesystem;

// Function to parse a JSON file into a JSON object
nlohmann::json parse_json_file(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + file_path);
    }

    nlohmann::json json_obj;
    file >> json_obj;

    return json_obj;
}

// Function to get all files in a directory (non-recursive)
std::vector<std::string> get_files_in_directory(const std::string& directory_path) {
    std::vector<std::string> file_paths;

    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (entry.is_regular_file()) {
            file_paths.push_back(entry.path().string());
        }
    }

    return file_paths;
}

// Function to get file name from path (excluding extension)
std::string get_file_name(const std::string& file_path) {
    fs::path path(file_path);
    return path.stem().string();  // stem() returns the filename without extension
}

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
};

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

struct MotionData {
    std::vector<std::string> parts;
    std::vector<std::vector<double>> smoothing;
};

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

MotionData ParseMotionData(const nlohmann::json& motion_json) {
    MotionData motionData;
    motionData.parts = motion_json["motion_parts"].get<std::vector<std::string>>();
    motionData.smoothing = motion_json["motion_smoothing"].get<std::vector<std::vector<double>>>();
    return motionData;
}

aiNode* CreateAiSkeletonNode(const Bone& bone) {
    aiNode* node = new aiNode();
    node->mName = aiString(bone.name);
    node->mTransformation = aiMatrix4x4(
        aiVector3D(1.0f, 1.0f, 1.0f),
        aiQuaternion(bone.orientation.x, bone.orientation.y, bone.orientation.z, bone.orientation.w),
        aiVector3D(bone.position.x, bone.position.y, bone.position.z)
    );

    node->mNumChildren = bone.children.size();
    node->mChildren = new aiNode*[node->mNumChildren];
    for (size_t i = 0; i < node->mNumChildren; ++i) {
        node->mChildren[i] = CreateAiSkeletonNode(bone.children[i]);
    }

    return node;
}

void AddAnimationToAiScene(aiScene* scene, aiNode* rootNode, const MotionData& motionData) {
    aiAnimation* anim = new aiAnimation();
    anim->mName = aiString("MotionAnimation");
    anim->mDuration = motionData.smoothing.size();
    anim->mTicksPerSecond = 30; // Assuming 30 FPS
    anim->mNumChannels = motionData.parts.size();
    anim->mChannels = new aiNodeAnim*[anim->mNumChannels];

    for (size_t i = 0; i < motionData.parts.size(); ++i) {
        aiNode* node = rootNode->FindNode(aiString(motionData.parts[i]));
        if (!node) {
            std::cerr << "Bone " << motionData.parts[i] << " not found in skeleton!" << std::endl;
            continue;
        }

        aiNodeAnim* channel = new aiNodeAnim();
        channel->mNodeName = aiString(motionData.parts[i]);
        channel->mNumRotationKeys = motionData.smoothing.size();
        channel->mRotationKeys = new aiQuatKey[channel->mNumRotationKeys];

        for (size_t frame = 0; frame < motionData.smoothing.size(); ++frame) {
            aiQuatKey& rotKey = channel->mRotationKeys[frame];
            rotKey.mTime = frame;
            rotKey.mValue = aiQuaternion(
                motionData.smoothing[frame][i * 3 + 3],
                motionData.smoothing[frame][i * 3 + 4],
                motionData.smoothing[frame][i * 3 + 5],
                1.0f // Assuming w=1.0f for simplicity
            );
        }

        anim->mChannels[i] = channel;
    }

    scene->mNumAnimations = 1;
    scene->mAnimations = new aiAnimation*[scene->mNumAnimations];
    scene->mAnimations[0] = anim;
}

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

aiMesh* CreateAiMesh(const MeshData& meshData) {
    aiMesh* mesh = new aiMesh();
    mesh->mNumVertices = meshData.vertices.size();
    mesh->mVertices = new aiVector3D[mesh->mNumVertices];
    mesh->mNormals = new aiVector3D[mesh->mNumVertices];
    mesh->mTextureCoords[0] = new aiVector3D[mesh->mNumVertices];

    for (size_t i = 0; i < mesh->mNumVertices; ++i) {
        mesh->mVertices[i] = meshData.vertices[i];
        mesh->mNormals[i] = meshData.normals[i];
        mesh->mTextureCoords[0][i] = meshData.uvs[i];
    }

    mesh->mNumFaces = meshData.indices.size() / 3;
    mesh->mFaces = new aiFace[mesh->mNumFaces];

    for (size_t i = 0; i < mesh->mNumFaces; ++i) {
        aiFace& face = mesh->mFaces[i];
        face.mNumIndices = 3;
        face.mIndices = new unsigned int[face.mNumIndices];
        face.mIndices[0] = meshData.indices[i * 3 + 0];
        face.mIndices[1] = meshData.indices[i * 3 + 1];
        face.mIndices[2] = meshData.indices[i * 3 + 2];
    }

    return mesh;
}

int main() {
    // Initialize Assimp structures
    aiScene* scene = new aiScene();
    scene->mRootNode = new aiNode();
    scene->mRootNode->mName = aiString("Root");

    // Example JSON strings for skeleton, motion, and mesh data (Replace with actual JSON input)
    std::string skeletonJsonString = R"(...)"; // Skeleton JSON here
    std::string motionJsonString = R"(...)"; // Motion JSON here
    std::string meshJsonString = R"(...)"; // Mesh JSON here

    // Parse the JSON
    auto skeleton_json = nlohmann::json::parse(skeletonJsonString);
    auto motion_json = nlohmann::json::parse(motionJsonString);
    auto mesh_json = nlohmann::json::parse(meshJsonString);

    // Parse skeleton, motion, and mesh data
    Bone rootBone = ParseBone(skeleton_json["skeleton_root"]);
    MotionData motionData = ParseMotionData(motion_json);
    MeshData meshData = ParseMeshData(mesh_json);

    // Create the skeleton root node and attach to the root node of the scene
    aiNode* skeletonNode = CreateAiSkeletonNode(rootBone);
    scene->mRootNode->addChildren(1, &skeletonNode);

    // Add animation data to the scene
    AddAnimationToAiScene(scene, scene->mRootNode, motionData);

    // Create and attach the mesh to the scene
    scene->mNumMeshes = 1;
    scene->mMeshes = new aiMesh*[scene->mNumMeshes];
    scene->mMeshes[0] = CreateAiMesh(meshData);

    // Attach mesh to a node
    aiNode* meshNode = new aiNode();
    meshNode->mName = aiString("MeshNode");
    meshNode->mMeshes = new unsigned int[1];
    meshNode->mMeshes[0] = 0; // Mesh index in the scene
    meshNode->mNumMeshes = 1;
    scene->mRootNode->addChildren(1, &meshNode);

    // Export the scene to an FBX file using Assimp
    Assimp::Exporter exporter;
    if (exporter.Export(scene, "fbx", "output.fbx") != AI_SUCCESS) {
        std::cerr << "Error exporting FBX: " << exporter.GetErrorString() << std::endl;
        return -1;
    }

    // Cleanup
    delete scene;
    return 0;
}


//int main(int argc, const char * argv[]) {
//    // insert code here...
//    std::cout << "Hello, World!\n";
//    return 0;
//}

//int main() {
//    // Initialize Assimp structures
//    aiScene* scene = new aiScene();
//    scene->mRootNode = new aiNode();
//    scene->mRootNode->mName = aiString("Root");
//
//    // Example JSON strings for skeleton and motion data (Replace with actual JSON input)
//    std::string skeletonJsonString = R"(...)"; // Skeleton JSON here
//    std::string motionJsonString = R"(...)"; // Motion JSON here
//
//    // Parse the JSON
//    auto skeleton_json = nlohmann::json::parse(skeletonJsonString);
//    auto motion_json = nlohmann::json::parse(motionJsonString);
//
//    // Parse skeleton and motion data
//    Bone rootBone = ParseBone(skeleton_json["skeleton_root"]);
//    MotionData motionData = ParseMotionData(motion_json);
//
//    // Create the skeleton root node and attach to the root node of the scene
//    aiNode* skeletonNode = CreateAiSkeletonNode(rootBone);
//    scene->mRootNode->addChildren(1, &skeletonNode);
//
//    // Add animation data to the scene
//    AddAnimationToAiScene(scene, scene->mRootNode, motionData);
//
//    // Export the scene to an FBX file using Assimp
//    Assimp::Exporter exporter;
//    if (exporter.Export(scene, "fbx", "output.fbx") != AI_SUCCESS) {
//        std::cerr << "Error exporting FBX: " << exporter.GetErrorString() << std::endl;
//        return -1;
//    }
//
//    // Cleanup
//    delete scene;
//    return 0;
//}




//int main() {
//    FbxManager* lSdkManager = FbxManager::Create();
//    FbxScene* lScene = FbxScene::Create(lSdkManager, "My Scene");
//
//    // Example JSON strings for skeleton and motion data (Replace with actual JSON input)
//    std::string skeletonJsonString = R"(...)" // Skeleton JSON here
//    std::string motionJsonString = R"(...)" // Motion JSON here
//
//    // Parse the JSON
//    auto skeleton_json = nlohmann::json::parse(skeletonJsonString);
//    auto motion_json = nlohmann::json::parse(motionJsonString);
//
//    // Parse skeleton and motion data
//    Bone rootBone = ParseBone(skeleton_json["skeleton_root"]);
//    MotionData motionData = ParseMotionData(motion_json);
//
//    // Create the skeleton root node
//    FbxNode* rootNode = CreateSkeleton(lSdkManager, rootBone);
//    lScene->GetRootNode()->AddChild(rootNode);
//
//    // Add the animation data to the skeleton
//    AddAnimationToSkeleton(lScene, rootNode, motionData);
//
//    // Export the scene to an FBX file
//    FbxExporter* exporter = FbxExporter::Create(lSdkManager, "");
//    if (!exporter->Initialize("output.fbx", -1, lSdkManager->GetIOSettings())) {
//        std::cerr << "Error initializing FBX exporter: " << exporter->GetStatus().GetErrorString() << std::endl;
//        return -1;
//    }
//    exporter->Export(lScene);
//    exporter->Destroy();
//
//    // Cleanup
//    lSdkManager->Destroy();
//    return 0;
//}



//FbxNode* CreateSkeleton(FbxManager* manager, const Bone& bone) {
//    FbxSkeleton* fbxSkeleton = FbxSkeleton::Create(manager, bone.name.c_str());
//    fbxSkeleton->SetSkeletonType(FbxSkeleton::eLimbNode);
//    fbxSkeleton->Size.Set(bone.length);
//
//    FbxNode* node = FbxNode::Create(manager, bone.name.c_str());
//    node->SetNodeAttribute(fbxSkeleton);
//    node->LclTranslation.Set(bone.position);
//    node->LclRotation.Set(bone.orientation);
//
//    for (const auto& child : bone.children) {
//        node->AddChild(CreateSkeleton(manager, child));
//    }
//
//    return node;
//}
//
//void AddAnimationToSkeleton(FbxScene* scene, FbxNode* rootNode, const MotionData& motionData) {
//    FbxAnimStack* animStack = FbxAnimStack::Create(scene, "MotionStack");
//    FbxAnimLayer* animLayer = FbxAnimLayer::Create(scene, "BaseLayer");
//    animStack->AddMember(animLayer);
//
//    // For each part (bone) in the AMC file, find the corresponding node in the FBX skeleton
//    for (size_t i = 0; i < motionData.parts.size(); ++i) {
//        FbxNode* node = rootNode->FindChild(motionData.parts[i].c_str(), true);
//        if (!node) {
//            std::cerr << "Bone " << motionData.parts[i] << " not found in skeleton!" << std::endl;
//            continue;
//        }
//
//        FbxAnimCurve* curveX = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
//        FbxAnimCurve* curveY = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
//        FbxAnimCurve* curveZ = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);
//
//        curveX->KeyModifyBegin();
//        curveY->KeyModifyBegin();
//        curveZ->KeyModifyBegin();
//
//        // Loop through each frame and set the corresponding rotation values
//        for (size_t frame = 0; frame < motionData.smoothing.size(); ++frame) {
//            FbxTime time;
//            time.SetFrame(frame, FbxTime::eFrames30); // Assume 30 FPS
//
//            double rotationX = motionData.smoothing[frame][i * 3 + 3];
//            double rotationY = motionData.smoothing[frame][i * 3 + 4];
//            double rotationZ = motionData.smoothing[frame][i * 3 + 5];
//
//            curveX->KeyAdd(time).SetValue(rotationX);
//            curveY->KeyAdd(time).SetValue(rotationY);
//            curveZ->KeyAdd(time).SetValue(rotationZ);
//        }
//
//        curveX->KeyModifyEnd();
//        curveY->KeyModifyEnd();
//        curveZ->KeyModifyEnd();
//    }
//}


//struct MotionData {
//    std::vector<std::string> parts;
//    std::vector<std::vector<double>> smoothing;
//};
//
//MotionData ParseMotionData(const nlohmann::json& motion_json) {
//    MotionData motionData;
//    motionData.parts = motion_json["motion_parts"].get<std::vector<std::string>>();
//    motionData.smoothing = motion_json["motion_smoothing"].get<std::vector<std::vector<double>>>();
//    return motionData;
//}



//struct Bone {
//    std::string name;
//    FbxVector4 direction;
//    double length;
//    FbxVector4 axis;
//    std::string dof;
//    FbxVector4 order;
//    FbxVector4 position;
//    FbxVector4 orientation;
//    bool u0;
//    bool u1;
//    std::vector<Bone> children;
//};
//
//Bone ParseBone(const nlohmann::json& bone_json) {
//    Bone bone;
//    bone.name = bone_json["bone_name"];
//    bone.direction = FbxVector4(bone_json["bone_direction"][0], bone_json["bone_direction"][1], bone_json["bone_direction"][2]);
//    bone.length = bone_json["bone_length"];
//    bone.axis = FbxVector4(bone_json["bone_axis"][0], bone_json["bone_axis"][1], bone_json["bone_axis"][2]);
//    bone.dof = bone_json["bone_dof"];
//    bone.order = FbxVector4(bone_json["bone_order"][0], bone_json["bone_order"][1], bone_json["bone_order"][2]);
//    bone.position = FbxVector4(bone_json["bone_position"][0], bone_json["bone_position"][1], bone_json["bone_position"][2]);
//    bone.orientation = FbxVector4(bone_json["bone_orientation"][0], bone_json["bone_orientation"][1], bone_json["bone_orientation"][2]);
//    bone.u0 = bone_json["bone_u0"];
//    bone.u1 = bone_json["bone_u1"];
//
//    for (const auto& child : bone_json["bone_hierarchy"]) {
//        bone.children.push_back(ParseBone(child));
//    }
//
//    return bone;
//}
