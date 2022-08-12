#pragma once

#include "Core/CoreTypes.h"
#include "RendererFwd.h"

#define UNIFORMBUFFER alignas(16)

#if ION_FORCE_SHADER_DEBUG || (ION_DEBUG && ION_ENABLE_SHADER_DEBUG)
#define SHADER_DEBUG_ENABLED 1
#else
#define SHADER_DEBUG_ENABLED 0
#endif

namespace Ion
{
	REGISTER_LOGGER(RendererLogger, "Renderer");

	struct EditorViewportTextures
	{
		TShared<RHITexture> SceneFinalColor;
		TShared<RHITexture> SceneFinalDepth;
		TShared<RHITexture> SelectedDepth;
	};

	struct ViewportDescription
	{
		int32 X;
		int32 Y;
		uint32 Width;
		uint32 Height;
		float MinDepth;
		float MaxDepth;

		ViewportDescription() :
			X(0),
			Y(0),
			Width(1),
			Height(1),
			MinDepth(0.0f),
			MaxDepth(1.0f)
		{
		}

		inline IVector2 GetOrigin()
		{
			return IVector2(X, Y);
		}

		inline UVector2 GetSize()
		{
			return UVector2(Width, Height);
		}

		inline bool operator==(const ViewportDescription& other) const
		{
			return
				X == other.X &&
				Y == other.Y &&
				Width == other.Width &&
				Height == other.Height &&
				MinDepth == other.MinDepth &&
				MaxDepth == other.MaxDepth;
		}
	};

	struct RBillboardRenderProxy
	{
		const RHITexture* Texture;
		Vector3 LocationWS;
		float Scale;
	};

	struct RPrimitiveRenderProxy
	{
		const RHIVertexBuffer* VertexBuffer;
		const RHIIndexBuffer* IndexBuffer;
		const RHIUniformBuffer* UniformBuffer;
		const MaterialOld* MaterialOld;
		const MaterialInstance* MaterialInstance;
		const RHIShader* Shader;
		Matrix4 Transform;
	};

	struct REditorPassPrimitive
	{
		GUID Guid = GUID::Zero;
		const RHIVertexBuffer* VertexBuffer;
		const RHIIndexBuffer* IndexBuffer;
		const RHIUniformBuffer* UniformBuffer;
		Matrix4 Transform;
	};

	struct REditorPassBillboardPrimitive
	{
		GUID Guid = GUID::Zero;
		RBillboardRenderProxy BillboardRenderProxy;
	};

	enum class EPolygonDrawMode : uint8
	{
		Fill,
		Lines,
		Points,
	};
}
