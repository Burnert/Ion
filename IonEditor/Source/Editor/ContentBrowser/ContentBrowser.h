#pragma once

#include "Core/Asset/Asset.h"

namespace Ion::Editor
{
	class ContentBrowser;

	struct UIContentBrowserAssetAnimData
	{
		float IconOpacity = 0.0f;
		bool bVisible = false;
	};

	class EDITOR_API UIContentBrowser
	{
	public:
		UIContentBrowser(ContentBrowser* owner);

		void Draw();

		bool& GetWindowOpenFlag();

	private:
		void DrawAsset(const Asset& asset, UIContentBrowserAssetAnimData& animData);

	private:
		ContentBrowser* m_Owner;

		TArray<UIContentBrowserAssetAnimData> m_AssetAnimData;

		bool m_bWindowOpen;
	};

	inline bool& UIContentBrowser::GetWindowOpenFlag()
	{
		return m_bWindowOpen;
	}

	class EDITOR_API ContentBrowser
	{
	public:
		ContentBrowser();
		~ContentBrowser();

		UIContentBrowser* AddUI();

		void DrawUI();

		bool& GetUIWindowOpenFlag();

		void UpdateRegisteredAssetsCache();
		const TArray<Asset>& GetRegisteredAssetsCache() const;

		void SetSelectedAsset(const Asset& asset);
		Asset GetSelectedAsset() const;

	private:
		UIContentBrowser* m_UI;
		TArray<Asset> m_RegisteredAssetsCache;
		Asset m_SelectedAsset;
	};

	inline const TArray<Asset>& ContentBrowser::GetRegisteredAssetsCache() const
	{
		return m_RegisteredAssetsCache;
	}

	inline void ContentBrowser::SetSelectedAsset(const Asset& asset)
	{
		m_SelectedAsset = asset;
	}

	inline Asset ContentBrowser::GetSelectedAsset() const
	{
		return m_SelectedAsset;
	}
}
