#include "RageBot.h"
#include "RenderManager.h"
#include "Resolver.h"
#include "Autowall.h"
#include <iostream>
#include "UTIL Functions.h"

void CRageBot::Init()
{

	IsAimStepping = false;
	IsLocked = false;
	TargetID = -1;
}



void FakeWalk(CUserCmd * pCmd, bool & bSendPacket)
{

	IClientEntity* pLocal = hackManager.pLocal();
	if (GetAsyncKeyState(VK_SHIFT))
	{

		static int iChoked = -1;
		iChoked++;

		if (iChoked < 1)
		{
			bSendPacket = false;



			pCmd->tick_count += 10.95; // 10.95
			pCmd->command_number += 5.07 + pCmd->tick_count % 2 ? 0 : 1; // 5
	
			pCmd->buttons |= pLocal->GetMoveType() == IN_BACK;
			pCmd->forwardmove = pCmd->sidemove = 0.f;
		}
		else
		{
			bSendPacket = true;
			iChoked = -1;

			Interfaces::Globals->frametime *= (pLocal->GetVelocity().Length2D()) / 10; // 10
			pCmd->buttons |= pLocal->GetMoveType() == IN_FORWARD;
		}
	}
}

void CRageBot::Draw()
{

}

bool IsAbleToShoot(IClientEntity* pLocal)
{
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	if (!pLocal)return false;
	if (!pWeapon)return false;
	float flServerTime = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
	return (!(pWeapon->GetNextPrimaryAttack() > flServerTime));
}

float hitchance(IClientEntity* pLocal, C_BaseCombatWeapon* pWeapon)
{
	float hitchance = 101;
	if (!pWeapon) return 0;
	if (Menu::Window.RageBotTab.AccuracyHitchance.GetValue() > 1)
	{
		float inaccuracy = pWeapon->GetInaccuracy();
		if (inaccuracy == 0) inaccuracy = 0.0000001;
		inaccuracy = 1 / inaccuracy;
		hitchance = inaccuracy;
	}
	return hitchance;
}

bool CanOpenFire() 
{
	IClientEntity* pLocalEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!pLocalEntity)
		return false;

	C_BaseCombatWeapon* entwep = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocalEntity->GetActiveWeaponHandle());

	float flServerTime = (float)pLocalEntity->GetTickBase() * Interfaces::Globals->interval_per_tick;
	float flNextPrimaryAttack = entwep->GetNextPrimaryAttack();

	std::cout << flServerTime << " " << flNextPrimaryAttack << std::endl;

	return !(flNextPrimaryAttack > flServerTime);
}

void CRageBot::Move(CUserCmd *pCmd, bool &bSendPacket)
{

	IClientEntity* pLocalEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (!pLocalEntity || !Menu::Window.RageBotTab.Active.GetState() || !Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;

	if (Menu::Window.RageBotTab.AntiAimEnable.GetState())
	{
		static int ChokedPackets = -1;

		C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
		if (!pWeapon)
			return;

		if (ChokedPackets < 1 && pLocalEntity->GetLifeState() == LIFE_ALIVE && pCmd->buttons & IN_ATTACK && CanOpenFire() && GameUtils::IsBallisticWeapon(pWeapon))
		{
			bSendPacket = false;
		}
		else
		{
			if (pLocalEntity->GetLifeState() == LIFE_ALIVE)
			{
				DoAntiAim(pCmd, bSendPacket);

			}
			ChokedPackets = -1;
		}
	}

	if (Menu::Window.RageBotTab.AimbotEnable.GetState())
		DoAimbot(pCmd, bSendPacket);

	if (Menu::Window.RageBotTab.AccuracyRecoil.GetState())
		DoNoRecoil(pCmd);

	if (Menu::Window.RageBotTab.AimbotAimStep.GetState())
	{
		Vector AddAngs = pCmd->viewangles - LastAngle;
		if (AddAngs.Length2D() > 25.f)
		{
			Normalize(AddAngs, AddAngs);
			AddAngs *= 25;
			pCmd->viewangles = LastAngle + AddAngs;
			GameUtils::NormaliseViewAngle(pCmd->viewangles);
		}
	}

	LastAngle = pCmd->viewangles;
}

Vector BestPoint(IClientEntity *targetPlayer, Vector &final)
{
	IClientEntity* pLocal = hackManager.pLocal();

	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	filter.pSkip = targetPlayer;
	ray.Init(final + Vector(0, 0, 10), final);
	Interfaces::Trace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	final = tr.endpos;
	return final;
}

void CRageBot::DoAimbot(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pTarget = nullptr;
	IClientEntity* pLocal = hackManager.pLocal();
	Vector Start = pLocal->GetViewOffset() + pLocal->GetOrigin();
	bool FindNewTarget = true;
	CSWeaponInfo* weapInfo = ((C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle()))->GetCSWpnData();
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	if (Menu::Window.RageBotTab.AutoRevolver.GetState())
		if (GameUtils::IsRevolver(pWeapon))
		{
			static int delay = 0;
			delay++;
			if (delay <= 15)pCmd->buttons |= IN_ATTACK;
			else delay = 0;
		}
	if (pWeapon)
	{
		if (pWeapon->GetAmmoInClip() == 0 || !GameUtils::IsBallisticWeapon(pWeapon)) return;
	}
	else return;
	if (IsLocked && TargetID >= 0 && HitBox >= 0)
	{
		pTarget = Interfaces::EntList->GetClientEntity(TargetID);
		if (pTarget  && TargetMeetsRequirements(pTarget))
		{
			HitBox = HitScan(pTarget);
			if (HitBox >= 0)
			{
				Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset(), View;
				Interfaces::Engine->GetViewAngles(View);
				float FoV = FovToPlayer(ViewOffset, View, pTarget, HitBox);
				if (FoV < Menu::Window.RageBotTab.AimbotFov.GetValue())	FindNewTarget = false;
			}
		}
	}


	if (FindNewTarget)
	{
		TargetID = 0;
		pTarget = nullptr;
		HitBox = -1;
		switch (Menu::Window.RageBotTab.TargetSelection.GetIndex())
		{
		case 0:TargetID = GetTargetCrosshair(); break;
		case 1:TargetID = GetTargetDistance(); break;
		case 2:TargetID = GetTargetHealth(); break;
		case 3:TargetID = GetTargetThreat(pCmd); break;
		case 4:TargetID = GetTargetNextShot(); break;
		}
		if (TargetID >= 0) pTarget = Interfaces::EntList->GetClientEntity(TargetID);
		else
		{


			pTarget = nullptr;
			HitBox = -1;
		}
	} 
	Globals::Target = pTarget;
	Globals::TargetID = TargetID;
	if (TargetID >= 0 && pTarget)
	{
		HitBox = HitScan(pTarget);

		if (!CanOpenFire()) return;

		if (Menu::Window.RageBotTab.AimbotKeyPress.GetState())
		{


			int Key = Menu::Window.RageBotTab.AimbotKeyBind.GetKey();
			if (Key >= 0 && !GUI.GetKeyState(Key))
			{
				TargetID = -1;
				pTarget = nullptr;
				HitBox = -1;
				return;
			}
		}
		float pointscale = Menu::Window.RageBotTab.TargetPointscale.GetValue() - 5.f; 
		Vector Point;
		Vector AimPoint = GetHitboxPosition(pTarget, HitBox) + Vector(0, 0, pointscale);
		if (Menu::Window.RageBotTab.TargetMultipoint.GetState()) Point = BestPoint(pTarget, AimPoint);
		else Point = AimPoint;

		if (GameUtils::IsScopedWeapon(pWeapon) && !pWeapon->IsScoped() && Menu::Window.RageBotTab.AccuracyAutoScope.GetState()) pCmd->buttons |= IN_ATTACK2;
		else if ((Menu::Window.RageBotTab.AccuracyHitchance.GetValue() * 1.5 <= hitchance(pLocal, pWeapon)) || Menu::Window.RageBotTab.AccuracyHitchance.GetValue() == 0 || *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() == 64)
			{
				if (AimAtPoint(pLocal, Point, pCmd, bSendPacket))
					if (Menu::Window.RageBotTab.AimbotAutoFire.GetState() && !(pCmd->buttons & IN_ATTACK))pCmd->buttons |= IN_ATTACK;
					else if (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2)return;
			}
		if (IsAbleToShoot(pLocal) && pCmd->buttons & IN_ATTACK) Globals::Shots += 1;
	}

}

bool CRageBot::TargetMeetsRequirements(IClientEntity* pEntity)
{
	if (pEntity && pEntity->IsDormant() == false && pEntity->IsAlive() && pEntity->GetIndex() != hackManager.pLocal()->GetIndex())
	{

		ClientClass *pClientClass = pEntity->GetClientClass();
		player_info_t pinfo;
		if (pClientClass->m_ClassID == (int)CSGOClassID::CCSPlayer && Interfaces::Engine->GetPlayerInfo(pEntity->GetIndex(), &pinfo))
		{
			if (pEntity->GetTeamNum() != hackManager.pLocal()->GetTeamNum() || Menu::Window.RageBotTab.TargetFriendlyFire.GetState())
			{
				if (!pEntity->HasGunGameImmunity())
				{
					return true;
				}
			}
		}

	}

	return false;
}

float CRageBot::FovToPlayer(Vector ViewOffSet, Vector View, IClientEntity* pEntity, int aHitBox)
{
	CONST FLOAT MaxDegrees = 180.0f;

	Vector Angles = View;

	Vector Origin = ViewOffSet;

	Vector Delta(0, 0, 0);

	Vector Forward(0, 0, 0);

	AngleVectors(Angles, &Forward);
	Vector AimPos = GetHitboxPosition(pEntity, aHitBox);

	VectorSubtract(AimPos, Origin, Delta);

	Normalize(Delta, Delta);

	FLOAT DotProduct = Forward.Dot(Delta);

	return (acos(DotProduct) * (MaxDegrees / PI));
}

int CRageBot::GetTargetCrosshair()
{

	int target = -1;
	float minFoV = Menu::Window.RageBotTab.AimbotFov.GetValue();

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (fov < minFoV)
				{
					minFoV = fov;
					target = i;
				}
			}

		}
	}

	return target;
}

int CRageBot::GetTargetDistance()
{

	int target = -1;
	int minDist = 99999;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{

			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				Vector Difference = pLocal->GetOrigin() - pEntity->GetOrigin();
				int Distance = Difference.Length();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (Distance < minDist && fov < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					minDist = Distance;
					target = i;
				}
			}

		}
	}

	return target;
}

int CRageBot::GetTargetNextShot()
{
	int target = -1;
	int minfov = 361;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{

		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				int Health = pEntity->GetHealth();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (fov < minfov && fov < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					minfov = fov;
					target = i;
				}
				else
					minfov = 361;
			}

		}
	}

	return target;
}

float GetFov(const QAngle& viewAngle, const QAngle& aimAngle)
{
	Vector ang, aim;

	AngleVectors(viewAngle, &aim);
	AngleVectors(aimAngle, &ang);

	return RAD2DEG(acos(aim.Dot(ang) / aim.LengthSqr()));
}

double inline __declspec (naked) __fastcall FASTSQRT(double n)
{
	_asm fld qword ptr[esp + 4]
		_asm fsqrt
	_asm ret 8
}

float VectorDistance(Vector v1, Vector v2)
{
	return FASTSQRT(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2) + pow(v1.z - v2.z, 2));
}

int CRageBot::GetTargetThreat(CUserCmd* pCmd)
{
	auto iBestTarget = -1;
	float flDistance = 8192.f;

	IClientEntity* pLocal = hackManager.pLocal();

	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			auto vecHitbox = pEntity->GetBonePos(NewHitBox);
			if (NewHitBox >= 0)
			{

				Vector Difference = pLocal->GetOrigin() - pEntity->GetOrigin();
				QAngle TempTargetAbs;
				CalcAngle(pLocal->GetEyePosition(), vecHitbox, TempTargetAbs);
				float flTempFOVs = GetFov(pCmd->viewangles, TempTargetAbs);
				float flTempDistance = VectorDistance(pLocal->GetOrigin(), pEntity->GetOrigin());
				if (flTempDistance < flDistance && flTempFOVs < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					flDistance = flTempDistance;
					iBestTarget = i;
				}
			}
		}
	}
	return iBestTarget;
}

int CRageBot::GetTargetHealth()
{

	int target = -1;
	int minHealth = 101;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);


	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				int Health = pEntity->GetHealth();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (Health < minHealth && fov < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					minHealth = Health;
					target = i;
				}
			}
		}

	}

	return target;
}

int CRageBot::HitScan(IClientEntity* pEntity)
{
	IClientEntity* pLocal = hackManager.pLocal();
	std::vector<int> HitBoxesToScan;

	// Get the hitboxes to scan
#pragma region GetHitboxesToScan
	int HitScanMode = Menu::Window.RageBotTab.TargetHitscan.GetIndex();
	bool AWall = Menu::Window.RageBotTab.AccuracyAutoWall.GetState();
	bool Multipoint = Menu::Window.RageBotTab.TargetMultipoint.GetState();

	{
		if (HitScanMode == 0)
		{
			// No Hitscan, just a single hitbox
			switch (Menu::Window.RageBotTab.TargetHitbox.GetIndex())
			{
			case 0:
				break;
			case 1:
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
				break;
			case 2:
				HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
				break;
			case 3:
				HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
				break;
			case 4:
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
				break;
			}
		}
		else
		{
			switch (HitScanMode)
			{
			case 0:
				break;
			case 1:
				// Low
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
				HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
				break;
			case 2:
				// Normal
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
				HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh);
				break;
			case 3:
				// High
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
				HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftLowerArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);

			}
		}
	}
#pragma endregion Get the list of shit to scan

	// check hits
	// check hits
	for (auto HitBoxID : HitBoxesToScan)
	{
		if (AWall)
		{
			Vector Point = GetHitboxPosition(pEntity, HitBoxID);
			float Damage = 0.f;
			Color c = Color(255, 255, 255, 255);
			if (CanHit(Point, &Damage))
			{
				c = Color(0, 255, 0, 255);
				if (Damage >= Menu::Window.RageBotTab.AccuracyMinimumDamage.GetValue())
				{
					return HitBoxID;
				}
			}
		}
		else
		{
			if (GameUtils::IsVisible(hackManager.pLocal(), pEntity, HitBoxID))
				return HitBoxID;
		}
	}

	return -1;
}
#pragma endregion Get the list of shit to scan


