#pragma once

namespace RE
{
	RE::NiPointer<RE::NiAVObject> GetInventoryModel()
	{
		auto inv3dMgr = RE::Inventory3DManager::GetSingleton();
		return !inv3dMgr->loadedModels.empty() ? inv3dMgr->loadedModels.back().spModel : nullptr;
	}
	
	inline static bool& startMouseRotation{
		*REL::Relocation<bool*>{ RELOCATION_ID(519620, 406167) }
	};
}
