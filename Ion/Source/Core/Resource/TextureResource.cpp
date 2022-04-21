#include "IonPCH.h"

#include "TextureResource.h"
#include "ResourceManager.h"

namespace Ion
{
	TShared<TextureResource> TextureResource::Query(const Asset& asset)
	{
		return Resource::Query<TextureResource>(asset);
	}
}