void CRageBot::DoNoRecoil(CUserCmd *pCmd)
{

	IClientEntity* pLocal = hackManager.pLocal();
	if (pLocal)
	{
		Vector AimPunch = pLocal->localPlayerExclusive()->GetAimPunchAngle();
		if (AimPunch.Length2D() > 0 && AimPunch.Length2D() < 150)
		{
			pCmd->viewangles -= AimPunch * 2;
			GameUtils::NormaliseViewAngle(pCmd->viewangles);
		}
	}

}

void CRageBot::aimAtPlayer(CUserCmd *pCmd)
{
	IClientEntity* pLocal = hackManager.pLocal();

	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (!pLocal || !pWeapon)
		return;

	Vector eye_position = pLocal->GetEyePosition();

	float best_dist = pWeapon->GetCSWpnData()->m_flRange;

	IClientEntity* target = nullptr;

	for (int i = 0; i < Interfaces::Engine->GetMaxClients(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			if (Globals::TargetID != -1)
				target = Interfaces::EntList->GetClientEntity(Globals::TargetID);
			else
				target = pEntity;

			Vector target_position = target->GetEyePosition();

			float temp_dist = eye_position.DistTo(target_position);

			if (best_dist > temp_dist)
			{
				best_dist = temp_dist;
				CalcAngle(eye_position, target_position, pCmd->viewangles);
			}
		}

	}
}

bool CRageBot::AimAtPoint(IClientEntity* pLocal, Vector point, CUserCmd *pCmd, bool &bSendPacket)
{
	bool ReturnValue = false;

	if (point.Length() == 0) return ReturnValue;

	Vector angles;
	Vector src = pLocal->GetOrigin() + pLocal->GetViewOffset();

	CalcAngle(src, point, angles);
	GameUtils::NormaliseViewAngle(angles);

	if (angles[0] != angles[0] || angles[1] != angles[1])
	{
		return ReturnValue;
	}

	IsLocked = true;

	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	if (!IsAimStepping)
		LastAimstepAngle = LastAngle; 

	float fovLeft = FovToPlayer(ViewOffset, LastAimstepAngle, Interfaces::EntList->GetClientEntity(TargetID), 0);

	if (fovLeft > 25.0f && Menu::Window.RageBotTab.AimbotAimStep.GetState())
	{

		Vector AddAngs = angles - LastAimstepAngle;
		Normalize(AddAngs, AddAngs);
		AddAngs *= 25;
		LastAimstepAngle += AddAngs;
		GameUtils::NormaliseViewAngle(LastAimstepAngle);
		angles = LastAimstepAngle;
	}
	else
	{
		ReturnValue = true;
	}

	if (Menu::Window.RageBotTab.AimbotSilentAim.GetState())
	{
		pCmd->viewangles = angles;

	}

	if (!Menu::Window.RageBotTab.AimbotSilentAim.GetState())
	{

		Interfaces::Engine->SetViewAngles(angles);
	}

	return ReturnValue;
}

namespace AntiAims 
{
	void JitterPitch(CUserCmd *pCmd)
	{
		static bool up = true;
		if (up)
		{
			pCmd->viewangles.x = 45;
			up = !up;
		}
		else
		{
			pCmd->viewangles.x = 89;
			up = !up;
		}
	}

	void FakePitch(CUserCmd *pCmd, bool &bSendPacket)
	{	
		static int ChokedPackets = -1;
		ChokedPackets++;
		if (ChokedPackets < 1)
		{
			bSendPacket = false;
			pCmd->viewangles.x = 89;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.x = 51;
			ChokedPackets = -1;
		}
	}

	void StaticJitter(CUserCmd *pCmd)
	{
		static bool down = true;
		if (down)
		{
			pCmd->viewangles.x = 179.0f;
			down = !down;
		}
		else
		{
			pCmd->viewangles.x = 89.0f;
			down = !down;
		}
	}

	// Yaws

	void FastSpin(CUserCmd *pCmd)
	{
		static int y2 = -179;
		int spinBotSpeedFast = 100;

		y2 += spinBotSpeedFast;

		if (y2 >= 179)
			y2 = -179;

		pCmd->viewangles.y = y2;
	}

	
	void BackJitter(CUserCmd *pCmd)
	{
		int random = rand() % 100;

		if (random < 98)

			pCmd->viewangles.y -= 180;

		if (random < 15)
		{
			float change = -70 + (rand() % (int)(140 + 1));
			pCmd->viewangles.y += change;
		}
		if (random == 69)
		{
			float change = -90 + (rand() % (int)(180 + 1));
			pCmd->viewangles.y += change;
		}
	}

	void AntiCorrection(CUserCmd* pCmd)
	{
		Vector newAngle = pCmd->viewangles;

		static int ChokedPackets = -1;
		ChokedPackets++;

		float yaw;
		static int state = 0;
		static bool LBYUpdated = false;

		float flCurTime = Interfaces::Globals->curtime;
		static float flTimeUpdate = 1.09f;
		static float flNextTimeUpdate = flCurTime + flTimeUpdate;
		if (flCurTime >= flNextTimeUpdate) {
			LBYUpdated = !LBYUpdated;
			state = 0;
		}

		if (flNextTimeUpdate < flCurTime || flNextTimeUpdate - flCurTime > 10.f)
			flNextTimeUpdate = flCurTime + flTimeUpdate;

		if (LBYUpdated)
			yaw = 90;
		else
			yaw = -90;

		if (yaw)
			newAngle.y += yaw;

		pCmd->viewangles = newAngle;
	}

	void AntiCorrectionALT(CUserCmd* pCmd)
	{
		Vector newAngle = pCmd->viewangles;

		static int ChokedPackets = -1;
		ChokedPackets++;

		float yaw;
		static int state = 0;
		static bool LBYUpdated = false;

		float flCurTime = Interfaces::Globals->curtime;
		static float flTimeUpdate = 1.09f;
		static float flNextTimeUpdate = flCurTime + flTimeUpdate;
		if (flCurTime >= flNextTimeUpdate) {
			LBYUpdated = !LBYUpdated;
			state = 0;
		}

		if (flNextTimeUpdate < flCurTime || flNextTimeUpdate - flCurTime > 10.f)
			flNextTimeUpdate = flCurTime + flTimeUpdate;

		if (LBYUpdated)
			yaw = -90;
		else
			yaw = 90;

		if (yaw)
			newAngle.y += yaw;

		pCmd->viewangles = newAngle;
	}

	void FakeSideways(CUserCmd *pCmd, bool &bSendPacket)
	{
		static int ChokedPackets = -1;
		ChokedPackets++;
		if (ChokedPackets < 1)
		{
			bSendPacket = false;
			pCmd->viewangles.y += 90;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.y -= 180;
			ChokedPackets = -1;
		}
	}

	void FastSpint(CUserCmd *pCmd)
	{
		int r1 = rand() % 100;
		int r2 = rand() % 1000;

		static bool dir;
		static float current_y = pCmd->viewangles.y;

		if (r1 == 1) dir = !dir;

		if (dir)
			current_y += 15 + rand() % 10;
		else
			current_y -= 15 + rand() % 10;

		pCmd->viewangles.y = current_y;

		if (r1 == r2)
			pCmd->viewangles.y += r1;
	}

	void BackwardJitter(CUserCmd *pCmd)
	{
		int random = rand() % 100;

		if (random < 98)

			pCmd->viewangles.y -= 180;

		if (random < 15)
		{
			float change = -70 + (rand() % (int)(140 + 1));
			pCmd->viewangles.y += change;
		}
		if (random == 69)
		{
			float change = -90 + (rand() % (int)(180 + 1));
			pCmd->viewangles.y += change;
		}
	}

	void Jitter(CUserCmd *pCmd)
	{
		static int jitterangle = 0;

		if (jitterangle <= 1)
		{
			pCmd->viewangles.y += 90;
		}
		else if (jitterangle > 1 && jitterangle <= 3)
		{
			pCmd->viewangles.y -= 90;
		}

		int re = rand() % 4 + 1;


		if (jitterangle <= 1)
		{
			if (re == 4)
				pCmd->viewangles.y += 180;
			jitterangle += 1;
		}
		else if (jitterangle > 1 && jitterangle <= 3)
		{
			if (re == 4)
				pCmd->viewangles.y -= 180;
			jitterangle += 1;
		}
		else
		{
			jitterangle = 0;
		}
	}

	void FakeStatic(CUserCmd *pCmd, bool &bSendPacket)
	{
		static int ChokedPackets = -1;
		ChokedPackets++;
		if (ChokedPackets < 1)
		{
			bSendPacket = false;
			static int y2 = -179;
			int spinBotSpeedFast = 360.0f / 1.618033988749895f;;

			y2 += spinBotSpeedFast;

			if (y2 >= 179)
				y2 = -179;

			pCmd->viewangles.y = y2;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.y -= 180;
			ChokedPackets = -1;
		}
	}

	void TJitter(CUserCmd *pCmd)
	{
		static bool Turbo = true;
		if (Turbo)
		{
			pCmd->viewangles.y -= 90;
			Turbo = !Turbo;
		}
		else
		{
			pCmd->viewangles.y += 90;
			Turbo = !Turbo;
		}
	}

	void TFake(CUserCmd *pCmd, bool &bSendPacket)
	{
		static int ChokedPackets = -1;
		ChokedPackets++;
		if (ChokedPackets < 1)
		{
			bSendPacket = false;
			pCmd->viewangles.y = -90;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.y = 90;
			ChokedPackets = -1;
		}
	}

	void FakeJitter(CUserCmd* pCmd, bool &bSendPacket)
	{
		static int jitterangle = 0;

		if (jitterangle <= 1)
		{
			pCmd->viewangles.y += 135;
		}
		else if (jitterangle > 1 && jitterangle <= 3)
		{
			pCmd->viewangles.y += 225;
		}

		static int iChoked = -1;
		iChoked++;
		if (iChoked < 1)
		{
			bSendPacket = false;
			if (jitterangle <= 1)
			{
				pCmd->viewangles.y += 45;
				jitterangle += 1;
			}
			else if (jitterangle > 1 && jitterangle <= 3)
			{
				pCmd->viewangles.y -= 45;
				jitterangle += 1;
			}
			else
			{
				jitterangle = 0;
			}
		}
		else
		{
			bSendPacket = true;
			iChoked = -1;
		}
	}


	void Up(CUserCmd *pCmd)
	{
		pCmd->viewangles.x = -89.0f;
	}

	void Zero(CUserCmd *pCmd)
	{
		pCmd->viewangles.x = 0.f;
	}

	void Static(CUserCmd *pCmd)
	{
		static bool aa1 = false;
		aa1 = !aa1;
		if (aa1)
		{
			static bool turbo = false;
			turbo = !turbo;
			if (turbo)
			{
				pCmd->viewangles.y -= 90;
			}
			else
			{
				pCmd->viewangles.y += 90;
			}
		}
		else
		{
			pCmd->viewangles.y -= 180;
		}
	}

	void fakelowerbody(CUserCmd *pCmd, bool &bSendPacket)
	{
		static bool f_flip = true;
		f_flip = !f_flip;

		if (f_flip)
		{
			pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + 90.00f;
			bSendPacket = false;
		}
		else if (!f_flip)
		{
			pCmd->viewangles.y += hackManager.pLocal()->GetLowerBodyYaw() - 90.00f;
			bSendPacket = true;
		}
	}

	void FakeSideLBY(CUserCmd *pCmd, bool &bSendPacket)
	{
		int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); ++i;
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

		static bool isMoving;
		float PlayerIsMoving = abs(pLocal->GetVelocity().Length());
		if (PlayerIsMoving > 0.1) isMoving = true;
		else if (PlayerIsMoving <= 0.1) isMoving = false;

		int flip = (int)floorf(Interfaces::Globals->curtime / 1.1) % 2;
		static bool bFlipYaw;
		float flInterval = Interfaces::Globals->interval_per_tick;
		float flTickcount = pCmd->tick_count;
		float flTime = flInterval * flTickcount;
		if (std::fmod(flTime, 1) == 0.f)
			bFlipYaw = !bFlipYaw;

		if (PlayerIsMoving <= 0.1)
		{
			if (bSendPacket)
			{
				pCmd->viewangles.y += 180.f;
			}
			else
			{
				if (flip)
				{
					pCmd->viewangles.y += bFlipYaw ? 90.f : -90.f;

				}
				else
				{
					pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + bFlipYaw ? 90.f : -90.f;
				}
			}
		}
		else if (PlayerIsMoving > 0.1)
		{
			if (bSendPacket)
			{
				pCmd->viewangles.y += 180.f;
			}
			else
			{
				pCmd->viewangles.y += 90.f;
			}
		}
	}
	void LBYJitter(CUserCmd* cmd, bool& packet)
	{
		static bool ySwitch;
		static bool jbool;
		static bool jboolt;
		ySwitch = !ySwitch;
		jbool = !jbool;
		jboolt = !jbool;
		if (ySwitch)
		{
			if (jbool)
			{
				if (jboolt)
				{
					cmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - 90.f;
					packet = false;
				}
				else
				{
					cmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 90.f;
					packet = false;
				}
			}
			else
			{
				if (jboolt)
				{
					cmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - 125.f;
					packet = false;
				}
				else
				{
					cmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 125.f;
					packet = false;
				}
			}
		}
		else
		{
			cmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw();
			packet = true;
		}
	}

	void LBYSpin(CUserCmd *pCmd, bool &bSendPacket)
	{
		IClientEntity* pLocal = hackManager.pLocal();
		static int skeet = 179;
		int SpinSpeed = 100;
		static int ChokedPackets = -1;
		ChokedPackets++;
		skeet += SpinSpeed;

		if
			(pCmd->command_number % 9)
		{
			bSendPacket = true;
			if (skeet >= pLocal->GetLowerBodyYaw() + 180);
			skeet = pLocal->GetLowerBodyYaw() - 0;
			ChokedPackets = -1;
		}
		else if
			(pCmd->command_number % 9)
		{
			bSendPacket = false;
			pCmd->viewangles.y += 179;
			ChokedPackets = -1;
		}
		pCmd->viewangles.y = skeet;
	}

	void SlowSpin(CUserCmd *pCmd)
	{
		int r1 = rand() % 100;
		int r2 = rand() % 1000;

		static bool dir;
		static float current_y = pCmd->viewangles.y;

		if (r1 == 1) dir = !dir;

		if (dir)
			current_y += 4 + rand() % 10;
		else
			current_y -= 4 + rand() % 10;

		pCmd->viewangles.y = current_y;

		if (r1 == r2)
			pCmd->viewangles.y += r1;
	}
}

