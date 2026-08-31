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
		public REX::TSingleton<Manager>,
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
		void DisableSelection(bool a_disable) const;
		void ToggleMenuFade();

		void ToggleItemZoom();

		void Reset();

		RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_evn, RE::BSTEventSource<RE::InputEvent*>*) override;
		RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_evn, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;

		// members
		RE::NiPointer<RE::NiAVObject> cachedModel{};
		RE::NiPoint2                  screenPos;

		MENU menuType;

		float boundRadius{};

		double menuRectX{};
		double menuRectY{};
		double menuRectWidth{};
		double menuRectHeight{};

		bool isInMenu{ false };
		bool skipRotate{ false };
		bool isHoveringOverItem{ false };
		bool isZoomedIn{ false };
		bool justZoomedOut{ false };
	};
}
