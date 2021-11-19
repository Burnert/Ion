#pragma once

#include "XML.h"
#include "Renderer/VertexAttribute.h"

namespace Ion
{
	class VertexLayout;

	struct ColladaData
	{
		float* VertexAttributes;
		uint32* Indices;
		uint64 VertexAttributeCount;
		uint64 IndexCount;
		TShared<VertexLayout> Layout;
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

			TriangleInput* AddTriangleInput(XMLNode* meshNode, XMLNode* inputNode);
			uint32 GetFullStride() const;
			TShared<VertexLayout> CreateLayout() const;

			TArray<TriangleInput> m_TriangleInputs;
			uint32 m_AttributeCount = 0;

		public:
			// @TODO: Create a TConstMap

			inline static const THashMap<const char*, const EVertexAttributeSemantic> SemanticStringToEnum = {
				{ "POSITION", EVertexAttributeSemantic::Position },
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
		ColladaDocument(FileOld* colladaFile);
		ColladaDocument() = delete;
		~ColladaDocument();

		inline const ColladaData& GetData() const { return m_Data; }

	protected:
		void Load();
		bool Parse();

		static uint32* ExtractTriangles(XMLNode* trianglesNode, uint64& outIndexCount);

		static TShared<TrianglesNodeData> ExtractTriangleInputs(XMLNode* trianglesNode);

		static XMLNode* ExtractSourceNode(XMLNode* meshNode, XMLNode* inputNode);
		static XMLNode* ExtractVerticesSourceNode(XMLNode* verticesNode);
		//static bool ExtractParams(const XMLNode* accessorNode);

		static bool ParseTriangleInputs(const TShared<TrianglesNodeData>& layout, float scale = 1.0f);
		static bool ParseTriangles(uint32* indices, uint64 indexCount, const TShared<TrianglesNodeData>& data, ColladaData& outMeshData);

		static float* ExtractFloatArray(XMLNode* sourceNode, uint64& outSize, TransformFn transformFunction = nullptr);

		static const char* CheckDocumentVersion(XMLNode* colladaNode);

	private:
		ColladaData m_Data;
		bool m_bParsed;
	};
}
