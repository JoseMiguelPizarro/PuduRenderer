//
// Created by Hojaverde on 7/19/2025.
//

#pragma once
#include <tracy/Tracy.hpp>

#define PROFILER_ZONE(name, color) \
{                                    \
ZoneScopedC(color);                \
ZoneName(name, strlen(name))
#define PROFILER_ZONE_END() }