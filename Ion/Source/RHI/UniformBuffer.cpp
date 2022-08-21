#include "IonPCH.h"

#include "UniformBuffer.h"
#include "RHI/RHI.h"

namespace Ion
{
	UniformBufferFactory::UniformBufferFactory() :
		m_CurrentSize(0)
	{
	}

	void UniformBufferFactory::Add(const String& name, EUniformType type)
	{
		ionassert(m_Uniforms.find(name) == m_Uniforms.end(), "Uniform already exists.");

		UniformData data { };
		data.Name = name;
		data.Type = type;
		data.Offset = m_CurrentSize;
		m_Uniforms[name] = Move(data);

		m_CurrentSize += (uint32)GetUniformTypeSize(type);
	}

	void UniformBufferFactory::Remove(const String& name)
	{
		auto it = m_Uniforms.find(name);
		if (it != m_Uniforms.end())
		{
			m_CurrentSize -= (uint32)GetUniformTypeSize(it->second.Type);
			m_Uniforms.erase(name);
		}
#if ION_DEBUG
		else
		{
			RHILogger.Warn("[UniformBufferFactory]: Uniform {0} does not exist.", name);
		}
#endif
	}

	std::shared_ptr<RHIUniformBufferDynamic> UniformBufferFactory::Construct()
	{
		void* data = _malloca(m_CurrentSize);
		size_t size = m_CurrentSize;

		for (auto& [name, uniform] : m_Uniforms)
		{
			GetDefaultUniformValue(uniform.Type, (uint8*)data + uniform.Offset);
		}

		std::shared_ptr<RHIUniformBufferDynamic> buffer(RHIUniformBufferDynamic::Create(data, size, m_Uniforms));

		_freea(data);

		return buffer;
	}

	RHIUniformBufferDynamic::RHIUniformBufferDynamic(const UniformDataMap& uniforms) :
		m_UniformDataMap(uniforms)
	{
	}
}
