#include "Visuals.h"
#include "Interfaces.h"
#include "RenderManager.h"




void CVisuals::Init()
{

}

void CVisuals::Move(CUserCmd *pCmd, bool &bSendPacket) {}

void CVisuals::Draw()
{

	if (!Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;

	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());


	if (Menu::Window.VisualsTab.OtherNoScope.GetState() && pLocal->IsAlive() && pLocal->IsScoped())
		NoScopeCrosshair();

	if (Menu::Window.VisualsTab.Normalcrosshair.GetState())
		DrawCrosshair();

	switch (Menu::Window.VisualsTab.XHairs.GetIndex())
	{
	case 0:
		Interfaces::Engine->ClientCmd_Unrestricted("crosshair 1");
		break;
	case 1:
		Interfaces::Engine->ClientCmd_Unrestricted("crosshair 0");
		DrawRecoilCrosshair();
		break;
	case 2:
		Interfaces::Engine->ClientCmd_Unrestricted("crosshair 0");
		SpreadCrosshair();
		break;
	}
}




void CVisuals::DrawCrosshair()
{
	RECT View = Render::GetViewport();
	int MidX = View.right / 2;
	int MidY = View.bottom / 2;
	Render::Line(MidX - 4, MidY - 4, MidX + 4, MidY + 4, Color(0, 255, 0, 255));
	Render::Line(MidX + 4, MidY - 4, MidX - 4, MidY + 4, Color(0, 255, 0, 255));
}


void CVisuals::NoScopeCrosshair()
{
	RECT View = Render::GetViewport();
	int MidX = View.right / 2;
	int MidY = View.bottom / 2;

	IClientEntity* pLocal = hackManager.pLocal();
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	if (GameUtils::IsSniper(pWeapon))
	{
		Render::Line(MidX - 1000, MidY, MidX + 1000, MidY, Color(0, 0, 0, 255));
		Render::Line(MidX, MidY - 1000, MidX, MidY + 1000, Color(0, 0, 0, 255));

	}
}



void CVisuals::DrawRecoilCrosshair()
{
	IClientEntity *pLocal = hackManager.pLocal();

	Vector ViewAngles;
	Interfaces::Engine->GetViewAngles(ViewAngles);
	ViewAngles += pLocal->localPlayerExclusive()->GetAimPunchAngle() * 2.f;

	Vector fowardVec;
	AngleVectors(ViewAngles, &fowardVec);
	fowardVec *= 10000;

	Vector start = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector end = start + fowardVec, endScreen;

	if (Render::WorldToScreen(end, endScreen) && pLocal->IsAlive())
	{
		Render::Line(endScreen.x - 10, endScreen.y, endScreen.x + 10, endScreen.y, Color(0, 255, 0, 255));
		Render::Line(endScreen.x, endScreen.y - 10, endScreen.x, endScreen.y + 10, Color(0, 255, 0, 255));
	}
}


void CVisuals::SpreadCrosshair()
{
	IClientEntity *pLocal = hackManager.pLocal();
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	IClientEntity* WeaponEnt = Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	if (!hackManager.pLocal()->IsAlive())
		return;

	if (!GameUtils::IsBallisticWeapon(pWeapon))
		return;

	if (pWeapon == nullptr)
		return;

	int xs;
	int ys;
	Interfaces::Engine->GetScreenSize(xs, ys);
	xs /= 2; ys /= 2;

	auto accuracy = pWeapon->GetInaccuracy() * 550.f; //3000

	Render::DrawFilledCircle(Vector2D(xs, ys), Color(1, 1, 1, 144), accuracy, 60);

	if (pLocal->IsAlive())
	{
		if (pWeapon)
		{

			float inaccuracy = pWeapon->GetInaccuracy() * 1000;
			char buffer4[64];
			//sprintf_s(buffer4, "Inaccuracy:  %f", inaccuracy);
			//Render::Text(xs + accuracy + 4, ys, Color(255, 255, 255, 255), Render::Fonts::ESP, buffer4);
		}

	}
	else
	{

		//Render::Text(10, 70, Color(255, 255, 255, 255), Render::Fonts::ESP, "Inaccuracy: --");
	}

}

