//
// Created by geier on 22/04/2020.
//

#ifndef RENDERINGX_INDICES_HPP
#define RENDERINGX_INDICES_HPP

#include <vector>
#include <AndroidLogger.hpp>
#include "GLES2/gl2.h"

namespace IndicesHelper{
    // takes a mesh that uses indices and removes indices by duplicating/adding vertices
    // less memory efficient, but easier to draw / debug
    template<class T>
    static std::vector<T> mergeIndicesIntoVertices(const std::vector<T>& vertices, const std::vector<GLuint>& indices){
        std::vector<T> ret={};
        ret.reserve(indices.size());
        for(unsigned int index:indices){
            if(index>=vertices.size()){
                MLOGE<<"Error wanted "<<index;
            }
            ret.push_back(vertices.at(index));
        }
        return ret;
    }
}

#endif //RENDERINGX_INDICES_HPP