void DoFAKELBYBreak(CUserCmd * pCmd, IClientEntity* pLocal, bool& bSendPacket)
{
	if (!bSendPacket)
	{
		pCmd->viewangles.y -= 90;
	}
	else
	{
		pCmd->viewangles.y += 90;
	}
}

void DoFAKELBYBreakReal(CUserCmd * pCmd, IClientEntity* pLocal, bool& bSendPacket)
{
	if (!bSendPacket)
	{
		pCmd->viewangles.y += 90;
	}
	else
	{
		pCmd->viewangles.y -= 90;
	}
}

void CorrectMovement(Vector old_angles, CUserCmd* cmd, float old_forwardmove, float old_sidemove)
{
	float delta_view, first_function, second_function;

	if (old_angles.y < 0.f) first_function = 360.0f + old_angles.y;
	else first_function = old_angles.y;
	if (cmd->viewangles.y < 0.0f) second_function = 360.0f + cmd->viewangles.y;
	else second_function = cmd->viewangles.y;

	if (second_function < first_function) delta_view = abs(second_function - first_function);
	else delta_view = 360.0f - abs(first_function - second_function);

	delta_view = 360.0f - delta_view;

	cmd->forwardmove = cos(DEG2RAD(delta_view)) * old_forwardmove + cos(DEG2RAD(delta_view + 90.f)) * old_sidemove;
	cmd->sidemove = sin(DEG2RAD(delta_view)) * old_forwardmove + sin(DEG2RAD(delta_view + 90.f)) * old_sidemove;
}

float GetLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{

		float Latency = nci->GetAvgLatency(FLOW_OUTGOING) + nci->GetAvgLatency(FLOW_INCOMING);
		return Latency;
	}
	else
	{

		return 0.0f;
	}
}
float GetOutgoingLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{

		float OutgoingLatency = nci->GetAvgLatency(FLOW_OUTGOING);
		return OutgoingLatency;
	}
	else
	{

		return 0.0f;
	}
}
float GetIncomingLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{
		float IncomingLatency = nci->GetAvgLatency(FLOW_INCOMING);
		return IncomingLatency;
	}
	else
	{

		return 0.0f;
	}
}

float OldLBY;
float LBYBreakerTimer;
float LastLBYUpdateTime;
bool bSwitch;
float CurrentVelocity(IClientEntity* LocalPlayer)
{
	int vel = LocalPlayer->GetVelocity().Length2D();
	return vel;
}
bool NextLBYUpdate()
{
	IClientEntity* LocalPlayer = hackManager.pLocal();

	float flServerTime = (float)(LocalPlayer->GetTickBase()  * Interfaces::Globals->interval_per_tick);


	if (OldLBY != LocalPlayer->GetLowerBodyYaw())
	{

		LBYBreakerTimer++;
		OldLBY = LocalPlayer->GetLowerBodyYaw();
		bSwitch = !bSwitch;
		LastLBYUpdateTime = flServerTime;
	}

	if (CurrentVelocity(LocalPlayer) > 0.5)
	{
		LastLBYUpdateTime = flServerTime;
		return false;
	}

	if ((LastLBYUpdateTime + 1 - (GetLatency() * 2) < flServerTime) && (LocalPlayer->GetFlags() & FL_ONGROUND))
	{
		if (LastLBYUpdateTime + 1.1 - (GetLatency() * 2) < flServerTime)
		{
			LastLBYUpdateTime += 1.1;
		}
		return true;
	}
	return false;
}

void SideJitterALT(CUserCmd * pCmd, IClientEntity* pLocal, bool& bSendPacket)
{
	if (!bSendPacket)
	{
		static bool Fast2 = false;
		if (Fast2)
		{
			pCmd->viewangles.y += 75;
		}
		else
		{
			pCmd->viewangles.y += 105;
		}
		Fast2 = !Fast2;
	}
	else
	{
		pCmd->viewangles.y += 90;
	}
}

void SideJitter(CUserCmd * pCmd, IClientEntity* pLocal, bool& bSendPacket)
{
	if (!bSendPacket)
	{
		static bool Fast2 = false;
		if (Fast2)
		{
			pCmd->viewangles.y -= 75;
		}
		else
		{
			pCmd->viewangles.y -= 105;
		}
		Fast2 = !Fast2;
	}
	else
	{
		pCmd->viewangles.y -= 90;
	}
}


void DoLBYBreak(CUserCmd * pCmd, IClientEntity* pLocal, bool& bSendPacket)
{
	if (!bSendPacket)
	{
		if (NextLBYUpdate())
			pCmd->viewangles.y += 90;
		else
			pCmd->viewangles.y -= 90;
	}
	else
	{
		if (NextLBYUpdate())
			pCmd->viewangles.y -= 90;
		else
			pCmd->viewangles.y += 90;
	}
}

void DoLBYBreakReal(CUserCmd * pCmd, IClientEntity* pLocal, bool& bSendPacket)
{
	if (!bSendPacket)
	{
		if (NextLBYUpdate())
			pCmd->viewangles.y -= 90;
		else
			pCmd->viewangles.y += 90;
	}
	else
	{
		if (NextLBYUpdate())
			pCmd->viewangles.y += 90;
		else
			pCmd->viewangles.y -= 90;
	}
}

static bool peja;
static bool switchbrak;
static bool safdsfs;



void DoRealAA(CUserCmd* pCmd, IClientEntity* pLocal, bool& bSendPacket)
{

	static bool switch2;
	Vector oldAngle = pCmd->viewangles;
	float oldForward = pCmd->forwardmove;
	float oldSideMove = pCmd->sidemove;
	if (!Menu::Window.RageBotTab.AntiAimEnable.GetState())
		return;


	switch (Menu::Window.RageBotTab.AntiAimYaw.GetIndex())
	{
	case 0:
		break;
	case 1:
		// Fast Spin
		AntiAims::FastSpint(pCmd);
		break;
	case 2:
		// Slow Spin
		AntiAims::SlowSpin(pCmd);
		break;
	case 3:
		AntiAims::Jitter(pCmd);
		break;
	case 4:
		// 180 Jitter
		AntiAims::BackJitter(pCmd);
		break;
	case 5:
		//backwards
		pCmd->viewangles.y -= 180;
		break;
	case 6:
		AntiAims::BackwardJitter(pCmd);
		break;
	case 7:
		//Sideways-switch
		if (switch2)
			pCmd->viewangles.y = 90;
		else
			pCmd->viewangles.y = -90;

		switch2 = !switch2;
		break;
	case 8:
		//Sideways
		pCmd->viewangles.y -= 90;
		break;
	case 9:
		pCmd->viewangles.y += 90;
		break;
	case 10:
		pCmd->viewangles.y = pLocal->GetLowerBodyYaw() + rand() % 180 - rand() % 50;
		break;
	case 11:
		AntiAims::LBYJitter(pCmd, bSendPacket);
		break;
	case 12:
		AntiAims::FakeSideLBY(pCmd, bSendPacket);
		break;
	case 13:
		AntiAims::LBYSpin(pCmd, bSendPacket);
		break;
	case 14:
		DoLBYBreakReal(pCmd, pLocal, bSendPacket);
		break;
	case 15:
		DoFAKELBYBreakReal(pCmd, pLocal, bSendPacket);
		break;
	}

	if (hackManager.pLocal()->GetVelocity().Length() > 0) {
		switch (Menu::Window.RageBotTab.MoveYaw.GetIndex())
		{
			//bSendPacket = false;
		case 0:
			break;
		case 1:
			// Fast Spin
			AntiAims::FastSpint(pCmd);
			break;
		case 2:
			// Slow Spin
			AntiAims::SlowSpin(pCmd);
			break;
		case 3:
			AntiAims::Jitter(pCmd);
			break;
		case 4:
			// 180 Jitter
			AntiAims::BackJitter(pCmd);
			break;
		case 5:
			//backwards
			pCmd->viewangles.y -= 180;
			break;
		case 6:
			AntiAims::BackwardJitter(pCmd);
			break;
		case 7:
			//Sideways-switch
			if (switch2)
				pCmd->viewangles.y = 90;
			else
				pCmd->viewangles.y = -90;

			switch2 = !switch2;
			break;
		case 8:
			//Sideways
			pCmd->viewangles.y -= 90;
			break;
		case 9:
			pCmd->viewangles.y += 90;
			break;
		case 10:
			pCmd->viewangles.y = pLocal->GetLowerBodyYaw() + rand() % 180 - rand() % 50;
			break;
		case 11:
			AntiAims::LBYJitter(pCmd, bSendPacket);
			break;
		case 12:
			AntiAims::FakeSideLBY(pCmd, bSendPacket);
			break;
		case 13:
			AntiAims::LBYSpin(pCmd, bSendPacket);
			break;
		case 14:
			DoLBYBreakReal(pCmd, pLocal, bSendPacket);
			break;
		case 15:
			DoFAKELBYBreakReal(pCmd, pLocal, bSendPacket);
			break;
			
		}
	}
}

void DoFakeAA(CUserCmd* pCmd, bool& bSendPacket, IClientEntity* pLocal)
{

	static bool switch2;
	Vector oldAngle = pCmd->viewangles;
	float oldForward = pCmd->forwardmove;
	float oldSideMove = pCmd->sidemove;
	if (!Menu::Window.RageBotTab.AntiAimEnable.GetState())
		return;
	switch (Menu::Window.RageBotTab.FakeYaw.GetIndex())
	{
	case 0:
		break;
	case 1:
		// Fast Spin 
		AntiAims::FastSpint(pCmd);
		break;
	case 2:
		// Slow Spin 
		AntiAims::SlowSpin(pCmd);
		break;
	case 3:
		AntiAims::Jitter(pCmd);
		break;
	case 4:
		// 180 Jitter 
		AntiAims::BackJitter(pCmd);
		break;
	case 5:
		//backwards
		pCmd->viewangles.y -= 180;
		break;
	case 6:
		AntiAims::BackwardJitter(pCmd);
		break;
	case 7:
		//Sideways-switch
		if (switch2)
			pCmd->viewangles.y = 90;
		else
			pCmd->viewangles.y = -90;

		switch2 = !switch2;
		break;
	case 8:
		pCmd->viewangles.y -= 90;
		break;
	case 9:
		pCmd->viewangles.y += 90;
		break;
	case 10:
		pCmd->viewangles.y = pLocal->GetLowerBodyYaw() + rand() % 180 - rand() % 50;
		break;
	case 11:
		AntiAims::LBYJitter(pCmd, bSendPacket);
		break;
	case 12:
		AntiAims::FakeSideLBY(pCmd, bSendPacket);
		break;
	case 13:
		AntiAims::LBYSpin(pCmd, bSendPacket);
		break;
	case 14:
		DoLBYBreak(pCmd, pLocal, bSendPacket);
		break;
	case 15:
		DoFAKELBYBreakReal(pCmd, pLocal, bSendPacket);
		break;
	}

		if (hackManager.pLocal()->GetVelocity().Length() > 0) {
		switch (Menu::Window.RageBotTab.MoveYawFake.GetIndex())
		{
			//bSendPacket = false;
		case 0:
			break;
		case 1:
			// Fast Spin 
			AntiAims::FastSpint(pCmd);
			break;
		case 2:
			// Slow Spin 
			AntiAims::SlowSpin(pCmd);
			break;
		case 3:
			AntiAims::Jitter(pCmd);
			break;
		case 4:
			// 180 Jitter 
			AntiAims::BackJitter(pCmd);
			break;
		case 5:
			//backwards
			pCmd->viewangles.y -= 180;
			break;
		case 6:
			AntiAims::BackwardJitter(pCmd);
			break;
		case 7:
			//Sideways-switch
			if (switch2)
				pCmd->viewangles.y = 90;
			else
				pCmd->viewangles.y = -90;

			switch2 = !switch2;
			break;
		case 8:
			pCmd->viewangles.y -= 90;
			break;
		case 9:
			pCmd->viewangles.y += 90;
			break;
		case 10:
			pCmd->viewangles.y = pLocal->GetLowerBodyYaw() + rand() % 180 - rand() % 50;
			break;
		case 11:
			AntiAims::LBYJitter(pCmd, bSendPacket);
			break;
		case 12:
			AntiAims::FakeSideLBY(pCmd, bSendPacket);
			break;
		case 13:
			AntiAims::LBYSpin(pCmd, bSendPacket);
			break;
		case 14:
			DoLBYBreak(pCmd, pLocal, bSendPacket);
			break;

		}
	}
}

