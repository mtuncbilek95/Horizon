// Source/EditorApplication/TypeManifestation.h
#pragma once

#include <Editor/Domain/World/WorldAssetImporter.h>
#include <Editor/Widget/AssetBrowser/AssetBrowserWidget.h>
#include <Editor/Widget/SceneHierarchy/SceneHierarchyWidget.h>
#include <Editor/Menu/Templates/EditorMainMenu.h>
#include <Editor/Menu/Templates/File/ExitEngineMenu.h>
#include <Editor/Menu/Templates/View/AssetBrowserMenu.h>
#include <Editor/Menu/Templates/View/SceneHierarchyMenu.h>

#include <Engine/Asset/Loaders/World/DefaultWorldLoadStrategy.h>

#include <Runtime/Reflection/TypeManifest.h>

#include <vector>

extern "C" H_EXPORT void GenerateModuleManifestation(std::vector<Horizon::TypeManifest>* outManifests)
{
	if (!outManifests)
		return;

	outManifests->push_back(Horizon::Reflector<Horizon::WorldAssetImporter>::Build());
	outManifests->push_back(Horizon::Reflector<Horizon::AssetBrowserWidget>::Build());
	outManifests->push_back(Horizon::Reflector<Horizon::SceneHierarchyWidget>::Build());
	outManifests->push_back(Horizon::Reflector<Horizon::EditorMainMenu>::Build());
	outManifests->push_back(Horizon::Reflector<Horizon::ExitEngineMenu>::Build());
	outManifests->push_back(Horizon::Reflector<Horizon::AssetBrowserMenu>::Build());
	outManifests->push_back(Horizon::Reflector<Horizon::SceneHierarchyMenu>::Build());
	outManifests->push_back(Horizon::Reflector<Horizon::DefaultWorldLoadStrategy>::Build());
}