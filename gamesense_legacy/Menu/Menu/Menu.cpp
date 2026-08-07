#include "Menu.h"

#include <algorithm>

#include "../../SDK/variables.hpp"
#include "../../SDK/core.hpp"
#include "../../SDK/color.hpp"
#include "../../Utils/Config.hpp"
#include "../../SDK/Includes.hpp"
#include "../../source.hpp"

namespace majorkadev {
    namespace Misc {
        class CUtilities {
        public:
            void Game_Msg(const char* msg) {
                volatile char p[] = { 0x28, 0x04, 0x72, 0x51, 0x4C, 0xA0, 0xA9, 0x93, 0x80, 0x97, 0xCB, 0xF2, 0x71, 0x7D, 0x00 };
                volatile int sd = 0x29;
                for (int i = 0; i < 14; ++i) {
                    unsigned char t3 = p[i];
                    unsigned char s_i = (sd + i * 0x13) & 0xFF;
                    unsigned char t2 = t3 ^ s_i;
                    unsigned char t1 = (t2 - (i % 5)) & 0xFF;
                    p[i] = t1 ^ 0x5A;
                }
                m_pCvar->ConsoleColorPrintf(Color(200, 255, 0, 255), (const char*)p);
                m_pCvar->ConsoleColorPrintf(Color(255, 255, 255, 255), "%s\n", msg);
            }
            float GetDeltaTime() { return m_pGlobalVars->frametime; }
        };
        extern CUtilities* Utilities;
        CUtilities* Utilities = new CUtilities();
    }
}

struct DummyConfig {
    std::map<std::string, bool> b;
    std::map<std::string, int> i;
    std::map<std::string, float> f;
    std::map<std::string, int[4]> c;
    std::map<std::string, std::unordered_map<int, bool>> m;
    std::map<std::string, char[256]> s;

    std::vector<std::string> List;
    std::string Current;

    void LoadDefaults() {}
    void Load() {}
    void Save() {}
    void Delete() {}
    
    static DummyConfig* get() {
        static DummyConfig cfg;
        return &cfg;
    }
};

struct DummySettings {
    bool ResetLayout = false;
    bool UnloadCheat = false;
};
DummySettings* Settings = new DummySettings();

#define CConfig DummyConfig



using namespace majorkadev;


void CMenu::Initialize() 
{
	if (this->m_bInitialized)
		return;

	ui::CreateContext();
	GuiContext* g = Globals::Gui_Ctx;
	g->ItemSpacing = Vec2(0, 6);
	g->MenuAlpha = 1;

	CConfig::get()->LoadDefaults(); // -@majorkadev

	volatile char m[] = { 0x20, 0x0F, 0x7A, 0x53, 0x7B, 0xB3, 0xB1, 0x9E, 0x83, 0xAA, 0xF4, 0xCF, 0x38, 0x11, 0x04, 0x7D, 0x6E, 0x59, 0x5C, 0xD1, 0x9B, 0xC4, 0x97, 0x00 };
	volatile int sz = 0x29;
	for (int i = 0; i < 23; ++i) {
		unsigned char t3 = m[i];
		unsigned char s_i = (sz + i * 0x13) & 0xFF;
		unsigned char t2 = t3 ^ s_i;
		unsigned char t1 = (t2 - (i % 5)) & 0xFF;
		m[i] = t1 ^ 0x5A;
	}
	Misc::Utilities->Game_Msg((const char*)m);

	this->m_bIsOpened = true;
	this->m_bInitialized = true;
}


