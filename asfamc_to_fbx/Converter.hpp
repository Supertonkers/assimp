//
//  Converter.hpp
//  asfamc_to_fbx
//
//  Created by Tom on 9/14/24.
//

#ifndef Converter_hpp
#define Converter_hpp

#include <stdio.h>
#include "Importer.hpp"
#include "scene.h"
#include "asfamc.hpp"

class Converter {
public:
    
    int Convert();

private:
    
    void init();
    
    aiMesh* CreateAiMesh(const mesh::MeshData& meshData);
    aiMaterial* CreateAiMaterial(const std::string& texturePath);
    
    aiNode* CreateAiSkeletonNode(const asf::Bone& bone);
    
    void AddAnimationToAiScene(aiScene* scene, aiNode* rootNode, const amc::MotionData& motionData, const mesh::MeshData& meshData);
    
    
    const std::string ASSETS = "../../../assets/test/";
    const std::string ASSETS_MESH = ASSETS + "MESH";
    const std::string ASSETS_MOTION = ASSETS + "MOTION";
    const std::string ASSETS_RENDER = ASSETS + "RENDER";
    const std::string ASSETS_SKELETON = ASSETS + "SKELETON";
    const std::string ASSETS_TEXTURE = ASSETS + "TEXTURE";
    
    std::vector<std::string> meshPaths;
    std::vector<std::string> motionPaths;
    std::vector<std::string> renderPaths;
    std::vector<std::string> skeletonPaths;
    std::vector<std::string> texturePaths;
    
};

#endif /* Converter_hpp */
