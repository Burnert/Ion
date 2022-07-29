#include "IonPCH.h"

#include "Resource.h"
#include "ResourceManager.h"
#include "Core/Asset/AssetRegistry.h"

namespace Ion
{
	Resource::~Resource()
	{
		ResourceManager::Unregister(this);
	}


	ResourcePtr Resource::GetPointer() const
	{
		return ResourceManager::Find(m_Guid);
	}

	Resource::Resource(const GUID& guid, const Asset& asset) :
		m_Guid(guid),
		m_Asset(asset)
	{
	}
}
