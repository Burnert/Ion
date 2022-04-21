#include "IonPCH.h"

#include "MeshResource.h"
#include "ResourceManager.h"

namespace Ion
{
	TShared<MeshResource> MeshResource::Query(const Asset& asset)
	{
		return Resource::Query<MeshResource>(asset);
	}

	bool MeshResource::IsLoaded() const
	{
		return m_RenderData.IsAvailable();
	}
}
