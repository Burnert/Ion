#include "Core/CorePCH.h"

#include "YAMLArchive.h"

namespace Ion
{
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
		ionassert(m_CurrentNode.valid());

		if (IsSaving())
		{
			m_CurrentNode << value;
		}
		else if (IsLoading())
		{
			m_CurrentNode >> value;
		}
	}

	void YAMLArchive::Serialize(IArrayItem& item)
	{
		ionassert(m_CurrentNode.is_seq());

		if (IsSaving())
		{
			m_CurrentNode = m_CurrentNode.append_child();
			// NOTE: Serializes the value using operator<<
			// which eventually calls operator<< or operator>> on current node (ryml::NodeRef).
			// That's why the current node has to be set for every item.
			item.Serialize(*this);
			m_CurrentNode = m_CurrentNode.parent();
		}
		else if (IsLoading())
		{
			ionassert(item.GetIndex() < m_CurrentNode.num_children());

			m_CurrentNode = m_CurrentNode[item.GetIndex()];
			item.Serialize(*this);
			m_CurrentNode = m_CurrentNode.parent();
		}
	}

	size_t YAMLArchive::GetCollectionSize() const
	{
		return m_CurrentNode.num_children();
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
			m_CurrentNode = m_YAMLTree->rootref();
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

	void YAMLArchive::EnterNode(const String& name)
	{
		ionassert(m_YAMLTree);
		ionassert(m_CurrentNode.valid());

		if (!m_CurrentNode.is_map())
		{
			if (IsLoading())
			{
				ionerror(IOError, "Current node is not a map.");
			}
			else if (IsSaving())
			{
				// In a saving archive, entering a node must change the current node to a map.
				// E.g. You cannot enter nodes by key when in a sequence node.
				m_CurrentNode |= ryml::MAP;
			}
		}

		ryml::NodeRef node;
		if (IsSaving())
		{
			node = m_CurrentNode.append_child() << ryml::key(ryml::to_csubstr(name));
		}
		else if (IsLoading())
		{
			node = m_CurrentNode[ryml::to_csubstr(name)];
			if (!node.has_key())
				ionerror(IOError, "Node \"{}\" not found.", name);
		}

		m_CurrentNode = node;
	}

	void YAMLArchive::ExitNode()
	{
		ionassert(m_YAMLTree);
		ionassert(m_CurrentNode.valid());

		// NOTE: A node must have a value or else ryml crashes on emit.
		if (m_CurrentNode.type() == ryml::KEY)
		{
			m_CurrentNode = ryml::csubstr();
		}

		if (!m_CurrentNode.has_parent())
			ionerror(IOError, "Current node has no parent.");
		
		m_CurrentNode = m_CurrentNode.parent();
	}

	void YAMLArchive::BeginSeq()
	{
		ionassert(m_YAMLTree);
		ionassert(m_CurrentNode.valid());

		m_SeqIndex = 0;

		if (IsLoading())
		{
			if (!m_CurrentNode.is_seq())
				ionerror(IOError, "Current node is not a sequence.");
		}
		else if (IsSaving())
		{
			m_CurrentNode |= ryml::SEQ;
		}
	}

	bool YAMLArchive::IterateSeq()
	{
		ionassert(m_YAMLTree);
		ionassert(m_CurrentNode.valid());

		ryml::NodeRef node;

		if (IsSaving())
		{
			// 0 means that the current seq node has not been iterated through yet.
			if (m_SeqIndex != 0)
			{
				// Return to the parent before selecting the next child
				// if it's not the first function call in the sequence.
				m_CurrentNode = m_CurrentNode.parent();
			}

			node = m_CurrentNode.append_child();

			++m_SeqIndex;
		}
		else if (IsLoading())
		{
			// 0 means that the current seq node has not been iterated through yet.
			if (m_SeqIndex != 0)
			{
				// Return to the parent before selecting the next child
				// if it's not the first function call in the sequence.
				m_CurrentNode = m_CurrentNode.parent();
			}

			node = m_CurrentNode.child(m_SeqIndex);

			if (!node.valid())
			{
				//if (m_SeqIndex != 0)
				//{
				//	// Return to the parent on the last call.
				//	// Don't do it if there were no children, as the node
				//	// has never been set to a child node.
				//	m_CurrentNode = m_CurrentNode.parent();
				//}
				m_SeqIndex = 0;
				return false;
			}

			++m_SeqIndex;
		}

		m_CurrentNode = node;

		return true;
	}

	void YAMLArchive::EndSeq()
	{
		ionassert(m_YAMLTree);
		ionassert(m_CurrentNode.valid());

		// Return to the parent if IterateSeq didn't.
		if (m_SeqIndex != 0)
		{
			m_CurrentNode = m_CurrentNode.parent();
			m_SeqIndex = 0;
		}

		if (!m_CurrentNode.is_seq())
			ionerror(IOError, "Current node is not a sequence.");
	}
}
