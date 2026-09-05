#include "Displacement.hpp"
#include "Classes/PropManager.hpp"
#include "../Utils/extern/FnvHash.hpp"

DllInitializeData Engine::Displacement{ };

namespace Engine
{
	__forceinline uintptr_t CallableFromRelative( DWORD nAddress ) {
		return nAddress + *( DWORD* )( nAddress + 1 ) + 5;
	}

	void Create( ) {
		auto image_vstdlib = GetModuleHandleA( XorStr( "vstdlib.dll" ) );
		auto image_client = ( std::uintptr_t )GetModuleHandleA( XorStr( "client.dll" ) );
		auto image_engine = ( std::uintptr_t )GetModuleHandleA( XorStr( "engine.dll" ) );
		auto image_server = ( std::uintptr_t )GetModuleHandleA( XorStr( "server.dll" ) );
		auto image_shaderapidx9 = ( std::uintptr_t )GetModuleHandleA( XorStr( "shaderapidx9.dll" ) );

		// TODO: datamap
		Displacement.C_BaseEntity.m_MoveType = g_PropManager.GetOffset( XorStr( "DT_BaseEntity" ), XorStr( "m_nRenderMode" ) ) + 1;
		Displacement.C_BaseEntity.m_rgflCoordinateFrame = g_PropManager.GetOffset( XorStr( "DT_BaseEntity" ), XorStr( "m_CollisionGroup" ) ) - 0x30;

		Displacement.DT_BaseEntity.m_bSpotted = g_PropManager.GetOffset( XorStr( "DT_BaseEntity" ), XorStr( "m_bSpotted" ) );
		Displacement.DT_BaseEntity.m_iTeamNum = g_PropManager.GetOffset( XorStr( "DT_BaseEntity" ), XorStr( "m_iTeamNum" ) );
		Displacement.DT_BaseEntity.m_vecOrigin = g_PropManager.GetOffset( XorStr( "DT_BaseEntity" ), XorStr( "m_vecOrigin" ) );
		Displacement.DT_BaseEntity.m_flSimulationTime = g_PropManager.GetOffset( XorStr( "DT_BaseEntity" ), XorStr( "m_flSimulationTime" ) );
		Displacement.DT_BaseEntity.m_fEffects = g_PropManager.GetOffset( XorStr( "DT_BaseEntity" ), XorStr( "m_fEffects" ) );
		Displacement.DT_BaseEntity.m_iEFlags = Displacement.DT_BaseEntity.m_fEffects - 0x8;
		Displacement.DT_BaseEntity.m_hOwnerEntity = g_PropManager.GetOffset( XorStr( "DT_BaseEntity" ), XorStr( "m_hOwnerEntity" ) );
		Displacement.DT_BaseEntity.moveparent = g_PropManager.GetOffset( XorStr( "DT_BaseEntity" ), XorStr( "moveparent" ) );
		Displacement.DT_BaseEntity.m_nModelIndex = g_PropManager.GetOffset( XorStr( "DT_BaseEntity" ), XorStr( "m_nModelIndex" ) );
		Displacement.DT_BaseEntity.m_Collision = g_PropManager.GetOffset( XorStr( "DT_BaseEntity" ), XorStr( "m_Collision" ) );
		Displacement.DT_BaseEntity.m_CollisionGroup = g_PropManager.GetOffset( XorStr( "DT_BaseEntity" ), XorStr( "m_CollisionGroup" ) );
		Displacement.DT_BaseEntity.m_flAnimTime = g_PropManager.GetOffset( XorStr( "DT_BaseEntity" ), XorStr( "m_flAnimTime" ) );

		Displacement.DT_BaseWeaponWorldModel.m_hCombatWeaponParent = g_PropManager.GetOffset( XorStr( "DT_BaseWeaponWorldModel" ), XorStr( "m_hCombatWeaponParent" ) );

		Displacement.DT_BaseEntity.m_nForceBone = g_PropManager.GetOffset( XorStr( "DT_BaseAnimating" ), XorStr( "m_nForceBone" ) );
		Displacement.C_BaseAnimating.InvalidateBoneCache = Memory::Scan( image_client, XorStr( "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81" ) );
		auto m_hLightingOrigin = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_hLightingOrigin" ) );
		Displacement.C_BaseAnimating.m_BoneAccessor = Displacement.DT_BaseEntity.m_nForceBone + 0x1C; // todo
		Displacement.C_BaseAnimating.m_iMostRecentModelBoneCounter = *( int* )( Displacement.C_BaseAnimating.InvalidateBoneCache + 0x1B );
		Displacement.C_BaseAnimating.m_iPrevBoneMask = Displacement.DT_BaseEntity.m_nForceBone + 0x10;
		Displacement.C_BaseAnimating.m_iAccumulatedBoneMask = Displacement.DT_BaseEntity.m_nForceBone + 0x14;
		Displacement.C_BaseAnimating.m_bIsJiggleBonesEnabled = m_hLightingOrigin - 0x18;
		Displacement.C_BaseAnimating.m_iOcclusionFramecount = 0xA30;
		Displacement.C_BaseAnimating.m_iOcclusionFlags = 0xA28;
		Displacement.C_BaseAnimating.m_flLastBoneSetupTime = *( int* )( Displacement.C_BaseAnimating.InvalidateBoneCache + 0x11 );
		// 2016-12-13: m_CachedBoneData = CUtlVector base right at the pushed operand (0x28FC), no +4
		Displacement.C_BaseAnimating.m_CachedBoneData = *( int* )( Memory::Scan( image_client, XorStr( "FF B7 ?? ?? ?? ?? 52" ) ) + 2 );
		Displacement.C_BaseAnimating.m_AnimOverlay = *( int* )( Memory::Scan( image_client, XorStr( "8B 89 ?? ?? ?? ?? 8D 0C D1" ) ) + 2 );

		auto BoneSnapshotsCall = Memory::Scan( image_client, XorStr( "8D 8F ?? ?? ?? ?? 6A 01 C7 87" ) );
		Displacement.C_BaseAnimating.m_pFirstBoneSnapshot = *( int* )( BoneSnapshotsCall + 0x2 );
		Displacement.C_BaseAnimating.m_pSecondBoneSnapshot = *( int* )( BoneSnapshotsCall + 0x1B );

		auto CacheBoneDataPos = Memory::Scan( image_client, XorStr( "8D 87 ? ? ? ? 50 E8 ? ? ? ? 8B 44 24 1C" ) ); // 2016-12-13 port: C_BaseAnimating::SetupBones cached positions memcpy
		auto CacheBoneDataRot = Memory::Scan( image_client, XorStr( "8D 87 ? ? ? ? 50 E8 ? ? ? ? 83 C4 0C" ) ); // 2016-12-13 port: cached rotations memcpy
		Displacement.C_BaseAnimating.m_nCachedBonesPosition = CacheBoneDataPos ? *( int* )( CacheBoneDataPos + 0x2 ) : 0;
		Displacement.C_BaseAnimating.m_nCachedBonesRotation = CacheBoneDataRot ? *( int* )( CacheBoneDataRot + 0x2 ) : 0;
		// 2016-12-13: m_pStudioHdr read straight from the operand (0x293C), no +4 like the 2021 build
		auto StudioHdrRead = Memory::Scan( image_client, XorStr( "8B 86 ? ? ? ? 89 44 24 0C 85 C0 74 05" ) );
		Displacement.C_BaseAnimating.m_pStudioHdr = StudioHdrRead ? *( int* )( StudioHdrRead + 0x2 ) : 0;
		Displacement.C_BaseAnimating.m_bShouldDraw = 0; // 2016-12-13: pattern unavailable, guarded in C_BaseAnimating::ShouldDraw
		Displacement.C_BaseAnimating.m_pBoneMerge = *( int* )( Memory::Scan( image_client, XorStr( "89 86 ?? ?? ?? ?? E8 ?? ?? ?? ?? FF 75 08" ) ) + 2 );
		Displacement.C_BaseAnimating.m_pIk = 0; // 2016-12-13: no equivalent layout, IK emulation guarded

		Displacement.DT_BaseAnimating.m_bClientSideAnimation = g_PropManager.GetOffset( XorStr( "DT_BaseAnimating" ), XorStr( "m_bClientSideAnimation" ) );
		Displacement.DT_BaseAnimating.m_bClientSideRagdoll = g_PropManager.GetOffset( XorStr( "DT_BaseAnimating" ), XorStr( "m_bClientSideRagdoll" ) );
		Displacement.DT_BaseAnimating.m_flPoseParameter = g_PropManager.GetOffset( XorStr( "DT_BaseAnimating" ), XorStr( "m_flPoseParameter" ) );
		Displacement.DT_BaseAnimating.m_nHitboxSet = g_PropManager.GetOffset( XorStr( "DT_BaseAnimating" ), XorStr( "m_nHitboxSet" ) );
		Displacement.DT_BaseAnimating.m_flCycle = g_PropManager.GetOffset( XorStr( "DT_BaseAnimating" ), XorStr( "m_flCycle" ) );
		Displacement.DT_BaseAnimating.m_nSequence = g_PropManager.GetOffset( XorStr( "DT_BaseAnimating" ), XorStr( "m_nSequence" ) );
		Displacement.DT_BaseAnimating.m_flEncodedController = g_PropManager.GetOffset( XorStr( "DT_BaseAnimating" ), XorStr( "m_flEncodedController" ) );

		Displacement.DT_BaseCombatCharacter.m_hActiveWeapon = g_PropManager.GetOffset( XorStr( "DT_BaseCombatCharacter" ), XorStr( "m_hActiveWeapon" ) );
		Displacement.DT_BaseCombatCharacter.m_flNextAttack = g_PropManager.GetOffset( XorStr( "DT_BaseCombatCharacter" ), XorStr( "m_flNextAttack" ) );
		Displacement.DT_BaseCombatCharacter.m_hMyWeapons = g_PropManager.GetOffset( XorStr( "DT_BaseCombatCharacter" ), XorStr( "m_hMyWeapons" ) ) / 2;
		Displacement.DT_BaseCombatCharacter.m_hMyWearables = g_PropManager.GetOffset( XorStr( "DT_BaseCombatCharacter" ), XorStr( "m_hMyWearables" ) );

		Displacement.C_BasePlayer.m_pCurrentCommand = 0; // 2016-12-13: unresolved on this build, guarded in C_CSPlayer::SetCurrentCommand
		auto relative_call = Memory::Scan( image_client, XorStr( "E8 ? ? ? ? 83 7D D8 00 7C 0F" ) );
		if( relative_call ) {
			auto offset = *( uintptr_t* )( relative_call + 0x1 );
			Displacement.C_BasePlayer.UpdateVisibilityAllEntities = ( DWORD32 )( relative_call + 5 + offset );
		}
		else {
			Displacement.C_BasePlayer.UpdateVisibilityAllEntities = 0;
		}

		Displacement.DT_BasePlayer.m_aimPunchAngle = g_PropManager.GetOffset( XorStr( "DT_BasePlayer" ), XorStr( "m_aimPunchAngle" ) );
		Displacement.DT_BasePlayer.m_aimPunchAngleVel = g_PropManager.GetOffset( XorStr( "DT_BasePlayer" ), XorStr( "m_aimPunchAngleVel" ) );
		Displacement.DT_BasePlayer.m_viewPunchAngle = g_PropManager.GetOffset( XorStr( "DT_BasePlayer" ), XorStr( "m_viewPunchAngle" ) );
		Displacement.DT_BasePlayer.m_vecViewOffset = g_PropManager.GetOffset( XorStr( "DT_BasePlayer" ), XorStr( "m_vecViewOffset[0]" ) );
		Displacement.DT_BasePlayer.m_vecVelocity = g_PropManager.GetOffset( XorStr( "DT_BasePlayer" ), XorStr( "m_vecVelocity[0]" ) );
		Displacement.DT_BasePlayer.m_vecBaseVelocity = g_PropManager.GetOffset( XorStr( "DT_BasePlayer" ), XorStr( "m_vecBaseVelocity" ) );
		Displacement.DT_BasePlayer.m_vecLadderNormal = g_PropManager.GetOffset( XorStr( "DT_BasePlayer" ), XorStr( "m_vecLadderNormal" ) );
		Displacement.DT_BasePlayer.m_flFallVelocity = g_PropManager.GetOffset( XorStr( "DT_BasePlayer" ), XorStr( "m_flFallVelocity" ) );
		Displacement.DT_BasePlayer.m_flDuckAmount = g_PropManager.GetOffset( XorStr( "DT_BasePlayer" ), XorStr( "m_flDuckAmount" ) );
		Displacement.DT_BasePlayer.m_flDuckSpeed = g_PropManager.GetOffset( XorStr( "DT_BasePlayer" ), XorStr( "m_flDuckSpeed" ) );
		Displacement.DT_BasePlayer.m_lifeState = g_PropManager.GetOffset( XorStr( "DT_BasePlayer" ), XorStr( "m_lifeState" ) );
		Displacement.DT_BasePlayer.m_nTickBase = g_PropManager.GetOffset( XorStr( "DT_BasePlayer" ), XorStr( "m_nTickBase" ) );
		Displacement.DT_BasePlayer.m_iHealth = g_PropManager.GetOffset( XorStr( "DT_BasePlayer" ), XorStr( "m_iHealth" ) );
		Displacement.DT_BasePlayer.m_iDefaultFOV = g_PropManager.GetOffset( XorStr( "DT_BasePlayer" ), XorStr( "m_iDefaultFOV" ) );
		Displacement.DT_BasePlayer.m_fFlags = g_PropManager.GetOffset( XorStr( "DT_BasePlayer" ), XorStr( "m_fFlags" ) );
		Displacement.DT_BasePlayer.m_iObserverMode = g_PropManager.GetOffset( XorStr( "DT_BasePlayer" ), XorStr( "m_iObserverMode" ) );
		Displacement.DT_BasePlayer.pl = g_PropManager.GetOffset( XorStr( "DT_BasePlayer" ), XorStr( "pl" ) );
		Displacement.DT_BasePlayer.m_hObserverTarget = g_PropManager.GetOffset( XorStr( "DT_BasePlayer" ), XorStr( "m_hObserverTarget" ) );
		Displacement.DT_BasePlayer.m_hViewModel = g_PropManager.GetOffset( XorStr( "DT_BasePlayer" ), XorStr( "m_hViewModel[0]" ) );
		Displacement.DT_BasePlayer.m_ubEFNoInterpParity = g_PropManager.GetOffset( XorStr( "DT_BasePlayer" ), XorStr( "m_ubEFNoInterpParity" ) );
		// 2016-12-13: m_ubOldEFNoInterpParity pattern unavailable on this build (value unused elsewhere), keep 0
		Displacement.DT_BasePlayer.m_ubOldEFNoInterpParity = 0;

		Displacement.DT_CSGameRulesProxy.m_bIsValveDS = g_PropManager.GetOffset( XorStr( "DT_CSGameRulesProxy" ), XorStr( "m_bIsValveDS" ) );
		Displacement.DT_CSGameRulesProxy.m_bFreezePeriod = g_PropManager.GetOffset( XorStr( "DT_CSGameRulesProxy" ), XorStr( "m_bFreezePeriod" ) );

		Displacement.DT_CSPlayerResource.m_bombsiteCenterA = g_PropManager.GetOffset( XorStr( "DT_CSPlayerResource" ), XorStr( "m_bombsiteCenterA" ) );
		Displacement.DT_CSPlayerResource.m_bombsiteCenterB = g_PropManager.GetOffset( XorStr( "DT_CSPlayerResource" ), XorStr( "m_bombsiteCenterB" ) );

		Displacement.C_CSPlayer.m_PlayerAnimState = *( int* )( Memory::Scan( image_client, XorStr( "8B 8E ?? ?? ?? ?? 85 C9 74 3E" ) ) + 2 );
		Displacement.C_CSPlayer.m_flSpawnTime = *( int* )( Memory::Scan( image_client, XorStr( "89 86 ?? ?? ?? ?? E8 ?? ?? ?? ?? 80 BE ?? ?? ?? ?? ??" ) ) + 2 );

		Displacement.DT_CSPlayer.m_angEyeAngles = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_angEyeAngles[0]" ) );
		Displacement.DT_CSPlayer.m_nSurvivalTeam = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_nSurvivalTeam" ) );
		Displacement.DT_CSPlayer.m_bHasHelmet = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_bHasHelmet" ) );
		Displacement.DT_CSPlayer.m_bHasHeavyArmor = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_bHasHeavyArmor" ) );
		Displacement.DT_CSPlayer.m_ArmorValue = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_ArmorValue" ) );
		Displacement.DT_CSPlayer.m_bScoped = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_bIsScoped" ) );
		Displacement.DT_CSPlayer.m_bIsWalking = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_bIsWalking" ) );
		Displacement.DT_CSPlayer.m_iAccount = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_iAccount" ) );
		Displacement.DT_CSPlayer.m_iShotsFired = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_iShotsFired" ) );
		Displacement.DT_CSPlayer.m_flFlashDuration = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_flFlashDuration" ) );
		Displacement.DT_CSPlayer.m_flLowerBodyYawTarget = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_flLowerBodyYawTarget" ) );
		Displacement.DT_CSPlayer.m_flVelocityModifier = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_flVelocityModifier" ) );
		Displacement.DT_CSPlayer.m_bGunGameImmunity = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_bGunGameImmunity" ) );
		Displacement.DT_CSPlayer.m_flHealthShotBoostExpirationTime = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_flHealthShotBoostExpirationTime" ) );
		Displacement.DT_CSPlayer.m_iMatchStats_Kills = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_iMatchStats_Kills" ) );
		Displacement.DT_CSPlayer.m_iMatchStats_Deaths = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_iMatchStats_Deaths" ) );
		Displacement.DT_CSPlayer.m_iMatchStats_HeadShotKills = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_iMatchStats_HeadShotKills" ) );
		Displacement.DT_CSPlayer.m_iMoveState = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_iMoveState" ) );
		Displacement.DT_CSPlayer.m_bWaitForNoAttack = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_bWaitForNoAttack" ) );
		Displacement.DT_CSPlayer.m_bCustomPlayer = *( int* )( Memory::Scan( image_client, XorStr( "80 BF ?? ?? ?? ?? ?? 0F 84 ?? ?? ?? ?? 83 BF ?? ?? ?? ?? ?? 74 7C" ) ) + 2 );
		Displacement.DT_CSPlayer.m_iPlayerState = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_iPlayerState" ) );
		Displacement.DT_CSPlayer.m_bIsDefusing = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_bIsDefusing" ) );
		Displacement.DT_CSPlayer.m_bHasDefuser = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_bHasDefuser" ) );
		Displacement.DT_CSPlayer.m_iFOV = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_iFOV" ) );
		Displacement.DT_CSPlayer.m_bIsPlayerGhost = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_bIsPlayerGhost" ) );
		Displacement.DT_CSPlayer.m_vecPlayerPatchEconIndices = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_vecPlayerPatchEconIndices" ) );
		Displacement.DT_CSPlayer.m_hRagdoll = g_PropManager.GetOffset( XorStr( "DT_CSPlayer" ), XorStr( "m_hRagdoll" ) );

		Displacement.DT_CSRagdoll.m_hPlayer = g_PropManager.GetOffset( XorStr( "DT_CSRagdoll" ), XorStr( "m_hPlayer" ) );

		Displacement.DT_FogController.m_fog_enable = g_PropManager.GetOffset( XorStr( "DT_FogController" ), XorStr( "m_fog.enable" ) );
		Displacement.DT_Precipitation.m_nPrecipType = g_PropManager.GetOffset( XorStr( "DT_Precipitation" ), XorStr( "m_nPrecipType" ) );

		Displacement.DT_BaseCombatWeapon.m_flNextPrimaryAttack = g_PropManager.GetOffset( XorStr( "DT_BaseCombatWeapon" ), XorStr( "m_flNextPrimaryAttack" ) );
		Displacement.DT_BaseCombatWeapon.m_flNextSecondaryAttack = g_PropManager.GetOffset( XorStr( "DT_BaseCombatWeapon" ), XorStr( "m_flNextSecondaryAttack" ) );
		Displacement.DT_BaseCombatWeapon.m_hOwner = g_PropManager.GetOffset( XorStr( "DT_BaseCombatWeapon" ), XorStr( "m_hOwner" ) );
		Displacement.DT_BaseCombatWeapon.m_iClip1 = g_PropManager.GetOffset( XorStr( "DT_BaseCombatWeapon" ), XorStr( "m_iClip1" ) );
		Displacement.DT_BaseCombatWeapon.m_iPrimaryReserveAmmoCount = g_PropManager.GetOffset( XorStr( "DT_BaseCombatWeapon" ), XorStr( "m_iPrimaryReserveAmmoCount" ) );
		Displacement.DT_BaseCombatWeapon.m_iItemDefinitionIndex = g_PropManager.GetOffset( XorStr( "DT_BaseCombatWeapon" ), XorStr( "m_iItemDefinitionIndex" ) );
		Displacement.DT_BaseCombatWeapon.m_hWeaponWorldModel = g_PropManager.GetOffset( XorStr( "DT_BaseCombatWeapon" ), XorStr( "m_hWeaponWorldModel" ) );
		Displacement.DT_BaseCombatWeapon.m_iWorldModelIndex = g_PropManager.GetOffset( XorStr( "DT_BaseCombatWeapon" ), XorStr( "m_iWorldModelIndex" ) );
		Displacement.DT_BaseCombatWeapon.m_iWorldDroppedModelIndex = g_PropManager.GetOffset( XorStr( "DT_BaseCombatWeapon" ), XorStr( "m_iWorldDroppedModelIndex" ) );
		Displacement.DT_BaseCombatWeapon.m_iViewModelIndex = g_PropManager.GetOffset( XorStr( "DT_BaseCombatWeapon" ), XorStr( "m_iViewModelIndex" ) );

		Displacement.DT_BaseCombatWeapon.m_CustomMaterials = 0; // 2016-12-13: different econ layout, unused elsewhere
		Displacement.DT_BaseCombatWeapon.m_bCustomMaterialInitialized = *( int* )( Memory::Scan( image_client, XorStr( "C6 86 ? ? ? ? ? FF 50 04" ) ) + 0x2 );

		Displacement.DT_WeaponCSBase.m_flRecoilIndex = g_PropManager.GetOffset( XorStr( "DT_WeaponCSBase" ), XorStr( "m_flRecoilIndex" ) );
		Displacement.DT_WeaponCSBase.m_weaponMode = g_PropManager.GetOffset( XorStr( "DT_WeaponCSBase" ), XorStr( "m_weaponMode" ) );
		Displacement.DT_WeaponCSBase.m_flPostponeFireReadyTime = g_PropManager.GetOffset( XorStr( "DT_WeaponCSBase" ), XorStr( "m_flPostponeFireReadyTime" ) );
		Displacement.DT_WeaponCSBase.m_fLastShotTime = g_PropManager.GetOffset( XorStr( "DT_WeaponCSBase" ), XorStr( "m_fLastShotTime" ) );

		Displacement.DT_WeaponCSBaseGun.m_zoomLevel = g_PropManager.GetOffset( XorStr( "DT_WeaponCSBaseGun" ), XorStr( "m_zoomLevel" ) );
		Displacement.DT_WeaponCSBaseGun.m_iBurstShotsRemaining = g_PropManager.GetOffset( XorStr( "DT_WeaponCSBaseGun" ), XorStr( "m_iBurstShotsRemaining" ) );
		Displacement.DT_WeaponCSBaseGun.m_fNextBurstShot = g_PropManager.GetOffset( XorStr( "DT_WeaponCSBaseGun" ), XorStr( "m_fNextBurstShot" ) );

		Displacement.DT_BaseCSGrenade.m_bPinPulled = g_PropManager.GetOffset( XorStr( "DT_BaseCSGrenade" ), XorStr( "m_bPinPulled" ) );
		Displacement.DT_BaseCSGrenade.m_fThrowTime = g_PropManager.GetOffset( XorStr( "DT_BaseCSGrenade" ), XorStr( "m_fThrowTime" ) );
		Displacement.DT_BaseCSGrenade.m_flThrowStrength = g_PropManager.GetOffset( XorStr( "DT_BaseCSGrenade" ), XorStr( "m_flThrowStrength" ) );

		Displacement.DT_BaseAttributableItem.m_flFallbackWear = g_PropManager.GetOffset( XorStr( "DT_BaseAttributableItem" ), XorStr( "m_flFallbackWear" ) );
		Displacement.DT_BaseAttributableItem.m_nFallbackPaintKit = g_PropManager.GetOffset( XorStr( "DT_BaseAttributableItem" ), XorStr( "m_nFallbackPaintKit" ) );
		Displacement.DT_BaseAttributableItem.m_nFallbackSeed = g_PropManager.GetOffset( XorStr( "DT_BaseAttributableItem" ), XorStr( "m_nFallbackSeed" ) );
		Displacement.DT_BaseAttributableItem.m_nFallbackStatTrak = g_PropManager.GetOffset( XorStr( "DT_BaseAttributableItem" ), XorStr( "m_nFallbackStatTrak" ) );
		Displacement.DT_BaseAttributableItem.m_OriginalOwnerXuidHigh = g_PropManager.GetOffset( XorStr( "DT_BaseAttributableItem" ), XorStr( "m_OriginalOwnerXuidHigh" ) );
		Displacement.DT_BaseAttributableItem.m_OriginalOwnerXuidLow = g_PropManager.GetOffset( XorStr( "DT_BaseAttributableItem" ), XorStr( "m_OriginalOwnerXuidLow" ) );
		Displacement.DT_BaseAttributableItem.m_szCustomName = g_PropManager.GetOffset( XorStr( "DT_BaseAttributableItem" ), XorStr( "m_szCustomName" ) );
		Displacement.DT_BaseAttributableItem.m_bInitialized = g_PropManager.GetOffset( XorStr( "DT_BaseAttributableItem" ), XorStr( "m_bInitialized" ) );
		Displacement.DT_BaseAttributableItem.m_iAccountID = g_PropManager.GetOffset( XorStr( "DT_BaseAttributableItem" ), XorStr( "m_iAccountID" ) );
		Displacement.DT_BaseAttributableItem.m_iEntityLevel = g_PropManager.GetOffset( XorStr( "DT_BaseAttributableItem" ), XorStr( "m_iEntityLevel" ) );
		Displacement.DT_BaseAttributableItem.m_iEntityQuality = g_PropManager.GetOffset( XorStr( "DT_BaseAttributableItem" ), XorStr( "m_iEntityQuality" ) );
		Displacement.DT_BaseAttributableItem.m_iItemDefinitionIndex = g_PropManager.GetOffset( XorStr( "DT_BaseAttributableItem" ), XorStr( "m_iItemDefinitionIndex" ) );
		Displacement.DT_BaseAttributableItem.m_Item = g_PropManager.GetOffset( XorStr( "DT_BaseAttributableItem" ), XorStr( "m_Item" ) );
		Displacement.DT_BaseAttributableItem.m_iItemIDLow = g_PropManager.GetOffset( XorStr( "DT_BaseAttributableItem" ), XorStr( "m_iItemIDLow" ) );
		Displacement.DT_BaseAttributableItem.m_iItemIDHigh = g_PropManager.GetOffset( XorStr( "DT_BaseAttributableItem" ), XorStr( "m_iItemIDHigh" ) );

		Displacement.DT_BaseViewModel.m_hOwner = g_PropManager.GetOffset( XorStr( "DT_BaseViewModel" ), XorStr( "m_hOwner" ) );
		Displacement.DT_BaseViewModel.m_hWeapon = g_PropManager.GetOffset( XorStr( "DT_BaseViewModel" ), XorStr( "m_hWeapon" ) );
		Displacement.DT_BaseViewModel.m_nSequence = g_PropManager.GetOffset( XorStr( "DT_BaseViewModel" ), XorStr( "m_nSequence" ) );

		Displacement.DT_SmokeGrenadeProjectile.m_nSmokeEffectTickBegin = g_PropManager.GetOffset( XorStr( "DT_SmokeGrenadeProjectile" ), XorStr( "m_nSmokeEffectTickBegin" ) );
		Displacement.DT_SmokeGrenadeProjectile.m_bDidSmokeEffect = g_PropManager.GetOffset( XorStr( "DT_SmokeGrenadeProjectile" ), XorStr( "m_bDidSmokeEffect" ) );
		//Displacement.DT_SmokeGrenadeProjectile.m_SmokeParticlesSpawned = *( int* )( Memory::Scan( image_client, XorStr( "80 BF ?? ?? ?? ?? ?? 0F 85 ?? ?? ?? ?? F3 0F 7E 87 ?? ?? ?? ??" ) ) + 2 );

		//Displacement.CBoneMergeCache.m_nConstructor = Memory::Scan( image_client, XorStr( "56 8B F1 0F 57 C0 C7 86 ?? ?? ?? ?? ?? ?? ?? ??" ) );
		Displacement.CBoneMergeCache.m_nInit = CallableFromRelative( Memory::Scan( image_client, XorStr( "E8 ?? ?? ?? ?? FF 75 08 8B 8E ?? ?? ?? ??" ) ) );
		// 2016-12-13: CBoneMergeCache::UpdateCache — "push [ebp+8]; mov ecx,[esi+m_pBoneMergeCache]; call UpdateCache"
		auto BoneMergeUpdateCacheCall = Memory::Scan( image_client, XorStr( "FF 75 08 8B 8E ? ? ? ? E8 ? ? ? ? 5E 8B E5 5D C2 04 00" ) );
		Displacement.CBoneMergeCache.m_nUpdateCache = BoneMergeUpdateCacheCall ? CallableFromRelative( BoneMergeUpdateCacheCall + 9 ) : 0;
		Displacement.CBoneMergeCache.m_CopyToFollow = 0; // 2016-12-13: unresolved on this build, IK/bone-merge emulation guarded
		Displacement.CBoneMergeCache.m_CopyFromFollow = CallableFromRelative( Memory::Scan( image_client, XorStr( "E8 ?? ?? ?? ?? F3 0F 10 45 ?? 8D 84 24 ?? ?? ?? ??" ) ) );

		//Displacement.CIKContext.m_nConstructor = Memory::Scan( image_client, XorStr( "53 8B D9 F6 C3 03 74 0B FF 15 ?? ?? ?? ?? 84 C0 74 01 CC C7 83 ?? ?? ?? ?? ?? ?? ?? ?? 8B CB" ) );
		Displacement.CIKContext.m_nDestructor = 0; // 2016-12-13: IK chain unresolved on this build, guarded in C_BaseAnimating ik helpers
		Displacement.CIKContext.m_nInit = 0;
		Displacement.CIKContext.m_nUpdateTargets = 0;
		Displacement.CIKContext.m_nSolveDependencies = 0;

		Displacement.CBoneSetup.InitPose = Memory::Scan( image_client, XorStr( "55 8B EC 83 EC 10 53 8B D9 89 55 F8 56 57 89 5D F4 8B 0B 89 4D F0" ) );
		Displacement.CBoneSetup.AccumulatePose = Memory::Scan( image_client, XorStr( "55 8B EC 83 E4 F0 B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? A1 ?? ?? ?? ??" ) );
		Displacement.CBoneSetup.CalcAutoplaySequences = 0; // 2016-12-13: unused on this build layout
		Displacement.CBoneSetup.CalcBoneAdj = 0;

		Displacement.IPrediction.m_nCommandsPredicted = 0x1C;

		// jmp patterns is not very reliable
		auto CL_Predict = Memory::Scan( image_engine, XorStr( "75 30 8B 87 ?? ?? ?? ??" ) );
		auto CL_Move = Memory::Scan( image_engine, XorStr( "74 0F 80 BF ?? ?? ?? ?? ??" ) );
		Displacement.CClientState.m_nLastCommandAck = *( int* )( CL_Predict + 0x20 );
		Displacement.CClientState.m_nDeltaTick = *( int* )( CL_Predict + 0x10 );
		Displacement.CClientState.m_nLastOutgoingCommand = *( int* )( CL_Predict + 0xA );
		Displacement.CClientState.m_nChokedCommands = *( int* )( CL_Predict + 0x4 );
		Displacement.CClientState.m_bIsHLTV = *( int* )( CL_Move + 0x4 );
		// 2016-12-13 port: m_nMaxPlayers lives at 0x308 in this build (0x388 is a 2021 layout
		// offset that pointed past the object and corrupted engine globals -> crash at map load).
		// Resolve dynamically via "mov eax,[clstate]; mov eax,[eax+X]; ret" with a safe fallback.
		{
			auto MaxPlayersRead = Memory::Scan( image_engine, XorStr( "A1 ? ? ? ? 8B 80 ? ? ? ? C3" ) );
			Displacement.CClientState.m_nMaxClients = MaxPlayersRead ? *( int* )( MaxPlayersRead + 7 ) : 0x308;
		}

		Displacement.DT_PlantedC4.m_flC4Blow = g_PropManager.GetOffset( XorStr( "DT_PlantedC4" ), XorStr( "m_flC4Blow" ) );
		Displacement.DT_PlantedC4.m_flDefuseCountDown = g_PropManager.GetOffset( XorStr( "DT_PlantedC4" ), XorStr( "m_flDefuseCountDown" ) );
		Displacement.DT_PlantedC4.m_bBombDefused = g_PropManager.GetOffset( XorStr( "DT_PlantedC4" ), XorStr( "m_bBombDefused" ) );

		Displacement.Data.m_uMoveHelper = **( std::uintptr_t** )( Memory::Scan( image_client, XorStr( "8B 0D ?? ?? ?? ?? 8B 46 08 68" ) ) + 2 );
		// 2016-12-13: g_Input via "mov ecx, g_Input; push [ebp+8]; call" (hazedumper 13.12.2016)
		Displacement.Data.m_uInput = *( std::uintptr_t* )( Memory::Scan( image_client, XorStr( "B9 ? ? ? ? FF 75 08 E8 ? ? ? ? 8B 06" ) ) + 1 );
		Displacement.Data.m_uPredictionRandomSeed = *( std::uintptr_t* )( Memory::Scan( image_client, XorStr( "8B 0D ?? ?? ?? ?? BA ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 C4" ) ) + 2 );
		Displacement.Data.m_uPredictionPlayer = *( std::uintptr_t* )( Memory::Scan( image_client, XorStr( "89 ? ? ? ? ? F3 0F 10 48 20" ) ) + 2 );
		Displacement.Data.m_uModelBoneCounter = *( std::uintptr_t* )( Displacement.C_BaseAnimating.InvalidateBoneCache + 0xA );
		Displacement.Data.m_uClientSideAnimationList = *( std::uintptr_t* )( Memory::Scan( image_client, XorStr( "A1 ?? ?? ?? ?? F6 44 F0 04 01 74 0B" ) ) + 1 );
		// 2016-12-13: glow object manager via "mov eax, [glow]; test al,1; jne; xorps xmm0" (hazedumper 13.12.2016)
		Displacement.Data.m_uGlowObjectManager = *( std::uintptr_t* )( Memory::Scan( image_client, XorStr( "A1 ? ? ? ? A8 01 75 4E 0F 57 C0" ) ) + 1 );
		Displacement.Data.m_uCamThink = 0; // 2016-12-13: unresolved, unused on this build
		Displacement.Data.m_uRenderBeams = ( std::uintptr_t )( Memory::Scan( image_client, XorStr( "A1 ?? ?? ?? ?? FF 10 A1 ?? ?? ?? ?? B9" ) ) + 0x1 );
		Displacement.Data.m_uSmokeCount = 0; // 2016-12-13: unresolved, guarded in FrameStageNotify
		Displacement.Data.m_uCenterPrint = ( std::uintptr_t )( Memory::Scan( image_client, XorStr( "8B 35 ? ? ? ? 8D 4C 24 20" ) ) + 0x2 );
		Displacement.Data.m_uHostFrameTicks = ( Memory::Scan( image_engine, XorStr( "03 05 ? ? ? ? 83 CF 10" ) ) + 2 );
		Displacement.Data.m_uServerGlobals = Memory::Scan( image_server, XorStr( "8B 15 ? ? ? ? 33 C9 83 7A 18 01" ) ) + 0x2;
		Displacement.Data.m_uServerPoseParameters = Memory::Scan( image_server, XorStr( "8D 87 ? ? ? ? 89 46 08 C7 46 ? ? ? ? ? EB 06" ) ) + 0x2;
		Displacement.Data.m_uServerAnimState = Memory::Scan( image_server, XorStr( "8B 8F ?? ?? ?? ?? 85 C9 74 06 56" ) ) + 2;
		// 2016-12-13: pattern missing on this build; value is not referenced anywhere - keep 0
		Displacement.Data.m_uTicksAllowed = 0;
		Displacement.Data.m_uHudElement = Memory::Scan( image_client, XorStr( "B9 ?? ?? ?? ?? E8 ?? ?? ?? ?? 8B 5D 08" ) ) + 1;
		//Displacement.Data.m_uListLeavesInBoxReturn = Memory::Scan( image_client, XorStr( "56 52 FF 50 18" ) ) + 0x5;
		Displacement.Data.s_bAllowExtrapolation = Memory::Scan( image_client, XorStr( "A2 ? ? ? ? 8B 45 E8" ) ) + 1;
		Displacement.Data.m_FireBulletsReturn = 0; // 2016-12-13: return point unresolved on this build (unused)
		Displacement.Data.m_D3DDevice = Memory::Scan( image_shaderapidx9, XorStr( "A1 ?? ?? ?? ?? 50 8B 08 FF 51 0C" ) ) + 1;
		//Displacement.Data.m_SoundService = Memory::Scan( image_engine, XorStr( "B9 ? ? ? ? 80 65 FC FE 6A 00" ) );
		Displacement.Data.m_InterpolateServerEntities = Memory::Scan( image_client, XorStr( "55 8B EC 83 EC 1C 8B 0D ? ? ? ? 53 56" ) ); // xref CVProfile::EnterScope(g_VProfCurrentProfile, XorStr("C_BaseEntity::InterpolateServerEntities"), 0, XorStr("Interpolation"), 0, 4);
		Displacement.Data.m_SendNetMsg = Memory::Scan( image_engine, XorStr( "55 8B EC 56 8B F1 8B 86 ? ? ? ? 85 C0 74 24 48 83 F8 02 77 2C 83 BE ? ? ? ? ? 8D 8E ? ? ? ? 74 06 32 C0 84 C0 EB 10 E8 ? ? ? ? 84 C0 EB 07 83 BE ? ? ? ? ? 0F 94 C0 84 C0 74 07 B0 01 5E 5D C2 0C 00" ) ); // xref volume || ConVarRef %s doesn't point to an existing ConVar\n
		// 2016-12-13: ModifyEyePos does not exist on this build - resolve defensively
		{
			auto ModifyEyePosCall = Memory::Scan( image_client, XorStr( "E8 ? ? ? ? 8B 06 8B CE FF 90 ? ? ? ? 85 C0 74 4E" ) );
			Displacement.Data.m_ModifyEyePos = ModifyEyePosCall ? CallableFromRelative( ModifyEyePosCall ) : 0;
		}
		Displacement.Data.m_ResetContentsCache = Memory::Scan( image_client, XorStr( "56 8D 51 3C BE" ) );
		Displacement.Data.m_ProcessInterpolatedList = Memory::Scan( image_client, XorStr( "0F B7 05 ? ? ? ? 3D ? ? ? ? 74 3F" ) ); // xref C_BaseEntity::InterpolateServerEntities
		Displacement.Data.CheckReceivingListReturn = *reinterpret_cast< DWORD32* >( Memory::Scan( image_engine, XorStr( "FF 50 34 8B 1D ? ? ? ? 85 C0 74 16 FF B6" ) ) + 0x3 );
		// 2016-12-13: ReadSubChannelData return point unresolved on this build, guarded by callers
		Displacement.Data.ReadSubChannelDataReturn = 0;
		Displacement.Data.SendDatagram = 0; // 2016-12-13: unresolved (hook stays disabled)
		Displacement.Data.ProcessPacket = Memory::Scan( image_engine, XorStr( "55 8B EC 83 E4 C0 81 EC ? ? ? ? 53 56 57 8B 7D 08 8B D9" ) );

		Displacement.Function.m_uRandomSeed = ( std::uintptr_t )( GetProcAddress( image_vstdlib, XorStr( "RandomSeed" ) ) );
		Displacement.Function.m_uRandomFloat = ( std::uintptr_t )( GetProcAddress( image_vstdlib, XorStr( "RandomFloat" ) ) );
		Displacement.Function.m_uRandomInt = ( std::uintptr_t )( GetProcAddress( image_vstdlib, XorStr( "RandomInt" ) ) );

		Displacement.Function.m_uSetAbsOrigin = Memory::Scan( image_client, XorStr( "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8" ) );
		Displacement.Function.m_uSetAbsAngles = Memory::Scan( image_client, XorStr( "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8" ) );
		Displacement.Function.m_uIsBreakable = Memory::Scan( image_client, XorStr( "55 8B EC 51 56 8B F1 85 F6 74 68" ) ); //xref
		//Displacement.Function.m_uClearHudWeaponIcon = Memory::Scan( image_client, XorStr( "55 8B EC 51 53 56 8B 75 08 8B D9 57 6B FE 2C 89 5D FC" ) );
		Displacement.Function.m_uLoadNamedSkys = Memory::Scan( image_engine, XorStr( "55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45" ) );
		Displacement.Function.m_uCreateAnimState = Memory::Scan( image_client, XorStr( "55 8B EC 56 8B F1 B9 ?? ?? ?? ?? C7 46" ) );
		Displacement.Function.m_uResetAnimState = Memory::Scan( image_client, XorStr( "56 6A 01 68 ?? ?? ?? ?? 8B F1" ) );
		// 2016-12-13: CCSGOPlayerAnimState::Update — direct prologue of the animstate update (0x3E6120 on this build)
		Displacement.Function.m_uUpdateAnimState = Memory::Scan( image_client, XorStr( "55 8B EC 83 EC 1C 56 57 8B F9 F3 0F 11 55 F8 F3 0F 11 4D F4" ) );
		Displacement.Function.m_uClanTagChange = Memory::Scan( image_engine, XorStr( "53 56 57 8B DA 8B F9 FF 15" ) );
		Displacement.Function.m_uGetSequenceActivity = Memory::Scan( image_client, XorStr( "55 8B EC 83 7D 08 FF 56 8B F1 74 3D" ) );
		Displacement.Function.m_uInvalidatePhysics = 0; // 2016-12-13: unresolved on this build, guarded in C_BaseEntity::InvalidatePhysics
		Displacement.Function.m_uPostThinkVPhysics = Memory::Scan( image_client, XorStr( "55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 8B D9 56 57 83 BB ? ? ? ? ? 75 50 8B 0D" ) );
		Displacement.Function.m_SimulatePlayerSimulatedEntities = 0; // 2016-12-13: does not exist in this build, guarded in EnginePrediction
		Displacement.Function.m_uImplPhysicsRunThink = 0; // 2016-12-13: prologue is cloned ~20x in this build, can't pick safely — guarded in C_CSPlayer::PhysicsRunThink
		//Displacement.Function.m_uClearDeathNotices = Memory::Scan( image_client, XorStr( "55 8B EC 83 EC 0C 53 56 8B 71 58" ) ); //55 8B EC 83 EC 0C 53 56 8B 71 58 33 DB 57 89 5D F8 8B 4E 04 8B 01 FF 90
		Displacement.Function.m_uSetTimeout = Memory::Scan( image_engine, XorStr( "55 8B EC 80 7D 0C 00 F3 0F 10 4D" ) );
		Displacement.Function.m_uFindHudElement = 0; // 2016-12-13: hud lookup uses a different shape on this build, guarded in sdk.hpp
		// 2016-12-13: CCollisionProperty::SetCollisionBounds located at 0x66CC10 on this build
		Displacement.Function.m_SetCollisionBounds = Memory::Scan( image_client, XorStr( "53 8B DC 83 EC 08 83 E4 F8 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 83 EC" ) );
		Displacement.Function.m_MD5PseudoRandom = Memory::Scan( image_client, XorStr( "55 8B EC 83 E4 F8 83 EC 70 6A 58 8D 44 24 1C 89 4C 24 08 6A 00 50" ) );
		Displacement.Function.m_WriteUsercmd = Memory::Scan( image_client, XorStr( "55 8B EC 83 E4 F8 51 53 56 8B D9 8B 0D" ) );
		Displacement.Function.m_StdStringAssign = Memory::Scan( image_engine, XorStr( "55 8B EC 53 8B 5D 08 56 8B F1 85 DB 74 57 8B 4E 14 83 F9 10 72 04 8B 06 EB 02" ) );
		Displacement.Function.m_pPoseParameter = Memory::Scan( image_client, XorStr( "55 8B EC 8B 45 08 57 8B F9 8B 4F 04 85 C9 75 15 8B" ) );
		Displacement.Function.m_AttachmentHelper = Memory::Scan( image_client, XorStr( "55 8B EC 83 EC 48 53 8B 5D 08 89 4D F4" ) );
		Displacement.Function.m_LockStudioHdr = CallableFromRelative( Memory::Scan( image_client, XorStr( "E8 ?? ?? ?? ?? 8D 47 FC" ) ) );
		Displacement.Function.m_LineGoesThroughSmoke = 0; // 2016-12-13: unresolved on this build (unused)
		Displacement.Function.m_RunSimulation = CallableFromRelative( Memory::Scan( image_client, XorStr( "E8 ? ? ? ? A1 ? ? ? ? F3 0F 10 45 ? F3 0F 11 40" ) ) );
		Displacement.Function.m_TraceFilterSimple = Memory::Scan( image_client, XorStr( "55 8B EC 83 E4 F0 83 EC 7C 56 52" ) ) + 0x3D; //xref : offset ??_7CTraceFilterSimple@@6B@ ; const CTraceFilterSimple::`vftable' 
	}

	bool CreateDisplacement( void* reserved ) {
		Create( );

		return true;
	}
}