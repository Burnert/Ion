#include "EditorPCH.h"

#include "ContentBrowser.h"

#include "EditorApplication/EditorApplication.h"
#include "Editor/EditorAssets.h"

#include "UserInterface/ImGui.h"

#include "Engine/World.h"
#include "Engine/Entity/MeshEntity.h"

#include "Resource/MeshResource.h"

#include "Renderer/Mesh.h"

namespace Ion::Editor
{
	// UIContentBrowser -----------------------------------------------------------------

	struct DNDMeshEntityCustomData
	{
		Asset MeshAsset;
	};

	UIContentBrowser::UIContentBrowser(ContentBrowser* owner) :
		m_Owner(owner),
		m_bWindowOpen(true)
	{
	}

	void UIContentBrowser::Draw()
	{
		if (m_bWindowOpen && ImGui::Begin("Content Browser", &m_bWindowOpen))
		{
			ImGui::PushID("ContentBrowser");

			// @TODO: Make some kind of a directory tree here.

			ImGui::BeginChild("AssetTree", ImVec2(160, 0));
			{
				if (ImGui::Button("Refresh"))
				{
					m_Owner->UpdateRegisteredAssetsCache();
				}

				ImGui::Text("Asset Tree Lol");
			}
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("Assets");
			{
				const TArray<Asset>& registeredAssetCache = m_Owner->GetRegisteredAssetsCache();
				m_AssetAnimData.resize(registeredAssetCache.size());

				size_t assetIndex = 0;
				for (const Asset& asset : registeredAssetCache)
				{
					DrawAsset(asset, m_AssetAnimData[assetIndex++]);
				}
			}
			ImGui::EndChild();
			// Deselect the asset on empty space click
			if (ImGui::IsItemClicked())
			{
				m_Owner->SetSelectedAsset(Asset());
			}

			ImGui::PopID();

			ImGui::End();
		}
	}

