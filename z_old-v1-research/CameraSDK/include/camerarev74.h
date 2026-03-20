//======================================================================================================
// Copyright 2025, NaturalPoint Inc.
//======================================================================================================
#pragma once

// >>> Mini Duo 13 <<< 

#include "camerarev16.h"
#include "camerarevisions.h"
#include "resourcemanager.h"
#include "devicedatastoragebase.h"

namespace CameraLibrary
{
	class cInputBase;
	class CameraRev74Child;
	class CameraRev39;

    class CLAPI CameraRev74 : public CameraRev21
    {
    public:

        CameraRev74();
        ~CameraRev74();
		
		bool IsHighPowerModeSupported() const override { return false; }
		bool IsIRIlluminationAvailable() const override { return false; }

		bool IsTBar() const override { return true; }
		bool IsSyncAuthority() const override { return false; }

		bool IsVideoTypeSupported(Core::eVideoMode mode) const override;

		eRinglightType RinglightType() const override;

		// Device Non-Volatile Data Storage
		int StorageMaxSize() override;

		// Device Non-Volatile File System
		int LoadFile(const char* Filename, unsigned char* buffer, int bufferSize) const override;
		bool SaveFile(const char* Filename, unsigned char* buffer, int bufferSize) const override;

		// Read/write 1024 bytes from EEPROM

		// @brief Append customer data from the device internal non-volatile storage to a buffer via std::vector::insert
		// @param The buffer to load bytes into
		// @param The number of bytes to read between 1 and 1024. 
		// Returns The number of bytes successfully inserted into buffer, and 0 if no data to read or error
		int LoadCustomerData( std::vector<uint8_t>& buffer, int loadLength );

		// @brief Save customer data to the device internal non-volatile storage.
		// @param The buffer to write bytes from.
		// Returns The number of bytes successfully written out, and 0 if no data to write or error
		int SaveCustomerData( const std::vector<uint8_t>& buffer ) const;

	};

	class CameraRev74Child : public CameraRev21
	{
	public:
		CameraRev74Child();
		~CameraRev74Child();


		bool IsHighPowerModeSupported() const override { return false; }
		bool IsIRIlluminationAvailable() const override { return false; }
		bool IsTBar() const override { return true; }

		bool IsVideoTypeSupported(Core::eVideoMode mode) const override;

		bool IsVirtual() const override;

		eCameraState State() const override;

	};
}


