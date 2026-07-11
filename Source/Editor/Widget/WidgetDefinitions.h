#pragma once

#include <Editor/Widget/IWidget.h>
#include <Editor/Widget/WidgetFactory.h>

#define HORIZON_BIND_WIDGET(Type, DisplayName, Icon, DockSide, AlwaysOpenFirst)						\
	namespace																						\
	{																								\
		Horizon::IWidget* Type##_Create(Horizon::Engine* pEngine)									\
		{																							\
			auto* pWidget = Horizon::Allocator::Create<Horizon::Type>(Horizon::CurrLoc());			\
			pWidget->SetEngine(pEngine);															\
			return pWidget;																			\
		}																							\
																									\
		struct Type##_Registrar																		\
		{																							\
			Type##_Registrar()																		\
			{																						\
				Horizon::WidgetFactory::Get().Register(												\
					Horizon::WidgetTypeInfo{ DisplayName, Icon, Horizon::DockSide,					\
						&Type##_Create, AlwaysOpenFirst , typeid(Horizon::Type)});					\
			}																						\
		};																							\
																									\
		static Type##_Registrar s_##Type##_Registrar;												\
	}