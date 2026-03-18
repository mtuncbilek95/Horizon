#pragma once

namespace Horizon
{
    struct IJob
    {
        virtual void Execute() = 0;
        virtual ~IJob() = default;
    };
}
