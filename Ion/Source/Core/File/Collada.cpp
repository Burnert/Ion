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

	ColladaDocument::ColladaDocument(File* colladaFile)
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

		ullong indexCount;
		uint* indices = ExtractTriangles(trianglesNode, indexCount);

		ParseTriangles(indices, indexCount, trianglesData, m_Data);

		m_Data.Layout = trianglesData->CreateLayout();

		delete[] indices;

		return true;
	}

	uint* ColladaDocument::ExtractTriangles(XMLNode* trianglesNode, ullong& outIndexCount)
	{
		#pragma warning(disable:26451)

		TRACE_FUNCTION();

		// Node structure
		// <triangles material="material" count="724">0 22 41</triangles>

		int maxOffset = -1;
		XMLNode* inputNode = trianglesNode->first_node("input");
		_ionexcept_r(inputNode, "The <triangles> node does not have an <input> node.");
		do
		{
			XMLAttribute* offsetAttribute = inputNode->first_attribute("offset");
			_ionexcept_r(offsetAttribute, "The <input> node does not have an offset attribute.");

			char* offsetStr = offsetAttribute->value();
			maxOffset = glm::max((int)strtol(offsetStr, nullptr, 10), maxOffset);
		}
		while (inputNode = inputNode->next_sibling("input"));

		XMLNode* primitiveNode = trianglesNode->first_node("p");
		_ionexcept_r(primitiveNode, "The <triangles> node does not have a <p> node.");

		ullong primitiveSize = primitiveNode->value_size();

		XMLAttribute* triangleCountAttribute = trianglesNode->first_attribute("count");
		_ionexcept_r(triangleCountAttribute, "The <triangles> node does not have a count attribute.");
		char* triangleCountStr = triangleCountAttribute->value();
		uint triangleCount = strtol(triangleCountStr, nullptr, 10);
		_ionexcept_r(triangleCount != 0);

		outIndexCount = (ullong)triangleCount * 3 * (maxOffset + 1);

		uint* indices = new uint[outIndexCount];
		uint* currentIndexPtr = indices;
		
		char* indexCharPtr = primitiveNode->value();
		_ionexcept_r(*indexCharPtr);
		char* indexStartPtr = indexCharPtr;
		DEBUG(ullong debugIndexCount = 0);
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
			uint stride = (uint)strtoul(strideStr, nullptr, 10);

			ullong dataSize;
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

	bool ColladaDocument::ParseTriangles(uint* indices, ullong indexCount, const TShared<TrianglesNodeData>& data, ColladaData& outMeshData)
	{
		TRACE_FUNCTION();

		using TriangleInput = TrianglesNodeData::TriangleInput;

		uint* endIndex = indices + indexCount;
		uint* indexPtr = indices;

		std::vector<ColladaDocument::Vertex> finalVertices;
		std::unordered_set<ColladaDocument::Vertex, ColladaDocument::Vertex::Hash> finalVerticesSet;
		std::vector<uint> finalIndices;

#if ION_ENABLE_TRACING
		uint debugCurrentIndex = 0;
#endif
		uint fullStride = data->GetFullStride();
		float* currentVertex = new float[fullStride];
		float* currentVertexPtr = currentVertex;
		uint currentAttribute = 0;
		do
		{
			TRACE_SCOPE("ColladaDocument::ParseTriangles - Parse index");

			TriangleInput& currentInput = data->m_TriangleInputs[currentAttribute];
			// One index is split into AttributeCount indices that point to the corresponding source
			// based on offset % stride, where stride is the attribute count
			// Then each attribute has *Stride* elements
			for (uint i = 0; i < currentInput.Stride; ++i)
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
				ColladaDocument::Vertex vertex(fullStride, currentVertex);
				TRACE_BEGIN(0, "ColladaDocument::ParseTriangles - Find identical vertex (Set)");
				// Inserting to a set and checking uniqueness this way is much faster 
				// than searching the whole vector for duplicates for EVERY single vertex.
				auto [iter, bUnique] = finalVerticesSet.insert(vertex);
				TRACE_END(0);
				
				uint index;
				if (bUnique)
				{
					TRACE_BEGIN(1, "ColladaDocument::ParseTriangles - Vertex push");
					finalVertices.push_back(vertex);
					TRACE_END(1);
					index = (uint)(finalVertices.size() - 1);
				}
				else
				{
					TRACE_BEGIN(1, "ColladaDocument::ParseTriangles - Find identical vertex (Vector)");
					auto foundIter = std::find_if(finalVertices.begin(), finalVertices.end(), [&](const Vertex& vert) { return vert == vertex; });
					TRACE_END(1);
					index = (uint)(foundIter - finalVertices.begin());
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

		ullong finalVertexAttributeCount = finalVertices.size() * fullStride;
		outMeshData.VertexAttributeCount = finalVertexAttributeCount;
		outMeshData.VertexAttributes = new float[finalVertexAttributeCount];

		float* currentAttributePtr = outMeshData.VertexAttributes;
		for (const ColladaDocument::Vertex& vertex : finalVertices)
		{
			memcpy(currentAttributePtr, &vertex.m_Elements[0], fullStride * sizeof(float));
			currentAttributePtr += fullStride;
		}

		// Write final index data

		ullong finalIndexCount = finalIndices.size();
		outMeshData.IndexCount = finalIndexCount;
		outMeshData.Indices = new uint[finalIndexCount];

		memcpy(outMeshData.Indices, &finalIndices[0], finalIndexCount * sizeof(uint));

		return true;
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
		uint offset = (uint)strtoul(offsetStr, nullptr, 10);

		XMLAttribute* setAttribute = inputNode->first_attribute("set");
		int set = -1;
		if (setAttribute)
		{
			char* setStr = setAttribute->value();
			set = (int)strtol(setStr, nullptr, 10);
		}

		XMLNode* sourceNode = ExtractSourceNode(meshNode, inputNode);

		TriangleInput* triangleInput = &m_TriangleInputs.emplace_back(TriangleInput { inputNode, sourceNode, semantic, source, offset, set, nullptr, 0, 0 });
		return triangleInput;
	}

	uint ColladaDocument::TrianglesNodeData::GetFullStride() const
	{
		TRACE_FUNCTION();

		uint stride = 0;
		for (const TriangleInput& input : m_TriangleInputs)
		{
			stride += input.Stride;
		}
		return stride;
	}

	TShared<VertexLayout> ColladaDocument::TrianglesNodeData::CreateLayout() const
	{
		TRACE_FUNCTION();

		TShared<VertexLayout> layout = MakeShareable(new VertexLayout((uint)m_TriangleInputs.size()));
		for (const TriangleInput& input : m_TriangleInputs)
		{
			uint elementCount = input.Stride;
			bool bNormalized = strcmp(input.Semantic, "NORMAL") == 0;
			layout->AddAttribute(EVertexAttributeType::Float, elementCount, bNormalized);
		}
		return layout;
	}
}
