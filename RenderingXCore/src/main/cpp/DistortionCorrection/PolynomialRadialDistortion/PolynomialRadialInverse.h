//
// Created by geier on 30/04/2020.
//

#ifndef RENDERINGX_POLYNOMIALRADIALINVERSE_H
#define RENDERINGX_POLYNOMIALRADIALINVERSE_H

#include "PolynomialRadialDistortion.h"

// Inverse polynomial is a close approximation of the inverse in the range [0..MAX_RAD_SQ]
// Needed for Vertex displacement distortion correction and can be used to speed up creation of distortion mesh
// TODO unit testing. Works but there is still something wrong with the max rad sq
class PolynomialRadialInverse:private PolynomialRadialDistortion{
public:
    // Takes a PolynomialRadialDistortion and creates the approximate inverse for the given range.
    // More coefficients do often, but not always result in a better fit
    PolynomialRadialInverse(const PolynomialRadialDistortion& parent,const float maxRad,const unsigned int NUM_COEFFICIENTS):
            PolynomialRadialDistortion(PolynomialRadialInverse::getApproximateInverseCoefficients(parent,maxRad,NUM_COEFFICIENTS)),maxRadSq(maxRad*maxRad){
    };
    //Default constructor: create a identity distortion (the inverse of 0 is also 0)
    PolynomialRadialInverse(const int NUM_COEFFICIENTS=2):PolynomialRadialDistortion(std::vector<float>((unsigned)NUM_COEFFICIENTS,0)),maxRadSq(100000){
    };
public:
    using PolynomialRadialDistortion::getCoefficients;
public:
    using PolynomialRadialDistortion::Distort;
    /*float DistortionFactor(float r_squared) const{
        if(r_squared>maxRadSq){
            r_squared=maxRadSq;
        }
        return PolynomialRadialDistortion::DistortionFactor(r_squared);
    }*/
    using PolynomialRadialDistortion::DistortionFactor;

    using PolynomialRadialDistortion::DistortRadius;
    // PolynomialRadialDistortion::DistortInverse makes no sense - we are already a inverse function
    std::string toStringX()const;
    float getMaxRadSq()const{ return maxRadSq;};
public:
    //calculate coefficients for a polynomial function that describes the inverse of this distortion
    static std::vector<float> getApproximateInverseCoefficients(const PolynomialRadialDistortion& distortion,float maxRadius,
                                                                unsigned int numCoefficients);

    // The un-distortion function created by getApproximateInverseCoefficients is not a perfect fit
    // calculate the deviation between the real un-distortion value (as obtained by @DistortInverse)
    // {Note: this value is also not 'absolute true'}
    // and the value generated by the inverse polynomial distortion function
    static float calculateDeviation(const PolynomialRadialDistortion& distortion,const PolynomialRadialInverse& inverseDistortion,const float radius);

    // Given a polynomialRadialDistortion and a inverse defined in the range [0..maxRadSq]
    // Loop trough the defined range using step size and return the largest deviation
    // also see @calculateDeviation
    static float calculateMaxDeviation(const PolynomialRadialDistortion& distortion,const PolynomialRadialInverse& inverse,const float stepSize=0.01f);

    //TODO
    // Calculate RMSE (Root mean square error) of the inverse in the range [0..maxRadSq]
public:
    float maxRadSq;
};


#endif //RENDERINGX_POLYNOMIALRADIALINVERSE_H
