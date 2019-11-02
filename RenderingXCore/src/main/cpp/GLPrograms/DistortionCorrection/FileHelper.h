//
// Created by Consti10 on 01/11/2019.
//

#ifndef RENDERINGX_FILEHELPER_H
#define RENDERINGX_FILEHELPER_H

#include <string>
#include <sys/stat.h>
#include <Helper/MDebug.hpp>

class FileHelper{
public:
    static std::string createDirectoriesIfNotYetExisting(const std::string& existingDirectory,const std::string& newDirectory){
        int result=mkdir((existingDirectory+newDirectory).c_str(), 0777);
        if(result==0){

        }
        return std::string();
    }
    static std::string renderingXCoreDistortionDirectory(const std::string& androidExternalStorageDirectory){
        return androidExternalStorageDirectory+std::string("RenderingXCore/")+std::string("Distortion/");
    }

    static void createRenderingXCoreDistortionDirectoryIfNotAlreadyExisting(const std::string &androidExternalStorageDirectory){
        //The external storage directory always already exists, first create the RenderingXCore dir
        const std::string renderingXCoreDir=androidExternalStorageDirectory+std::string("RenderingXCore/");
        int result=mkdir(renderingXCoreDir.c_str(), 0777);
        LOGD("Mkdir1 returned %d", result);
        const std::string distortionValuesDirectory=renderingXCoreDir+std::string("Distortion/");
        result=mkdir(distortionValuesDirectory.c_str(), 0777);
        LOGD("Mkdir2 returned %d", result);
    }
};
#endif //RENDERINGX_FILEHELPER_H
