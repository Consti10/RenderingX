//
// Created by geier on 22/04/2020.
//

#ifndef RENDERINGX_INDICES_HPP
#define RENDERINGX_INDICES_HPP

#include <vector>
#include <MDebug.hpp>
#include "GLES2/gl2.h"

namespace Indices{
    // takes a mesh that uses indices and removes indices by duplicating/adding vertices
    // less memory efficient, but easier to draw / debug
    template<class T>
    static T mergeIndicesIntoVertices(const T& vertices, const std::vector<GLuint>& indices){
        T ret={};
        ret.reserve(indices.size());
        for(unsigned int index:indices){
            if(index>=vertices.size()){
                LOGE("Indices")<<"Error wanted "<<index;
            }
            ret.push_back(vertices.at(index));
        }
        return ret;
    }
}

#endif //RENDERINGX_INDICES_HPP
