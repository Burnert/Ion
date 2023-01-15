#include "Core/CorePCH.h"

#include "YAMLArchive.h"

namespace Ion
{
	YAMLArchive::YAMLArchive(EArchiveType type) :
		Archive(type),
		m_YAMLTree(nullptr),
		m_CurrentNodeIndex(TNumericLimits<size_t>::max())
	{
		SetFlag(EArchiveFlags::Text);
		if (type == EArchiveType::Saving)
		{
			m_YAMLTree = MakeShared<ryml::Tree>();
			m_YAMLTree->to_map(m_YAMLTree->root_id());
		}
	}

	void YAMLArchive::Serialize(void* const bytes, size_t size)
	{
		// @TODO: Encode the binary in base64
	}

	void YAMLArchive::Serialize(bool& value)
	{
		SerializeFundamental(value);
	}

	void YAMLArchive::Serialize(int8& value)
	{
		SerializeFundamental(value);
	}

	void YAMLArchive::Serialize(int16& value)
	{
		SerializeFundamental(value);
	}

	void YAMLArchive::Serialize(int32& value)
	{
		SerializeFundamental(value);
	}

	void YAMLArchive::Serialize(int64& value)
	{
		SerializeFundamental(value);
	}

	void YAMLArchive::Serialize(uint8& value)
	{
		SerializeFundamental(value);
	}

	void YAMLArchive::Serialize(uint16& value)
	{
		SerializeFundamental(value);
	}

	void YAMLArchive::Serialize(uint32& value)
	{
		SerializeFundamental(value);
	}

	void YAMLArchive::Serialize(uint64& value)
	{
		SerializeFundamental(value);
	}

	void YAMLArchive::Serialize(float& value)
	{
		SerializeFundamental(value);
	}

	void YAMLArchive::Serialize(double& value)
	{
		SerializeFundamental(value);
	}

	void YAMLArchive::Serialize(String& value)
	{
		ionassert(m_YAMLTree);
		//ionassert(m_CurrentNode.valid());

		Serialize_Private(value);
	}

	void YAMLArchive::Serialize(ArchiveArrayItem& item)
	{
		ionassert(m_YAMLTree->is_seq(m_CurrentNodeIndex));

		size_t originalSeqNodeIndex = m_CurrentNodeIndex;

		if (IsSaving())
		{
			m_CurrentNodeIndex = m_YAMLTree->append_child(m_CurrentNodeIndex);
			InitYAMLNode(EArchiveNodeType::Value, m_CurrentNodeIndex, ryml::csubstr {}, ryml::csubstr {}, false);
			// NOTE: Serializes the value using operator<<
			// That's why m_CurrentNodeIndex has to be set for every item.
			item.Serialize(*this);
			m_CurrentNodeIndex = originalSeqNodeIndex;
		}
		else if (IsLoading())
		{
			ionassert(item.GetIndex() < m_YAMLTree->num_children(m_CurrentNodeIndex));

			m_CurrentNodeIndex = m_YAMLTree->child(m_CurrentNodeIndex, item.GetIndex());
			item.Serialize(*this);
			m_CurrentNodeIndex = originalSeqNodeIndex;
		}
	}

	size_t YAMLArchive::GetCollectionSize() const
	{
		return m_YAMLTree->num_children(m_CurrentNodeIndex);
	}
	
	void YAMLArchive::LoadFromFile(File& file)
	{
		ionassert(IsLoading());
		ionassert(!file.IsOpen());

		ionassert(!m_YAMLTree);

		if (file.Open(EFileMode::Read)
			.Err([](Error& err)
		{
			SerializationLogger.Error("Cannot load YAML Archive from file.\n{}", err.Message);
		}))
		{
			String sYAML = file.Read()
				.Err([&](Error& err) { SerializationLogger.Error("Cannot read file \"{}\".\n{}", file.GetFullPath(), err.Message); })
				.UnwrapOr(EmptyString);

			m_YAMLTree = MakeShared<ryml::Tree>(ryml::parse_in_arena(ryml::to_csubstr(sYAML)));
		}
	}

	void YAMLArchive::SaveToFile(File& file) const
	{
		ionassert(IsSaving());
		ionassert(!file.IsOpen());

		if (file.Open(EFileMode::Write | EFileMode::CreateNew | EFileMode::Reset)
			.Err([](Error& err)
		{
			SerializationLogger.Error("Cannot save YAML Archive to file.\n{}", err.Message);
		}))
		{
			ryml::csubstr output = ryml::emit_yaml(*m_YAMLTree, m_YAMLTree->root_id(), ryml::substr(), false);
			ionassert(output.str == nullptr);
			ionassert(output.len > 0);

			String outputString;
			outputString.resize(output.len);
			output = ryml::emit_yaml(*m_YAMLTree, m_YAMLTree->root_id(), ryml::to_substr(outputString), true);
			ionassert(outputString == output);

			file.Write(outputString)
				.Err([&](Error& err) { SerializationLogger.Error("Cannot save YAMLArchive to file \"{}\".\n{}", file.GetFullPath(), err.Message); });
		}
	}

