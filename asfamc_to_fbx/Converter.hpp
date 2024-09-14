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
    aiNode* CreateAiSkeletonNode(const asf::Bone& bone);
    
    void AddAnimationToAiScene(aiScene* scene, aiNode* rootNode, const amc::MotionData& motionData);
    
    aiMesh* CreateAiMesh(const mesh::MeshData& meshData);
};

#endif /* Converter_hpp */
