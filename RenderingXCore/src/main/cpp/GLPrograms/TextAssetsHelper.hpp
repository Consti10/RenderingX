//
// Created by Constantin on 8/16/2018.
//

#ifndef FPV_VR_TEXTASSETS_H
#define FPV_VR_TEXTASSETS_H

#include <vector>
#include <string>
#include <sstream>

class TextAssetsHelper {
public:
    enum TEXT_STYLE{ARIAL_PLAIN,ARIAL_BOLD,ARIAL_ITALIC,ARIAL_BOLDITALIC,MONOSPACE};
public:
    static std::string getDistanceFieldNameByStyle(const TEXT_STYLE& textStyle){
        const int index=static_cast<int>(textStyle);
        std::stringstream ss;
        ss<<"Text/"<<"DistanceFields/"<<"TEXT_ATLAS_DF_"<<index<<".png";
        return ss.str();
    }
    static std::string getOtherDataNameByStyle(const TEXT_STYLE& textStyle){
        const int index=static_cast<int>(textStyle);
        std::stringstream ss;
        ss<<"Text/"<<"OtherData/"<<"TEXT_OTHER_DATA_"<<index<<".ser";
        return ss.str();
    }
};
#endif //FPV_VR_TEXTASSETS_H
