#pragma once

#include <Editor/Domain/ProducedAsset.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace Horizon
{
	class Engine;

	class ImportContext
	{
	public:
		ImportContext(Engine* pEngine, std::filesystem::path source)
			: m_engine(pEngine), m_source(std::move(source))
		{
		}

		Engine* GetEngine() const { return m_engine; }
		const std::filesystem::path& Source() const { return m_source; }

		b8 IsEnabled(std::string_view key) const;
		i32 GetInt(std::string_view key, i32 fallback) const;

		void* IR() const { return m_ir; }
		void SetIR(void* ir) { m_ir = ir; }

		ProducedAsset& Emit(std::string subName, std::string assetTypeName)
		{
			m_produced.push_back(ProducedAsset{ std::move(subName), std::move(assetTypeName), {}, {} });
			return m_produced.back();
		}

		std::vector<ProducedAsset>& Produced() { return m_produced; }
		const std::vector<ProducedAsset>& Produced() const { return m_produced; }

		void FlagError() { m_hasErrors = true; }
		b8 HasErrors() const { return m_hasErrors; }

	private:
		Engine* m_engine = nullptr;
		std::filesystem::path m_source;

		void* m_ir = nullptr;
		std::vector<ProducedAsset> m_produced;

		b8 m_hasErrors = false;
	};
}