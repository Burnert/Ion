#pragma once

#include "XML.h"

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
		};

		class Vertex
		{
			friend class ColladaDocument;
		public:
			Vertex(uint32 elementCount, float* elements) :
				m_Elements(elementCount)
			{
				memcpy(&m_Elements[0], elements, elementCount * sizeof(float));
			}
			Vertex(const Vertex& other) = default;
			Vertex(Vertex&& other) noexcept = default;

			inline bool operator==(const Vertex& other) const
			{
				for (uint32 i = 0; i < m_Elements.size(); ++i)
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
					String temp = "";
					for (uint32 i = 0; i < vertex.m_Elements.size(); ++i)
					{
						temp += std::to_string(vertex.m_Elements[i]);
					}
					return std::hash<String>()(temp);
				}
			};

		private:
			TArray<float> m_Elements;
		};

	public:
		ColladaDocument(const String& collada);
		/* Takes the ownership of the xml character buffer */
		ColladaDocument(char* collada);
		ColladaDocument(File* colladaFile);
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

		template<typename Fn = nullptr_t>
		static float* ExtractFloatArray(XMLNode* sourceNode, uint64& outSize, Fn transformFunction = nullptr)
		{
			TRACE_FUNCTION();

			XMLNode* floatArrayNode = sourceNode->first_node("float_array");
			_ionexcept_r(floatArrayNode, "The <source> node does not have a <float_array> node.");

			// Node structure
			// <float_array id="mesh-positions-array" count="1234">1.123456 -5.282121 10.33126</float_array>

			uint64 floatArraySize = floatArrayNode->value_size();

			XMLAttribute* countAttribute = floatArrayNode->first_attribute("count");
			_ionexcept_r(countAttribute);

			const char* countStr = countAttribute->value();
			uint32 count = strtoul(countStr, nullptr, 10);
			_ionexcept_r(count != 0);

			outSize = count;

			float* floatArray = new float[count];
			float* currentValuePtr = floatArray;

			// Extract vertices one by one
			char* valueCharPtr = floatArrayNode->value();
			_ionexcept_r(*valueCharPtr);
			char* valueStartPtr = valueCharPtr;
			DEBUG(uint64 debugValueCount = 0);
			do
			{
				// Float values are separated by space
				// Handle the edge case of the pointer being at the end of the buffer
				if (*valueCharPtr == ' ' || !*valueCharPtr)
				{
					*currentValuePtr = strtof(valueStartPtr, nullptr);
					// Transform values using the specified transform function
					// Useful for changing the scale of the model
					if constexpr (!std::is_null_pointer_v<Fn>)
					{
						*currentValuePtr = transformFunction(*currentValuePtr);
					}
					currentValuePtr++;
					valueStartPtr = valueCharPtr + 1;
					DEBUG(debugValueCount++);
				}
			}
			while (*valueCharPtr++); // The last character is null
			ionassert(debugValueCount == count);
			return floatArray;
		}

		static const char* CheckDocumentVersion(XMLNode* colladaNode);

	private:
		ColladaData m_Data;
		bool m_bParsed;
	};
}
