#include "Manager.h"

#include "RE.h"

namespace Zoom
{
	void Manager::Register()
	{
		if (const auto inputMgr = RE::BSInputDeviceManager::GetSingleton()) {
			inputMgr->AddEventSink<RE::InputEvent*>(GetSingleton());
			logger::info("Registered for hotkey event");
		}

		RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(GetSingleton());
		logger::info("Registered for menu open/close event");
	}

	bool Manager::IsZoomed() const
	{
		return isZoomedIn || justZoomedOut;
	}

	USER_EVENT Manager::GetUserEvent(RE::INPUT_DEVICE a_device, RE::ButtonEvent* a_event)
	{
		const auto& userEvent = a_event->QUserEvent();

		if (!userEvent.empty()) {
			if (userEvent == RE::UserEvents::GetSingleton()->itemZoom) {
				return USER_EVENT::kZoom;
			} else if (userEvent == RE::UserEvents::GetSingleton()->zoomIn || userEvent == RE::UserEvents::GetSingleton()->zoomOut) {
				return USER_EVENT::kZoomWheel;
			} else if (userEvent == RE::UserEvents::GetSingleton()->cancel) {
				return USER_EVENT::kQuitZoom;
			}
		}

		// fallback
		switch (a_device) {
		case RE::INPUT_DEVICE::kMouse:
			{
				switch (static_cast<RE::BSWin32MouseDevice::Key>(a_event->GetIDCode())) {
				case RE::BSWin32MouseDevice::Key::kWheelUp:
				case RE::BSWin32MouseDevice::Key::kWheelDown:
					return USER_EVENT::kZoomWheel;
				case RE::BSWin32MouseDevice::Key::kLeftButton:
					return USER_EVENT::kMouseRotate;
				default:
					break;
				}
			}
			break;
		case RE::INPUT_DEVICE::kKeyboard:
			{
				switch (static_cast<RE::BSWin32KeyboardDevice::Key>(a_event->GetIDCode())) {
				case RE::BSWin32KeyboardDevice::Key::kEscape:
				case RE::BSWin32KeyboardDevice::Key::kTab:
					return USER_EVENT::kQuitZoom;
				case RE::BSWin32KeyboardDevice::Key::kC:
					return USER_EVENT::kZoom;
				default:
					break;
				}
			}
			break;
		case RE::INPUT_DEVICE::kGamepad:
			{
				if (RE::ControlMap::GetSingleton()->GetGamePadType() == RE::PC_GAMEPAD_TYPE::kOrbis) {
					switch (static_cast<RE::BSPCOrbisGamepadDevice::Key>(a_event->GetIDCode())) {
					case RE::BSPCOrbisGamepadDevice::Key::kPS3_R3:
						return USER_EVENT::kZoom;
					case RE::BSPCOrbisGamepadDevice::Key::kPS3_B:
						return USER_EVENT::kQuitZoom;
					default:
						break;
					}
				} else {
					switch (static_cast<RE::BSWin32GamepadDevice::Key>(a_event->GetIDCode())) {
					case RE::BSWin32GamepadDevice::Key::kRightThumb:
						return USER_EVENT::kZoom;
					case RE::BSWin32GamepadDevice::Key::kB:
						return USER_EVENT::kQuitZoom;
					default:
						break;
					}
				}
			}
			break;
		default:
			break;
		}

		return USER_EVENT::kNone;
	}

