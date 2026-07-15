#pragma once

#include <Engine/Asset/AssetLoaderRegistry.h>
#include <Engine/Asset/IAssetImporter.h>

#define HORIZON_ASSET_LOADER(Type, TypeName)												\
	namespace																				\
	{																						\
		Horizon::IAssetLoader* Type##_CreateLoader()										\
		{																					\
			return Horizon::Allocator::Create<Horizon::Type>(Horizon::CurrLoc());			\
		}																					\
		struct Type##_LoaderRegistrar														\
		{																					\
			Type##_LoaderRegistrar()														\
			{																				\
				Horizon::AssetLoaderRegistry::Get().Register(								\
					Horizon::AssetLoaderTypeInfo{ TypeName, &Type##_CreateLoader });		\
			}																				\
		};																					\
		static Type##_LoaderRegistrar s_##Type##_LoaderRegistrar;							\
	}