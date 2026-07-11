#pragma once

#include <Editor/Domain/Importer/IAssetImporter.h>
#include <Editor/Domain/Importer/ImporterRegistry.h>

#define HORIZON_SCRIPTED_IMPORTER(Type, Extension, Version)								\
	namespace																			\
	{																					\
		Horizon::IAssetImporter* Type##_Create()										\
		{																				\
			return Horizon::Allocator::Create<Horizon::Type>(Horizon::CurrLoc());		\
		}																				\
																						\
		struct Type##_Registrar															\
		{																				\
			Type##_Registrar()															\
			{																			\
				Horizon::ImporterRegistry::Get().Register(								\
					Horizon::ImporterTypeInfo{ Extension, Version, &Type##_Create });	\
			}																			\
		};																				\
																						\
		static Type##_Registrar s_##Type##_Registrar;									\
	}