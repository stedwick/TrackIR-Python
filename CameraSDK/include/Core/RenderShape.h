//======================================================================================================
// Copyright 2019, NaturalPoint Inc.
//======================================================================================================
#pragma once

// Local includes
#include <string>
#include "Vector3.h"
#include "Vector4.h"
#include "Quaternion.h"
#include "UID.h"

namespace Core
{
	/// <summary>
	/// Convenience shapes that developers can render when debugging pipelines, algorithms, etc.
	/// Internal Use Only
	/// </summary>
	struct sRenderShape
	{
		enum eShapeType
		{
			Point,
			Line,
			Axis,
			Arrow
		};

		eShapeType mType = Point;
		cVector3f mPosition = cVector3f::kZero;
		cVector3f mEndPosition = cVector3f::kZero;
		cQuaternionf mOrientation = cQuaternionf::kIdentity;
		cVector4f mColor = cVector4f::kZero;
        float mSize = 0;
		std::wstring mText;
		cUID mID = cUID::kInvalid; //if needed

		sRenderShape( cVector3f position, cVector4f color, float size = 0.02f )
			: mType( Point ), mPosition( position ), mColor( color ), mSize( size ) { }

		sRenderShape( cVector3f position, cVector3f endPosition, cVector4f color )
			: mType( Line ), mPosition( position ), mEndPosition( endPosition ), mColor( color ) { }

		sRenderShape( cVector3f position, cQuaternionf orientation, float size = 0.08f )
			: mType( Axis ), mPosition( position ), mOrientation( orientation ), mSize( size ) { }

		sRenderShape( cVector3f position, cVector3f endPosition, float tipLength, cVector4f color )
			: mType( Arrow ), mPosition( position ), mEndPosition( endPosition ), mSize( tipLength ), mColor( color ) { }
	};

	enum eGeometryTypes
	{
		None,
		Sphere,
		Box,
		Cylinder,
		SkeletonSegment,
		CustomModel
	};
}

