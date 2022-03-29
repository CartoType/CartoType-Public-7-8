/*
cartotype_transform.h
Copyright (C) 2004-2018 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_TRANSFORM_H__
#define CARTOTYPE_TRANSFORM_H__

#include <cartotype_base.h>
#include <cartotype_arithmetic.h>
#include <cartotype_stream.h>
#include <array>

namespace CartoType
{

/** Transform type flag bits; used to reduce calculation. */
namespace TTransformType
    {
    /** The transform has no effect. */
    constexpr uint32_t Identity = 0;
    /** The transform reflects around the line (y = 0). */
    constexpr uint32_t YReflect = 1;
    /** The transform does translation. */
    constexpr uint32_t Translate = 2;
    /** The transform has an effect, and does something other than reflection and translation. */ 
    constexpr uint32_t General = 4;
    };

/** The components of a transform. */
class TTransformComponents
    {
    public:
    /** The translation. */
    TPointFP iTranslation;
    /** The center around which the transform performas any rotation. */
    TPointFP iCenterOfRotation;
    /** The rotation in radians. */
    double iRotation = 0;
    /** The scale. */
    TPointFP iScale = { 1, 1 };
    /** The shear. */
    TPointFP iShear;
    };

/** A floating point affine 2D transformation. */
class TTransform
    {
    public:
    TTransform();
    TTransform(double aA,double aB,double aC,double aD,double aTx,double aTy);
    explicit TTransform(TDataInputStream& aInput);

    /** Transforms the point (aX,aY). */
    void Transform(double& aX,double& aY) const
        {
        double new_x(iA * aX + iC * aY + iTx);
        double new_y(iB * aX + iD * aY + iTy);
        aX = new_x;
        aY = new_y;
        }
    void Transform(TPoint& aPoint) const;
    void Transform(TPointFP& aPoint) const;
    void Transform(TRect& aRect) const;
    void Transform(TRectFP& aRect) const;
    void Concat(const TTransform& aTransform);
    void Prefix(const TTransform& aTransform);
    void Scale(double aXScale,double aYScale);
    void ExtractScale(double& aXScale,double& aYScale);
    void Invert();
    /** Concatenates the translation (aX,aY) to this transform. */
    void Translate(double aX,double aY) { iTx += aX; iTy += aY; }
    void Rotate(double aAngle);
    /** Concatenates the rotation defined by Atan(aX,aY) to this transform. */
    void Rotate(double aX,double aY) { Rotate(Atan2(aY,aX)); }
    /** Concatenates a reflection about the line y=aY to this transform. */
    void ReflectY(double aY)
        {
        iTy -= aY;
        Scale(1,-1);
        iTy += aY;
        }
    /** Return the A (x scale) parameter. */
    double A() const { return iA; }
    /** Returns the B (y shear) parameter. */
    double B() const { return iB; }
    /** Returns the C (x shear) parameter. */
    double C() const { return iC; }
    /** Returns the D (y scale) parameter. */
    double D() const { return iD; }
    /** Returns the TX (x translation) parameter. */
    double Tx() const { return iTx; }
    /** Returns the TY (y translation) parameter. */
    double Ty() const { return iTy; }
    /** Returns the center of rotation of the projection. */
    TPointFP CenterOfRotation() const;
    TTransformComponents Components() const;
    TTransform Interpolate(const TTransform& aOther,double aTime);
    /** The equality operator. */
    bool operator==(const TTransform& aTransform) const;
    /** The inequality operator. */
    bool operator!=(const TTransform& aTransform) const { return !(*this == aTransform); }
    uint32_t Type() const;
    /** Returns true if this is the identity transform. */
    bool IsIdentity() const { return iA==1 && iB==0 && iC==0 && iD==1 && iTx==0 && iTy==0; }
    void Write(TDataOutputStream& aOutput) const;

    private:
    double iA, iB, iC, iD, iTx, iTy;
    };

/** A 3D transformation. */
class TTransform3D
    {
    public:
    TTransform3D();
    TTransform3D(const TTransform& aTransform);
    /** The equality operator. */
    bool operator==(const TTransform3D& aOther) const { return iM == aOther.iM; }
    void Transform(TPoint3FP& aPoint) const;
    void Transform(double& aX,double& aY,double& aZ,double& aW) const;
    void Concat(const TTransform3D& aTransform);
    void Translate(double aX,double aY,double aZ);
    void Scale(double aXScale,double aYScale,double aZScale);
    void RotateX(double aAngle);
    void RotateY(double aAngle);
    void RotateZ(double aAngle);
    void ReflectY(double aY);
    void Invert();
    double Determinant() const;
    TResult Perspective(double aFieldOfViewYDegrees,double aAspect,double aNear,double aFar);
    TResult Frustum(double aLeft,double aRight,double aBottom,double aTop,double aNear,double aFar);
    TTransform Affine2DTransform() const;
    /** Returns a pointer to the contiguous array of 16 values representing the 4x4 transformation matrix. */
    const double* Data() const { return iM.data(); }
   
    private:
    std::array<double,16> iM; // the transform matrix
    };

/** 
Parameters defining a camera position relative to a flat plane
representing the earth's surface projected on to a map.
*/
class TCameraParam
    {
    public:
    /** The position of the camera. The z coordinate represents height above the surface. */
    TPoint3FP iPosition;
    /** The azimuth of the camera in degrees going clockwise, where 0 is N, 90 is E, etc. */
    double iAzimuthDegrees = 0;
    /** The declination of the camera downward from the horizontal plane. */
    double iDeclinationDegrees = 30;
    /** The amount by which the camera is rotated about its axis, after applying the declination, in degrees going clockwise. */
    double iRotationDegrees = 0;
    /** The camera's field of view in degrees. */
    double iFieldOfViewDegrees = 22.5;
    /**
    The display rectangle on to which the camera view is projected.
    The view width is scaled to that of the display, and the aspect
    ratio of the camera view is preserved.
    */
    TRectFP iDisplay = { 0,0,1,1 };
    /** True if y values increase upwards in the display coordinate system. */
    bool iYAxisUp = false;
    };

} // namespace CartoType

#endif
