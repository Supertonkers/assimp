//
//  main.cpp
//  asfamc_to_fbx
//
//  Created by Tom on 9/9/24.
//
#include <iostream>
#include "Converter.hpp"
//
//#include <vector>
//#include <string>
//#include <nlohmann/json.hpp>
//
//#include "scene.h"
//#include "Importer.hpp"
//#include "Exporter.hpp"
//#include "postprocess.h"
//#include "Utils.hpp"
//#include "asfamc.hpp"

//aiNode* CreateAiSkeletonNode(const asf::Bone& bone) {
//    aiNode* node = new aiNode();
//    node->mName = aiString(bone.name);
//    node->mTransformation = aiMatrix4x4(
//        aiVector3D(1.0f, 1.0f, 1.0f),
//        aiQuaternion(bone.orientation.x, bone.orientation.y, bone.orientation.z, bone.orientation.w),
//        aiVector3D(bone.position.x, bone.position.y, bone.position.z)
//    );
//
//    node->mNumChildren = bone.children.size();
//    node->mChildren = new aiNode*[node->mNumChildren];
//    for (size_t i = 0; i < node->mNumChildren; ++i) {
//        node->mChildren[i] = CreateAiSkeletonNode(bone.children[i]);
//    }
//
//    return node;
//}
//
//void AddAnimationToAiScene(aiScene* scene, aiNode* rootNode, const amc::MotionData& motionData) {
//    aiAnimation* anim = new aiAnimation();
//    anim->mName = aiString("MotionAnimation");
//    anim->mDuration = motionData.smoothing.size();
//    anim->mTicksPerSecond = 30; // Assuming 30 FPS
//    anim->mNumChannels = motionData.parts.size();
//    anim->mChannels = new aiNodeAnim*[anim->mNumChannels];
//
//    for (size_t i = 0; i < motionData.parts.size(); ++i) {
//        aiNode* node = rootNode->FindNode(aiString(motionData.parts[i]));
//        if (!node) {
//            std::cerr << "Bone " << motionData.parts[i] << " not found in skeleton!" << std::endl;
//            continue;
//        }
//
//        aiNodeAnim* channel = new aiNodeAnim();
//        channel->mNodeName = aiString(motionData.parts[i]);
//        channel->mNumRotationKeys = motionData.smoothing.size();
//        channel->mRotationKeys = new aiQuatKey[channel->mNumRotationKeys];
//
//        for (size_t frame = 0; frame < motionData.smoothing.size(); ++frame) {
//            aiQuatKey& rotKey = channel->mRotationKeys[frame];
//            rotKey.mTime = frame;
//            rotKey.mValue = aiQuaternion(
//                motionData.smoothing[frame][i * 3 + 3],
//                motionData.smoothing[frame][i * 3 + 4],
//                motionData.smoothing[frame][i * 3 + 5],
//                1.0f // Assuming w=1.0f for simplicity
//            );
//        }
//
//        anim->mChannels[i] = channel;
//    }
//
//    scene->mNumAnimations = 1;
//    scene->mAnimations = new aiAnimation*[scene->mNumAnimations];
//    scene->mAnimations[0] = anim;
//}
//
//aiMesh* CreateAiMesh(const mesh::MeshData& meshData) {
//    aiMesh* mesh = new aiMesh();
//    mesh->mNumVertices = meshData.vertices.size();
//    mesh->mVertices = new aiVector3D[mesh->mNumVertices];
//    mesh->mNormals = new aiVector3D[mesh->mNumVertices];
//    mesh->mTextureCoords[0] = new aiVector3D[mesh->mNumVertices];
//
//    for (size_t i = 0; i < mesh->mNumVertices; ++i) {
//        mesh->mVertices[i] = meshData.vertices[i];
//        mesh->mNormals[i] = meshData.normals[i];
//        mesh->mTextureCoords[0][i] = meshData.uvs[i];
//    }
//
//    mesh->mNumFaces = meshData.indices.size() / 3;
//    mesh->mFaces = new aiFace[mesh->mNumFaces];
//
//    for (size_t i = 0; i < mesh->mNumFaces; ++i) {
//        aiFace& face = mesh->mFaces[i];
//        face.mNumIndices = 3;
//        face.mIndices = new unsigned int[face.mNumIndices];
//        face.mIndices[0] = meshData.indices[i * 3 + 0];
//        face.mIndices[1] = meshData.indices[i * 3 + 1];
//        face.mIndices[2] = meshData.indices[i * 3 + 2];
//    }
//
//    return mesh;
//}

int main() {
    // WOrking direcotry
    //"/Users/tom/work/asfamc_to_fbx/assimp/build/Debug"
//    std::filesystem::path currentPath = std::filesystem::current_path();
//        std::cout << "Current working directory: " << currentPath << std::endl;
//        return 0;
    
    Converter converter;
    
    
    return converter.Convert();
    
//    // Initialize Assimp structures
//    aiScene* scene = new aiScene();
//    scene->mRootNode = new aiNode();
//    scene->mRootNode->mName = aiString("Root");
//
//    // Example JSON strings for skeleton, motion, and mesh data (Replace with actual JSON input)
//    std::string skeletonJsonString = R"(...)"; // Skeleton JSON here
//    std::string motionJsonString = R"(...)"; // Motion JSON here
//    std::string meshJsonString = R"(...)"; // Mesh JSON here
//
//    // Parse the JSON
//    auto skeleton_json = nlohmann::json::parse(skeletonJsonString);
//    auto motion_json = nlohmann::json::parse(motionJsonString);
//    auto mesh_json = nlohmann::json::parse(meshJsonString);
//
//    // Parse skeleton, motion, and mesh data
//    asf::Bone rootBone = asf::ParseBone(skeleton_json["skeleton_root"]);
//    amc::MotionData motionData = amc::ParseMotionData(motion_json);
//    mesh::MeshData meshData = mesh::ParseMeshData(mesh_json);
//
//    // Create the skeleton root node and attach to the root node of the scene
//    aiNode* skeletonNode = CreateAiSkeletonNode(rootBone);
//    scene->mRootNode->addChildren(1, &skeletonNode);
//
//    // Add animation data to the scene
//    AddAnimationToAiScene(scene, scene->mRootNode, motionData);
//
//    // Create and attach the mesh to the scene
//    scene->mNumMeshes = 1;
//    scene->mMeshes = new aiMesh*[scene->mNumMeshes];
//    scene->mMeshes[0] = CreateAiMesh(meshData);
//
//    // Attach mesh to a node
//    aiNode* meshNode = new aiNode();
//    meshNode->mName = aiString("MeshNode");
//    meshNode->mMeshes = new unsigned int[1];
//    meshNode->mMeshes[0] = 0; // Mesh index in the scene
//    meshNode->mNumMeshes = 1;
//    scene->mRootNode->addChildren(1, &meshNode);
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
}