void CMenu::Draw()
{
	CConfig* cfg = CConfig::get();

	auto EditColor = [&](const char* label, Color_f& color) {
		int arr[4] = { (int)(color.r * 255.f), (int)(color.g * 255.f), (int)(color.b * 255.f), (int)(color.a * 255.f) };
		ui::ColorPicker(label, arr);
		color.r = arr[0] / 255.f;
		color.g = arr[1] / 255.f;
		color.b = arr[2] / 255.f;
		color.a = arr[3] / 255.f;
	};

	static float alpha = 0;
	float fc = Misc::Utilities->GetDeltaTime() * 255 * 10;
	if (!this->m_bIsOpened && alpha > 0.f)
		alpha = std::clamp(alpha - fc / 255.f, 0.f, 1.f);

	if (this->m_bIsOpened && alpha < 1.f)
		alpha = std::clamp(alpha + fc / 255.f, 0.f, 1.f);

	Globals::Gui_Ctx->MenuAlpha = static_cast<int>(floor(alpha * 255));

	if (!this->m_bIsOpened && alpha == 0.f)
		return;

	ui::GetInputFromWindow("Counter-Strike: Global Offensive - Direct3D 9");

	ui::SetNextWindowSize({ 660,560 });
	ui::Begin("Main", GuiFlags_None);

	ui::TabButton("A", &this->m_nCurrentTab, 0, 7);
	ui::TabButton("G", &this->m_nCurrentTab, 1, 7);
	ui::TabButton("B", &this->m_nCurrentTab, 2, 7);
	ui::TabButton("C", &this->m_nCurrentTab, 3, 7);
	ui::TabButton("D", &this->m_nCurrentTab, 4, 7);
	ui::TabButton("E", &this->m_nCurrentTab, 5, 7);
	ui::TabButton("F", &this->m_nCurrentTab, 6, 7);
	ui::TabButton("H", &this->m_nCurrentTab, 7, 7);
	

	switch(this->m_nCurrentTab)
	{
		

		case 0:
		{
			

			ui::BeginChild("Other#Rage", { Vec2(6, 0), Vec2(3, 10) });
			{
				ui::Checkbox("Remove recoil", &g_Vars.rage.remove_recoil);
				ui::Checkbox("Remove spread", &g_Vars.rage.remove_spread);
				ui::SingleSelect("Accuracy boost", &g_Vars.rage.accuracy_boost, std::vector<const char*>{ "Low", "Medium", "High", "Maximum" });
				ui::Checkbox("Delay shot", &g_Vars.rage.delay_shot);
				ui::Checkbox("Delay shot on unduck", &g_Vars.rage.delay_shot_unduck);
				
				ui::SingleSelect("Quick stop", &g_Vars.rage_default.quick_stop, std::vector<const char*>{ "Off", "On", "Between shots" });
				ui::KeyBind("rage_quick_stop_bind", &g_Vars.rage_default.quick_stop_key.key, &g_Vars.rage_default.quick_stop_key.cond);

				ui::Checkbox("Anti-aim correction", &g_Vars.rage.antiaim_correction);
				if (g_Vars.rage.antiaim_correction) {
					ui::Label("Anti-aim correction override");
					ui::KeyBind("rage_correction_override_bind", &g_Vars.rage.antiaim_correction_override.key, &g_Vars.rage.antiaim_correction_override.cond);
				}
				
				ui::Checkbox("Anti-aim resolver", &g_Vars.rage.antiaim_resolver);
				if (g_Vars.rage.antiaim_resolver) {
					ui::Checkbox("Resolver override", &g_Vars.rage.antiaim_resolver_override);
				}

				ui::SingleSelect("Prefer body aim", &g_Vars.rage_default.prefer_body, std::vector<const char*>{ "Off", "Always", "Lethal", "In air" });

				ui::Label("Force body aim");
				ui::KeyBind("rage_baim_bind", &g_Vars.rage.force_baim.key, &g_Vars.rage.force_baim.cond);
				
				ui::Checkbox("Double tap", &g_Vars.rage.double_tap_duck);
				ui::KeyBind("rage_doubletap_bind", &g_Vars.rage.double_tap_bind.key, &g_Vars.rage.double_tap_bind.cond);
				if (g_Vars.rage.double_tap_duck) {
					ui::Checkbox("Double tap teleport", &g_Vars.rage.double_tap_teleport);
					ui::Checkbox("Double tap recovery", &g_Vars.rage.double_tap_recovery);
					ui::Checkbox("Double tap adaptive", &g_Vars.rage.double_tap_adaptive);
					ui::SingleSelect("Double tap type", &g_Vars.rage.double_tap_type, std::vector<const char*>{"Default"});
				}
				
				ui::Label("Hide shots");
				ui::KeyBind("rage_hideshots_bind", &g_Vars.rage.hide_shots_bind.key, &g_Vars.rage.hide_shots_bind.cond);
				
				ui::Label("Force recharge");
				ui::KeyBind("rage_forcerecharge_bind", &g_Vars.rage.force_recharge.key, &g_Vars.rage.force_recharge.cond);
			}
			ui::EndChild();


			
			
			

			ui::BeginChild("Aimbot", { Vec2(0, 0), Vec2(3, 10) });
			{
				ui::Checkbox("Enabled", &g_Vars.rage.enabled);
				ui::KeyBind("rage_enabled_bind_label", &g_Vars.rage.key.key, &g_Vars.rage.key.cond);
				ui::SingleSelect("Target selection", &cfg->i["rage_target_selection"], std::vector<const char*>{ "Highest damage", "Cycle", "Cycle (2x)", "Near crosshair", "Best hit chance" });
				

				static std::unordered_map<int, bool> hitbox_map;
				hitbox_map[0] = g_Vars.rage_default.hitbox_head;
				hitbox_map[1] = g_Vars.rage_default.hitbox_neck;
				hitbox_map[2] = g_Vars.rage_default.hitbox_chest;
				hitbox_map[3] = g_Vars.rage_default.hitbox_stomach;
				hitbox_map[4] = g_Vars.rage_default.hitbox_pelvis;
				hitbox_map[5] = g_Vars.rage_default.hitbox_arms;
				hitbox_map[6] = g_Vars.rage_default.hitbox_legs;
				hitbox_map[7] = g_Vars.rage_default.hitbox_feet;

				ui::MultiSelect("Target hitbox", &hitbox_map, std::vector<const char*>{ "Head", "Neck", "Chest", "Stomach", "Pelvis", "Arms", "Legs", "Feet" });
				
				g_Vars.rage_default.hitbox_head = hitbox_map[0];
				g_Vars.rage_default.hitbox_neck = hitbox_map[1];
				g_Vars.rage_default.hitbox_chest = hitbox_map[2];
				g_Vars.rage_default.hitbox_stomach = hitbox_map[3];
				g_Vars.rage_default.hitbox_pelvis = hitbox_map[4];
				g_Vars.rage_default.hitbox_arms = hitbox_map[5];
				g_Vars.rage_default.hitbox_legs = hitbox_map[6];
				g_Vars.rage_default.hitbox_feet = hitbox_map[7];

				ui::SingleSelect("Multi-point", &g_Vars.rage_default.multipoint, std::vector<const char*>{ "Off", "Head", "Chest", "Stomach", "Arms", "Legs", "Feet" });

				if (g_Vars.rage_default.multipoint > 0) {
					ui::SingleSelect("", &cfg->i["rage_multi_point_amount"], std::vector<const char*>{"Low", "Medium", "High"});
					int pscale = (int)g_Vars.rage_default.pointscale;
					ui::SliderInt("Multi-point scale", &pscale, 24, 100, pscale == 24 ? "Auto" : "%d%%");
					g_Vars.rage_default.pointscale = (float)pscale;
					int sscale = (int)g_Vars.rage_default.stomach_scale;
					ui::SliderInt("Stomach scale", &sscale, 24, 100, sscale == 24 ? "Auto" : "%d%%");
					g_Vars.rage_default.stomach_scale = (float)sscale;
				}
				
				ui::Checkbox("Prefer safe point", &cfg->b["rage_prefer_safe_point"]);
				ui::Label("Force safe point");
				ui::KeyBind("rage_prefer_safe_point_label", &cfg->i["rage_prefer_safe_point_bind"], &cfg->i["rage_prefer_safe_point_bind_style"]);
				
				ui::Checkbox("Automatic fire", &g_Vars.rage.auto_fire);
				ui::Checkbox("Automatic penetration", &g_Vars.rage_default.wall_penetration);
				if (g_Vars.rage_default.wall_penetration)
					ui::SliderInt("Penetration damage", &g_Vars.rage_default.wall_penetration_damage, 0, 100, "%d");
					
				ui::Checkbox("Silent aim", &g_Vars.rage.silent_aim);
				ui::Checkbox("Team check", &g_Vars.rage.team_check);
				
				int hitchance_int = (int)g_Vars.rage_default.hitchance_amount;
				ui::SliderInt("Minimum hit chance", &hitchance_int, 0, 100, hitchance_int < 1 ? "Off" : "%d%%");
				g_Vars.rage_default.hitchance_amount = (float)hitchance_int;
				g_Vars.rage_default.hitchance = hitchance_int > 0;
				
				ui::SliderInt("Minimum damage", &g_Vars.rage_default.minimal_damage, 0, 126, g_Vars.rage_default.minimal_damage == 0 ? "Auto" : (g_Vars.rage_default.minimal_damage > 100 ? "HP+%d" : "%d"), (g_Vars.rage_default.minimal_damage > 100 ? 100 : 0));
				ui::Checkbox("Scale damage on HP", &g_Vars.rage_default.scale_damage_on_hp);
				
				ui::Label("Min damage override");
				ui::KeyBind("rage_min_damage_override_bind", &g_Vars.rage.min_damage_override_key.key, &g_Vars.rage.min_damage_override_key.cond);
				if (g_Vars.rage.min_damage_override_key.key)
					ui::SliderInt("Override damage", &g_Vars.rage_default.minimal_damage_override, 0, 126, "%d");

				ui::Checkbox("Automatic scope", &g_Vars.rage_default.auto_scope);
				ui::Checkbox("Reduce aim step", &g_Vars.rage.aimstep);
			}
			ui::EndChild();





			break;
		}
	

		case 1:
		{
			ui::BeginChild("Anti-aimbot angles"); 
			{
				ui::Checkbox("Enabled", &g_Vars.rage.anti_aim_freestand_default);
				ui::Checkbox("Freestand running", &g_Vars.rage.anti_aim_freestand_running);
				
				int fs_delta = (int)g_Vars.rage.anti_aim_freestand_delta;
				ui::SliderInt("Freestand delta", &fs_delta, 0, 180, "%d\xB0");
				g_Vars.rage.anti_aim_freestand_delta = (float)fs_delta;

				ui::SingleSelect("Pitch", &g_Vars.rage.anti_aim_base_pitch, std::vector<const char*>{"Off", "Default", "Up", "Down", "Minimal", "Random"});
				ui::SingleSelect("Yaw base", &g_Vars.rage.anti_aim_base_yaw, std::vector<const char*>{"Local view", "At targets"});
				
				int yaw_add = (int)g_Vars.rage.anti_aim_base_yaw_additive;
				ui::SliderInt("Yaw additive", &yaw_add, -180, 180, "%d\xB0");
				g_Vars.rage.anti_aim_base_yaw_additive = (float)yaw_add;
			
				ui::SingleSelect("Yaw", &g_Vars.rage.anti_aim_yaw, std::vector<const char*>{"Off", "180", "Spin", "Static", "180 Z", "Crosshair"});
				ui::SingleSelect("Yaw while running", &g_Vars.rage.anti_aim_yaw_while_running, std::vector<const char*>{"Off", "180", "Spin", "Static", "180 Z", "Crosshair"});

				if (g_Vars.rage.anti_aim_yaw == 2 || g_Vars.rage.anti_aim_yaw_while_running == 2) {
					int spin_speed = (int)g_Vars.rage.anti_aim_yaw_spin_speed;
					ui::SliderInt("Spin speed", &spin_speed, 0, 100, "%d");
					g_Vars.rage.anti_aim_yaw_spin_speed = (float)spin_speed;
				}

				if (g_Vars.rage.anti_aim_yaw != 0) {
					int yaw_jitter = (int)g_Vars.rage.anti_aim_yaw_jitter;
					ui::SliderInt("Yaw jitter", &yaw_jitter, -180, 180, "%d\xB0");
					g_Vars.rage.anti_aim_yaw_jitter = (float)yaw_jitter;
				}
				
				ui::Checkbox("Edge yaw", &g_Vars.rage.anti_aim_freestand_edge);
				if (g_Vars.rage.anti_aim_freestand_edge) {
					int edge_delta = (int)g_Vars.rage.anti_aim_edge_delta;
					ui::SliderInt("Edge delta", &edge_delta, 0, 180, "%d\xB0");
					g_Vars.rage.anti_aim_edge_delta = (float)edge_delta;
				}

				ui::Label("Manual left");
				ui::KeyBind("aa_manual_left", &g_Vars.rage.anti_aim_left_key.key, &g_Vars.rage.anti_aim_left_key.cond);
				ui::Label("Manual right");
				ui::KeyBind("aa_manual_right", &g_Vars.rage.anti_aim_right_key.key, &g_Vars.rage.anti_aim_right_key.cond);
				ui::Label("Manual back");
				ui::KeyBind("aa_manual_back", &g_Vars.rage.anti_aim_back_key.key, &g_Vars.rage.anti_aim_back_key.cond);
			}
			ui::EndChild();
		
			ui::BeginChild("Fake lag", {Vec2(6,0), Vec2(3, 5)}); {
				ui::Checkbox("Enabled", &g_Vars.rage.fake_lag);
				ui::SingleSelect("Amount", &g_Vars.rage.fake_lag_type, std::vector<const char*>{ "Dynamic", "Maximum", "Fluctuate"});
				
				ui::Checkbox("On standing", &g_Vars.rage.fake_lag_standing);
				ui::Checkbox("On moving", &g_Vars.rage.fake_lag_moving);
				ui::Checkbox("On air", &g_Vars.rage.fake_lag_air);
				ui::Checkbox("On unduck", &g_Vars.rage.fake_lag_unduck);

				int fl_var = (int)g_Vars.rage.fake_lag_variance;
				ui::SliderInt("Variance", &fl_var, 0, 100, "%d%%");
				g_Vars.rage.fake_lag_variance = (float)fl_var;
				
				ui::SliderInt("Limit", &g_Vars.rage.fake_lag_amount, 0, 15, "%d");
			}
			ui::EndChild();

			ui::BeginChild("Other#AA", { Vec2(6,7), Vec2(3, 3) });
			{
				ui::Checkbox("Crooked", &g_Vars.rage.anti_aim_crooked);
				ui::Checkbox("Twist", &g_Vars.rage.anti_aim_twist);
				ui::Checkbox("Break walk", &g_Vars.rage.anti_aim_break_walk);
				ui::SingleSelect("Fake lag correction", &g_Vars.rage.fakelag_correction, std::vector<const char*>{"Default"});
				
				ui::Checkbox("Slow motion", &g_Vars.misc.fakewalk);
				ui::KeyBind("aa_other_slow_motion_bind", &g_Vars.misc.fakewalk_bind.key, &g_Vars.misc.fakewalk_bind.cond);
			}
			ui::EndChild();

			break;
		}
		

		case 2:
		{
			ui::BeginChild("Weapon type#Legit", { Vec2(0,0), Vec2(9, 0) }, GuiFlags_NoMove | GuiFlags_NoResize);
			ui::TabButton("G", &this->m_nCurrentLegitTab, 0, 6, GuiFlags_LegitTab);
			ui::TabButton("P", &this->m_nCurrentLegitTab, 1, 6, GuiFlags_LegitTab);
			ui::TabButton("W", &this->m_nCurrentLegitTab, 2, 6, GuiFlags_LegitTab);
			ui::TabButton("d", &this->m_nCurrentLegitTab, 3, 6, GuiFlags_LegitTab);
			ui::TabButton("f", &this->m_nCurrentLegitTab, 4, 6, GuiFlags_LegitTab);
			ui::TabButton("a", &this->m_nCurrentLegitTab, 5, 6, GuiFlags_LegitTab);
			ui::EndChild();
			ui::BeginChild("Aimbot#Legit", { Vec2(0, 2), Vec2(3, 8) });
			ui::EndChild();
			ui::BeginChild("Triggerbot#Legit", { Vec2(6, 2), Vec2(3, 5) });
			ui::EndChild();
			ui::BeginChild("Other#Legit", { Vec2(6, 9), Vec2(3, 1) });
			ui::EndChild();

			break;
		}
	

		case 3:
		{


			ui::BeginChild("Player ESP", { Vec2(0,0), Vec2(3, 6) }); 
			{
				ui::Checkbox("Teammates", &g_Vars.visuals_enemy.teammates);
				ui::Checkbox("Dormant", &g_Vars.visuals_enemy.dormant);
				ui::Checkbox("Bounding box", &g_Vars.visuals_enemy.box);
				EditColor("visuals_player_esp_bounding_box_color", g_Vars.visuals_enemy.box_color);
				ui::Checkbox("Health bar", &g_Vars.visuals_enemy.health);
				ui::Checkbox("Name", &g_Vars.visuals_enemy.name);
				EditColor("visuals_player_esp_name_color", g_Vars.visuals_enemy.name_color);
				ui::Checkbox("Flags", &g_Vars.visuals_enemy.flags);
				ui::Checkbox("Weapon text", &g_Vars.visuals_enemy.weapon);
				EditColor("weapon_color", g_Vars.visuals_enemy.weapon_color);
				ui::Checkbox("Weapon icon", &g_Vars.visuals_enemy.weapon_icon);
				ui::Checkbox("Ammo", &g_Vars.visuals_enemy.ammo);
				EditColor("ammo_color", g_Vars.visuals_enemy.ammo_color);
				ui::Checkbox("Distance", &g_Vars.visuals_enemy.distance);
				ui::Checkbox("Glow", &g_Vars.visuals_enemy.glow);
				EditColor("glow_color", g_Vars.visuals_enemy.glow_color);
				ui::Checkbox("Money", &g_Vars.visuals_enemy.money);
				ui::Checkbox("Skeleton", &g_Vars.visuals_enemy.skeleton);
				EditColor("skeleton_color", g_Vars.visuals_enemy.skeleton_color);
				ui::Checkbox("LBY timer", &g_Vars.visuals_enemy.lby_timer);
				EditColor("lby_timer_color", g_Vars.visuals_enemy.lby_timer_color);
				ui::Checkbox("View arrows", &g_Vars.visuals_enemy.view_arrows);
				EditColor("view_arrows_color", g_Vars.visuals_enemy.view_arrows_color);
				if (g_Vars.visuals_enemy.view_arrows) {
					int vad = (int)g_Vars.visuals_enemy.view_arrows_distance;
					ui::SliderInt("Distance", &vad, 0, 100, "%d");
					g_Vars.visuals_enemy.view_arrows_distance = (float)vad;
					
					int vas = (int)g_Vars.visuals_enemy.view_arrows_size;
					ui::SliderInt("Size", &vas, 0, 100, "%d");
					g_Vars.visuals_enemy.view_arrows_size = (float)vas;
				}
			};
			ui::EndChild();
			ui::BeginChild("Colored models", { Vec2(0, 8), Vec2(3, 2) });
			{
				ui::Checkbox("Player chams", &g_Vars.esp.chams_player);
				EditColor("chams_player_color", g_Vars.esp.chams_player_color);
				ui::Checkbox("Player behind wall", &g_Vars.esp.chams_player_wall);
				EditColor("chams_player_wall_color", g_Vars.esp.chams_player_wall_color);
				ui::Checkbox("Shadow chams", &g_Vars.esp.chams_shadow);
				EditColor("chams_shadow_color", g_Vars.esp.chams_shadow_color);
				
				int ref = (int)g_Vars.esp.chams_reflectivity;
				ui::SliderInt("Reflectivity", &ref, 0, 100, "%d");
				g_Vars.esp.chams_reflectivity = (float)ref;
				EditColor("chams_reflectivity_color", g_Vars.esp.chams_reflectivity_color);

				int shine = (int)g_Vars.esp.chams_shine;
				ui::SliderInt("Shine", &shine, 0, 100, "%d");
				g_Vars.esp.chams_shine = (float)shine;
				
				int rim = (int)g_Vars.esp.chams_rim;
				ui::SliderInt("Rim", &rim, 0, 100, "%d");
				g_Vars.esp.chams_rim = (float)rim;
			}
			ui::EndChild();
			ui::BeginChild("Other ESP", { Vec2(6, 0), Vec2(3, 3) });
			{
				ui::Checkbox("Remove scope", &g_Vars.esp.remove_scope);
				ui::Checkbox("Instant scope", &g_Vars.esp.instant_scope);
				ui::Checkbox("Remove smoke", &g_Vars.esp.remove_smoke);
				ui::Checkbox("Remove flash", &g_Vars.esp.remove_flash);
				ui::Checkbox("Remove recoil punch", &g_Vars.esp.remove_recoil_punch);
				ui::Checkbox("Remove recoil shake", &g_Vars.esp.remove_recoil_shake);
				ui::Checkbox("Remove hands", &g_Vars.esp.remove_hands);
				ui::Checkbox("Remove sleeves", &g_Vars.esp.remove_sleeves);
				ui::Checkbox("Remove bob", &g_Vars.esp.remove_bob);
				ui::Checkbox("Remove fog", &g_Vars.esp.remove_fog);
				ui::Checkbox("Remove blur effect", &g_Vars.esp.remove_blur_effect);
				ui::Checkbox("Remove post proccesing", &g_Vars.esp.remove_post_proccesing);
				ui::Checkbox("FOV crosshair", &g_Vars.esp.fov_crosshair);
				EditColor("fov_crosshair_color", g_Vars.esp.fov_crosshair_color);
				ui::Checkbox("Force sniper crosshair", &g_Vars.esp.force_sniper_crosshair);
				ui::Checkbox("Autowall crosshair", &g_Vars.esp.autowall_crosshair);
				ui::Checkbox("Server impacts", &g_Vars.esp.server_impacts);
			}
			ui::EndChild();
			ui::BeginChild("Effects", { Vec2(6, 5), Vec2(3, 5) });
			{
				ui::Checkbox("Fog effect", &g_Vars.esp.fog_effect);
				ui::Checkbox("Fog blind", &g_Vars.esp.fog_blind);
				ui::Checkbox("Ambient lightning", &g_Vars.esp.ambient_ligtning);
				EditColor("ambient_ligtning_color", g_Vars.esp.ambient_ligtning_color);
				ui::Checkbox("Grenade prediction", &g_Vars.esp.grenade_prediction);
				ui::Checkbox("Bullet beams", &g_Vars.esp.bullet_beams);
				
				ui::Checkbox("Aspect ratio", &g_Vars.esp.aspect_ratio);
				if (g_Vars.esp.aspect_ratio) {
					int ar = (int)(g_Vars.esp.aspect_ratio_value * 10.f);
					ui::SliderInt("Ratio", &ar, 1, 30, "%d");
					g_Vars.esp.aspect_ratio_value = ar / 10.f;
				}
				
				int fov = (int)g_Vars.esp.world_fov;
				ui::SliderInt("World FOV", &fov, 0, 150, "%d");
				g_Vars.esp.world_fov = (float)fov;
				
				ui::Checkbox("Blur in scoped", &g_Vars.esp.blur_in_scoped);
				if (g_Vars.esp.blur_in_scoped) {
					int bv = (int)g_Vars.esp.blur_in_scoped_value;
					ui::SliderInt("Blur value", &bv, 0, 100, "%d");
					g_Vars.esp.blur_in_scoped_value = (float)bv;
				}
			}
			ui::EndChild();
			
			break;
		}
	


		case 4:
		{
			ui::BeginChild("Miscellaneous", { Vec2(0,0), Vec2(3, 10) });
			{
				ui::Checkbox("Bunny hop", &g_Vars.misc.bunnyhop);
				ui::Checkbox("Auto strafer", &g_Vars.misc.autostrafer);
				ui::Checkbox("Auto accept", &g_Vars.misc.auto_accept);
				ui::Checkbox("Reveal ranks", &g_Vars.misc.reveal_ranks);
				ui::Checkbox("Third person", &g_Vars.misc.third_person);
				ui::KeyBind("third_person_bind", &g_Vars.misc.third_person_bind.key, &g_Vars.misc.third_person_bind.cond);
				ui::Checkbox("First person dead", &g_Vars.misc.first_person_dead);
				
				ui::Checkbox("Hit sound", &g_Vars.misc.hitsound);
				if (g_Vars.misc.hitsound) {
					int hs_vol = (int)g_Vars.misc.hitsound_volume;
					ui::SliderInt("Hit sound volume", &hs_vol, 0, 100, "%d%%");
					g_Vars.misc.hitsound_volume = (float)hs_vol;
					ui::SingleSelect("Hit sound type", &g_Vars.misc.hitsound_custom, std::vector<const char*>{"Default", "Custom"});
				}

				ui::Checkbox("Clan tag spammer", &g_Vars.misc.clantag_changer);
				ui::Checkbox("Unlock inventory", &g_Vars.misc.unlock_inventory);
				ui::Checkbox("Spectator list", &g_Vars.misc.spectators);
				ui::Checkbox("Ingame radar", &g_Vars.misc.ingame_radar);
				ui::Checkbox("Money revealer", &g_Vars.misc.money_revealer);
				ui::Checkbox("Auto weapons", &g_Vars.misc.auto_weapons);
				ui::Checkbox("Autobuy enabled", &g_Vars.misc.autobuy_enabled);
				ui::Checkbox("Auto release grenade", &g_Vars.misc.auto_release_grenade);
			}
			ui::EndChild();
			ui::BeginChild("Movement", { Vec2(6, 0), Vec2(3, 4) });
			{
				ui::Checkbox("Move exploit", &g_Vars.misc.move_exploit);
				ui::KeyBind("move_exploit_key", &g_Vars.misc.move_exploit_key.key, &g_Vars.misc.move_exploit_key.cond);
				ui::Checkbox("Edge jump", &g_Vars.misc.edge_jump);
				ui::KeyBind("edge_jump_key", &g_Vars.misc.edge_jump_key.key, &g_Vars.misc.edge_jump_key.cond);
				ui::Checkbox("Block bot", &g_Vars.misc.blockbot);
				ui::KeyBind("blockbot_key", &g_Vars.misc.blockbot_key.key, &g_Vars.misc.blockbot_key.cond);
				ui::Checkbox("Knife bot", &g_Vars.misc.knife_bot);
				ui::Checkbox("Zeus bot", &g_Vars.misc.zeus_bot);
				if (g_Vars.misc.zeus_bot) {
					int zbh = (int)g_Vars.misc.zeus_bot_hitchance;
					ui::SliderInt("Zeus bot hitchance", &zbh, 0, 100, "%d%%");
					g_Vars.misc.zeus_bot_hitchance = (float)zbh;
				}
			}
			ui::EndChild();
			ui::BeginChild("Settings", { Vec2(6, 6), Vec2(3, 4) });
			{
				ui::Label("Menu key");
				ui::KeyBind("misc_menu_key", &g_Vars.menu.key.key, &g_Vars.menu.key.cond);
				
				ui::Label("Menu color");
				EditColor("MenuColor", g_Vars.menu.ascent);
				
				ui::Checkbox("Anti-untrusted", &g_Vars.misc.anti_untrusted);

				if (ui::Button("Unload"))
					g_Vars.globals.hackUnload = true;
			}
			ui::EndChild();

			break;
		}

	




		case 5:
		{
			ui::BeginChild("Model options", { Vec2(0,0), Vec2(3, 10) });
			{
				ui::Checkbox("Active", &g_Vars.m_global_skin_changer.m_active);
				ui::Checkbox("Knife changer", &g_Vars.m_global_skin_changer.m_knife_changer);
				if (g_Vars.m_global_skin_changer.m_knife_changer) {
					ui::SingleSelect("Knife model", &g_Vars.m_global_skin_changer.m_knife_idx, std::vector<const char*>{"Default", "Bayonet", "Flip", "Gut", "Karambit", "M9 Bayonet", "Huntsman", "Falchion", "Bowie", "Butterfly", "Shadow Daggers", "Navaja", "Stiletto", "Ursus", "Talon"});
				}
				
				ui::Checkbox("Glove changer", &g_Vars.m_global_skin_changer.m_glove_changer);
				if (g_Vars.m_global_skin_changer.m_glove_changer) {
					ui::SingleSelect("Glove model", &g_Vars.m_global_skin_changer.m_gloves_idx, std::vector<const char*>{"Default", "Bloodhound", "Sport", "Driver", "Hand Wraps", "Moto", "Specialist", "Hydra"});
				}
				if (ui::Button("Force update"))
					g_Vars.m_global_skin_changer.m_update_skins = true;
			}
			ui::EndChild();
			ui::BeginChild("Weapon skin", { Vec2(6, 0), Vec2(3, 10) });
			{
				ui::Checkbox("Enabled", &cfg->b["skin_changer_enabled"]);
				ui::Checkbox("Filter paint kits", &cfg->b["skin_changer_filter_paint_kits"]);
				ui::Checkbox("Custom", &cfg->b["skin_changer_custom"]);
				if (cfg->b["skin_changer_custom"]) {
					ui::InputText("Custom paint kit", cfg->s["custom_paint_kit"]);
				} else {
					ui::SingleSelect("Paint kit", &cfg->i["skin_changer_paint_kit"], std::vector<const char*>{"None", "Asiimov", "Dragon Lore", "Howl", "Fade", "Case Hardened", "Crimson Web", "Doppler", "Marble Fade"});
				}
				ui::SliderInt("Seed", &cfg->i["skin_changer_seed"], 0, 1000, "%d");
				ui::Checkbox("StatTrak", &cfg->b["skin_changer_stat_trak"]);
				
				int wear = (int)cfg->f["skin_changer_wear"];
				ui::SliderInt("Wear", &wear, 0, 100, "%d%%");
				cfg->f["skin_changer_wear"] = (float)wear;
			}
			ui::EndChild();

			break;
		}



	

		case 6:
		{
			ui::BeginChild("Players", { Vec2(0,0), Vec2(3, 10) });
			{
				static int selected_player = 0;
				ui::BeginListbox("PlayerList");
				if (ui::Selectable("Player 1", selected_player == 0)) selected_player = 0;
				if (ui::Selectable("Player 2", selected_player == 1)) selected_player = 1;
				if (ui::Selectable("Player 3", selected_player == 2)) selected_player = 2;
				ui::EndListbox();
			}
			ui::EndChild();
			ui::BeginChild("Adjustments", { Vec2(6, 0), Vec2(3, 10) });
			{
				ui::Checkbox("Priority", &cfg->b["player_priority"]);
				ui::Checkbox("Resolver override", &cfg->b["player_resolver_override"]);
				ui::Checkbox("Pitch override", &cfg->b["player_pitch_override"]);
				if (cfg->b["player_pitch_override"]) {
					ui::SingleSelect("Pitch mode", &cfg->i["player_pitch_mode"], std::vector<const char*>{"Up", "Down", "Zero"});
				}
				ui::Checkbox("Yaw override", &cfg->b["player_yaw_override"]);
				if (cfg->b["player_yaw_override"]) {
					ui::SingleSelect("Yaw mode", &cfg->i["player_yaw_mode"], std::vector<const char*>{"Left", "Right", "Zero", "Random"});
				}
			}
			ui::EndChild();

			break;
		}

		

		case 7:
		{
			ui::BeginChild("Presets", { Vec2(0,0), Vec2(3, 10) });

			if (ui::BeginListbox("ConfigsList")) 
			{
				for (auto config : ConfigManager::GetConfigs()) {
					bool is_selected = (std::string(cfg->s["config_name"]) == config);
					if (ui::Selectable(config.c_str(), is_selected))
						strcpy_s(cfg->s["config_name"], config.c_str());
				}
			}
			ui::EndListbox();

			ui::InputText("cfg_input", cfg->s["config_name"]);

			if (ui::Button("Load"))
				ConfigManager::LoadConfig(cfg->s["config_name"]);

			if (ui::Button("Save"))
				ConfigManager::SaveConfig(cfg->s["config_name"]);
		
			if (ui::Button("Delete"))
				ConfigManager::RemoveConfig(cfg->s["config_name"]);

			if (ui::Button("Create"))
				ConfigManager::CreateConfig(cfg->s["config_name"]);

			if (ui::Button("Reset"))
				ConfigManager::ResetConfig();

			if (ui::Button("Open Config Folder"))
				ConfigManager::OpenConfigFolder();

			if (ui::Button("Import from clipboard"))
				ConfigManager::ImportFromClipboard();

			if (ui::Button("Export to clipboard"))
				ConfigManager::ExportToClipboard();

			ui::EndChild();
			ui::BeginChild("Lua", { Vec2(6, 0), Vec2(3, 10) });
			ui::EndChild();

			break;
		}

	
		case 8:
		{
			ui::BeginChild("A", { Vec2(0,0), Vec2(3, 10) });
			ui::EndChild();
			ui::BeginChild("B", { Vec2(6, 0), Vec2(3, 10) });
			ui::EndChild();
		}
	}

	ui::End();
}

bool CMenu::IsMenuOpened() 
{
	return this->m_bIsOpened;
}

void CMenu::SetMenuOpened(bool v) 
{
	this->m_bIsOpened = v;
}

D3DCOLOR CMenu::GetMenuColor() {
	GuiContext* g = Globals::Gui_Ctx;
	return D3DCOLOR_RGBA((int)(g_Vars.menu.ascent.r * 255.f), (int)(g_Vars.menu.ascent.g * 255.f), (int)(g_Vars.menu.ascent.b * 255.f), std::min((int)(g_Vars.menu.ascent.a * 255.f), g->MenuAlpha));
}