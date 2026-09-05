#include "../../Menu/Renderer/Renderer.h"
#include "../Hooked.hpp"
#include <intrin.h>
#include "../../Utils/InputSys.hpp"
#include "../../Features/Visuals/Hitmarker.hpp"
#include "../../Renderer/Render.hpp"
#include "../../SDK/Classes/entity.hpp"
#include "../../SDK/Classes/player.hpp"
#include "../../Utils/Config.hpp"
#include "../../Utils/LogSystem.hpp" // 2016-12-13 port: render-path diagnostics

HRESULT __stdcall Hooked::Present( LPDIRECT3DDEVICE9 pDevice, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion ) {
	g_Vars.globals.szLastHookCalled = XorStr( "27" );

	static bool s_diagLogged = false; // 2016-12-13 port: render-path diagnostics
	if( !s_diagLogged ) {
		s_diagLogged = true;
		g_Log.Log( XorStr( ".pdr" ), XorStr( "diag: Present hook alive, menuOpen=%d, menuKey=%d" ),
			( int )g_Vars.globals.menuOpen, g_Vars.menu.key.key );
	}

	if( !Render::DirectX::initialized ) {
		if( pDevice->TestCooperativeLevel( ) == D3D_OK ) {
			Render::DirectX::init( pDevice );
			Render::Engine::Initialise( );
			Render::Draw->Init( pDevice );
			Render::Draw->Reset(); // populate Screen.Width/Height from current viewport
		}
		return oPresent( pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion );
	}

	Render::DirectX::begin( );
	{
		bool aa_left_enabled = g_Vars.rage.anti_aim_left_key.enabled; // -@majorkadev
		bool aa_right_enabled = g_Vars.rage.anti_aim_right_key.enabled;
		bool aa_back_enabled = g_Vars.rage.anti_aim_back_key.enabled;

		if( !g_Vars.globals.m_bChatOpen && !g_Vars.globals.m_bConsoleOpen ) {

			for( auto& keybind : g_keybinds ) {
				if( !keybind )
					continue;

				// hold
				if( keybind->cond == KeyBindType::HOLD ) {
					keybind->enabled = g_InputSystem.IsKeyDown( keybind->key );
				}
				// toggle
				else if( keybind->cond == KeyBindType::TOGGLE ) {
					if( g_InputSystem.WasKeyPressed( keybind->key ) )
						keybind->enabled = !keybind->enabled;
				}
				// hold off
				else if( keybind->cond == KeyBindType::HOLD_OFF ) {
					keybind->enabled = !g_InputSystem.IsKeyDown( keybind->key );
				}
				// always on
				else if( keybind->cond == KeyBindType::ALWAYS_ON ) {
					keybind->enabled = true;
				}
			}
		}
		else {
			for( auto& keybind : g_keybinds ) {
				if( !keybind )
					continue;

				if( keybind == &g_Vars.misc.third_person_bind )
					continue;

				if( keybind->cond == KeyBindType::HOLD ) {
					keybind->enabled = false;
				}
			}
		}

		// amazing logic, I know, thanks.
		if( aa_left_enabled && g_Vars.rage.anti_aim_left_key.enabled ) {
			if( g_Vars.rage.anti_aim_right_key.enabled || g_Vars.rage.anti_aim_back_key.enabled )
				g_Vars.rage.anti_aim_left_key.enabled = false;
		}

		if( aa_right_enabled && g_Vars.rage.anti_aim_right_key.enabled ) {
			if( g_Vars.rage.anti_aim_left_key.enabled || g_Vars.rage.anti_aim_back_key.enabled )
				g_Vars.rage.anti_aim_right_key.enabled = false;
		}

		if( aa_back_enabled && g_Vars.rage.anti_aim_back_key.enabled ) {
			if( g_Vars.rage.anti_aim_left_key.enabled || g_Vars.rage.anti_aim_right_key.enabled )
				g_Vars.rage.anti_aim_back_key.enabled = false;
		}

		{
			auto pizda = g_Vars.menu.ascent.ToRegularColor( );
			size_t hash = size_t( ( uintptr_t )g_Vars.globals.menuOpen + ( uintptr_t )g_Vars.menu.size_x + ( uintptr_t )g_Vars.menu.size_y + uintptr_t( pizda.r( ) + pizda.g( ) + pizda.b( ) + pizda.a( ) ) );
			static size_t old_hash = hash;

			// 2016-12-13 port: fall back to INSERT when no menu key is bound; never toggle on
			// an invalid key (0) - WasKeyPressed(0) could phantom-toggle the menu closed instantly
			const int menuKey = g_Vars.menu.key.key ? g_Vars.menu.key.key : 45 /* VK_INSERT */;
			if( menuKey > 0 && g_InputSystem.WasKeyPressed( menuKey ) ) {
				g_Vars.globals.menuOpen = !g_Vars.globals.menuOpen;
			}

			// let's save the global_data on open/close :)
			if( old_hash != hash ) {
				ConfigManager::SaveConfig( XorStr( "global_data" ), true );
				old_hash = hash;
			}

			CMenu::get()->Initialize();
			CMenu::get()->SetMenuOpened(g_Vars.globals.menuOpen);
			CMenu::get()->Draw();

			alpha_mod = -1.f;
			g_Vars.menu.ascent.a = 1.f;
		}

		g_InputSystem.SetScrollMouse( 0.f );
	}
	Render::DirectX::end( );

	return oPresent( pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion );
}