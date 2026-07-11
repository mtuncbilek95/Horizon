#pragma once

#include <Editor/Menu/IMenuItem.h>
#include <Editor/Menu/MenuFactory.h>

#define HORIZON_CONCAT_(a, b) a##b
#define HORIZON_CONCAT(a, b) HORIZON_CONCAT_(a, b)

#define HORIZON_BIND_MENU(Type, MenuPath, Order)													\
	namespace																						\
	{																								\
		Horizon::IMenuItem* Type##_Create(Horizon::Engine* pEngine)									\
		{																							\
			auto* pMenuItem = Horizon::Allocator::Create<Horizon::Type>(Horizon::CurrLoc());		\
			pMenuItem->SetEngine(pEngine);															\
			return pMenuItem;																		\
		}																							\
																									\
		struct Type##_Registrar																		\
		{																							\
			Type##_Registrar()																		\
			{																						\
				Horizon::MenuFactory::Get().RegisterMenu(											\
					Horizon::MenuTypeInfo{ MenuPath, Order, &Type##_Create });						\
			}																						\
		};																							\
																									\
		static Type##_Registrar s_##Type##_Registrar;												\
	}

#define HORIZON_DEFINE_MENU(MenuPath, Order)														\
	namespace																						\
	{																								\
		struct HORIZON_CONCAT(MenuCategory_, __LINE__)												\
		{																							\
			HORIZON_CONCAT(MenuCategory_, __LINE__)()												\
			{																						\
				Horizon::MenuFactory::Get().RegisterCategory(										\
					Horizon::MenuCategoryInfo{ MenuPath, Order });									\
			}																						\
		};																							\
																									\
		static HORIZON_CONCAT(MenuCategory_, __LINE__) HORIZON_CONCAT(s_menuCategory_, __LINE__);	\
	}

HORIZON_DEFINE_MENU("File", 10)
HORIZON_DEFINE_MENU("Edit", 20)
HORIZON_DEFINE_MENU("Assets", 30)
HORIZON_DEFINE_MENU("View", 40)
HORIZON_DEFINE_MENU("Window", 50)
HORIZON_DEFINE_MENU("Tools", 60)
HORIZON_DEFINE_MENU("Build", 70)
HORIZON_DEFINE_MENU("Help", 100)