	void Manager::TryCacheModel()
	{
		if (const auto model = RE::GetInventoryModel()) {
			if (cachedModel != model) {
				cachedModel = model;

				float z{};
				RE::UI3DSceneManager::GetSingleton()->camera->WorldPtToScreenPt3(cachedModel->worldBound.center, screenPos.x, screenPos.y, z, 1.0E-5F);

				screenPos.x *= RE::BSGraphics::Renderer::GetScreenSize().width;
				screenPos.y = (1.0f - screenPos.y) * RE::BSGraphics::Renderer::GetScreenSize().height;  // inverting produces more correct y values?

				boundRadius = std::clamp(cachedModel->worldBound.radius, 15.f, 20.f);
				boundRadius *= boundRadius;

				skipRotate = false;

				RE::BSVisit::TraverseScenegraphObjects(cachedModel.get(), [&](RE::NiAVObject* a_object) -> RE::BSVisit::BSVisitControl {
					if (auto node = a_object ? netimmerse_cast<RE::NiBillboardNode*>(a_object) : nullptr) {
						if (node->GetMode() == RE::NiBillboardNode::FaceMode::kRigidFaceCenter) {
							skipRotate = true;
							return RE::BSVisit::BSVisitControl::kStop;
						}
					}
					return RE::BSVisit::BSVisitControl::kContinue;
				});
			}
		} else {
			cachedModel.reset();
		}
	}

	bool Manager::IsHoveringOverItem()
	{
		if (cachedModel) {
			RE::NiPoint2 mousePos(RE::MenuCursor::GetSingleton()->cursorPosX, RE::MenuCursor::GetSingleton()->cursorPosY);
			return mousePos.GetDistance(screenPos) <= boundRadius;
		}
		return false;
	}

	void Manager::DisableSelection(bool a_disable)
	{
		if (menuType == MENU::kCrafting) {
			if (auto menu = RE::UI::GetSingleton()->GetMenu<RE::CraftingMenu>()) {
				if (auto movie = menu->subMenu ? menu->subMenu->view : nullptr) {
					movie->SetVariable("Menu.ItemList.disableSelection", a_disable);
					movie->SetVariable("Menu.ItemList.disableInput", a_disable);
				}
			}
		}
	}

	void Manager::ToggleMenuFade()
	{
		if (menuType == MENU::kCrafting) {
			if (auto menu = RE::UI::GetSingleton()->GetMenu<RE::CraftingMenu>()) {
				if (auto movie = menu->subMenu ? menu->subMenu->view : nullptr) {
					isZoomedIn = !isZoomedIn;

					movie->SetVariable("Menu.InventoryLists._visible", !isZoomedIn);
					movie->SetVariable("Menu.ItemInfoHolder._visible", !isZoomedIn);
					movie->SetVariable("Menu.MenuNameHolder._visible", !isZoomedIn);
					movie->SetVariable("Menu.BottomBarInfo._visible", !isZoomedIn);
					movie->SetVariable("Menu.MenuName._visible", !isZoomedIn);
					movie->SetVariable("Menu.MenuDescription._visible", !isZoomedIn);

					if (isZoomedIn) {
						menuRectX = movie->GetVariableDouble("Menu.MouseRotationRect._x");
						menuRectY = movie->GetVariableDouble("Menu.MouseRotationRect._y");
						menuRectWidth = movie->GetVariableDouble("Menu.MouseRotationRect._width");
						menuRectHeight = movie->GetVariableDouble("Menu.MouseRotationRect._height");

						movie->SetVariableDouble("Menu.MouseRotationRect._width", 10000);
						movie->SetVariableDouble("Menu.MouseRotationRect._height", 10000);
						movie->SetVariableDouble("Menu.MouseRotationRect._x", 0.0);
						movie->SetVariableDouble("Menu.MouseRotationRect._y", 0.0);

						movie->SetVariable("Menu.ItemList.disableSelection", true);
						movie->SetVariable("Menu.ItemList.disableInput", true);

					} else {
						movie->SetVariable("Menu.MouseRotationRect._width", menuRectWidth);
						movie->SetVariable("Menu.MouseRotationRect._height", menuRectHeight);
						movie->SetVariable("Menu.MouseRotationRect._x", menuRectX);
						movie->SetVariable("Menu.MouseRotationRect._y", menuRectY);

						justZoomedOut = true;
					}
				}
			}
		} else {
			if (auto movie = RE::UI::GetSingleton()->GetMovieView(RE::MagicMenu::MENU_NAME)) {
				isZoomedIn = !isZoomedIn;

				movie->SetVariable("_root.Menu_mc._visible", !isZoomedIn);

				if (!isZoomedIn) {
					justZoomedOut = true;
				}
			}
		}
	}

