#include "IonPCH.h"

#include "UniformBuffer.h"
#include "RHI/RHI.h"

namespace Ion
{
	RHIUniformBuffer::RHIUniformBuffer(const UniformDataMap& uniforms) :
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

	void UniformBufferFactory::Construct(TShared<RHIUniformBuffer>& outUniformBuffer)
	{
		RHIUniformBuffer* buffer = nullptr;
		Construct(buffer);
		outUniformBuffer.reset(buffer);
	}

	void UniformBufferFactory::Construct(RHIUniformBuffer*& outUniformBuffer)
	{
		size_t size = 0;

		void* data = malloc(size);
		outUniformBuffer = RHIUniformBuffer::Create(data, size, m_Uniforms);
	}
}