void CRageBot::DoAntiAim(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pLocal = hackManager.pLocal();

	if ((pCmd->buttons & IN_USE) || pLocal->GetMoveType() == MOVETYPE_LADDER)
		return;

	if (IsAimStepping || pCmd->buttons & IN_ATTACK)
		return;

	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
	if (pWeapon)
	{
		CSWeaponInfo* pWeaponInfo = pWeapon->GetCSWpnData();

		if (!GameUtils::IsBallisticWeapon(pWeapon))
		{
			if (!CanOpenFire() || pCmd->buttons & IN_ATTACK2)
				return;

		}
	}
	if (Menu::Window.RageBotTab.AntiAimTarget.GetState())
	{
		aimAtPlayer(pCmd);

	}

	FakeWalk(pCmd, bSendPacket);

	switch (Menu::Window.RageBotTab.AntiAimPitch.GetIndex())
	{
	case 0:
		break;
	case 1:
		pCmd->viewangles.x = 45.f;
		break;
	case 2:
		AntiAims::JitterPitch(pCmd);
		break;
	case 3:
		pCmd->viewangles.x = 89.000000;
		break;
	case 4:
		AntiAims::Up(pCmd);
		break;
	case 5:
		AntiAims::Zero(pCmd);
		break;

	}
	switch (Menu::Window.RageBotTab.AntiAimEdge.GetIndex())
	{
	case 0:
		break;
	case 1:
		//EdgeDetect(pCmd, bSendPacket);
		break;
	case 2:
		//FakeEdge(pCmd, bSendPacket);
		break;


	}

	if (Menu::Window.RageBotTab.AntiAimEnable.GetState())
	{
		static int ChokedPackets = -1;
		ChokedPackets++;
		if (ChokedPackets < 1)
		{

			bSendPacket = true;
			DoFakeAA(pCmd, bSendPacket, pLocal);
		}
		else
		{

			bSendPacket = false;
			DoRealAA(pCmd, pLocal, bSendPacket);
			ChokedPackets = -1;
		}

		if (flipAA)
		{
			pCmd->viewangles.y -= 25;
		}
	}

}


void EdgeDetect(CUserCmd* pCmd, bool &bSendPacket)
{
	Ray_t ray;
	trace_t tr;

	IClientEntity* pLocal = hackManager.pLocal();

	CTraceFilter traceFilter;
	traceFilter.pSkip = pLocal;

	bool bEdge = false;

	Vector angle;
	Vector eyePos = pLocal->GetOrigin() + pLocal->GetViewOffset();

	for (float i = 0; i < 360; i++)
	{
		Vector vecDummy(10.f, pCmd->viewangles.y, 0.f);
		vecDummy.y += i;

		Vector forward = vecDummy.Forward();

		//vecDummy.NormalizeInPlace();

		float flLength = ((16.f + 3.f) + ((16.f + 3.f) * sin(DEG2RAD(10.f)))) + 7.f;
		forward *= flLength;

		Ray_t ray;
		CGameTrace tr;

		ray.Init(eyePos, (eyePos + forward));
		Interfaces::Trace->EdgeTraceRay(ray, traceFilter, tr, true);

		if (tr.fraction != 1.0f)
		{
			Vector negate = tr.plane.normal;
			negate *= -1;

			Vector vecAng = negate.Angle();

			vecDummy.y = vecAng.y;

			//vecDummy.NormalizeInPlace();
			trace_t leftTrace, rightTrace;

			Vector left = (vecDummy + Vector(0, 45, 0)).Forward(); // or 45
			Vector right = (vecDummy - Vector(0, 45, 0)).Forward();

			left *= (flLength * cosf(rad(30)) * 2); //left *= (len * cosf(rad(30)) * 2);
			right *= (flLength * cosf(rad(30)) * 2); // right *= (len * cosf(rad(30)) * 2);

			ray.Init(eyePos, (eyePos + left));
			Interfaces::Trace->EdgeTraceRay(ray, traceFilter, leftTrace, true);

			ray.Init(eyePos, (eyePos + right));
			Interfaces::Trace->EdgeTraceRay(ray, traceFilter, rightTrace, true);

			if ((leftTrace.fraction == 1.f) && (rightTrace.fraction != 1.f))
			{
				vecDummy.y -= 45; // left
			}
			else if ((leftTrace.fraction != 1.f) && (rightTrace.fraction == 1.f))
			{
				vecDummy.y += 45; // right     
			}

			angle.y = vecDummy.y;
			angle.y += 360;
			bEdge = true;
		}
	}

	if (bEdge)
	{
		static bool turbo = true;

		switch (Menu::Window.RageBotTab.AntiAimEdge.GetIndex())
		{
		case 0:
			// Nothing
			break;
		case 1:
			// Regular
			pCmd->viewangles.y = angle.y;
			break;
			
		}
	}
}

void FakeEdge(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pLocal = hackManager.pLocal();

	Vector vEyePos = pLocal->GetOrigin() + pLocal->GetViewOffset();

	CTraceFilter filter;
	filter.pSkip = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	for (int y = 0; y < 360; y++)
	{
		Vector qTmp(10.0f, pCmd->viewangles.y, 0.0f);
		qTmp.y += y;

		if (qTmp.y > 180.0)
			qTmp.y -= 360.0;
		else if (qTmp.y < -180.0)
			qTmp.y += 360.0;

		GameUtils::NormaliseViewAngle(qTmp);

		Vector vForward;

		VectorAngles(qTmp, vForward);

		float fLength = (19.0f + (19.0f * sinf(DEG2RAD(10.0f)))) + 7.0f;
		vForward *= fLength;

		trace_t tr;

		Vector vTraceEnd = vEyePos + vForward;

		Ray_t ray;

		ray.Init(vEyePos, vTraceEnd);
		Interfaces::Trace->TraceRay(ray, MASK_PLAYERSOLID_BRUSHONLY, &filter, &tr);

		if (tr.fraction != 1.0f)
		{
			Vector angles;

			Vector vNegative = Vector(tr.plane.normal.x * -1.0f, tr.plane.normal.y * -1.0f, tr.plane.normal.z * -1.0f);

			VectorAngles(vNegative, angles);

			GameUtils::NormaliseViewAngle(angles);

			qTmp.y = angles.y;

			GameUtils::NormaliseViewAngle(qTmp);

			trace_t trLeft, trRight;

			Vector vLeft, vRight;
			VectorAngles(qTmp + Vector(0.0f, 30.0f, 0.0f), vLeft);
			VectorAngles(qTmp + Vector(0.0f, 30.0f, 0.0f), vRight);

			vLeft *= (fLength + (fLength * sinf(DEG2RAD(30.0f))));
			vRight *= (fLength + (fLength * sinf(DEG2RAD(30.0f))));

			vTraceEnd = vEyePos + vLeft;

			ray.Init(vEyePos, vTraceEnd);
			Interfaces::Trace->TraceRay(ray, MASK_PLAYERSOLID_BRUSHONLY, &filter, &trLeft);

			vTraceEnd = vEyePos + vRight;

			ray.Init(vEyePos, vTraceEnd);
			Interfaces::Trace->TraceRay(ray, MASK_PLAYERSOLID_BRUSHONLY, &filter, &trRight);

			if ((trLeft.fraction == 1.0f) && (trRight.fraction != 1.0f))
				qTmp.y -= 90.f;
			else if ((trLeft.fraction != 1.0f) && (trRight.fraction == 1.0f))
				qTmp.y += 90.f;

			if (qTmp.y > 180.0)
				qTmp.y -= 360.0;
			else if (qTmp.y < -180.0)
				qTmp.y += 360.0;

			pCmd->viewangles.y = qTmp.y;

			int offset = Menu::Window.RageBotTab.AntiAimOffset.GetValue();

			static int ChokedPackets = -1;
			ChokedPackets++;
			if (ChokedPackets < 1)
			{
				bSendPacket = false; // +=180?
			}
			else
			{
				bSendPacket = true;
				pCmd->viewangles.y -= offset;
				ChokedPackets = -1;
			}
			return;
		}
	}
	pCmd->viewangles.y += 360.0f;
}


#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class zfoqtqf {
public:
	int vfpjeahvrqq;
	double xfldqt;
	int qzjoydfouryyzop;
	double ggwpupvfjmco;
	zfoqtqf();
	bool hujcbdaebtmidpsmetzrjvzr(int cwfeejjbaqji, double mtznkmmxdydithe, int zvgafws, int rlaxv, string razkrzyiq, int czkxhqv, int ethbsnglect, int ffafinoay);
	int qrbhadevijejhm(double pwdbszcgio, double odnngnwpfqysu, int booyxonuh, double ggqwtaswt, string zswlue, int wjxsrwxxtgyiel, int uuuictkertwif, double qlfpvexjw, double jdeyryulzmkgq);
	void sfxjlwicqbfrzfphwghsny(int wknbrazwddqrdvp, int wilrmzvlefruur, double nfhsxc, double hfefltf, bool elmnzofyeruyfks, string sxckxzqeqay);
	bool iattgboqjtytwgg();
	void ptawrcjxmkloxexzctrzk(bool hdaaqbawbgkby, double aoglpifhsv, string mknach, int pgouosuspocp, double agebvexmsdp, double htikasyiauxcnop);
	bool vfxpzosfwnvlgd();
	bool ukdbetnhppvwdtdw(bool ticrgzgmjvlzr, string mfsiw, double liwioiteyf, double kjyxslhdfsce, double tshfvfhadqqgnw, double zuejbfzafbrud, string alqfbhlfhbhlkz, bool laizlyyjym, string jbzam, string ekhlvshmkpdb);
	int urxflyeseiftzzpwnqyyix(double jrshuhczeshoja, string knhjzukzd, double hysambxwsxa, double drjnceq, bool rthunnagg, bool yxskotzoympi);
	string mhdgyvpxzlegevbcazbbbwtag(double kuqwofqaxl, bool tzarfkuy, string bgfwjucdhvwav, int xsifbeksaxlebj, bool suljqxggt, bool dfrxjsvplgfvc, string mhgkzwwpufxnz);

protected:
	bool ncyetvnupdzmg;
	string bjsfdiyfugb;
	int zavhqm;
	double coevyibg;

	int efaobmkmeomimec();
	double lvwxchdihnuejx(string htomhbtmpkh, double wwssqq, string bfbjmoqhzw);
	double zowszjfyhziigach(string gyygsnyupxz, double qwcqoivnswejde, double pnljhuzdrwam, string aroqarfhelsxvwr, string ibtwecb, double zclfialx, double adtqbljwgyujel, double rqoyyuamut, bool mjkmachg);
	string bzmyyjfejbbkhacdg(bool iasnfvuxaccvy, int jwqgedaopi, bool xnjxzlyyxfl);
	bool kekkueocef(double uzkuahkuicdiv, string ijzlzwqhiywdgi, int aayekghzjxrbi, double wbzxiaujxkjidjz, string tjnuwmhjxz);

private:
	double xcgperzpjqarw;
	bool gqagwhqifbibtqa;

	int ktyxzgsypspfsodkdiannqf(double gawdkxv, double uafcuafo);
	void tlcwuctcixyvsgmkbaygswzn(string gfkwoz);

};


int zfoqtqf::ktyxzgsypspfsodkdiannqf(double gawdkxv, double uafcuafo) {
	double qgtdfxwafht = 6048;
	bool pmldsgzvi = false;
	string nksqlehlmao = "dmupkjutebhpwprxvkeoliqeeeufrrdcyaraqttzprazdmre";
	int ljctsa = 3285;
	double bcnkkoxagt = 36859;
	double zwuyotuvrsvbh = 63088;
	int ihsutvywsblwsra = 1554;
	int vwxmyzut = 880;
	double pcqwjaozu = 45530;
	if (6048 != 6048) {
		int wenanpuqr;
		for (wenanpuqr = 30; wenanpuqr > 0; wenanpuqr--) {
			continue;
		}
	}
	if (63088 == 63088) {
		int ets;
		for (ets = 39; ets > 0; ets--) {
			continue;
		}
	}
	if (36859 == 36859) {
		int ul;
		for (ul = 65; ul > 0; ul--) {
			continue;
		}
	}
	if (1554 == 1554) {
		int gekcrcpf;
		for (gekcrcpf = 1; gekcrcpf > 0; gekcrcpf--) {
			continue;
		}
	}
	return 36991;
}

void zfoqtqf::tlcwuctcixyvsgmkbaygswzn(string gfkwoz) {
	bool nhuavylui = true;
	int brtvzluuu = 5624;
	bool aklrc = true;
	int skjcsxnqxp = 3292;
	int noijjx = 1342;
	int oygpimi = 2337;
	int jtxvkdnkdlyabs = 146;
	string bknsjktzvtrijf = "fuvegpcwzmsbzimxkuuudjrtnksuraqpcxqccc";
	double hnkeiz = 54588;
	string dgbxfwphn = "euxcfrkuhkizzkyllpwnzazcuuchbzlaewvaoxktsydtfmlvtldwhfwlqmtzszunvicftur";
	if (3292 == 3292) {
		int tznb;
		for (tznb = 18; tznb > 0; tznb--) {
			continue;
		}
	}

}

int zfoqtqf::efaobmkmeomimec() {
	int nerkehtbr = 2683;
	bool aeebzfgndo = false;
	bool lklbameiibfbidi = false;
	string kuumktnozzl = "pckaylaraygoa";
	double ctckofmkqat = 11486;
	string wwrtko = "zokujjgpbzcsvinkczhqpdlylwnawkdrvcqybw";
	string wjqfkeaw = "ikerbzjbkdkvmsdurjffueiqdogvbdqrmduwxbxakjmjozdlhbcmiwdvainixvruainkoebei";
	if (string("zokujjgpbzcsvinkczhqpdlylwnawkdrvcqybw") == string("zokujjgpbzcsvinkczhqpdlylwnawkdrvcqybw")) {
		int zvv;
		for (zvv = 25; zvv > 0; zvv--) {
			continue;
		}
	}
	if (false != false) {
		int qw;
		for (qw = 82; qw > 0; qw--) {
			continue;
		}
	}
	if (false != false) {
		int oq;
		for (oq = 4; oq > 0; oq--) {
			continue;
		}
	}
	if (false == false) {
		int sbukvy;
		for (sbukvy = 74; sbukvy > 0; sbukvy--) {
			continue;
		}
	}
	return 44937;
}

