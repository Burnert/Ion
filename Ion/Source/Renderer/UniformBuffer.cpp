#include "IonPCH.h"

#include "UniformBuffer.h"
#include "RenderAPI/RenderAPI.h"
#include "RenderAPI/DX11/DX11Buffer.h"

namespace Ion
{
	TShared<UniformBuffer> UniformBuffer::Create(void* data, size_t size)
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::DX11:
			return MakeShared<DX11UniformBuffer>(data, size);
		default:
			return TShared<UniformBuffer>(nullptr);
		}
	}
}
