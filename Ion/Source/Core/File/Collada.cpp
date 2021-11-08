#include "IonPCH.h"

#include "Collada.h"
#include "Renderer/VertexBuffer.h"

namespace Ion
{
	ColladaDocument::ColladaDocument(const String& collada)
		: XMLDocument(collada),
		m_Data({ }),
		m_bParsed(false)
	{
		Load();
	}

	ColladaDocument::ColladaDocument(char* collada)
		: XMLDocument(collada),
		m_Data({ }),
		m_bParsed(false)
	{
		Load();
	}

	ColladaDocument::ColladaDocument(FileOld* colladaFile)
		: XMLDocument(colladaFile),
		m_Data({ }),
		m_bParsed(false)
	{
		ionassert(colladaFile->GetExtension() == L"dae", "The file does not have a Collada (.dae) extension. This is most likely an error.");
		Load();
	}

	ColladaDocument::~ColladaDocument()
	{
		if (m_Data.VertexAttributes)
		{
			delete[] m_Data.VertexAttributes;
		}
		if (m_Data.Indices)
		{
			delete[] m_Data.Indices;
		}
	}

	void ColladaDocument::Load()
	{
		TRACE_FUNCTION();

		if (!Parse())
		{
			LOG_ERROR("Could not parse Collada file!");
			return;
		}
		m_bParsed = true;
	}

	bool ColladaDocument::Parse()
	{
		// @TODO: Parse the <up_axis> node

		// <COLLADA>

		XMLNode* colladaNode = m_XML.first_node("COLLADA");
		_ionexcept_r(colladaNode, "The file is not a valid Collada format!");
		_ionexcept_r(_strcmpi(CheckDocumentVersion(colladaNode), "1.4.1") == 0, "For now, only Collada version 1.4.1 supported.");

		// <library_geometries>

		XMLNode* libGeometriesNode = colladaNode->first_node("library_geometries");
		_ionexcept_r(libGeometriesNode, "The Collada file does not have a <library_geometries> node.");

		// <geometry>

		XMLNode* geometryNode = libGeometriesNode->first_node("geometry");
		_ionexcept_r(geometryNode, "The <library_geometries> node does not have a <geometry> node.");

		// <mesh>

		XMLNode* meshNode = geometryNode->first_node("mesh");
		_ionexcept_r(meshNode, "The <geometry> node does not have a <mesh> node.");

		// <triangles>

		XMLNode* trianglesNode = meshNode->first_node("triangles");
		_ionexcept_r(trianglesNode, "The <mesh> node does not have a <triangles> node.");

		TShared<TrianglesNodeData> trianglesData = ExtractTriangleInputs(trianglesNode);
		if (!ParseTriangleInputs(trianglesData, 0.01f))
		{
			return false;
		}

		uint64 indexCount;
		uint32* indices = ExtractTriangles(trianglesNode, indexCount);

		ParseTriangles(indices, indexCount, trianglesData, m_Data);

		m_Data.Layout = trianglesData->CreateLayout();

		delete[] indices;

		return true;
	}

