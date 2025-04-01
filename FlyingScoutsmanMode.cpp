#include "FlyingScoutsmanMode.h"
#include "cs_player.h" // na bazie kodu cs
#include "weapon_ssg08.h"

static CFlyingScoutsmanMode g_FlyingScoutsmanMode;

void CFlyingScoutsmanMode::PostInit() {
    m_hookPlayerSpawn.HookMethod(GetClientClass("CCSPlayer"), 
        offsets::PlayerSpawn, &CFlyingScoutsmanMode::Hook_PlayerSpawn);
    
    m_hookPlayerDeath.HookMethod(GetClientClass("CCSPlayer"),
        offsets::PlayerDeath, &CFlyingScoutsmanMode::Hook_PlayerDeath);

    m_hookWeaponCanUse.HookMethod(GetClientClass("CBaseCombatWeapon"),
        offsets::WeaponCanUse, &CFlyingScoutsmanMode::Hook_WeaponCanUse);
}

void CFlyingScoutsmanMode::LevelInitPostEntity() {
    ApplyGameRules();
}

void CFlyingScoutsmanMode::ApplyGameRules() {
    ConVarRef mp_respawn_on_death_t("mp_respawn_on_death_t");
    ConVarRef mp_respawn_on_death_ct("mp_respawn_on_death_ct");
    mp_respawn_on_death_t.SetValue(1);
    mp_respawn_on_death_ct.SetValue(1);
    
    ConVarRef sv_gravity("sv_gravity");
    sv_gravity.SetValue(600 * FS_GRAVITY);
    
    ConVarRef mp_force_assign_teams("mp_force_assign_teams");
    mp_force_assign_teams.SetValue(1);
}

void CFlyingScoutsmanMode::ModifyPlayerAttributes(CCSPlayer* player) {
    if (!player) return;
    
    player->SetMaxSpeed(250 * FS_MOVESPEED);
    player->SetGravity(FS_GRAVITY);
    
    player->SetArmorValue(0);
    player->SetHelmetEnabled(false);
}

void CFlyingScoutsmanMode::ForceScoutLoadout(CCSPlayer* player) {
    player->RemoveAllWeapons();
    
    CBaseEntity* pWeapon = GiveNamedItem(player, "weapon_ssg08");
    if (pWeapon) {
        player->Weapon_Equip(pWeapon);
        CWeaponCSBase* pCSWeapon = dynamic_cast<CWeaponCSBase*>(pWeapon);
        if (pCSWeapon) {
            pCSWeapon->GiveReserveAmmo(90);
        }
    }
    
    GiveNamedItem(player, "weapon_knife");
}

void CFlyingScoutsmanMode::HandleRespawn(CCSPlayer* player) {
    if (player->GetTeamNumber() == TEAM_SPECTATOR) return;
    
    CreateTimer(FS_RESPAWN_TIME, [player]() {
        if (player && player->IsDead()) {
            player->Respawn();
        }
    });
}

void __fastcall CFlyingScoutsmanMode::Hook_PlayerSpawn(void* thisptr, int edx) {
    CCSPlayer* player = static_cast<CCSPlayer*>(thisptr);
    g_FlyingScoutsmanMode.ForceScoutLoadout(player);
    g_FlyingScoutsmanMode.ModifyPlayerAttributes(player);
    original_method(thisptr, edx);
}

void __fastcall CFlyingScoutsmanMode::Hook_PlayerDeath(void* thisptr, int edx, const CTakeDamageInfo& info) {
    CCSPlayer* player = static_cast<CCSPlayer*>(thisptr);
    g_FlyingScoutsmanMode.HandleRespawn(player);
    original_method(thisptr, edx, info);
}

bool __fastcall CFlyingScoutsmanMode::Hook_WeaponCanUse(void* thisptr, int edx, CBaseEntity* weapon) {
    const char* className = weapon->GetClassname();
    if (Q_strcmp(className, "weapon_ssg08") != 0 && 
        Q_strcmp(className, "weapon_knife") != 0) {
        return false; 
    }
    return original_method(thisptr, edx, weapon);
}

void CFlyingScoutsmanMode::FrameUpdatePostEntityThink() {
    ForEachValidPlayer([](CCSPlayer* player) {
        if (player->GetFlags() & FL_ONGROUND) return;
        
        Vector velocity = player->GetAbsVelocity();
        velocity.z *= 0.98f; 
        player->SetAbsVelocity(velocity);
    });
}
