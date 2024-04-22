#include "Hooks.h"

#include "Manager.h"

namespace Hooks
{
	void Install()
	{
		logger::info("{:*^30}", "HOOKS");

		stl::write_vfunc<RE::MagicMenu, ProcessMessage<RE::MagicMenu>>();
		stl::write_vfunc<RE::CraftingMenu, ProcessMessage<RE::CraftingMenu>>();

		logger::info("Installed ProcessMessage hooks");
	}
}