	uint32* ColladaDocument::ExtractTriangles(XMLNode* trianglesNode, uint64& outIndexCount)
	{
		#pragma warning(disable:26451)

		TRACE_FUNCTION();

		// Node structure
		// <triangles material="material" count="724">0 22 41</triangles>

		int32 maxOffset = -1;
		XMLNode* inputNode = trianglesNode->first_node("input");
		_ionexcept_r(inputNode, "The <triangles> node does not have an <input> node.");
		do
		{
			XMLAttribute* offsetAttribute = inputNode->first_attribute("offset");
			_ionexcept_r(offsetAttribute, "The <input> node does not have an offset attribute.");

			char* offsetStr = offsetAttribute->value();
			maxOffset = glm::max((int32)strtol(offsetStr, nullptr, 10), maxOffset);
		}
		while (inputNode = inputNode->next_sibling("input"));

		XMLNode* primitiveNode = trianglesNode->first_node("p");
		_ionexcept_r(primitiveNode, "The <triangles> node does not have a <p> node.");

		uint64 primitiveSize = primitiveNode->value_size();

		XMLAttribute* triangleCountAttribute = trianglesNode->first_attribute("count");
		_ionexcept_r(triangleCountAttribute, "The <triangles> node does not have a count attribute.");
		char* triangleCountStr = triangleCountAttribute->value();
		uint32 triangleCount = strtol(triangleCountStr, nullptr, 10);
		_ionexcept_r(triangleCount != 0);

		outIndexCount = (uint64)triangleCount * 3 * (maxOffset + 1);

		uint32* indices = new uint32[outIndexCount];
		uint32* currentIndexPtr = indices;
		
		char* indexCharPtr = primitiveNode->value();
		_ionexcept_r(*indexCharPtr);
		char* indexStartPtr = indexCharPtr;
		DEBUG(uint64 debugIndexCount = 0);
		do
		{
			// Integer values are separated by space
			// Handle the edge case of the pointer being at the end of the buffer
			if (*indexCharPtr == ' ' || !*indexCharPtr)
			{
				*currentIndexPtr = strtol(indexStartPtr, nullptr, 10);
				currentIndexPtr++;
				indexStartPtr = indexCharPtr + 1;
				DEBUG(debugIndexCount++);
			}
		}
		while (*indexCharPtr++); // The last character is null
		ionassert(debugIndexCount == outIndexCount);
		return indices;
	}

	TShared<ColladaDocument::TrianglesNodeData> ColladaDocument::ExtractTriangleInputs(XMLNode* trianglesNode)
	{
		TRACE_FUNCTION();

		TShared<TrianglesNodeData> layout = MakeShared<TrianglesNodeData>();
		layout->m_TriangleInputs.reserve(8);
		layout->m_AttributeCount = 0;
		XMLNode* inputNode = trianglesNode->first_node("input");
		// <mesh> is always the parent of <triangles>
		XMLNode* meshNode = trianglesNode->parent();
		while (inputNode)
		{
			layout->AddTriangleInput(meshNode, inputNode);
			inputNode = inputNode->next_sibling("input");
			layout->m_AttributeCount++;
		}
		return layout;
	}

	XMLNode* ColladaDocument::ExtractSourceNode(XMLNode* meshNode, XMLNode* inputNode)
	{
		TRACE_FUNCTION();

		XMLAttribute* sourceAttribute = inputNode->first_attribute("source");
		_ionexcept_r(sourceAttribute, "The <input> node does not have a source attribute.");
		char* source = sourceAttribute->value();

		// Find the correct source node based on id attribute
		// Add 1 to the pointer because there is a # symbol at the start
		XMLNode* sourceNode = FindNode(meshNode, "source", WithAttributeValuePredicate("id", source + 1));
		_ionexcept_r(sourceNode, "The <mesh> node does not have a <source> node.");

		return sourceNode;
	}

	XMLNode* ColladaDocument::ExtractVerticesSourceNode(XMLNode* verticesNode)
	{
		TRACE_FUNCTION();

		XMLNode* inputNode = verticesNode->first_node("input");
		_ionexcept_r(inputNode, "The <vertices> node does not have an <input> node.");

		XMLAttribute* semanticAttribute = inputNode->first_attribute("semantic");
		_ionexcept_r(semanticAttribute, "The <input> node does not have a semantic attribute.");

		char* semantic = semanticAttribute->value();
		char* source = nullptr;
		if (strstr(semantic, "POSITION"))
		{
			XMLAttribute* sourceAttribute = inputNode->first_attribute("source");
			_ionexcept_r(sourceAttribute, "The <input> node does not have a source attribute.");
			source = sourceAttribute->value();
		}
		_ionexcept_r(source, "The source attribute does not have a value.");

		XMLNode* meshNode = verticesNode->parent();
		ionassert(meshNode);
		// Find the correct source node based on id attribute
		// Add 1 to the pointer because there is a # symbol at the start
		XMLNode* sourceNode = FindNode(meshNode, "source", WithAttributeValuePredicate("id", source + 1));
		return sourceNode;
	}

	const char* ColladaDocument::CheckDocumentVersion(XMLNode* colladaNode)
	{
		XMLAttribute* versionAttribute = colladaNode->first_attribute("version");
		_ionexcept_r(versionAttribute, "Cannot find version of the Collada file.");
		const char* version = versionAttribute->value();
		return version;
	}

