#include "Resolver.h"
#include "Ragebot.h"
#include "Hooks.h"










void PitchCorrection()
{
	CUserCmd* pCmd;
	for (int i = 0; i < Interfaces::Engine->GetMaxClients(); ++i)
	{
		IClientEntity* pLocal = hackManager.pLocal();
		IClientEntity *player = (IClientEntity*)Interfaces::EntList->GetClientEntity(i);

		if (!player || player->IsDormant() || player->GetHealth() < 1 || (DWORD)player == (DWORD)pLocal)
			continue;

		if (!player)
			continue;

		if (pLocal)
			continue;

		if (pLocal && player && pLocal->IsAlive())
		{
			if (Menu::Window.RageBotTab.AimbotResolver.GetState())
			{
				Vector* eyeAngles = player->GetEyeAnglesXY();
				if (eyeAngles->x < -179.f) eyeAngles->x += 360.f;
				else if (eyeAngles->x > 90.0 || eyeAngles->x < -90.0) eyeAngles->x = 89.f;
				else if (eyeAngles->x > 89.0 && eyeAngles->x < 91.0) eyeAngles->x -= 90.f;
				else if (eyeAngles->x > 179.0 && eyeAngles->x < 181.0) eyeAngles->x -= 180;
				else if (eyeAngles->x > -179.0 && eyeAngles->x < -181.0) eyeAngles->x += 180;
				else if (fabs(eyeAngles->x) == 0) eyeAngles->x = std::copysign(89.0f, eyeAngles->x);
			}
		}
	}
}



void ResolverSetup::Resolve(IClientEntity* pEntity)
{
	bool MeetsLBYReq;
	if (pEntity->GetFlags() & FL_ONGROUND)
		MeetsLBYReq = true;
	else
		MeetsLBYReq = false;

	bool IsMoving;
	if (pEntity->GetVelocity().Length2D() >= 0.5)
		IsMoving = true;
	else
		IsMoving = false;

	ResolverSetup::NewANgles[pEntity->GetIndex()] = *pEntity->GetEyeAnglesXY();
	ResolverSetup::newlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	ResolverSetup::newsimtime = pEntity->GetSimulationTime();
	ResolverSetup::newdelta[pEntity->GetIndex()] = pEntity->GetEyeAnglesXY()->y;
	ResolverSetup::newlbydelta[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	ResolverSetup::finaldelta[pEntity->GetIndex()] = ResolverSetup::newdelta[pEntity->GetIndex()] - ResolverSetup::storeddelta[pEntity->GetIndex()];
	ResolverSetup::finallbydelta[pEntity->GetIndex()] = ResolverSetup::newlbydelta[pEntity->GetIndex()] - ResolverSetup::storedlbydelta[pEntity->GetIndex()];
	if (newlby == storedlby)
		ResolverSetup::lbyupdated = false;
	else
		ResolverSetup::lbyupdated = true;


	 if (Menu::Window.RageBotTab.AimbotResolver.GetState())
	{
		resolvokek::resolvemode = 2;
		if (Globals::missedshots > 1 && Globals::missedshots < 21)
		{
			if (MeetsLBYReq && lbyupdated)
			{
				pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
			}
			else if (!MeetsLBYReq && lbyupdated)
			{
				switch (Globals::Shots % 4)
				{
				case 1:
					pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 15;
					break;
				case 2:
					pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 40;
					break;
				case 3:
					pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 15;
					break;
				case 4:
					pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 40;
					break;
				}
			}
			else
				pEntity->GetEyeAnglesXY()->y = rand() % 180 - rand() % 35;
		}

		else if (Globals::missedshots > 4)
		{

		}
		else if (Globals::missedshots > 6 && Globals::missedshots <= 3)
		{
			if (MeetsLBYReq && lbyupdated)
			{
				pEntity->GetEyeAnglesXY()->y = ResolverSetup::finallbydelta[pEntity->GetIndex()];
			}
			else
				pEntity->GetEyeAnglesXY()->y = ResolverSetup::finaldelta[pEntity->GetIndex()];
		}
		else
		{
			if (MeetsLBYReq && lbyupdated)
			{
				bool timer = 0;
				if (timer)
					pEntity->GetEyeAnglesXY()->y = ResolverSetup::finallbydelta[pEntity->GetIndex()] + rand() % 35;
				else
					pEntity->GetEyeAnglesXY()->y = ResolverSetup::finallbydelta[pEntity->GetIndex()] - rand() % 35;
				timer = !timer;
			}
			else
			{
				bool timer = 0;
				if (timer)
					pEntity->GetEyeAnglesXY()->y = ResolverSetup::finaldelta[pEntity->GetIndex()] + rand() % 35;
				else
					pEntity->GetEyeAnglesXY()->y = ResolverSetup::finaldelta[pEntity->GetIndex()] - rand() % 35;
				timer = !timer;
			}
		}
	}
	PitchCorrection();
}

void ResolverSetup::StoreFGE(IClientEntity* pEntity)
{
	ResolverSetup::storedanglesFGE = pEntity->GetEyeAnglesXY()->y;
	ResolverSetup::storedlbyFGE = pEntity->GetLowerBodyYaw();
	ResolverSetup::storedsimtimeFGE = pEntity->GetSimulationTime();
}

void ResolverSetup::StoreThings(IClientEntity* pEntity)
{
	ResolverSetup::StoredAngles[pEntity->GetIndex()] = *pEntity->GetEyeAnglesXY();
	ResolverSetup::storedlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	ResolverSetup::storedsimtime = pEntity->GetSimulationTime();
	ResolverSetup::storeddelta[pEntity->GetIndex()] = pEntity->GetEyeAnglesXY()->y;
	ResolverSetup::storedlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
}

void ResolverSetup::CM(IClientEntity* pEntity)
{
	for (int x = 1; x < Interfaces::Engine->GetMaxClients(); x++)
	{

		pEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(x);

		if (!pEntity
			|| pEntity == hackManager.pLocal()
			|| pEntity->IsDormant()
			|| !pEntity->IsAlive())
			continue;

		ResolverSetup::StoreThings(pEntity);
	}
}

void ResolverSetup::FSN(IClientEntity* pEntity, ClientFrameStage_t stage)
{
	if (stage == ClientFrameStage_t::FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		for (int i = 1; i < Interfaces::Engine->GetMaxClients(); i++)
		{

			pEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(i);

			if (!pEntity
				|| pEntity == hackManager.pLocal()
				|| pEntity->IsDormant()
				|| !pEntity->IsAlive())
				continue;

			ResolverSetup::Resolve(pEntity);
		}
	}
}