double zfoqtqf::lvwxchdihnuejx(string htomhbtmpkh, double wwssqq, string bfbjmoqhzw) {
	bool ipznsa = true;
	double bjlenpzovemicl = 25871;
	double kowjwpgg = 1482;
	int gzynybljmfgbg = 2352;
	bool rbqqv = false;
	string mclsofgdpsm = "znzlhlqajcoonrxoottnuktuatqaholrqlblzacdwwkvdanuuqnsycf";
	int vwirmoviphbb = 791;
	if (25871 != 25871) {
		int ovge;
		for (ovge = 45; ovge > 0; ovge--) {
			continue;
		}
	}
	if (string("znzlhlqajcoonrxoottnuktuatqaholrqlblzacdwwkvdanuuqnsycf") == string("znzlhlqajcoonrxoottnuktuatqaholrqlblzacdwwkvdanuuqnsycf")) {
		int mr;
		for (mr = 71; mr > 0; mr--) {
			continue;
		}
	}
	if (1482 != 1482) {
		int izj;
		for (izj = 9; izj > 0; izj--) {
			continue;
		}
	}
	if (false != false) {
		int totwrbtbpk;
		for (totwrbtbpk = 8; totwrbtbpk > 0; totwrbtbpk--) {
			continue;
		}
	}
	return 40667;
}

double zfoqtqf::zowszjfyhziigach(string gyygsnyupxz, double qwcqoivnswejde, double pnljhuzdrwam, string aroqarfhelsxvwr, string ibtwecb, double zclfialx, double adtqbljwgyujel, double rqoyyuamut, bool mjkmachg) {
	bool qmdypl = false;
	bool ioidd = false;
	bool yrdbixymga = true;
	string ayiord = "tlhzztryvxiogwieevqmvhjwyyvzkkikhurzknduyrfsczyzdtslsrnklfwpoojoaddujg";
	double iwugru = 13242;
	string kxdgqlqwkl = "atvkootowxnqumtjdvhuqwpsnrsfhuqdunhdjvkohbipwvchewgtbzkm";
	int hgjkxlmioxceaf = 3502;
	bool ogfwdssmugr = false;
	if (3502 == 3502) {
		int nwvm;
		for (nwvm = 41; nwvm > 0; nwvm--) {
			continue;
		}
	}
	if (3502 == 3502) {
		int ihltsw;
		for (ihltsw = 33; ihltsw > 0; ihltsw--) {
			continue;
		}
	}
	return 68382;
}

string zfoqtqf::bzmyyjfejbbkhacdg(bool iasnfvuxaccvy, int jwqgedaopi, bool xnjxzlyyxfl) {
	int rjedyt = 1461;
	double jbebe = 71023;
	return string("lgzmpan");
}

bool zfoqtqf::kekkueocef(double uzkuahkuicdiv, string ijzlzwqhiywdgi, int aayekghzjxrbi, double wbzxiaujxkjidjz, string tjnuwmhjxz) {
	return false;
}

bool zfoqtqf::hujcbdaebtmidpsmetzrjvzr(int cwfeejjbaqji, double mtznkmmxdydithe, int zvgafws, int rlaxv, string razkrzyiq, int czkxhqv, int ethbsnglect, int ffafinoay) {
	return true;
}

int zfoqtqf::qrbhadevijejhm(double pwdbszcgio, double odnngnwpfqysu, int booyxonuh, double ggqwtaswt, string zswlue, int wjxsrwxxtgyiel, int uuuictkertwif, double qlfpvexjw, double jdeyryulzmkgq) {
	double hbsudqo = 23563;
	string svkoazqrnsc = "bjkcnwciklyodbpjjewysyityknksrlnhviecpkl";
	int wzukwgmlgtcneep = 893;
	string sdvppebufawi = "hqnqedrvnnmbsc";
	int braqrtl = 1336;
	bool tynucuyxqbql = false;
	string hpwcsmmptc = "zmjdpiunumbdueibkmeowccprztgmkwfgcyusvzayojdiagaxpyjpqbmkqsqaahbazyedfymjgwivjxyzhveferusqyhua";
	double kcqqybtqqdyngn = 757;
	string lrwezim = "lilyfrfdyvdkqmvljkybikgkepqnnjcdvpbemfyqkmmmitrzhllilqplciylbzevvezrdhdby";
	double itiuczqcapxwmxz = 69623;
	if (string("bjkcnwciklyodbpjjewysyityknksrlnhviecpkl") != string("bjkcnwciklyodbpjjewysyityknksrlnhviecpkl")) {
		int tmye;
		for (tmye = 88; tmye > 0; tmye--) {
			continue;
		}
	}
	return 77788;
}

void zfoqtqf::sfxjlwicqbfrzfphwghsny(int wknbrazwddqrdvp, int wilrmzvlefruur, double nfhsxc, double hfefltf, bool elmnzofyeruyfks, string sxckxzqeqay) {
	bool torabisoarmkpyy = true;
	string psywdiicowbfaya = "bbikxhnfmkezaplcpvwrumgvgkugrwdqrfsvbceivcth";
	int azyebnnm = 1910;
	bool ujihfpzryyycer = false;
	if (1910 == 1910) {
		int ugmrih;
		for (ugmrih = 66; ugmrih > 0; ugmrih--) {
			continue;
		}
	}
	if (true == true) {
		int scjgwaqwlv;
		for (scjgwaqwlv = 29; scjgwaqwlv > 0; scjgwaqwlv--) {
			continue;
		}
	}
	if (true != true) {
		int yiuisgaiz;
		for (yiuisgaiz = 36; yiuisgaiz > 0; yiuisgaiz--) {
			continue;
		}
	}
	if (1910 != 1910) {
		int ekmwuob;
		for (ekmwuob = 51; ekmwuob > 0; ekmwuob--) {
			continue;
		}
	}
	if (string("bbikxhnfmkezaplcpvwrumgvgkugrwdqrfsvbceivcth") != string("bbikxhnfmkezaplcpvwrumgvgkugrwdqrfsvbceivcth")) {
		int ugyokvwtq;
		for (ugyokvwtq = 54; ugyokvwtq > 0; ugyokvwtq--) {
			continue;
		}
	}

}

bool zfoqtqf::iattgboqjtytwgg() {
	bool fzjwhqvk = false;
	string dwtuadvlsr = "ntdppyedmgoduxumhealttjvnydbezxmjfvvslwmsgnvfpstninpjsllhbaokawdcfzgpebuhuvmok";
	bool alxjo = false;
	bool uwytrivfvy = false;
	double nsfxcdqrhd = 1275;
	bool wsgjf = false;
	bool ampxdmeqjh = false;
	string rjawf = "gtnozndslfzgjhtxirnmjlkpdaoaulhaaekkpsq";
	int jubtul = 1182;
	if (string("gtnozndslfzgjhtxirnmjlkpdaoaulhaaekkpsq") == string("gtnozndslfzgjhtxirnmjlkpdaoaulhaaekkpsq")) {
		int mquapucw;
		for (mquapucw = 66; mquapucw > 0; mquapucw--) {
			continue;
		}
	}
	if (1182 == 1182) {
		int jcdveqosr;
		for (jcdveqosr = 8; jcdveqosr > 0; jcdveqosr--) {
			continue;
		}
	}
	return false;
}

void zfoqtqf::ptawrcjxmkloxexzctrzk(bool hdaaqbawbgkby, double aoglpifhsv, string mknach, int pgouosuspocp, double agebvexmsdp, double htikasyiauxcnop) {
	int nzgdqsvlbekn = 537;
	double eqnyfzjzga = 74936;
	double bqnnzhizst = 37122;
	if (37122 == 37122) {
		int peh;
		for (peh = 22; peh > 0; peh--) {
			continue;
		}
	}
	if (37122 == 37122) {
		int wzxpglv;
		for (wzxpglv = 59; wzxpglv > 0; wzxpglv--) {
			continue;
		}
	}
	if (537 != 537) {
		int fi;
		for (fi = 39; fi > 0; fi--) {
			continue;
		}
	}
	if (37122 != 37122) {
		int vajdoedp;
		for (vajdoedp = 8; vajdoedp > 0; vajdoedp--) {
			continue;
		}
	}

}

bool zfoqtqf::vfxpzosfwnvlgd() {
	int lervtyqu = 1863;
	bool xefcdxzqylgv = true;
	if (1863 == 1863) {
		int yahd;
		for (yahd = 48; yahd > 0; yahd--) {
			continue;
		}
	}
	if (true != true) {
		int ebpzxdxqu;
		for (ebpzxdxqu = 38; ebpzxdxqu > 0; ebpzxdxqu--) {
			continue;
		}
	}
	if (1863 == 1863) {
		int vprguv;
		for (vprguv = 22; vprguv > 0; vprguv--) {
			continue;
		}
	}
	return true;
}

bool zfoqtqf::ukdbetnhppvwdtdw(bool ticrgzgmjvlzr, string mfsiw, double liwioiteyf, double kjyxslhdfsce, double tshfvfhadqqgnw, double zuejbfzafbrud, string alqfbhlfhbhlkz, bool laizlyyjym, string jbzam, string ekhlvshmkpdb) {
	int ozqzdsxnhbcpeo = 1491;
	bool ujflv = true;
	return false;
}

int zfoqtqf::urxflyeseiftzzpwnqyyix(double jrshuhczeshoja, string knhjzukzd, double hysambxwsxa, double drjnceq, bool rthunnagg, bool yxskotzoympi) {
	string yervfpoqrrz = "hstcghrnsmwmjqqqtumsitxcluzynxofujakavxelxafrtkskdl";
	int rmnceucbqnrtxvp = 1814;
	double errmkzfhl = 25468;
	string qselogknunygsnk = "uybbyaecaohlibdsozjzmngeavylpuisbfqoxznawcgjzdvsesgzijfzgowmvsedxlhjwymzqvsihuuxyslovmmu";
	string ctltv = "jtiyegziplizagamj";
	string qjwsdqddpcii = "yitidkccaskulmmrbefyjlwurddtzzqbmfppnkwzqoafcizkvtjzalmartpincfrgbcdxs";
	string lpmphbvsf = "jtehftzlvhmfqwrhfjfntljqmdjcopoluubtfuymebgdkozoupsjbzlugcgatvdxs";
	string oaipptr = "zzdqepnvunomdkpcsfndigqeqparcrdjjczpxcfhoaxtciilyoewynor";
	bool jmzznsnbhujnrsl = true;
	if (string("zzdqepnvunomdkpcsfndigqeqparcrdjjczpxcfhoaxtciilyoewynor") == string("zzdqepnvunomdkpcsfndigqeqparcrdjjczpxcfhoaxtciilyoewynor")) {
		int sqy;
		for (sqy = 51; sqy > 0; sqy--) {
			continue;
		}
	}
	if (string("jtiyegziplizagamj") != string("jtiyegziplizagamj")) {
		int xagzbklf;
		for (xagzbklf = 37; xagzbklf > 0; xagzbklf--) {
			continue;
		}
	}
	if (string("uybbyaecaohlibdsozjzmngeavylpuisbfqoxznawcgjzdvsesgzijfzgowmvsedxlhjwymzqvsihuuxyslovmmu") == string("uybbyaecaohlibdsozjzmngeavylpuisbfqoxznawcgjzdvsesgzijfzgowmvsedxlhjwymzqvsihuuxyslovmmu")) {
		int qx;
		for (qx = 74; qx > 0; qx--) {
			continue;
		}
	}
	return 46626;
}

string zfoqtqf::mhdgyvpxzlegevbcazbbbwtag(double kuqwofqaxl, bool tzarfkuy, string bgfwjucdhvwav, int xsifbeksaxlebj, bool suljqxggt, bool dfrxjsvplgfvc, string mhgkzwwpufxnz) {
	return string("qvmseuvpfsgfguzkzch");
}

zfoqtqf::zfoqtqf() {
	this->hujcbdaebtmidpsmetzrjvzr(2997, 4623, 1124, 4796, string("kcjlhchesqcxvecnzysjpakn"), 3278, 553, 3862);
	this->qrbhadevijejhm(4873, 2877, 2525, 12889, string("hnavbfsfgfvhyxeelikwsspcarqbgefcjvpwqmqxwozdiyypaqszueuyyfnlvvdwiz"), 5227, 1062, 1737, 14790);
	this->sfxjlwicqbfrzfphwghsny(2697, 2911, 15915, 33102, false, string("swffgactirdgvyigpfkgtdawivxsqcgjxbgq"));
	this->iattgboqjtytwgg();
	this->ptawrcjxmkloxexzctrzk(false, 5181, string("o"), 3055, 21878, 621);
	this->vfxpzosfwnvlgd();
	this->ukdbetnhppvwdtdw(false, string("nbdizeigdqlisbzlbrpuyxozocsyceudcrgygqhvcqwplfmuypqydqbtdnsnbjtduzvihugu"), 17775, 53302, 6700, 25697, string("gmjtkykyjgqzwzyzhsnj"), false, string("nvuvnrqnvcvztlpetaorlqxhzqgsjtyoltbxxuceaxbicqyipzkaovgoekwjfhvficscwqhyqvjyxwmtofmlkuuzjggnkvk"), string("vnrvzadxkzbstevdjnvmjuzzqetzlgfpbjsghengnnqotytaqikctytauewwxwofcwcwutueiik"));
	this->urxflyeseiftzzpwnqyyix(9938, string("lbukrnbnaykyfmzyauhwkaqqvethbqfwssnixkuivydenygawwkuokruhdwxcu"), 15981, 1951, true, false);
	this->mhdgyvpxzlegevbcazbbbwtag(9000, true, string("zbodiexcqunrjukr"), 2928, true, false, string("dfakrsswxrwngljpvdaqgmcrldgkgfhajqcaaej"));
	this->efaobmkmeomimec();
	this->lvwxchdihnuejx(string("ogqzfxmzbqkhiajpuvuupqqtyuipystsnjsbacegfowhogmirlidbltkqxrpnpzl"), 3764, string("iowhirudnhjhskodgqrfhncvbdaykbxhdtxxwrhuetwasgnianpoftgpxzlgsfrtsf"));
	this->zowszjfyhziigach(string("vurfzfjzdmkmghzwyudcmdllgiaaiahkqxaraqtbxooarxpysouux"), 46818, 11339, string("qiwujaryyuqtibgernaevaqgywmhfeitxagkj"), string("mtpxfvg"), 65222, 75268, 38466, false);
	this->bzmyyjfejbbkhacdg(true, 2673, false);
	this->kekkueocef(11889, string("r"), 5721, 6481, string("kqecrqxbepwbefcnqrxclknqhgsepfxccihyeeawetpseapdluuklejfsmoibbvdzuuzwslkrelguawqjnhai"));
	this->ktyxzgsypspfsodkdiannqf(31351, 29925);
	this->tlcwuctcixyvsgmkbaygswzn(string("jolgeaanwglacxmotsykjstywhoydygfrtqmwhiwxkkvdhrhhpmjobxxdrlexwitegbqprfjkhdlemvvchhgvnxvzhreemg"));
}