	void Manager::ToggleItemZoom()
	{
		bool result = true;
		auto inv3dMgr = RE::Inventory3DManager::GetSingleton();

		if (inv3dMgr->zoomDistance != 0.0 || inv3dMgr->zoomProgress != 1.0 || !inv3dMgr->ToggleItemZoom()) {
			if (inv3dMgr->zoomDistance != 0.0 || inv3dMgr->zoomProgress != 0.0 || !inv3dMgr->ToggleItemZoom()) {
				result = false;
			}
		}

		if (result) {
			ToggleMenuFade();
		}
	}

	RE::BSEventNotifyControl Manager::ProcessEvent(RE::InputEvent* const* a_evn, RE::BSTEventSource<RE::InputEvent*>*)
	{
		if (!a_evn || !isInMenu) {
			return RE::BSEventNotifyControl::kContinue;
		}

		for (auto event = *a_evn; event; event = event->next) {
			auto eventType = event->GetEventType();
			auto device = event->GetDevice();
			if (const auto buttonEvent = event->AsButtonEvent()) {
				switch (GetUserEvent(device, buttonEvent)) {
				case USER_EVENT::kZoom:
					if (buttonEvent->IsDown()) {
						ToggleItemZoom();
					}
					break;
				case USER_EVENT::kZoomWheel:
					if (buttonEvent->IsDown() && (isZoomedIn || isHoveringOverItem)) {
						ToggleItemZoom();
					}
					break;
				case USER_EVENT::kQuitZoom:
					if (buttonEvent->IsDown()) {
						if (isZoomedIn) {
							ToggleItemZoom();
						} else if (justZoomedOut) {
							RE::UIMessageQueue::GetSingleton()->AddMessage(menuType == MENU::kMagic ? RE::MagicMenu::MENU_NAME : RE::CraftingMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
							justZoomedOut = false;
						}
					}
					break;
				case USER_EVENT::kMouseRotate:
					if (menuType == MENU::kMagic && device == RE::INPUT_DEVICE::kMouse) {
						if (buttonEvent->IsHeld() && (isHoveringOverItem || isZoomedIn) && !skipRotate && !RE::startMouseRotation) {
							RE::startMouseRotation = true;
						} else if (RE::startMouseRotation && buttonEvent->IsUp()) {
							RE::startMouseRotation = false;
						}
					}
					break;
				default:
					break;
				}
			} else if (eventType == RE::INPUT_EVENT_TYPE::kMouseMove || eventType == RE::INPUT_EVENT_TYPE::kThumbstick) {
				if (justZoomedOut) {
					DisableSelection(false);
					justZoomedOut = false;
				} else {
					TryCacheModel();
					isHoveringOverItem = IsHoveringOverItem();
				}
			}
		}

		return RE::BSEventNotifyControl::kContinue;
	}

	RE::BSEventNotifyControl Manager::ProcessEvent(const RE::MenuOpenCloseEvent* a_evn, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
	{
		if (!a_evn) {
			return RE::BSEventNotifyControl::kContinue;
		}

		if (a_evn->menuName == RE::CraftingMenu::MENU_NAME || a_evn->menuName == RE::MagicMenu::MENU_NAME) {
			isInMenu = a_evn->opening;
			menuType = (a_evn->menuName == RE::CraftingMenu::MENU_NAME) ? MENU::kCrafting : MENU::kMagic;

			if (isInMenu && menuType == MENU::kMagic) {
				// allow native mouse rotation
				RE::Inventory3DManager::GetSingleton()->enableUserInput = true;
			}

			// reset variables
			cachedModel.reset();
			isHoveringOverItem = false;
			isZoomedIn = false;
			justZoomedOut = false;
		}

		return RE::BSEventNotifyControl::kContinue;
	}
}
