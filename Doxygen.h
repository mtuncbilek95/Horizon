// DoxygenShowcase.h
//
// Every supported Doxygen command. Hover the NAME of each declaration.
// Note: the comment must live in the same file, above the declaration.

#pragma once

#include <cstdint>

namespace Showcase
{
	/// @brief A one-line short description.
	///
	/// Everything after the blank line is free description; @brief ends at the
	/// first blank line. This paragraph renders unlabelled, as plain text.
	class BasicUsage
	{
	};

	/// @brief The full function documentation set.
	///
	/// @param      desc     A plain parameter with no direction.
	/// @param[in]  source   An input parameter.
	/// @param[out] result   An output parameter.
	/// @param[in,out] state Read and written.
	/// @return The new object on success, otherwise nullptr.
	/// @throws std::bad_alloc When allocation fails.
	/// @pre  source must not be nullptr.
	/// @post result is always written.
	/// @invariant The size of state never changes.
	void* CreateThing(int desc, const void* source, void* result, int state);

	/// @brief The notice family.
	///
	/// @warning Not thread safe.
	/// @attention Initialize the device before calling this.
	/// @note No longer takes a lock since version 2.0.
	/// @remark Shares an internal buffer.
	/// @bug Fails silently when given an empty name.
	/// @todo Convert this to return an error code.
	void NoticeFamily();

	/// @brief Code samples.
	///
	/// Without a language:
	/// @code
	///     RHI::GfxBufferDesc bufDesc = {};
	///     RHI::GfxBuffer* buf = nullptr;
	///     buf = myRhiDevice->CreateBuffer(bufDesc);
	/// @endcode
	///
	/// With a language, which titles the block "Code (cpp)":
	/// @code{.cpp}
	///     const char* name = "directional";
	///     float intensity = 1.0f;
	///     auto* ptr = new Thing(name, intensity);
	///     delete ptr;
	/// @endcode
	///
	/// @verbatim
	///     An unformatted block, drawn exactly like @code.
	/// @endverbatim
	void CodeSamples();

	/// @brief Links, credits and lifecycle commands.
	///
	/// @see    Showcase::BasicUsage
	/// @sa     Showcase::CreateThing
	/// @ref    NoticeFamily
	/// @since  1.4
	/// @author mtuncbilek
	/// @deprecated Use CreateThing instead.
	/// @example samples/basic.cpp
	void MetadataFamily();

	/// @brief Template parameters.
	///
	/// @tparam T The element type.
	/// @tparam N The element count.
	template <typename T, int N>
	class TemplateUsage
	{
	};

	/**
	 * @brief Javadoc block comments work too.
	 *
	 * Continuation asterisks are stripped, but an asterisk inside code survives:
	 * @code
	 *     GfxBuffer* buf = nullptr;
	 *     int* p = &value;
	 * @endcode
	 */
	class JavadocStyle
	{
	};

	/*!
		\brief Backslash syntax inside a slash-bang block.

		\warning The two syntaxes may be mixed.
		\code{.cpp}
		int* p = nullptr;
		\endcode
	*/
	class BackslashStyle
	{
	};

	//! @brief A single-line //! comment works as well.
	//! @note And a second line.
	class BangStyle
	{
	};

	/// No explicit @brief here. The first sentence becomes the brief automatically.
	/// The rest flows into the description.
	class AutoBrief
	{
	};

	/// @brief Unknown commands are never dropped.
	///
	/// @customtag An unrecognized command shows up labelled "@customtag:".
	/// @horizonspecific You can invent your own and it still renders.
	class UnknownCommands
	{
	};

	/**
	 * @brief A reflection macro sits between this comment and the class.
	 *
	 * The comment still binds to Asset rather than to HCLASS().
	 */
	HCLASS();
	class H_EXPORT Asset : public Reflect::Base
	{
	public:
		Asset() = default;
		virtual ~Asset() = default;
	};

	/**
	 * @brief A directional light source.
	 * @reflect
	 *    Attributes:
	 *       ComponentTypeAttribute("Directional Light Component", "Lighting", true)
	 *    Fields:
	 *       Math::Vec3f -> m_direction
	 *       Math::Vec3f -> m_color
	 *       f32 -> m_intensity
	 * @endreflect
	 */
	HCLASS(ComponentType["Directional Light Component", "Lighting", true]);
	class H_EXPORT DirectionalLightComponent : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(DirectionalLightComponent);

	public:
		DirectionalLightComponent() = default;
		~DirectionalLightComponent() = default;

		HFIELD();
		Math::Vec3f m_direction;

		HFIELD();
		Math::Vec3f m_color;

		HFIELD();
		f32 m_intensity = 1.f;
	};
}