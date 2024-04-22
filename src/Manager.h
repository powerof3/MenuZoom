#pragma once

namespace Zoom
{
	enum class MENU
	{
		kNone,
		kCrafting,
		kMagic
	};

	enum class USER_EVENT
	{
		kNone,
		kZoom,
		kZoomWheel,
		kQuitZoom,
		kMouseRotate
	};

	class Manager final :
		public ISingleton<Manager>,
		public RE::BSTEventSink<RE::MenuOpenCloseEvent>,
		public RE::BSTEventSink<RE::InputEvent*>
	{
	public:
		static void Register();
		bool        IsZoomed() const;

	private:
		USER_EVENT GetUserEvent(RE::INPUT_DEVICE a_device, RE::ButtonEvent* a_event);

		void TryCacheModel();
		bool IsHoveringOverItem();
		void DisableSelection(bool a_disable);
		void ToggleMenuFade();

		void ToggleItemZoom();

		RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_evn, RE::BSTEventSource<RE::InputEvent*>*) override;
		RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_evn, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;

		// members
		bool isInMenu{ false };
		MENU menuType;

		bool isHoveringOverItem{ false };
		bool isZoomedIn{ false };
		bool justZoomedOut{ false };

		RE::NiPointer<RE::NiAVObject> cachedModel{};
		RE::NiPoint2                  screenPos;
		float                         boundRadius{};
		bool                          skipRotate{ false };

		double menuRectX{};
		double menuRectY{};
		double menuRectWidth{};
		double menuRectHeight{};
	};
}
