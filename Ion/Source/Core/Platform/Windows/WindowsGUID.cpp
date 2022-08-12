#include "IonPCH.h"

#include "WindowsCore.h"
#include "Core/GUID.h"

#pragma comment(lib, "Rpcrt4.lib")

namespace Ion
{
	static void ConvertWindowsUUIDToBytes(const UUID& uuid, uint8* outBytes)
	{
		*outBytes++ = (uint8)((uuid.Data1 >> 24) & 0xFF);
		*outBytes++ = (uint8)((uuid.Data1 >> 16) & 0xFF);
		*outBytes++ = (uint8)((uuid.Data1 >> 8) & 0xFF);
		*outBytes++ = (uint8)((uuid.Data1) & 0xFF);

		*outBytes++ = (uint8)((uuid.Data2 >> 8) & 0xFF);
		*outBytes++ = (uint8)((uuid.Data2) & 0xFF);

		*outBytes++ = (uint8)((uuid.Data3 >> 8) & 0xFF);
		*outBytes++ = (uint8)((uuid.Data3) & 0xFF);

		*outBytes++ = uuid.Data4[0];
		*outBytes++ = uuid.Data4[1];
		*outBytes++ = uuid.Data4[2];
		*outBytes++ = uuid.Data4[3];
		*outBytes++ = uuid.Data4[4];
		*outBytes++ = uuid.Data4[5];
		*outBytes++ = uuid.Data4[6];
		*outBytes++ = uuid.Data4[7];
	}

	static void ConvertBytesToWindowsUUID(const uint8* bytes, UUID& outUUID)
	{
		outUUID = { };

		outUUID.Data1 |= ((uint32)*bytes++) << 24;
		outUUID.Data1 |= ((uint32)*bytes++) << 16;
		outUUID.Data1 |= ((uint32)*bytes++) << 8;
		outUUID.Data1 |= ((uint32)*bytes++);

		outUUID.Data2 |= ((uint16)*bytes++) << 8;
		outUUID.Data2 |= ((uint16)*bytes++);

		outUUID.Data3 |= ((uint16)*bytes++) << 8;
		outUUID.Data3 |= ((uint16)*bytes++);

		outUUID.Data4[0] = *bytes++;
		outUUID.Data4[1] = *bytes++;
		outUUID.Data4[2] = *bytes++;
		outUUID.Data4[3] = *bytes++;
		outUUID.Data4[4] = *bytes++;
		outUUID.Data4[5] = *bytes++;
		outUUID.Data4[6] = *bytes++;
		outUUID.Data4[7] = *bytes++;
	}

	GUIDBytesArray GUID::PlatformGenerateGUID()
	{
		UUID uuid;
		RPC_STATUS status = UuidCreate(&uuid);
		
		if (status == RPC_S_UUID_LOCAL_ONLY)
		{
			WindowsLogger.Warn("The UUID is guaranteed to be unique to this computer only.");
		}
		if (status == RPC_S_UUID_NO_ADDRESS)
		{
			WindowsLogger.Warn("Cannot get Ethernet or token - ring hardware address for this computer.");
		}

		GUIDBytesArray bytes;
		ConvertWindowsUUIDToBytes(uuid, (uint8*)&bytes);
		return bytes;
	}

	Result<GUIDBytesArray, StringConversionError> GUID::PlatformGenerateGUIDFromString(const String& str)
	{
		UUID uuid;
		RPC_STATUS status = UuidFromStringA((unsigned char*)str.c_str(), &uuid);

		if (status == RPC_S_INVALID_STRING_UUID)
		{
			ionthrow(StringConversionError, "Invalid UUID string. -> {0}", str);
		}

		GUIDBytesArray bytes;
		ConvertWindowsUUIDToBytes(uuid, (uint8*)&bytes);
		return bytes;
	}

	String GUID::PlatformGUIDToString() const
	{
		UUID uuid;
		ConvertBytesToWindowsUUID((uint8*)&m_Bytes, uuid);

		RPC_CSTR rpcUuidStr;
		RPC_STATUS status = UuidToStringA(&uuid, &rpcUuidStr);

		if (status != RPC_S_OK)
		{
			WindowsLogger.Warn("Could not convert UUID to String.");
			return "00000000-0000-0000-0000-000000000000";
		}

		String uuidStr = (char*)rpcUuidStr;

		RpcStringFreeA(&rpcUuidStr);

		return uuidStr;
	}

#if ION_DEBUG
	void GUID::CacheString()
	{
		m_AsString = ToString();
	}
#endif
}
