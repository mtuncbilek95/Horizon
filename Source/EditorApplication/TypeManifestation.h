#pragma once

#include <Editor/Domain/Importer/AssetImporterAttribute.h>
#include <Editor/Domain/World/WorldAssetImporter.h>

#include <Editor/Widget/WidgetAttribute.h>
#include <Editor/Widget/DockLayout.h>
#include <Editor/Widget/AssetBrowser/AssetBrowserWidget.h>
#include <Editor/Widget/SceneHierarchy/SceneHierarchyWidget.h>

#include <Editor/Menu/MenuAttribute.h>
#include <Editor/Menu/Templates/EditorMainMenu.h>
#include <Editor/Menu/Templates/FileMenu.h>
#include <Editor/Menu/Templates/ViewMenu.h>

#include <Editor/Font/IconsFontAwesome6.h>

#include <Engine/Asset/AssetLoadStrategyAttribute.h>
#include <Engine/Asset/Loaders/World/DefaultWorldLoadStrategy.h>

#include <Runtime/Reflection/Type.h>
#include <Runtime/Reflection/TypeAttribute.h>
#include <Runtime/Reflection/TypeManifest.h>
#include <Runtime/Reflection/TypeManifestBuilder.h>

#include <vector>
#include <string>
#include <string_view>
#include <typeindex>

extern "C" H_EXPORT void GenerateModuleManifestation(std::vector<Horizon::TypeManifest>* outManifests)
{
	if (!outManifests)
		return;

	// WorldAssetImporter
	outManifests->push_back(
		Horizon::TypeManifestBuilder::For<Horizon::WorldAssetImporter>("WorldAssetImporter")
		.WithBase(Horizon::TypeIdOf<Horizon::IAssetImporter>())
		.WithAttribute<Horizon::AssetImporterAttribute>(
			std::vector<std::string_view>{ ".hworld" }, "DefaultScene", 1u)
		.Build());

	// AssetBrowserWidget
	outManifests->push_back(
		Horizon::TypeManifestBuilder::For<Horizon::AssetBrowserWidget>("AssetBrowserWidget")
		.WithBase(Horizon::TypeIdOf<Horizon::IWidget>())
		.WithAttribute<Horizon::WidgetTypeAttribute>(
			"Asset Browser", ICON_FA_DATABASE, Horizon::DockLayout::Bottom, true)
		.Build());

	// SceneHierarchyWidget
	outManifests->push_back(
		Horizon::TypeManifestBuilder::For<Horizon::SceneHierarchyWidget>("SceneHierarchyWidget")
		.WithBase(Horizon::TypeIdOf<Horizon::IWidget>())
		.WithAttribute<Horizon::WidgetTypeAttribute>(
			"Scene Hierarchy", ICON_FA_SITEMAP, Horizon::DockLayout::Left, true)
		.Build());

	// EditorMainMenu
	outManifests->push_back(
		Horizon::TypeManifestBuilder::For<Horizon::EditorMainMenu>("EditorMainMenu")
		.WithAttribute<Horizon::MainMenuAttribute>("File", 10u)
		.WithAttribute<Horizon::MainMenuAttribute>("Edit", 20u)
		.WithAttribute<Horizon::MainMenuAttribute>("View", 40u)
		.WithAttribute<Horizon::MainMenuAttribute>("View/Widgets", 1u)
		.Build());

	// MainMenuLeaf - ExitEngineMenu
	outManifests->push_back(
		Horizon::TypeManifestBuilder::For<Horizon::ExitEngineMenu>("ExitEngineMenu")
		.WithBase(Horizon::TypeIdOf<Horizon::IMenuItem>())
		.WithAttribute<Horizon::MenuItemAttribute>("File/Exit", 100u)
		.Build());

	// MainMenuLeaf - AssetBrowserMenu
	outManifests->push_back(
		Horizon::TypeManifestBuilder::For<Horizon::AssetBrowserMenu>("AssetBrowserMenu")
		.WithBase(Horizon::TypeIdOf<Horizon::IMenuItem>())
		.WithAttribute<Horizon::MenuItemAttribute>("View/Widgets/Asset Browser", 1u)
		.Build());

	// MainMenuLeaf - SceneHierarchyMenu
	outManifests->push_back(
		Horizon::TypeManifestBuilder::For<Horizon::SceneHierarchyMenu>("SceneHierarchyMenu")
		.WithBase(Horizon::TypeIdOf<Horizon::IMenuItem>())
		.WithAttribute<Horizon::MenuItemAttribute>("View/Widgets/Scene Hierarchy", 2u)
		.Build());

	// DefaultWorldLoadStrategy
	outManifests->push_back(
		Horizon::TypeManifestBuilder::For<Horizon::DefaultWorldLoadStrategy>("DefaultWorldLoadStrategy")
		.WithBase(Horizon::TypeIdOf<Horizon::IAssetLoadStrategy>())
		.WithAttribute<Horizon::AssetLoadStrategyAttribute>("World", true)
		.Build());
}