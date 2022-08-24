#pragma once

#include "Core.h"

#include "RHI/VertexAttribute.h"

namespace Ion
{
	class RHIVertexLayout;

	struct ColladaData
	{
		float* VertexAttributes;
		uint32* Indices;
		uint64 VertexAttributeCount;
		uint64 IndexCount;
		TRef<RHIVertexLayout> Layout;
	};

	class ION_API ColladaDocument : public XMLDocument
	{
		class TrianglesNodeData
		{
			friend class ColladaDocument;
		private:
			struct TriangleInput
			{
				XMLNode* InputNode;
				XMLNode* LinkedSourceNode;
				char* Semantic;
				char* Source;
				uint32 Offset;
				int32 Set;
				float* Data;
				uint64 DataSize;
				uint32 Stride;
			};

			Result<TriangleInput*, IOError> AddTriangleInput(XMLNode* meshNode, XMLNode* inputNode);
			uint32 GetFullStride() const;
			TRef<RHIVertexLayout> CreateLayout() const;

			TArray<TriangleInput> m_TriangleInputs;
			uint32 m_AttributeCount = 0;

		public:
			// @TODO: Create a TConstMap

			inline static const THashMap<String, const EVertexAttributeSemantic> SemanticStringToEnum = {
				{ "VERTEX",   EVertexAttributeSemantic::Position },
				{ "NORMAL",   EVertexAttributeSemantic::Normal   },
				{ "TEXCOORD", EVertexAttributeSemantic::TexCoord },
			};

			inline static bool IsSemanticTranslatable(const char* name)
			{
				return SemanticStringToEnum.find(name) != SemanticStringToEnum.end();
			}

			inline static EVertexAttributeSemantic TranslateSemantic(const char* name)
			{
				return SemanticStringToEnum.at(name);
			}
		};

		class Vertex
		{
			friend class ColladaDocument;
		public:
			constexpr static uint32 MaxVertexElements = 64;

			Vertex(float* elements, uint32 elementCount) :
				m_ElementCount(elementCount)
			{
				ionassert(elementCount <= MaxVertexElements, "Too many vertex elements.");
				memcpy(&m_Elements[0], elements, elementCount * sizeof(float));
			}
			Vertex(const Vertex& other) = default;
			Vertex(Vertex&& other) noexcept = default;

			inline bool operator==(const Vertex& other) const
			{
				if (m_ElementCount != other.m_ElementCount)
				{
					return false;
				}

				for (uint32 i = 0; i < m_ElementCount; ++i)
				{
					if (m_Elements[i] != other.m_Elements[i])
					{
						return false;
					}
				}
				return true;
			}

			struct Hash
			{
				size_t operator() (const Vertex& vertex) const
				{
					size_t hash = 0;
					std::hash<float> hasher;
					for (uint32 i = 0; i < vertex.m_ElementCount; ++i)
					{
						hash ^= hasher(vertex.m_Elements[i]) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
					}
					return hash;
				}
			};

		private:
			uint32 m_ElementCount;
			float m_Elements[MaxVertexElements];
		};

	public:
		using TransformFn = TFunction<float(float)>;

		ColladaDocument(const String& collada);
		/* Takes the ownership of the xml character buffer */
		ColladaDocument(char* collada);
		ColladaDocument() = delete;
		~ColladaDocument();

		inline const ColladaData& GetData() const { ionbreak("DEPRECATED, Use Parse once instead."); return m_Data; }

		Result<ColladaData, IOError> Parse();

	protected:
		static Result<uint32*, IOError> ExtractTriangles(XMLNode* trianglesNode, uint64& outIndexCount);

		static std::shared_ptr<TrianglesNodeData> ExtractTriangleInputs(XMLNode* trianglesNode);

		static Result<XMLNode*, IOError> ExtractSourceNode(XMLNode* meshNode, XMLNode* inputNode);
		static Result<XMLNode*, IOError> ExtractVerticesSourceNode(XMLNode* verticesNode);
		//static bool ExtractParams(const XMLNode* accessorNode);

		static Result<void, IOError> ParseTriangleInputs(const std::shared_ptr<TrianglesNodeData>& layout, float scale = 1.0f);
		static void ParseTriangles(uint32* indices, uint64 indexCount, const std::shared_ptr<TrianglesNodeData>& data, ColladaData& outMeshData);

		static Result<float*, IOError> ExtractFloatArray(XMLNode* sourceNode, uint64& outSize, TransformFn transformFunction = nullptr);

		static Result<const char*, IOError> CheckDocumentVersion(XMLNode* colladaNode);

	private:
		ColladaData m_Data;
		bool m_bParsed;
	};
}
