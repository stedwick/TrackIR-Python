//======================================================================================================
// Copyright 2019, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include <string>
#include <vector>
#include <limits>

#include "Core/UID.h"

namespace Core
{
    // NOTE don't change this without rewriting cAssetNode::Load
    struct cDOFElement
    {
        enum eChannel
        {
            Error = -1,
            TX = 0,
            TY,
            TZ,
            RX,
            RY,
            RZ,
            QuaternionX,
            QuaternionY,
            QuaternionZ,
            QuaternionW,
        };

        Core::cUID mBoneID;
        eChannel mChannel;
        float mWeight;

        cDOFElement() : mBoneID( Core::cUID::kInvalid ), mChannel( Error ), mWeight( 1.0f ) { }

        cDOFElement( const Core::cUID& id, const eChannel channel, float weight = 1.0f )
            : mBoneID( id )
            , mWeight( weight )
            , mChannel( channel )
        {
        }

        cDOFElement( const Core::cUID& id, const std::wstring& channelName, float weight = 1.0f )
            : mBoneID( id )
            , mWeight( weight )
            , mChannel( Error )
        {
            
            if (!channelName.empty())
            {
                wchar_t first = channelName.front();
                wchar_t last = channelName.back();
                bool isX = ( last == 'X' || last == 'x' );
                bool isY = ( last == 'Y' || last == 'y' );
                bool isZ = ( last == 'Z' || last == 'z' );
                bool isW = ( last == 'W' || last == 'w' );

                if (first == 'T' || first == 't')
                {
                    if (isX) mChannel = TX;
                    else if (isY) mChannel = TY;
                    else if (isZ) mChannel = TZ;
                }
                else if (first == 'R' || first == 'r')
                {
                    if (isX) mChannel = RX;
                    else if (isY) mChannel = RY;
                    else if (isZ) mChannel = RZ;
                }
                else if( first == 'Q' || first == 'q' )
                {
                    if( isX ) mChannel = QuaternionX;
                    else if( isY ) mChannel = QuaternionY;
                    else if( isZ ) mChannel = QuaternionZ;
                    else if( isW ) mChannel = QuaternionW;
                }
            }
        }

        std::wstring ChannelName() const
        {
            static std::wstring names[10] = { L"TX", L"TY", L"TZ", L"RX", L"RY", L"RZ", L"QX", L"QY", L"QZ", L"QW" };

            if( mChannel >= 0 && mChannel < 10 )
            {
                return names[mChannel];
            }
            return L"";
        }
    };

    class cDOFDefinition
    {
    private:
        std::wstring mName;
        float mMinValue;
        float mMaxValue;
        float mSpringWeight;
        float mMinSpringWeight;
        float mSpringTarget;
        std::vector<cDOFElement> mElements;

    public:
        cDOFDefinition( const std::wstring& name, float minValue = -std::numeric_limits<float>::infinity(), 
            float maxValue = std::numeric_limits<float>::infinity(), float springWeight = 0.0f, float springTarget = 0.0f, 
            float minSpringWeight = 0.0f )
        {
            mName = name;
            mMinValue = minValue;
            mMaxValue = maxValue;
            mSpringWeight = springWeight;
            mMinSpringWeight = minSpringWeight;
            mSpringTarget = springTarget;
        }

        bool IsDefault() const
        {
            return mMinValue == -std::numeric_limits<float>::infinity() &&
                   mMaxValue == std::numeric_limits<float>::infinity() &&
                   mSpringWeight == 0.0f &&
                   mSpringTarget == 0.0f &&
                   mElements.size() == 1;
        }

        const std::wstring& Name() const { return mName; }

        float MinValue() const { return mMinValue; }
        void SetMinValue( float value ) { mMinValue = value; }

        float MaxValue() const { return mMaxValue; }
        void SetMaxValue( float value ) { mMaxValue = value; }

        float SpringWeight() const { return mSpringWeight; }
        void SetSpringWeight( float value ) { mSpringWeight = value; }

        float MinSpringWeight() const { return mMinSpringWeight; }
        void SetMinSpringWeight( float value ) { mMinSpringWeight = value; }

        float SpringTarget() const { return mSpringTarget; }
        void SetSpringTarget( float value ) { mSpringTarget = value; }

        std::vector<cDOFElement>& Elements() { return mElements; }
        const std::vector<cDOFElement>& Elements() const { return mElements; }

        void AddElement( const cDOFElement& element ) { mElements.push_back( element ); }
        void ClearElements() { mElements.clear(); }

        void NormalizeWeight()
        {
            float totalWeight = 0;
            for( const cDOFElement& elem : mElements )
            {
                totalWeight += std::abs( elem.mWeight );
            }

            if( totalWeight > 0 && totalWeight != 1 )
            {
                float scale = 1.0f / totalWeight;
                for( cDOFElement& elem : mElements )
                {
                    elem.mWeight *= scale;
                }
            }
        }
    };
}
