#include "IonPCH.h"

#include "UniformBuffer.h"
#include "RenderAPI/RenderAPI.h"
#include "RenderAPI/DX11/DX11Buffer.h"

namespace Ion
{
	UniformBuffer* UniformBuffer::Create(void* initialData, size_t size)
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::DX11:
			return new DX11UniformBuffer(initialData, size);
		default:
			return nullptr;
		}
	}

	UniformBuffer* UniformBuffer::Create(void* data, size_t size, const UniformDataMap& uniforms)
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::DX11:
			return new DX11UniformBuffer(data, size, uniforms);
		default:
			return nullptr;
		}
	}

	UniformBuffer::UniformBuffer(const UniformDataMap& uniforms) :
		m_Uniforms(MakeUnique<const UniformDataMap>(uniforms))
	{ }

	void UniformBufferFactory::Add(const String& name, EUniformType type)
	{
		ionassert(m_Uniforms.find(name) == m_Uniforms.end(), "Uniform already exists.");

		UniformData data { };
		data.Name = name;
		data.Type = type;
		m_Uniforms[name] = Move(data);
	}

	void UniformBufferFactory::Remove(const String& name)
	{
		if (m_Uniforms.find(name) != m_Uniforms.end())
		{
			m_Uniforms.erase(name);
		}
#if ION_DEBUG
		else
		{
			LOG_WARN("[UniformBufferFactory]: Uniform {0} does not exist.", name);
		}
#endif
	}

	void UniformBufferFactory::Construct(TShared<UniformBuffer>& outUniformBuffer)
	{
		UniformBuffer* buffer = nullptr;
		Construct(buffer);
		outUniformBuffer.reset(buffer);
	}

	void UniformBufferFactory::Construct(UniformBuffer*& outUniformBuffer)
	{
		size_t size = 0;

		void* data = malloc(size);
		outUniformBuffer = UniformBuffer::Create(data, size, m_Uniforms);
	}
}
