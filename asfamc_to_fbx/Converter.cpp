//
//  Converter.cpp
//  asfamc_to_fbx
//
//  Created by Tom on 9/14/24.
//

#include <iostream>
#include "Converter.hpp"

#include <vector>
#include <string>
#include <nlohmann/json.hpp>

#include "scene.h"
#include "Importer.hpp"
#include "Exporter.hpp"
#include "postprocess.h"
#include "Utils.hpp"
#include "asfamc.hpp"

void func() {
    
}

void Converter::init() {
    meshPaths = get_files_in_directory(ASSETS_MESH);
    motionPaths = get_files_in_directory(ASSETS_MOTION);
    renderPaths = get_files_in_directory(ASSETS_RENDER);
    skeletonPaths = get_files_in_directory(ASSETS_SKELETON);
    texturePaths = get_files_in_directory(ASSETS_TEXTURE);
}
    
int Converter::Convert() {
    init();
    
    // Initialize Assimp structures
    aiScene* scene = new aiScene();
    scene->mRootNode = new aiNode();
    scene->mRootNode->mName = aiString("Root");
    
    auto meshPath = meshPaths[13];

    // Example JSON strings for skeleton, motion, and mesh data (Replace with actual JSON input)
    std::string skeletonJsonString = parse_json_file_to_string(skeletonPaths[0]); // Skeleton JSON here
    std::string motionJsonString = parse_json_file_to_string(motionPaths[0]); // Motion JSON here
    std::string meshJsonString = parse_json_file_to_string(meshPath); // Mesh JSON here

    // Parse the JSON
    auto skeleton_json = nlohmann::json::parse(skeletonJsonString);
    auto motion_json = nlohmann::json::parse(motionJsonString);
    auto mesh_json = nlohmann::json::parse(meshJsonString);

    // Parse skeleton, motion, and mesh data
    asf::Bone rootBone = asf::ParseBone(skeleton_json["skeleton_root"]);
    amc::MotionData motionData = amc::ParseMotionData(motion_json);
    mesh::MeshData meshData = mesh::ParseMeshData(mesh_json);
    
    // TODO: Assign texture path to mesh.
    meshData.texturePath = get_file_name_with_extension(texturePaths[0]);

    // Create the skeleton root node and attach to the root node of the scene
    aiNode* skeletonNode = CreateAiSkeletonNode(rootBone);
    scene->mRootNode->addChildren(1, &skeletonNode);

    // Add animation data to the scene
    AddAnimationToAiScene(scene, scene->mRootNode, motionData, meshData);

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

    // Create material with the texture and assign it to the mesh
    scene->mNumMaterials = 1;
    scene->mMaterials = new aiMaterial*[scene->mNumMaterials];
    scene->mMaterials[0] = CreateAiMaterial(meshData.texturePath);

    // Link the mesh to the material
    scene->mMeshes[0]->mMaterialIndex = 0;

    // Export the scene to an FBX file using Assimp
    Assimp::Exporter exporter;
    auto outputPath = get_file_name(meshPath) + ".fbx";
    if (exporter.Export(scene, "fbx", outputPath) != AI_SUCCESS) {
        std::cerr << "Error exporting FBX: " << exporter.GetErrorString() << std::endl;
        return -1;
    }

    // Cleanup
    delete scene;
    return 0;
}

void Converter::AddAnimationToAiScene(aiScene* scene, aiNode* rootNode, const amc::MotionData& motionData, const mesh::MeshData& meshData) {
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

aiNode* Converter::CreateAiSkeletonNode(const asf::Bone& bone) {
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

aiMesh* Converter::CreateAiMesh(const mesh::MeshData& meshData) {
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

aiMaterial* Converter::CreateAiMaterial(const std::string& texturePath) {
    aiMaterial* material = new aiMaterial();
    
    // Load the texture
    aiString path(texturePath);
    material->AddProperty(&path, AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0));

    return material;
}
