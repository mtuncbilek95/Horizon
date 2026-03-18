#pragma once

#include <Engine/Engine/EngineReport.h>
#include <string_view>
#include <typeindex>

namespace Horizon
{
    class ISystem
    {
    public:
        virtual ~ISystem() = default;

        virtual EngineReport OnInitialize() = 0;
        virtual void OnSync() = 0;
        virtual void OnFinalize() = 0;

        virtual std::string_view GetObjectType() const = 0;
        virtual std::type_index GetObjectIndex() const = 0;

    protected:
        template<typename U>
        static std::string_view TypeName()
        {
#if defined(HORIZON_LINUX)
            std::string_view name = __PRETTY_FUNCTION__;
            auto start = name.rfind("U = ") + 4;
            auto end = name.rfind("]");
            name = name.substr(start, end - start);

            if (name.starts_with("class "))
                name.remove_prefix(6);
            if (name.starts_with("struct "))
                name.remove_prefix(7);
            auto ns = name.rfind("::");

            if (ns != std::string_view::npos)
                name = name.substr(ns + 2);

            return name;
#elif defined(HORIZON_WINDOWS)
            std::string_view name = __FUNCSIG__;
            auto start = name.rfind("TypeName<") + 9;
            auto end = name.rfind(">(");
            name = name.substr(start, end - start);

            if (name.starts_with("class "))
                name.remove_prefix(6);
            if (name.starts_with("struct "))
                name.remove_prefix(7);
            auto ns = name.rfind("::");

            if (ns != std::string_view::npos)
                name = name.substr(ns + 2);

            return name;
#else
#error "Unsupported platform: define HORIZON_LINUX or HORIZON_WINDOWS"
#endif
        }
    };
}