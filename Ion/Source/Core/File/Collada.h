#pragma once

#include "XML.h"

namespace Ion
{
	struct ColladaData
	{
		float* Vertices = nullptr;
		uint* Indices = nullptr;
		ullong VertexCount = 0;
		uint IndexCount = 0;
	};

	class ION_API ColladaDocument : public XMLDocument
	{
	public:
		ColladaDocument(const String& collada);
		/* Takes the ownership of the xml character buffer */
		ColladaDocument(char* collada);
		ColladaDocument(File* colladaFile);
		~ColladaDocument();

		inline const ColladaData& GetData() const { return m_Data; }

	protected:
		bool Parse();
		static float* ExtractVertexPositions(XMLNode* floatArrayNode, ullong& outSize);
		static uint* ExtractTriangles(XMLNode* trianglesNode, uint& outIndexCount);

		static const char* CheckDocumentVersion(XMLNode* colladaNode);

	private:
		ColladaData m_Data;
	};
}
