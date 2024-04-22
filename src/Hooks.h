#pragma once

#include "Manager.h"

namespace Hooks
{
	template <class T>
	struct ProcessMessage
	{
		static RE::UI_MESSAGE_RESULTS thunk(T* a_this, RE::UIMessage& a_message)
		{
			if constexpr (std::is_same_v<T, RE::MagicMenu>) {
				if ((a_message.type == RE::UI_MESSAGE_TYPE::kScaleformEvent || a_message.type == RE::UI_MESSAGE_TYPE::kUserEvent) && Zoom::Manager::GetSingleton()->IsZoomed()) {
					return RE::UI_MESSAGE_RESULTS::kIgnore;
				}
			} else {
				if (a_message.type == RE::UI_MESSAGE_TYPE::kUserEvent && Zoom::Manager::GetSingleton()->IsZoomed()) {
					return RE::UI_MESSAGE_RESULTS::kIgnore;
				}			
			}

			return func(a_this, a_message);
		}
		static inline REL::Relocation<decltype(thunk)> func;
		static inline constexpr std::size_t            size = 0x04;
	};

	void Install();
}
