#include "IonPCH.h"

#include "Collada.h"

#define _ionexcept_r(x, ...) ionexcept(x, __VA_ARGS__) return 0

namespace Ion
{
	ColladaDocument::ColladaDocument(const String& collada)
		: XMLDocument(collada)
	{
		Parse();
	}

	ColladaDocument::ColladaDocument(char* collada)
		: XMLDocument(collada)
	{
		Parse();
	}

	ColladaDocument::ColladaDocument(File* colladaFile)
		: XMLDocument(colladaFile)
	{
		ionassert(colladaFile->GetExtension() == L"dae", "The file does not have a Collada (.dae) extension. This is most likely an error.");

		Parse();
	}

	ColladaDocument::~ColladaDocument()
	{
		if (m_Data.Vertices)
		{
			delete[] m_Data.Vertices;
		}
		if (m_Data.Indices)
		{
			delete[] m_Data.Indices;
		}
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

		// <vertices>

		XMLNode* verticesNode = meshNode->first_node("vertices");
		_ionexcept_r(verticesNode, "The <mesh> node does not have a <vertices> node.");

		XMLNode* verticesInputNode = verticesNode->first_node("input");
		_ionexcept_r(verticesInputNode, "The <vertices> node does not have an <input> node.");
		XMLAttribute* verticesInputSemanticAttribute = verticesInputNode->first_attribute("semantic");
		_ionexcept_r(verticesInputSemanticAttribute, "The <input> node does not have a semantic attribute.");

		char* verticesInputSemantic = verticesInputSemanticAttribute->value();
		char* verticesSource = nullptr;
		if (strstr(verticesInputSemantic, "POSITION"))
		{
			XMLAttribute* verticesInputSourceAttribute = verticesInputNode->first_attribute("source");
			_ionexcept_r(verticesInputSourceAttribute, "The <input> node does not have a source attribute.");
			verticesSource = verticesInputSourceAttribute->value();
		}
		// Find the correct source node based on id attribute
		// Add 1 to the pointer because there is a # symbol at the start
		XMLNode* meshSourceNode = FindNode(meshNode, "source", WithAttributeValuePredicate("id", verticesSource + 1));
		_ionexcept_r(meshSourceNode, "The <mesh> node does not have a <source> node.");

		XMLNode* meshTechniqueNode = meshSourceNode->first_node("technique_common");
		_ionexcept_r(meshTechniqueNode, "The <source> node does not have a <technique_common> node.");

		// @TODO: Parse <technique>

		XMLNode* meshFloatArrayNode = meshSourceNode->first_node("float_array");
		_ionexcept_r(meshFloatArrayNode, "The <source> node does not have a <float_array> node.");

		m_Data.Vertices = ExtractVertexPositions(meshFloatArrayNode, m_Data.VertexCount);

		// <triangles>

		XMLNode* trianglesNode = meshNode->first_node("triangles");
		_ionexcept_r(trianglesNode, "The <mesh> node does not have a <triangles> node.");

		m_Data.Indices = ExtractTriangles(trianglesNode, m_Data.IndexCount);

		return true;
	}

	float* ColladaDocument::ExtractVertexPositions(XMLNode* floatArrayNode, ullong& outSize)
	{
		// Node structure
		// <float_array id="mesh-positions-array" count="1234">1.123456 -5.282121 10.33126</float_array>

		ullong floatArraySize = floatArrayNode->value_size();

		XMLAttribute* countAttribute = floatArrayNode->first_attribute("count");
		_ionexcept_r(countAttribute);

		const char* countStr = countAttribute->value();
		uint count = strtoul(countStr, nullptr, 10);
		_ionexcept_r(count != 0);

		outSize = count;

		float* vertices = new float[count];
		float* currentVertexPtr = vertices;

		// Extract vertices one by one
		char* vertexCharPtr = floatArrayNode->value();
		_ionexcept_r(*vertexCharPtr);
		char* vertexStartPtr = vertexCharPtr;
		DEBUG(ullong debugVertexCount = 0);
		do
		{
			// Float values are separated by space
			// Handle the edge case of the pointer being at the end of the buffer
			if (*vertexCharPtr == ' ' || !*vertexCharPtr)
			{
				*currentVertexPtr = strtof(vertexStartPtr, nullptr);
				*currentVertexPtr *= 0.01f; // Temporarily scale down by 100
				currentVertexPtr++;
				vertexStartPtr = vertexCharPtr + 1;
				DEBUG(debugVertexCount++);
			}
		}
		while (*vertexCharPtr++); // The last character is null
		ionassert(debugVertexCount == count);
		return vertices;
	}

	uint* ColladaDocument::ExtractTriangles(XMLNode* trianglesNode, uint& outIndexCount)
	{
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

		outIndexCount = triangleCount * 3;

		uint* indices = new uint[triangleCount * 3];
		uint* currentIndexPtr = indices;
		int currentOffset = 0;
		
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
				if (currentOffset == 0)
				{
					*currentIndexPtr = strtol(indexStartPtr, nullptr, 10);
					currentIndexPtr++;
				}
				indexStartPtr = indexCharPtr + 1;

				// Cycle available offsets
				currentOffset = (currentOffset + 1) % (maxOffset + 1);
				DEBUG(debugIndexCount++);
			}
		}
		while (*indexCharPtr++); // The last character is null
		ionassert(debugIndexCount == triangleCount * 3 * (maxOffset + 1));
		return indices;
	}

	const char* ColladaDocument::CheckDocumentVersion(XMLNode* colladaNode)
	{
		XMLAttribute* versionAttribute = colladaNode->first_attribute("version");
		_ionexcept_r(versionAttribute, "Cannot find version of the Collada file.");
		const char* version = versionAttribute->value();
		return version;
	}
}