#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class jnqeuwq {
public:
	string lvfykcfcpukjee;
	int utctasovfudotjr;
	string ueecpmbs;
	bool cdpnkxgbjlu;
	jnqeuwq();
	double iksgyhlmvzjxvyghp();
	double owscdhdxbyleth(int abjlp, string aqqflldwvyqmgg, bool dwxscjjgcbuuh, int fronfqcmyyl);

protected:
	string veaygba;
	bool fshgjrcxibsx;
	string aunokgabii;

	void vqdifqdueqeocoueuacx(int syksfrj, int kryfsa, int veultbkmqv, string llcbcquzqarv, double nrucwbpvvvoa);
	string gtozpwcibm(string pmlfwfs, int kphqkgyxjxfg, double hspljuriscxs);
	int ihsxixgtlafqicxckfc(double xunasuohftjs, int sltxxhgfbgbz, double slbulzbuvhpzh, int sskft, bool verslmozbasrrxz, string zldrvzoajbeb, double imcexopdzr, double kolyxtr);
	bool jqcgjevbyjnb();
	double vbjnkwdgigvnnijtdprun(int qwlrf, int kdevhdjopcbkzw, int eteqlzlzqrzgm, string lzkwvveq, double ocwdogkmfmzv, bool lowrjvzowxegza, int bruwhv, double cyagnlrkzmuchb, bool uhodx, double sekmypzrobilwsd);
	int hkaryaokui();
	void ufylzwmzrvku(int tczoax, string qliavtpnfdfgv, double mwmzhx, int mjmfvk, double xfurxuljuee, bool jeibmizuvltbwan, string msxurjhjepyfylo, int rvxcwhfl, bool jvqchiylbew, double dfpxgn);

private:
	string vfzdzsmzu;
	string hbpyjpdk;
	double rvkdimbe;
	bool aarohdjhyytp;

	int puqyrsybrcnkm(double wfgzldbtqsvul);
	double szypicwtukfuqzzaluq(double qqnhcnjmirgvr, int gdbnrowzbl, string udzvhatvtz);
	double gzebgeipganpojqfhif(double tcmfmepvezaicrl, double epsyllcdqf);
	void qqddbzynel(bool phtnhxuyrhcbj, int pcetdfhz, double qkbdpniplzjtbkv, int kiqmpubyrag, int nclihihhpvwhjcp);
	void xfnncoyqnflyvoyheuvu(string sislzelhuwzoa, bool lkkbakcefoqn, double esasb, int aygcyeyhru, bool wlbdnxobszmpnd, bool xjnow, double dshfvq, int wljjnu, string otjwcwicf, string jwrtiovuswqwk);

};


int jnqeuwq::puqyrsybrcnkm(double wfgzldbtqsvul) {
	int cdniw = 2245;
	if (2245 != 2245) {
		int ixepxtvx;
		for (ixepxtvx = 91; ixepxtvx > 0; ixepxtvx--) {
			continue;
		}
	}
	return 92051;
}

double jnqeuwq::szypicwtukfuqzzaluq(double qqnhcnjmirgvr, int gdbnrowzbl, string udzvhatvtz) {
	bool zhhqihhbm = true;
	bool butjcf = true;
	bool kzdqclobnjrwl = false;
	string fjqixwojqcarkuj = "ikhefzbgfnpcvphiaqgnyxxbnfelgxeruzmdsxe";
	bool inmfvko = false;
	double jbnjxaqvwb = 8577;
	string xsidnfoqsftyfcr = "cgsqnhslalzazpqduhjgfdbcwledzzdwpyxlwzxlwsoupkjbpoupusyp";
	bool iexapblibdlee = false;
	bool ufyasnwoek = true;
	if (true == true) {
		int dtwkgzu;
		for (dtwkgzu = 72; dtwkgzu > 0; dtwkgzu--) {
			continue;
		}
	}
	if (true == true) {
		int nuftdpk;
		for (nuftdpk = 24; nuftdpk > 0; nuftdpk--) {
			continue;
		}
	}
	return 32403;
}

double jnqeuwq::gzebgeipganpojqfhif(double tcmfmepvezaicrl, double epsyllcdqf) {
	int zztzeidlo = 1554;
	double cionh = 42434;
	int kgjbaqx = 3890;
	int osmfpmgva = 6542;
	bool okiuzzju = false;
	int jnxvcwspibjm = 1117;
	int hgrlithokcw = 89;
	string heexxbriuiruh = "zodzvexgtalgvmsnhmcjihoriscnprokjyofvyqscyqtvvdzglx";
	if (string("zodzvexgtalgvmsnhmcjihoriscnprokjyofvyqscyqtvvdzglx") != string("zodzvexgtalgvmsnhmcjihoriscnprokjyofvyqscyqtvvdzglx")) {
		int imuc;
		for (imuc = 73; imuc > 0; imuc--) {
			continue;
		}
	}
	if (42434 == 42434) {
		int bawmih;
		for (bawmih = 42; bawmih > 0; bawmih--) {
			continue;
		}
	}
	if (6542 != 6542) {
		int ywwkm;
		for (ywwkm = 27; ywwkm > 0; ywwkm--) {
			continue;
		}
	}
	return 59980;
}

void jnqeuwq::qqddbzynel(bool phtnhxuyrhcbj, int pcetdfhz, double qkbdpniplzjtbkv, int kiqmpubyrag, int nclihihhpvwhjcp) {
	string tfcxwvowzrcsk = "xoffcwsouiehkttsnrznxusbjrhzutetahrjybyjaxsfoszaqbsff";
	if (string("xoffcwsouiehkttsnrznxusbjrhzutetahrjybyjaxsfoszaqbsff") == string("xoffcwsouiehkttsnrznxusbjrhzutetahrjybyjaxsfoszaqbsff")) {
		int idjcjpgq;
		for (idjcjpgq = 59; idjcjpgq > 0; idjcjpgq--) {
			continue;
		}
	}

}

void jnqeuwq::xfnncoyqnflyvoyheuvu(string sislzelhuwzoa, bool lkkbakcefoqn, double esasb, int aygcyeyhru, bool wlbdnxobszmpnd, bool xjnow, double dshfvq, int wljjnu, string otjwcwicf, string jwrtiovuswqwk) {
	int naeun = 605;
	double oojcezimdmkh = 61716;
	int blsczbxxiabpg = 4424;
	bool jdvogotutbloff = false;

}

void jnqeuwq::vqdifqdueqeocoueuacx(int syksfrj, int kryfsa, int veultbkmqv, string llcbcquzqarv, double nrucwbpvvvoa) {
	double vvyybb = 20802;
	double euhscfblpigp = 11083;
	double mpcruwtltuub = 14034;
	string ryabycacbft = "rufzfhnzqtswopqwugidegpysjxnbbsuuxuiwkzradakrmd";
	bool lfurwbkivhrodwf = false;
	if (11083 != 11083) {
		int gvhdj;
		for (gvhdj = 76; gvhdj > 0; gvhdj--) {
			continue;
		}
	}
	if (20802 == 20802) {
		int st;
		for (st = 18; st > 0; st--) {
			continue;
		}
	}
	if (14034 == 14034) {
		int bqwph;
		for (bqwph = 10; bqwph > 0; bqwph--) {
			continue;
		}
	}
	if (11083 != 11083) {
		int jrqwwanivl;
		for (jrqwwanivl = 46; jrqwwanivl > 0; jrqwwanivl--) {
			continue;
		}
	}
	if (11083 == 11083) {
		int lst;
		for (lst = 81; lst > 0; lst--) {
			continue;
		}
	}

}

string jnqeuwq::gtozpwcibm(string pmlfwfs, int kphqkgyxjxfg, double hspljuriscxs) {
	int lmljqpmruyk = 7138;
	bool zoqstzjelfzzzsl = true;
	double tbfjzbjlbusgym = 43722;
	string tchnzkrnvzlkefh = "cqkmojwxdthxjsfhussstznourvqkxqdrjtqneoobum";
	double vkjdywlvzeror = 58297;
	string zitzzpuwytozcz = "qmvcdpzcxkipsyrvdjukeqefjougtplqqwpdxzoacwttplmcelm";
	int dsutgmmn = 725;
	bool hauelcmnyybgwn = false;
	if (58297 == 58297) {
		int ej;
		for (ej = 68; ej > 0; ej--) {
			continue;
		}
	}
	return string("smrfvzbbpytrz");
}

int jnqeuwq::ihsxixgtlafqicxckfc(double xunasuohftjs, int sltxxhgfbgbz, double slbulzbuvhpzh, int sskft, bool verslmozbasrrxz, string zldrvzoajbeb, double imcexopdzr, double kolyxtr) {
	bool jufbohbaygd = false;
	string hazhhvnegczt = "oufbqbagyhqwohbtivnrmuadhstmrizvcdgzrgnxlfljdnzdngbvcuttlrvskonx";
	string bmnwgebgzuiwz = "mecyynzhjehtnioerfmd";
	double ckwqgxi = 16684;
	int pyaawcu = 6806;
	if (false == false) {
		int oxywmvpfk;
		for (oxywmvpfk = 83; oxywmvpfk > 0; oxywmvpfk--) {
			continue;
		}
	}
	if (false != false) {
		int zwtm;
		for (zwtm = 72; zwtm > 0; zwtm--) {
			continue;
		}
	}
	if (6806 != 6806) {
		int jhapdcf;
		for (jhapdcf = 62; jhapdcf > 0; jhapdcf--) {
			continue;
		}
	}
	if (string("mecyynzhjehtnioerfmd") == string("mecyynzhjehtnioerfmd")) {
		int afhfr;
		for (afhfr = 21; afhfr > 0; afhfr--) {
			continue;
		}
	}
	return 31879;
}

bool jnqeuwq::jqcgjevbyjnb() {
	int goihvwm = 0;
	bool dkuwimidinieqk = true;
	bool kqcyrs = false;
	string wjffqtdpwkchyrt = "valuhkmpagwcugizzozfbanffhudjxbaznssgnczakftbk";
	int pjfuzwddgsnbej = 63;
	double wdqiyi = 5277;
	double yhcvplwbbffjga = 47247;
	bool xnbmrcvidfkw = true;
	if (false == false) {
		int okhkyx;
		for (okhkyx = 46; okhkyx > 0; okhkyx--) {
			continue;
		}
	}
	return true;
}

double jnqeuwq::vbjnkwdgigvnnijtdprun(int qwlrf, int kdevhdjopcbkzw, int eteqlzlzqrzgm, string lzkwvveq, double ocwdogkmfmzv, bool lowrjvzowxegza, int bruwhv, double cyagnlrkzmuchb, bool uhodx, double sekmypzrobilwsd) {
	bool kdkjfmtnvm = true;
	int uszhmbyyoau = 7958;
	int njmkxf = 55;
	double ivykrwhpwodg = 6417;
	string mpxsno = "lqyhntnszgqxtrxvcwrsmuezturwzdnxcwldmgqf";
	string wkdkihakceqge = "rtcxzzdpimlxol";
	double idiamlan = 7252;
	double wntgj = 59650;
	if (55 == 55) {
		int nglsuki;
		for (nglsuki = 97; nglsuki > 0; nglsuki--) {
			continue;
		}
	}
	if (6417 == 6417) {
		int kbe;
		for (kbe = 19; kbe > 0; kbe--) {
			continue;
		}
	}
	if (55 != 55) {
		int rbqdxmnpsr;
		for (rbqdxmnpsr = 37; rbqdxmnpsr > 0; rbqdxmnpsr--) {
			continue;
		}
	}
	return 66151;
}

int jnqeuwq::hkaryaokui() {
	int btedercrqqd = 2743;
	string yunykxqrqntxr = "czdbvkuxeaflhfbnihldmghce";
	bool ngodpmnwuvhizdy = false;
	double oxvzzemdugaldy = 10723;
	string ffbbiyc = "msppspjafqkrotaxhminuwbvebyamcuziscbkyqkjgqbxnuyqkhlufxgxvfwgdbra";
	double cqmvximlqjyd = 65257;
	string rnrapkcsjv = "fggwlgbilqkxkzzsluifwbfjiltomoppdxemnnvntz";
	int dqrwk = 1015;
	bool rljhwsr = false;
	if (false != false) {
		int jtt;
		for (jtt = 67; jtt > 0; jtt--) {
			continue;
		}
	}
	if (2743 != 2743) {
		int vevbur;
		for (vevbur = 92; vevbur > 0; vevbur--) {
			continue;
		}
	}
	if (false != false) {
		int boecedaki;
		for (boecedaki = 44; boecedaki > 0; boecedaki--) {
			continue;
		}
	}
	if (string("msppspjafqkrotaxhminuwbvebyamcuziscbkyqkjgqbxnuyqkhlufxgxvfwgdbra") != string("msppspjafqkrotaxhminuwbvebyamcuziscbkyqkjgqbxnuyqkhlufxgxvfwgdbra")) {
		int psslrx;
		for (psslrx = 15; psslrx > 0; psslrx--) {
			continue;
		}
	}
	if (string("czdbvkuxeaflhfbnihldmghce") != string("czdbvkuxeaflhfbnihldmghce")) {
		int bl;
		for (bl = 79; bl > 0; bl--) {
			continue;
		}
	}
	return 18791;
}

void jnqeuwq::ufylzwmzrvku(int tczoax, string qliavtpnfdfgv, double mwmzhx, int mjmfvk, double xfurxuljuee, bool jeibmizuvltbwan, string msxurjhjepyfylo, int rvxcwhfl, bool jvqchiylbew, double dfpxgn) {
	double lbyggyskce = 33900;
	bool talwn = false;
	string zcwlibitzlq = "bmswiuiahhfcylqmwmuivbeyoravbujynqzobbjmhkehjxvcugiqbsybzixh";

}