	bool ColladaDocument::ParseTriangleInputs(const TShared<TrianglesNodeData>& layout, float scale)
	{
		TRACE_FUNCTION();

		_ionexcept_r(!layout->m_TriangleInputs.empty(), "The layout is empty.");
		for (TrianglesNodeData::TriangleInput& input : layout->m_TriangleInputs)
		{
			XMLNode*& sourceNode = input.LinkedSourceNode;

			// VERTEX source points to <vertices> instead of <source>
			bool bVertexInput = strcmp(input.Semantic, "VERTEX") == 0;
			if (bVertexInput)
			{
				sourceNode = ExtractVerticesSourceNode(sourceNode);
			}

			XMLNode* techniqueNode = sourceNode->first_node("technique_common");
			XMLNode* accessorNode = techniqueNode->first_node("accessor");
			XMLAttribute* strideAttribute = accessorNode->first_attribute("stride");
			char* strideStr = strideAttribute->value();
			uint32 stride = (uint32)strtoul(strideStr, nullptr, 10);

			uint64 dataSize;
			float* floatArray;

			if (bVertexInput)
			{
				// Scale the vertices by the specified scale
				floatArray = ExtractFloatArray(sourceNode, dataSize, [=](float value) { return value * scale; });
			}
			else
			{
				floatArray = ExtractFloatArray(sourceNode, dataSize);
			}
			// Set remaining fields
			input.Data = floatArray;
			input.DataSize = dataSize;
			input.Stride = stride;
		}
		return true;
	}

	bool ColladaDocument::ParseTriangles(uint32* indices, uint64 indexCount, const TShared<TrianglesNodeData>& data, ColladaData& outMeshData)
	{
		TRACE_FUNCTION();

		using TriangleInput = TrianglesNodeData::TriangleInput;

		uint32* endIndex = indices + indexCount;
		uint32* indexPtr = indices;

		TArray<ColladaDocument::Vertex> finalVertices;
		THashSet<ColladaDocument::Vertex, ColladaDocument::Vertex::Hash> finalVerticesSet;
		TArray<uint32> finalIndices;

		finalVertices.reserve(indexCount);
		finalVerticesSet.reserve(indexCount);
		finalIndices.reserve(indexCount);

#if ION_ENABLE_TRACING
		uint32 debugCurrentIndex = 0;
#endif
		uint32 fullStride = data->GetFullStride();
		float* currentVertex = new float[fullStride];
		float* currentVertexPtr = currentVertex;
		uint32 currentAttribute = 0;
		do
		{
			TRACE_SCOPE("ColladaDocument::ParseTriangles - Parse index");

			TriangleInput& currentInput = data->m_TriangleInputs[currentAttribute];
			// One index is split into AttributeCount indices that point to the corresponding source
			// based on offset % stride, where stride is the attribute count
			// Then each attribute has *Stride* elements
			for (uint32 i = 0; i < currentInput.Stride; ++i)
			{
				float value = currentInput.Data[(*indexPtr) * currentInput.Stride + i];
				*currentVertexPtr = value;
				currentVertexPtr++;
			}

			// @TODO: Fix this, attribute counting shouldn't work like this because 
			// there can be more than one attribute with the same offset (texcoords)
			currentAttribute = (currentAttribute + 1) % data->m_AttributeCount;

			// When a full interleaved vertex was created, add it to the pool
			if (currentAttribute == 0)
			{
				ColladaDocument::Vertex vertex(currentVertex, fullStride);
				TRACE_BEGIN(0, "ColladaDocument::ParseTriangles - Find identical vertex (Set)");
				// Inserting to a set and checking uniqueness this way is much faster 
				// than searching the whole vector for duplicates for EVERY single vertex.
				auto [iter, bUnique] = finalVerticesSet.insert(vertex);
				TRACE_END(0);
				
				uint32 index;
				if (bUnique)
				{
					TRACE_BEGIN(1, "ColladaDocument::ParseTriangles - Vertex push");
					finalVertices.push_back(vertex);
					TRACE_END(1);
					index = (uint32)(finalVertices.size() - 1);
				}
				else
				{
					TRACE_BEGIN(1, "ColladaDocument::ParseTriangles - Find identical vertex (Vector)");
					auto foundIter = std::find_if(finalVertices.begin(), finalVertices.end(), [&](const Vertex& vert) { return vert == vertex; });
					TRACE_END(1);
					index = (uint32)(foundIter - finalVertices.begin());
				}
				TRACE_BEGIN(2, "ColladaDocument::ParseTriangles - Index push");
				finalIndices.push_back(index);
				TRACE_END(2);
				// Don't clear the array just reset the pointer
				currentVertexPtr = currentVertex;
			}
#if ION_ENABLE_TRACING
			debugCurrentIndex++;
#endif
		}
		while (++indexPtr != endIndex);

		delete[] currentVertex;

		// Write final vertex data

		uint64 finalVertexAttributeCount = finalVertices.size() * fullStride;
		outMeshData.VertexAttributeCount = finalVertexAttributeCount;
		outMeshData.VertexAttributes = new float[finalVertexAttributeCount];

		float* currentAttributePtr = outMeshData.VertexAttributes;
		for (const ColladaDocument::Vertex& vertex : finalVertices)
		{
			memcpy(currentAttributePtr, &vertex.m_Elements[0], fullStride * sizeof(float));
			currentAttributePtr += fullStride;
		}

		// Write final index data

		uint64 finalIndexCount = finalIndices.size();
		outMeshData.IndexCount = finalIndexCount;
		outMeshData.Indices = new uint32[finalIndexCount];

		memcpy(outMeshData.Indices, &finalIndices[0], finalIndexCount * sizeof(uint32));

		return true;
	}

