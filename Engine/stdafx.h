#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include <cassert>
#include <cstdint>
#include <tchar.h>

#include <exception>

// ReSharper disable once CppInconsistentNaming
#define _XM_NO_INTRINSICS_
#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <dinput.h>

// Désactive tous les warnings concernés avant les includes PhysX
#pragma warning(push)
#pragma warning(disable : 26495)  // Variable membre non initialisée
#pragma warning(disable : 33010)  // Verification de borne d'énumération
#pragma warning(disable : 6297)   // Dépassement arithmétique

#include <PxPhysicsAPI.h>
#include <PxSimulationStatistics.h>
#include <PxNodeIndex.h>

// Restore les warnings originaux
#pragma warning(pop)


#include <stdexcept>