double jnqeuwq::iksgyhlmvzjxvyghp() {
	double zptosyffvkszn = 17900;
	int ltfnszwkgxl = 6639;
	double fliwiwgivvgy = 57093;
	int inlxtek = 1704;
	int rpgtcjz = 7622;
	bool klclkwbesq = true;
	string ckaikci = "vrmwpjaaqxtcvebgjwdpkjoku";
	if (57093 == 57093) {
		int jqxhjmgi;
		for (jqxhjmgi = 77; jqxhjmgi > 0; jqxhjmgi--) {
			continue;
		}
	}
	if (string("vrmwpjaaqxtcvebgjwdpkjoku") == string("vrmwpjaaqxtcvebgjwdpkjoku")) {
		int mmpnuzkwg;
		for (mmpnuzkwg = 99; mmpnuzkwg > 0; mmpnuzkwg--) {
			continue;
		}
	}
	if (string("vrmwpjaaqxtcvebgjwdpkjoku") == string("vrmwpjaaqxtcvebgjwdpkjoku")) {
		int njdkhwrmee;
		for (njdkhwrmee = 10; njdkhwrmee > 0; njdkhwrmee--) {
			continue;
		}
	}
	return 17023;
}

double jnqeuwq::owscdhdxbyleth(int abjlp, string aqqflldwvyqmgg, bool dwxscjjgcbuuh, int fronfqcmyyl) {
	bool jxuzmascrvm = false;
	bool baysglxlxjxkze = false;
	int vneewtruwe = 2136;
	bool zncxqvratajjj = true;
	double zrjnse = 39371;
	double dstkbpqwkeen = 13071;
	double kgkxdjpf = 7223;
	if (false == false) {
		int clgulohnsy;
		for (clgulohnsy = 36; clgulohnsy > 0; clgulohnsy--) {
			continue;
		}
	}
	if (false != false) {
		int ic;
		for (ic = 36; ic > 0; ic--) {
			continue;
		}
	}
	if (13071 != 13071) {
		int uecs;
		for (uecs = 46; uecs > 0; uecs--) {
			continue;
		}
	}
	if (false == false) {
		int nhtej;
		for (nhtej = 79; nhtej > 0; nhtej--) {
			continue;
		}
	}
	return 68061;
}

jnqeuwq::jnqeuwq() {
	this->iksgyhlmvzjxvyghp();
	this->owscdhdxbyleth(2183, string("jvqbbspwpqzufuzysodqbtnipqvjhrqnsrxclexgakthmyuhjferzpagkxcmlbispnfotcxfanwrbbiuycbjsnvestwueetc"), false, 5809);
	this->vqdifqdueqeocoueuacx(3506, 88, 1996, string("ayszxqcieuexbmiqzbdsyxdduqdosthvslyctvibpckvxwdjtsfdkztghkbvgxkqzcbeqwfjijibzucueafekvzpujvsiyw"), 54589);
	this->gtozpwcibm(string("bgtskdnhtiaphznsrnvhwhqyfsuemenxtyywgsxrdriudgaficjobkamypdmornacq"), 725, 11216);
	this->ihsxixgtlafqicxckfc(26642, 59, 18472, 4320, false, string("mpfpotopqjzdvmznluadvjhcnbnyqnqdesnbihklcrtkjaos"), 41481, 4387);
	this->jqcgjevbyjnb();
	this->vbjnkwdgigvnnijtdprun(1930, 478, 621, string("pplujgdnhqvlrmyhhggdvcmuenirjkucayefdversmnnneuchzuztgcrxwnpiabmaafdpsouxdvabjunxqzxgnlzatb"), 8046, false, 3507, 22088, false, 31403);
	this->hkaryaokui();
	this->ufylzwmzrvku(3487, string("kemtosqocvzxfpzxpwajkonecwdexafpqolvxbcsrbobg"), 34335, 854, 7683, false, string("wqmykvptpbdvanrfjjhxndekaxtbudp"), 778, false, 86127);
	this->puqyrsybrcnkm(15177);
	this->szypicwtukfuqzzaluq(18477, 2501, string("eccgwhvsmtdlxdmzrmhbthcknmylscqqeobjkyljesaoghfisluicpfxkynnqintcdenepfwyakonprqocnoojcupb"));
	this->gzebgeipganpojqfhif(5701, 32370);
	this->qqddbzynel(true, 396, 3528, 6035, 2061);
	this->xfnncoyqnflyvoyheuvu(string("ljbokxxcsvbqmwiocuclmqqlcyugzfkvitnwiqqobkucjkchiqsfacoxaduykhhehiqggtnpbbufpixhyysc"), false, 60377, 2395, true, true, 76273, 1397, string("njlyybozlvhxtxhmhempoezaybxeveyujfjolpauuhexadwenmylicueo"), string("rwnirjsmipknypscjwxtfnawjnpqcntqyjlzxmjohliyzfzqbmcqjvpogmyfpttyrvxrynlladpmdlicpijchekjowrg"));
}
#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class uwvxumj {
public:
	int ezwqqxzjxplbt;
	string xcjogsyphhpw;
	int tyrfoxewzjgu;
	int njbnieufg;
	uwvxumj();
	bool ytaxsjfnpvwnmc(int fdeks, double urxnf, bool tvcsdbpqsrrnd, int szzpzojmpupqvj, double saxvpbid, bool mddiytrphkuelht);

protected:
	int vgmmiodyk;

	bool pgorokkwfkuavvafacbp(int xlpev, double csbgjzqi, bool qhtaynsqpb, int rolnm, bool txvxkrzjxi, int syytekiipgdjlb);
	double ivoitlczsfwteefthexqh(double wqcdy, double dstvflezomf, string qtkut, int gzskvtkafl, double rdqjjtsyfdntj);
	string wplvwqciaygcnqhrmv(string qqxaunkzqg);
	string ihbzpnumrsy(bool ojjlmmrfdzdoahf, int rxvqchp, string ylwhtrtsipz, int jhsudsyvzb, string vkvqsthvfmcmara, int dfkiu, double uzvpxtbvdmhlnmx, string dhzxiekllorx, double vfagbgc, bool tegqta);
	double efkhpnwcpxoojsjnigju(double uvprbam, int lnklxyxscrwo, bool tkdxwsdpu);
	int cvbvzfguyuwcnjzkgffeah(double tzmfqvjzi, double gpsmdlknqx, bool gozpjn, bool bgzlr, bool cwpsnvatuhcwvpp, double ziuoa, int mjnxtkcdavlswb, double vlnjjuz, bool byzra, int rogktiyjtutpzg);
	void guhgnefwrcddmbaqevoxf(double ogbyhrseare);
	int jplzjmuavzfimueoeskmoe(string hpwwbfopquq, int cgblwqc);
	string sykduhslirlrhjxhzunalcrj(bool ywyhd, int ihbkwszr, double vdyadpyzqmgcsx, string kreckf, string gnrzjgcaailss);
	bool llwdsbcipry(double rwleoynwbyt, int wbowhdepavwj, bool ktlbu, int ixgbizkrgpkqc, int ywfczh, int byxppflrd, double eqfccbena, string lbzmetih, bool lyddpcut);

private:
	double nnvbyjfitvu;

	bool jivtfzsdriqezkxhctbeuos(string fzbbklrbe, bool rwpgseoabekxsd, bool gtgmbbsnz, bool giggeg, bool vdgiealfhxyk, string cojqiienyrfe, bool mdosewimcypg, string qahsvvyd, double jftsgqvy);
	double gomaqhsshneqpj(int zbfxdtjs);
	bool infqahxcqypi(double suzpmw, string lfyjfclientm, bool dtuixcxmrcb, bool njihmbjo, string bxunogv, double ffmogaownscubhg, bool htdrpcshw);
	int cdwbwmfauayfxirgo(bool lgtckpd, int vfvrn, string pljojtwm, string ukxdymnuny, int rpjdsfddx, int tzkrgr, bool txtuplw, int iylqdyx, double xmckspgbrulpf, double ehptoidjqapdo);
	void kzgtyvbboajwzpymos(string aolut, double suvefw, double sifmjdm, double jhcblzm, int sgdtpgqf);
	void ynhmbbayezthwooisepvwwbf(bool lrliiztie, int ihjuqmftysgtgoc, bool vtijkcdc, int lyfpl, double dssinx, int ygzsqhbouher);
	int lvqiayctrikvtijpizos(int bndbnyxdxqegats, double svusxskg, string svhlsqop, bool ledqg, bool ipqoqiwy);
	void xtoybqggigqslle(bool drwhaijr, int ytqlt, double jiywmcdoepmo, int hbsbmskhjrx, double hrddrqhdeyhtnk, string pomnkyvqtfulq, bool ciwpcmqschqh);

};


bool uwvxumj::jivtfzsdriqezkxhctbeuos(string fzbbklrbe, bool rwpgseoabekxsd, bool gtgmbbsnz, bool giggeg, bool vdgiealfhxyk, string cojqiienyrfe, bool mdosewimcypg, string qahsvvyd, double jftsgqvy) {
	return false;
}

double uwvxumj::gomaqhsshneqpj(int zbfxdtjs) {
	int ubawnemsxlmnbe = 1520;
	if (1520 == 1520) {
		int gcn;
		for (gcn = 48; gcn > 0; gcn--) {
			continue;
		}
	}
	if (1520 == 1520) {
		int bijyqbfmq;
		for (bijyqbfmq = 63; bijyqbfmq > 0; bijyqbfmq--) {
			continue;
		}
	}
	if (1520 == 1520) {
		int fk;
		for (fk = 75; fk > 0; fk--) {
			continue;
		}
	}
	if (1520 != 1520) {
		int cmsqxxik;
		for (cmsqxxik = 71; cmsqxxik > 0; cmsqxxik--) {
			continue;
		}
	}
	if (1520 != 1520) {
		int pnxissg;
		for (pnxissg = 73; pnxissg > 0; pnxissg--) {
			continue;
		}
	}
	return 60138;
}

bool uwvxumj::infqahxcqypi(double suzpmw, string lfyjfclientm, bool dtuixcxmrcb, bool njihmbjo, string bxunogv, double ffmogaownscubhg, bool htdrpcshw) {
	int righovlugmo = 2790;
	int jfvpevg = 2827;
	bool bimiv = true;
	double ilutqjjwpuwdd = 55256;
	int trdnkircxeg = 5087;
	bool ieqvzxpuz = true;
	int pamchtqzi = 5686;
	bool udzyz = false;
	bool bezwbbo = true;
	if (2790 != 2790) {
		int nbuikkmx;
		for (nbuikkmx = 87; nbuikkmx > 0; nbuikkmx--) {
			continue;
		}
	}
	if (2827 != 2827) {
		int kmopegmy;
		for (kmopegmy = 78; kmopegmy > 0; kmopegmy--) {
			continue;
		}
	}
	return false;
}

int uwvxumj::cdwbwmfauayfxirgo(bool lgtckpd, int vfvrn, string pljojtwm, string ukxdymnuny, int rpjdsfddx, int tzkrgr, bool txtuplw, int iylqdyx, double xmckspgbrulpf, double ehptoidjqapdo) {
	string xizhhihwflqnxbl = "kdmwyfkuwvvmrhlypirocwuuzwctefoicigrwsyluvhfklvuylanwfd";
	int wzchay = 3847;
	bool aszxraqlzd = true;
	string faivqob = "mfzzffydsqtcslpdoactpewvtbllrlnkiqfhubx";
	if (string("mfzzffydsqtcslpdoactpewvtbllrlnkiqfhubx") == string("mfzzffydsqtcslpdoactpewvtbllrlnkiqfhubx")) {
		int lbii;
		for (lbii = 86; lbii > 0; lbii--) {
			continue;
		}
	}
	return 74530;
}

void uwvxumj::kzgtyvbboajwzpymos(string aolut, double suvefw, double sifmjdm, double jhcblzm, int sgdtpgqf) {
	string hcsqulktkxt = "wznardqmnxglhvjftchtbrslpijabvgwqiinknhqplfmnoygkietmmkirgubirzfcnitumilgotzbz";
	int cljrtmqdamx = 1012;
	int abyplgglfwuc = 3606;
	bool ablyfqxtr = false;
	bool hflreu = false;
	if (false == false) {
		int rmhpbiv;
		for (rmhpbiv = 13; rmhpbiv > 0; rmhpbiv--) {
			continue;
		}
	}
	if (3606 != 3606) {
		int suedbge;
		for (suedbge = 33; suedbge > 0; suedbge--) {
			continue;
		}
	}
	if (false == false) {
		int gwlfed;
		for (gwlfed = 33; gwlfed > 0; gwlfed--) {
			continue;
		}
	}
	if (false != false) {
		int rv;
		for (rv = 2; rv > 0; rv--) {
			continue;
		}
	}
	if (string("wznardqmnxglhvjftchtbrslpijabvgwqiinknhqplfmnoygkietmmkirgubirzfcnitumilgotzbz") == string("wznardqmnxglhvjftchtbrslpijabvgwqiinknhqplfmnoygkietmmkirgubirzfcnitumilgotzbz")) {
		int urc;
		for (urc = 3; urc > 0; urc--) {
			continue;
		}
	}

}

void uwvxumj::ynhmbbayezthwooisepvwwbf(bool lrliiztie, int ihjuqmftysgtgoc, bool vtijkcdc, int lyfpl, double dssinx, int ygzsqhbouher) {
	double tymunusnuf = 36723;
	int geqsu = 4957;
	double mgmaxesxragg = 30247;
	int uqgvnlpi = 1472;
	bool keulnpkvmybevqo = true;
	double vprfcwmgqszwkkw = 18394;
	int vwecfharosadr = 1315;
	string kyblws = "iquyvnwgmathdunlvvrnizfbwgvqivocsaxtusiugzuoqmrnhkypqthsbltcxxlsdsoduen";
	double suryuhqs = 21124;
	if (30247 == 30247) {
		int kibjh;
		for (kibjh = 26; kibjh > 0; kibjh--) {
			continue;
		}
	}
	if (true != true) {
		int bopqylhe;
		for (bopqylhe = 74; bopqylhe > 0; bopqylhe--) {
			continue;
		}
	}
	if (36723 != 36723) {
		int lrgb;
		for (lrgb = 82; lrgb > 0; lrgb--) {
			continue;
		}
	}
	if (1315 == 1315) {
		int vgadd;
		for (vgadd = 40; vgadd > 0; vgadd--) {
			continue;
		}
	}

}