	void UIContentBrowser::DrawAsset(const Asset& asset, UIContentBrowserAssetAnimData& animData)
	{
		ImVec2 assetIconSize = { 96, 128 };
		float assetIconSeparation = 8;

		String sType = ToString(asset->GetType());
		String sName = asset->GetInfo().Name;

		ImVec2 avail = ImGui::GetContentRegionAvail();

		// Place the image in the same place the selectable is
		ImVec2 selectableCursor = ImGui::GetCursorPos();

		bool bSelected = m_Owner->GetSelectedAsset() == asset;

		// Draw an empty selectable
		if (ImGui::Selectable(("##" + sType + sName).c_str(), bSelected, ImGuiSelectableFlags_None, assetIconSize))
		{
			m_Owner->SetSelectedAsset(asset);
		}

		bool bHovered = ImGui::IsItemHovered();
		if (bHovered)
		{
			EditorApplication::Get()->SetCursor(ECursorType::Hand);
		}
		animData.bVisible = bHovered || bSelected;
		float fadeInTime = 0.15f;
		float offsetOpacityBy = EditorApplication::Get()->GetGlobalDeltaTime() / fadeInTime * (animData.bVisible ? 1.0f : -1.0f);
		animData.IconOpacity = Math::Clamp(animData.IconOpacity + offsetOpacityBy, 0.0f, 1.0f);

		ImGuiDragDropFlags dndFlags = ImGuiDragDropFlags_None;
		if (ImGui::BeginDragDropSource(dndFlags))
		{
			switch (asset->GetType())
			{
				case EAssetType::Mesh:
				{
					DNDInsertEntityData data { };
					data.CustomData = new DNDMeshEntityCustomData { asset };
					data.Instantiate = [](World* context, void* customDataPtr) -> Entity*
					{
						DNDMeshEntityCustomData customData = *(DNDMeshEntityCustomData*)customDataPtr;
						delete (DNDMeshEntityCustomData*)customDataPtr;

						TShared<Mesh> mesh;

						TResourcePtr<MeshResource> meshResource = MeshResource::Query(customData.MeshAsset);
						mesh = Mesh::CreateFromResource(meshResource);

						MeshEntity* meshEntity = context->SpawnEntityOfClass<MeshEntity>();
						meshEntity->GetMeshComponent()->SetMeshResource(meshResource);
						meshEntity->GetMeshComponent()->SetMeshAsset(customData.MeshAsset);
						meshEntity->SetMesh(mesh);

						return meshEntity;
					};
					ImGui::SetDragDropPayload("Ion_DND_InsertEntity", &data, sizeof(DNDInsertEntityData), ImGuiCond_Once);
					ImGui::Text(sName.c_str());
					break;
				}
			}
			ImGui::EndDragDropSource();
		}

		// Where to place the next item
		ImVec2 nextCursor;
		if (avail.x - assetIconSize.x - assetIconSeparation > assetIconSize.x)
		{
			ImGui::SameLine();
			nextCursor = ImGui::GetCursorPos();
			nextCursor.x += assetIconSeparation;
		}
		else
		{
			nextCursor = ImGui::GetCursorPos();
			nextCursor.y += assetIconSeparation;
		}

		// Draw the asset icon

		ImGui::SetCursorPos(selectableCursor);

		void* iconTexture;
		switch (asset->GetType())
		{
		case EAssetType::Mesh:  iconTexture = EditorIcons::IconMeshAsset.Texture->GetNativeID();  break;
		case EAssetType::Image: iconTexture = EditorIcons::IconImageAsset.Texture->GetNativeID(); break;
		default:                iconTexture = EditorIcons::IconAsset.Texture->GetNativeID();      break;
		}

		ImGui::Image(iconTexture, ImVec2(assetIconSize.x, assetIconSize.x), ImVec2(0, 1), ImVec2(1, 0));

		// Draw resource usage icons

		if (animData.IconOpacity != 0.0f)
		{
			ImGui::SetCursorPos(selectableCursor);

			for (const String& resource : asset->GetInfo().ResourceUsage)
			{
				TShared<RHITexture> resourceIconTexture;

				if (resource == "Mesh")
				{
					resourceIconTexture = EditorIcons::IconMeshResource.Texture;
				}
				else if (resource == "Texture")
				{
					resourceIconTexture = EditorIcons::IconTextureResource.Texture;
				}

				ImGui::Image(resourceIconTexture->GetNativeID(), ImVec2(32, 32), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1, 1, 1, animData.IconOpacity));
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text("The asset can be used as a %s resource.", resource.c_str());
					ImGui::EndTooltip();
				}
				ImGui::SameLine();
			}
		}

		// Draw the label

		ImVec2 textSize = ImGui::CalcTextSize(sName.c_str());
		ImGui::SetCursorPos(ImVec2(selectableCursor.x + (assetIconSize.x - textSize.x) * 0.5f, selectableCursor.y + assetIconSize.x));
		ImGui::Text(sName.c_str());

		ImGui::SetCursorPos(nextCursor);
	}

	// ContentBrowser -----------------------------------------------------------------

	ContentBrowser::ContentBrowser() :
		m_UI(nullptr),
		m_RegisteredAssetsCache(),
		m_SelectedAsset()
	{
	}

	ContentBrowser::~ContentBrowser()
	{
		checked_delete(m_UI);
	}

	UIContentBrowser* ContentBrowser::AddUI()
	{
		if (!m_UI)
		{
			m_UI = new UIContentBrowser(this);
		}
		return m_UI;
	}

	void ContentBrowser::DrawUI()
	{
		m_UI->Draw();
	}

	bool& ContentBrowser::GetUIWindowOpenFlag()
	{
		return m_UI->GetWindowOpenFlag();
	}

	void ContentBrowser::UpdateRegisteredAssetsCache()
	{
		m_RegisteredAssetsCache = AssetRegistry::GetAllRegisteredAssets();
		// @TODO: Update the tree
	}
}
