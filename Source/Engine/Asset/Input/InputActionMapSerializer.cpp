#include "InputActionMapSerializer.h"

#include <Runtime/Log/Terminal.h>

namespace Horizon::Asset
{
	b8 InputActionMapSerializer::Serialize(const List<InputAction>& actions, IArchiveWriter& writer)
	{
		writer.BeginObject();

		writer.Key("version");
		writer.WriteU64(Version);

		writer.Key("actions");
		writer.BeginArray(actions.GetCount());

		for (const InputAction& action : actions)
		{
			writer.BeginObject();

			writer.Key("name");
			writer.WriteString(action.name);

			writer.Key("bindings");
			writer.BeginArray(action.bindings.GetCount());

			for (const InputBinding& binding : action.bindings)
			{
				writer.BeginObject();

				writer.Key("source");
				writer.WriteU64(static_cast<u64>(binding.source));

				writer.Key("code");
				writer.WriteU64(binding.code);

				writer.Key("scaleX");
				writer.WriteF64(binding.scale.X());

				writer.Key("scaleY");
				writer.WriteF64(binding.scale.Y());

				writer.EndObject();
			}

			writer.EndArray();
			writer.EndObject();
		}

		writer.EndArray();
		writer.EndObject();

		return true;
	}

	b8 InputActionMapSerializer::Deserialize(List<InputAction>& actions, IArchiveReader& reader)
	{
		actions.Clear();

		reader.BeginObject();

		if (reader.Key("version") && reader.ReadU64() > Version)
		{
			Terminal::Error("InputActionMapSerializer", "InputActionMap archive is newer than the supported version {}", Version);
			reader.EndObject();
			return false;
		}

		if (!reader.Key("actions"))
		{
			Terminal::Error("InputActionMapSerializer", "InputActionMap archive carries no actions");
			reader.EndObject();
			return false;
		}

		const usize actionCount = reader.BeginArray();
		actions.Reserve(actionCount);

		for (usize actionIndex = 0; actionIndex < actionCount; ++actionIndex)
		{
			InputAction& action = actions.EmplaceBack();

			reader.BeginObject();

			if (reader.Key("name"))
				action.name = reader.ReadString();

			if (reader.Key("bindings"))
			{
				const usize bindingCount = reader.BeginArray();
				action.bindings.Reserve(bindingCount);

				for (usize bindingIndex = 0; bindingIndex < bindingCount; ++bindingIndex)
				{
					InputBinding& binding = action.bindings.EmplaceBack();

					reader.BeginObject();

					if (reader.Key("source"))
					{
						const u64 source = reader.ReadU64();

						if (source > static_cast<u64>(InputBindingSource::GamepadRightTrigger))
						{
							Terminal::Error("InputActionMapSerializer", "Action {} carries an unknown binding source {}", action.name, source);
							return false;
						}

						binding.source = static_cast<InputBindingSource>(source);
					}

					if (reader.Key("code"))
						binding.code = reader.ReadU64();

					if (reader.Key("scaleX"))
						binding.scale.X() = static_cast<f32>(reader.ReadF64());

					if (reader.Key("scaleY"))
						binding.scale.Y() = static_cast<f32>(reader.ReadF64());

					reader.EndObject();
				}

				reader.EndArray();
			}

			reader.EndObject();
		}

		reader.EndArray();
		reader.EndObject();

		return !reader.HasError();
	}
}