int uwvxumj::lvqiayctrikvtijpizos(int bndbnyxdxqegats, double svusxskg, string svhlsqop, bool ledqg, bool ipqoqiwy) {
	int inhhdehnmktb = 8709;
	double boupkcdhhjaf = 73223;
	double wzgyvjmillhmovd = 45195;
	string tvgdgwhchy = "tczmkihilyodbhpymqwlzibnxkatqzqqmxxhberxnvfgtadembpormdidoggxvhtbzheyenjk";
	double pkstf = 66034;
	string rbbksiiscw = "zvpdmoikuqxbxrdimvklj";
	bool zhdnzcv = true;
	string ozicyroq = "fdnxpavanlfvwxbbdlqhlkp";
	int elxljlwzbgnksb = 1208;
	int mkzqlizeg = 812;
	if (1208 == 1208) {
		int rlfm;
		for (rlfm = 79; rlfm > 0; rlfm--) {
			continue;
		}
	}
	return 29179;
}

void uwvxumj::xtoybqggigqslle(bool drwhaijr, int ytqlt, double jiywmcdoepmo, int hbsbmskhjrx, double hrddrqhdeyhtnk, string pomnkyvqtfulq, bool ciwpcmqschqh) {
	string ynojhxowawuc = "ogumurckesjbrfolwywrzjdhrj";
	string rgukytdesmg = "tmffdjnryqvjmhqjjfullxxxbpxrhnfavtyarphznwgfbmrgeajkrfekdovpme";
	string wnxsrr = "lreqwcglthlgxcklksplbfotbcrhoheufizs";
	if (string("lreqwcglthlgxcklksplbfotbcrhoheufizs") == string("lreqwcglthlgxcklksplbfotbcrhoheufizs")) {
		int vrdj;
		for (vrdj = 54; vrdj > 0; vrdj--) {
			continue;
		}
	}

}

bool uwvxumj::pgorokkwfkuavvafacbp(int xlpev, double csbgjzqi, bool qhtaynsqpb, int rolnm, bool txvxkrzjxi, int syytekiipgdjlb) {
	int mnnyfmoremd = 5504;
	double tmnhaqlz = 33434;
	string isepbe = "lcwgtpgxxfwslneauxctuqisttaykgobcsdcvfirykptnxcpbxywesdetyyhrnfarpaacvul";
	bool eqlsbqgxfxy = false;
	bool firrvf = false;
	int rvjxj = 198;
	if (false == false) {
		int jrsexvcsam;
		for (jrsexvcsam = 73; jrsexvcsam > 0; jrsexvcsam--) {
			continue;
		}
	}
	if (5504 != 5504) {
		int gwekmx;
		for (gwekmx = 2; gwekmx > 0; gwekmx--) {
			continue;
		}
	}
	if (false != false) {
		int ratxyznsk;
		for (ratxyznsk = 88; ratxyznsk > 0; ratxyznsk--) {
			continue;
		}
	}
	return false;
}

double uwvxumj::ivoitlczsfwteefthexqh(double wqcdy, double dstvflezomf, string qtkut, int gzskvtkafl, double rdqjjtsyfdntj) {
	int hbvmrluauni = 356;
	if (356 == 356) {
		int fw;
		for (fw = 1; fw > 0; fw--) {
			continue;
		}
	}
	if (356 != 356) {
		int abdfj;
		for (abdfj = 71; abdfj > 0; abdfj--) {
			continue;
		}
	}
	if (356 == 356) {
		int nrpvr;
		for (nrpvr = 14; nrpvr > 0; nrpvr--) {
			continue;
		}
	}
	if (356 != 356) {
		int xpkb;
		for (xpkb = 15; xpkb > 0; xpkb--) {
			continue;
		}
	}
	if (356 != 356) {
		int ble;
		for (ble = 41; ble > 0; ble--) {
			continue;
		}
	}
	return 97174;
}

string uwvxumj::wplvwqciaygcnqhrmv(string qqxaunkzqg) {
	double nelebtvbfdjbfl = 40093;
	bool gjsxvmmcflmv = false;
	string njdayo = "qypzjczvqbaznbvbeskcozqucawrvxrjfxtmmyriyraequjtjcpazaodopzvbqsbfsmhgqgvmfygyxzejumhikfdmx";
	if (false != false) {
		int eekwmzzli;
		for (eekwmzzli = 31; eekwmzzli > 0; eekwmzzli--) {
			continue;
		}
	}
	if (string("qypzjczvqbaznbvbeskcozqucawrvxrjfxtmmyriyraequjtjcpazaodopzvbqsbfsmhgqgvmfygyxzejumhikfdmx") != string("qypzjczvqbaznbvbeskcozqucawrvxrjfxtmmyriyraequjtjcpazaodopzvbqsbfsmhgqgvmfygyxzejumhikfdmx")) {
		int fyekccsu;
		for (fyekccsu = 33; fyekccsu > 0; fyekccsu--) {
			continue;
		}
	}
	return string("zibnh");
}

string uwvxumj::ihbzpnumrsy(bool ojjlmmrfdzdoahf, int rxvqchp, string ylwhtrtsipz, int jhsudsyvzb, string vkvqsthvfmcmara, int dfkiu, double uzvpxtbvdmhlnmx, string dhzxiekllorx, double vfagbgc, bool tegqta) {
	double idsnlhqzs = 600;
	double bsqukxy = 34287;
	bool hwllr = false;
	string foxhsledz = "qxwdmvlkrnupihjvznixouiyyaonoty";
	int bkubfu = 3198;
	if (600 == 600) {
		int eoersgygnd;
		for (eoersgygnd = 8; eoersgygnd > 0; eoersgygnd--) {
			continue;
		}
	}
	return string("epi");
}

double uwvxumj::efkhpnwcpxoojsjnigju(double uvprbam, int lnklxyxscrwo, bool tkdxwsdpu) {
	double fquqbzmgkhoii = 7480;
	double illvoxxtg = 3370;
	bool wkmegwyvuccsgn = false;
	return 86701;
}

int uwvxumj::cvbvzfguyuwcnjzkgffeah(double tzmfqvjzi, double gpsmdlknqx, bool gozpjn, bool bgzlr, bool cwpsnvatuhcwvpp, double ziuoa, int mjnxtkcdavlswb, double vlnjjuz, bool byzra, int rogktiyjtutpzg) {
	int qxxadrqkusmm = 2310;
	double kokwtlnsawri = 9549;
	int afixnv = 4742;
	if (9549 == 9549) {
		int wcgpig;
		for (wcgpig = 0; wcgpig > 0; wcgpig--) {
			continue;
		}
	}
	return 49029;
}

void uwvxumj::guhgnefwrcddmbaqevoxf(double ogbyhrseare) {

}

int uwvxumj::jplzjmuavzfimueoeskmoe(string hpwwbfopquq, int cgblwqc) {
	return 90322;
}

string uwvxumj::sykduhslirlrhjxhzunalcrj(bool ywyhd, int ihbkwszr, double vdyadpyzqmgcsx, string kreckf, string gnrzjgcaailss) {
	double cgatgxxanxjjt = 66925;
	int trcxdhtgamx = 202;
	string idpun = "jpfdnvgtealkvvcofdpxxshffffgzfodirzdmnczxitrbzzvadegjkzkkvefeghwbagngbcl";
	int jsllq = 395;
	if (66925 != 66925) {
		int zcnxaw;
		for (zcnxaw = 39; zcnxaw > 0; zcnxaw--) {
			continue;
		}
	}
	if (202 != 202) {
		int kmsrrcbgo;
		for (kmsrrcbgo = 76; kmsrrcbgo > 0; kmsrrcbgo--) {
			continue;
		}
	}
	return string("rv");
}

bool uwvxumj::llwdsbcipry(double rwleoynwbyt, int wbowhdepavwj, bool ktlbu, int ixgbizkrgpkqc, int ywfczh, int byxppflrd, double eqfccbena, string lbzmetih, bool lyddpcut) {
	string ucaiuapdsjsv = "wvrosptwaxbhzajzvovviwhsjhlmtcntksffkslcojamntxwvlqq";
	double vlqutvcgqxfwe = 75479;
	int kaejk = 965;
	string zgfygqrdnypec = "yfenrbmgsshuzykhqmxuhfafzkrjbrjuppagijrcqiszaqgvmtuomympyeulluqdn";
	bool mjlyufrajzi = true;
	bool qynhcvsnobxi = false;
	string psuyidnp = "rdgtehdbpltdpxnhrqxyfijyfdabcvvygsuwrxkclsicvgppiwvvkfso";
	return true;
}

bool uwvxumj::ytaxsjfnpvwnmc(int fdeks, double urxnf, bool tvcsdbpqsrrnd, int szzpzojmpupqvj, double saxvpbid, bool mddiytrphkuelht) {
	bool cwsrmat = true;
	bool plknigfrkeacsny = true;
	double xvmgu = 21903;
	double pbmsmng = 7187;
	double uaghuklq = 9250;
	int izomtptrcafcu = 261;
	string oappirbnq = "";
	bool wjlorqscws = false;
	bool wazrq = false;
	if (true == true) {
		int qgwiyci;
		for (qgwiyci = 60; qgwiyci > 0; qgwiyci--) {
			continue;
		}
	}
	if (true == true) {
		int vm;
		for (vm = 97; vm > 0; vm--) {
			continue;
		}
	}
	return true;
}

uwvxumj::uwvxumj() {
	this->ytaxsjfnpvwnmc(8222, 513, false, 2370, 75171, true);
	this->pgorokkwfkuavvafacbp(619, 61455, false, 356, false, 1354);
	this->ivoitlczsfwteefthexqh(97438, 19628, string("vlexfodjdhcvdmdrfrkzrfwqnrkdsyspgvpsxsjsvtlredqxwlhpvttoruxqnzdveahwdqwyhsniuikjawicchjmyqdudbmdizz"), 5311, 16847);
	this->wplvwqciaygcnqhrmv(string("glzycxrmcfnxrhthdcddcryrklhgbwopwtufdsuafpeybxmvikncbclqagmkq"));
	this->ihbzpnumrsy(true, 262, string("aiphr"), 61, string("wltzzfjfhmrqgvjkgqzjmguhetjktgkxiolxwudxoxupbkzanbtqsfbfikjuzoodhbaobiigcmwhntzzvjavqrgswwpkwagnyjq"), 6048, 35816, string("ilxavddqoswfmzpgdxr"), 13221, true);
	this->efkhpnwcpxoojsjnigju(10299, 3272, false);
	this->cvbvzfguyuwcnjzkgffeah(7946, 77575, false, true, true, 39416, 3866, 4606, false, 866);
	this->guhgnefwrcddmbaqevoxf(82464);
	this->jplzjmuavzfimueoeskmoe(string("vslropaymaqmnmsrvnvlxntwgfggknineqncvzhqxusczbkga"), 2849);
	this->sykduhslirlrhjxhzunalcrj(false, 2494, 4849, string("uvsiarwqijrzhgltpcbvurnsfyfmzmqvbku"), string("wdxgzazgxndxystxahxhdmlnnwwmibkhsuqkjmjeqiwrecdwtrmizufaxzwrkrcf"));
	this->llwdsbcipry(26569, 1139, false, 3766, 599, 2002, 1896, string("omsbfajwqtfjawvllbbmzvbexzyovhimzibqajzkdbzczzysnvrr"), false);
	this->jivtfzsdriqezkxhctbeuos(string("yyeyhxyqfqcsvpxqgavqsuohazhvmzquaqvxvbtdpxrdkvleowbatpqywdhciqifndjkdblmiyilmutqlrwvfnfvl"), true, true, false, true, string("wejrojngsadjfbjvxracdoasirxxivrsjhzmbbplou"), true, string("xjeddklzwyqowkcpcxlzhsceicodxrosfeaalbpxvvweqlirfcyoqmhqxgkzxrwjdlmtsvzjhlaapdqyrmecs"), 2481);
	this->gomaqhsshneqpj(672);
	this->infqahxcqypi(399, string("mttqvjybtepetmsyupunolzgclmbxpwvqqwlozdpweplxzdxvl"), true, false, string("osokkaoyiyqudbawrcuihplqzmqijagqwceveatxoeoepomneodstkztnufomtrwbpwldfdvpajckoqcturxkajol"), 43831, true);
	this->cdwbwmfauayfxirgo(false, 325, string("onhtmnrqmcjbfsmdfeotijuklhwzfdagarxeuvpdckmxuemoxxccspticmoyppktipdwtyjbirrfewv"), string("msqftxfqxktsnxutqctdpkffsgjfhkmjkkefmdyxxvfrcznemhrvpoomewfktduotymowxceo"), 3731, 6215, true, 1176, 28108, 28368);
	this->kzgtyvbboajwzpymos(string("mkgzjtzpkmhjpggulqaukvbhaqerllyyaqfyytpolpoiiyovulkffwknhriw"), 14334, 43918, 69944, 1470);
	this->ynhmbbayezthwooisepvwwbf(true, 5100, false, 4352, 503, 2140);
	this->lvqiayctrikvtijpizos(8697, 7951, string("ksponfvptiefleklgpmrmuywopsxbcfkiwkwlytudmpoydutfztymnboicdmxqfytyaymsbzwxglyrmsj"), false, true);
	this->xtoybqggigqslle(false, 5719, 6956, 3928, 4133, string("trsjhamrfqlcrmklphdunzaqidfqqskaaddsianmlxcgnlfgrvqcpspfvgwmkmtmt"), false);
}
