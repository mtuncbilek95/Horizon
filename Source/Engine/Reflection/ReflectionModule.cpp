#include "ReflectionModule.h"

#include <Runtime/Reflection/TypeDispatcher.h>
#include <Runtime/Reflection/ReflectionManifestDescriptor.h>

namespace Horizon
{
	namespace
	{
		using GenerateManifestDescriptorFn = ReflectionManifestDescriptor * (*)(void);
		using GenerateManifestAttributesFn = void (*)(ReflectionManifestDescriptor*);

		String MakeFullName(const String& namespaces, const String& name)
		{
			return namespaces.IsEmpty() ? name : namespaces + "::" + name;
		}
	}

	void ReflectionModule::LoadMainModule()
	{
		SymbolLibrary library(SymbolLibraryDesc{ Path(), true });
		ProcessManifest(library);
	}

	void ReflectionModule::LoadModule(const Path& path)
	{
		SymbolLibrary library(SymbolLibraryDesc{ path, false });
		ProcessManifest(library);
	}

	void ReflectionModule::ProcessManifest(const SymbolLibrary& library)
	{
		auto generateDescriptor = (GenerateManifestDescriptorFn)library.GetSymbol("GenerateModuleManifestDescriptor");
		if (generateDescriptor == nullptr)
			return;

		ReflectionManifestDescriptor* pManifest = generateDescriptor();
		const u64 count = pManifest->Types.Size();

		List<Type*> canonicalOf(count);
		List<b8> createdHere(count, false);

		for (u64 i = 0; i < count; i++)
		{
			ReflectionTypeDescriptor& desc = pManifest->Types[i];
			const String fullName = MakeFullName(desc.Namespaces, desc.Name);

			Type* pCanonical = m_context.FindByFullName(fullName);
			if (pCanonical == nullptr)
			{
				pCanonical = TypeDispatcher::CreateType(desc.Name, desc.Namespaces, desc.SizeInBytes,
					desc.Mode, desc.Code, desc.DefaultObjectGeneratorFunction, desc.ppLibraryAddress);
				m_context.Register(pCanonical);
				createdHere[i] = true;
			}
			else
				TypeDispatcher::IncrementTypeShare(pCanonical);

			*desc.ppLibraryAddress = pCanonical;
			canonicalOf[i] = pCanonical;
		}

		for (u64 i = 0; i < count; i++)
		{
			if (!createdHere[i])
				continue;

			ReflectionTypeDescriptor& desc = pManifest->Types[i];
			Type* pType = canonicalOf[i];

			if (desc.BaseTypeIndex != -1)
				TypeDispatcher::SetBaseType(pType, canonicalOf[desc.BaseTypeIndex]);

			for (const ReflectionFieldDescriptor& field : desc.Fields)
				TypeDispatcher::RegisterField(field.Name, field.OffsetInBytes,
					canonicalOf[field.FieldTypeIndex], field.Mode, pType);

			for (const EnumValue& enumValue : desc.Enums)
				TypeDispatcher::RegisterEnum(enumValue.Name, enumValue.Value, pType);
		}

		auto generateAttributes = (GenerateManifestAttributesFn)library.GetSymbol("GenerateModuleManifestAttributes");
		if (generateAttributes != nullptr)
		{
			generateAttributes(pManifest);

			for (u64 i = 0; i < count; i++)
			{
				if (!createdHere[i])
					continue;

				ReflectionTypeDescriptor& desc = pManifest->Types[i];
				for (Attribute* pAttribute : desc.Attributes)
					TypeDispatcher::RegisterAttribute(canonicalOf[i], pAttribute);
			}
		}

		// Can't use Allocator::Delete since pManifest is coming from GetSymbol.
		delete pManifest;
	}
}