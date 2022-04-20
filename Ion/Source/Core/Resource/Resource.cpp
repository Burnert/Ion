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

	Resource::Resource(const Asset& asset) :
		m_Asset(asset)
	{
	}

	TShared<TextureResource> TextureResource::Query(const Asset& asset)
	{
		return Resource::Query<TextureResource>(asset);
	}

	TShared<MeshResource> MeshResource::Query(const Asset& asset)
	{
		return Resource::Query<MeshResource>(asset);
	}
}