	ArchiveNode YAMLArchive::EnterRootNode()
	{
		ionassert(m_YAMLTree);

		size_t root = m_YAMLTree->root_id();
		// @TODO: Resolve the root node type
		ArchiveNode node(this, "ROOT", EArchiveNodeType::Map);

		node.SetCustomData(YAMLNodeData { root });

		return node;
	}

	ArchiveNode YAMLArchive::EnterNode(const ArchiveNode& parentNode, StringView name, EArchiveNodeType type)
	{
		ionassert(m_YAMLTree);

		if (!parentNode)
		{
			return ArchiveNode(this);
		}

		const YAMLNodeData& parentYamlNodeData = GetYAMLNodeDataFromArchiveNode(parentNode);

		size_t parentIndex = parentYamlNodeData.NodeIndex;
		size_t nodeIndex = ryml::NONE;
		if (IsLoading())
		{
			if (parentNode.Type == EArchiveNodeType::Seq)
			{
				nodeIndex = m_YAMLTree->first_child(parentIndex);
			}
			else
			{
				nodeIndex = m_YAMLTree->find_child(parentIndex, ryml::to_csubstr(name.data()));
			}
		}
		else if (IsSaving())
		{
			nodeIndex = m_YAMLTree->append_child(parentIndex);
			ryml::NodeType rymlType = ArchiveNodeTypeToYAMLNodeType(type);

			// Node name only matters if the parent is a map,
			// it is then used as a key.
			if (parentNode.Type == EArchiveNodeType::Map)
			{
				ionassert(m_YAMLTree->is_map(parentYamlNodeData.NodeIndex));

				ryml::csubstr strKey = m_YAMLTree->to_arena(name.data());
				InitYAMLNode(type, nodeIndex, strKey, ryml::csubstr {}, true);
			}
			else
			{
				ionassert(!m_YAMLTree->is_map(parentYamlNodeData.NodeIndex));

				InitYAMLNode(type, nodeIndex, ryml::csubstr {}, ryml::csubstr {}, false);
			}
		}
		else
		{
			ionerror(IOError, "Archive is neither loading or saving.");
		}

		// Node not found
		if (nodeIndex == ryml::NONE)
		{
			return ArchiveNode(this);
		}

		// @TODO: Resolve the node type
		ArchiveNode node(this, name, type);
		node.SetCustomData(YAMLNodeData { nodeIndex });

		return node;
	}

	ArchiveNode YAMLArchive::EnterNextNode(const ArchiveNode& currentNode, EArchiveNodeType type)
	{
		ionassert(m_YAMLTree);

		if (!currentNode)
		{
			return ArchiveNode(this);
		}

		const YAMLNodeData& yamlNodeData = GetYAMLNodeDataFromArchiveNode(currentNode);

		// Not needed actually
		//ionassert(m_YAMLTree->parent_is_seq(yamlNodeData.NodeIndex));

		size_t nextNodeIndex = ryml::NONE;
		if (IsLoading())
		{
			nextNodeIndex = m_YAMLTree->next_sibling(yamlNodeData.NodeIndex);
		}
		else if (IsSaving())
		{
			nextNodeIndex = m_YAMLTree->append_sibling(yamlNodeData.NodeIndex);
			InitYAMLNode(type, nextNodeIndex, ryml::csubstr {}, ryml::csubstr {}, false);
		}
		else
		{
			ionerror(IOError, "Archive is neither loading or saving.");
		}

		if (nextNodeIndex != ryml::NONE)
		{
			ArchiveNode node(this, "", type);
			node.SetCustomData(YAMLNodeData { nextNodeIndex });
			return node;
		}

		return ArchiveNode(this);
	}

	void YAMLArchive::UseNode(const ArchiveNode& node)
	{
		ionassert(m_YAMLTree);

		m_CurrentNodeIndex = GetYAMLNodeDataFromArchiveNode(node).NodeIndex;
	}

	ArchiveNode YAMLArchive::GetCurrentNode()
	{
		ionassert(m_YAMLTree);

		if (m_CurrentNodeIndex == ryml::NONE)
		{
			return ArchiveNode(this);
		}

		ryml::csubstr key = m_YAMLTree->has_key(m_CurrentNodeIndex) ? m_YAMLTree->key(m_CurrentNodeIndex) : "";
		ryml::NodeType type = m_YAMLTree->type(m_CurrentNodeIndex);

		ArchiveNode node(this, key.data(), YAMLNodeTypeToArchiveNodeType(type));
		node.SetCustomData(YAMLNodeData { m_CurrentNodeIndex });
		return node;
	}

	void YAMLArchive::InitYAMLNode(EArchiveNodeType type, size_t node, ryml::csubstr key, ryml::csubstr val, bool bKey)
	{
		ionassert(m_YAMLTree);

		switch (type)
		{
			case EArchiveNodeType::Value:
			{
				if (bKey)
					m_YAMLTree->to_keyval(node, key, val);
				else
					m_YAMLTree->to_val(node, val);
				break;
			}
			case EArchiveNodeType::Map:
			{
				if (bKey)
					m_YAMLTree->to_map(node, key);
				else
					m_YAMLTree->to_map(node);
				break;
			}
			case EArchiveNodeType::Seq:
			{
				if (bKey)
					m_YAMLTree->to_seq(node, key);
				else
					m_YAMLTree->to_seq(node);
				break;
			}
		}
	}
}