	float* ColladaDocument::ExtractFloatArray(XMLNode* sourceNode, uint64& outSize, TransformFn transformFunction)
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
				if (transformFunction)
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

	ColladaDocument::TrianglesNodeData::TriangleInput* ColladaDocument::TrianglesNodeData::AddTriangleInput(XMLNode* meshNode, XMLNode* inputNode)
	{
		TRACE_FUNCTION();

		XMLAttribute* semanticAttribute = inputNode->first_attribute("semantic");
		_ionexcept_r(semanticAttribute, "The <input> node does not have a semantic attribute.");
		char* semantic = semanticAttribute->value();

		XMLAttribute* sourceAttribute = inputNode->first_attribute("source");
		_ionexcept_r(sourceAttribute, "The <input> node does not have a source attribute.");
		char* source = sourceAttribute->value();

		XMLAttribute* offsetAttribute = inputNode->first_attribute("offset");
		_ionexcept_r(offsetAttribute, "The <input> node does not have an offset attribute.");
		char* offsetStr = offsetAttribute->value();
		uint32 offset = (uint32)strtoul(offsetStr, nullptr, 10);

		XMLAttribute* setAttribute = inputNode->first_attribute("set");
		int32 set = -1;
		if (setAttribute)
		{
			char* setStr = setAttribute->value();
			set = (int32)strtol(setStr, nullptr, 10);
		}

		XMLNode* sourceNode = ExtractSourceNode(meshNode, inputNode);

		TriangleInput* triangleInput = &m_TriangleInputs.emplace_back(TriangleInput { inputNode, sourceNode, semantic, source, offset, set, nullptr, 0, 0 });
		return triangleInput;
	}

	uint32 ColladaDocument::TrianglesNodeData::GetFullStride() const
	{
		TRACE_FUNCTION();

		uint32 stride = 0;
		for (const TriangleInput& input : m_TriangleInputs)
		{
			stride += input.Stride;
		}
		return stride;
	}

	TShared<VertexLayout> ColladaDocument::TrianglesNodeData::CreateLayout() const
	{
		TRACE_FUNCTION();

		TShared<VertexLayout> layout = MakeShareable(new VertexLayout((uint32)m_TriangleInputs.size()));
		for (const TriangleInput& input : m_TriangleInputs)
		{
			uint32 elementCount = input.Stride;
			bool bNormalized = strcmp(input.Semantic, "NORMAL") == 0;
			layout->AddAttribute(EVertexAttributeType::Float, elementCount, bNormalized);
		}
		return layout;
	}
}
