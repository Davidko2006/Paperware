#include "LagComp.h"
#include "backtracking.h"
#include "Backtracking Rage.h"
//#include "Resolver.h"
#include "Hooks.h"
#include "Hacks.h"
#include "Chams.h"
#include "Menu.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include "MiscHacks.h"
#include "CRC32.h"
#include "hitmarker.h"
#include <intrin.h>
#include "Entities.h"

extern float lineLBY;
extern float lineRealAngle;
extern float lineFakeAngle;
Vector LastAngleAAReal;
Vector LBYThirdpersonAngle;



#define MakePtr(cast, ptr, addValue) (cast)( (DWORD)(ptr) + (DWORD)(addValue))

#ifdef NDEBUG
#define strenc( s ) std::string( cx_make_encrypted_string( s ) )
#define charenc( s ) strenc( s ).c_str()
#define wstrenc( s ) std::wstring( strenc( s ).begin(), strenc( s ).end() )
#define wcharenc( s ) wstrenc( s ).c_str()
#else
#define strenc( s ) ( s )
#define charenc( s ) ( s )
#define wstrenc( s ) ( s )
#define wcharenc( s ) ( s )
#endif

#ifdef NDEBUG
#define XorStr( s ) ( XorCompileTime::XorString< sizeof( s ) - 1, __COUNTER__ >( s, std::make_index_sequence< sizeof( s ) - 1>() ).decrypt() )
#else
#define XorStr( s ) ( s )
#endif




int currentfov;
Vector LastAngleAA;
bool Resolver::didhitHS;
CUserCmd* Globals::UserCmd;
IClientEntity* Globals::Target;
int Globals::Shots;
bool Globals::change;
int Globals::TargetID;
std::map<int, QAngle>Globals::storedshit;
int Globals::missedshots;

typedef void(__thiscall* DrawModelEx_)(void*, void*, void*, const ModelRenderInfo_t&, matrix3x4*);
typedef void(__thiscall* PaintTraverse_)(PVOID, unsigned int, bool, bool);
typedef bool(__thiscall* InPrediction_)(PVOID);
typedef void(__stdcall *FrameStageNotifyFn)(ClientFrameStage_t);
typedef bool(__thiscall *FireEventClientSideFn)(PVOID, IGameEvent*);
typedef void(__thiscall* RenderViewFn)(void*, CViewSetup&, CViewSetup&, int, int);
using OverrideViewFn = void(__fastcall*)(void*, void*, CViewSetup*);
typedef float(__stdcall *oGetViewModelFOV)();
typedef void(__thiscall *SceneEnd_t)(void *pCmd);

PaintTraverse_ oPaintTraverse;
DrawModelEx_ oDrawModelExecute;
FrameStageNotifyFn oFrameStageNotify;
OverrideViewFn oOverrideView;
FireEventClientSideFn oFireEventClientSide;
RenderViewFn oRenderView;
SceneEnd_t pSceneEnd;


void __fastcall PaintTraverse_Hooked(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce);
bool __stdcall Hooked_InPrediction();
bool __fastcall Hooked_FireEventClientSide(PVOID ECX, PVOID EDX, IGameEvent *Event);
void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld);
bool __stdcall CreateMoveClient_Hooked(float frametime, CUserCmd* pCmd);
void  __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage);
void __fastcall Hooked_OverrideView(void* ecx, void* edx, CViewSetup* pSetup);
float __stdcall GGetViewModelFOV();
void __fastcall Hooked_RenderView(void* ecx, void* edx, CViewSetup &setup, CViewSetup &hudViewSetup, int nClearFlags, int whatToDraw);
void __fastcall	hkSceneEnd(void *pEcx, void *pEdx);

namespace Hooks
{
	Utilities::Memory::VMTManager VMTPanel; 
	Utilities::Memory::VMTManager VMTClient;
	Utilities::Memory::VMTManager VMTClientMode;
	Utilities::Memory::VMTManager VMTModelRender; 
	Utilities::Memory::VMTManager VMTPrediction; 
	Utilities::Memory::VMTManager VMTRenderView;
	Utilities::Memory::VMTManager VMTEventManager;
};

void Hooks::UndoHooks()
{
	VMTPanel.RestoreOriginal();
	VMTPrediction.RestoreOriginal();
	VMTModelRender.RestoreOriginal();
	VMTClientMode.RestoreOriginal();
}

void Hooks::Initialise()
{

	Interfaces::Engine->ExecuteClientCmd("clear");
	Interfaces::CVar->ConsoleColorPrintf(Color(0, 255, 0, 255), ("\n paperware 2137\n"));
	VMTPanel.Initialise((DWORD*)Interfaces::Panels);
	oPaintTraverse = (PaintTraverse_)VMTPanel.HookMethod((DWORD)&PaintTraverse_Hooked, Offsets::VMT::Panel_PaintTraverse);

	VMTPrediction.Initialise((DWORD*)Interfaces::Prediction);
	VMTPrediction.HookMethod((DWORD)&Hooked_InPrediction, 14);

	VMTModelRender.Initialise((DWORD*)Interfaces::ModelRender);
	oDrawModelExecute = (DrawModelEx_)VMTModelRender.HookMethod((DWORD)&Hooked_DrawModelExecute, Offsets::VMT::ModelRender_DrawModelExecute);

	VMTClientMode.Initialise((DWORD*)Interfaces::ClientMode);
	VMTClientMode.HookMethod((DWORD)CreateMoveClient_Hooked, 24);

	VMTRenderView.Initialise((DWORD*)Interfaces::RenderView);
	pSceneEnd = (SceneEnd_t)VMTRenderView.HookMethod((DWORD)&hkSceneEnd, 9);


	oOverrideView = (OverrideViewFn)VMTClientMode.HookMethod((DWORD)&Hooked_OverrideView, 18);
	VMTClientMode.HookMethod((DWORD)&GGetViewModelFOV, 35);

	VMTClient.Initialise((DWORD*)Interfaces::Client);
	oFrameStageNotify = (FrameStageNotifyFn)VMTClient.HookMethod((DWORD)&Hooked_FrameStageNotify, 36);

	VMTEventManager.Initialise((DWORD*)Interfaces::EventManager);
	oFireEventClientSide = (FireEventClientSideFn)VMTEventManager.HookMethod((DWORD)&Hooked_FireEventClientSide, 9);
}

void MovementCorrection(CUserCmd* pCmd)
{

}

float clip(float n, float lower, float upper)
{

	return (std::max)(lower, (std::min)(n, upper));
}

int LagCompBreak() {
	IClientEntity *pLocalPlayer = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	Vector velocity = pLocalPlayer->GetVelocity();
	velocity.z = 0;
	float speed = velocity.Length();
	if (speed > 0.f) {
		auto distance_per_tick = speed *
			Interfaces::Globals->interval_per_tick;
		int choked_ticks = std::ceilf(65.f / distance_per_tick);
		return std::min<int>(choked_ticks, 14);
	}
	return 1;
}

BYTE bMoveData[0x200];
void Prediction(CUserCmd* pCmd, IClientEntity* LocalPlayer)
{

	if (Interfaces::MoveHelper && Menu::Window.RageBotTab.AimbotEnable.GetState() && LocalPlayer->IsAlive())
	{

		float curtime = Interfaces::Globals->curtime;
		float frametime = Interfaces::Globals->frametime;
		int iFlags = LocalPlayer->GetFlags();

		Interfaces::Globals->curtime = (float)LocalPlayer->GetTickBase() * Interfaces::Globals->interval_per_tick;
		Interfaces::Globals->frametime = Interfaces::Globals->interval_per_tick;		
		Interfaces::MoveHelper->SetHost(LocalPlayer);

		Interfaces::Prediction1->SetupMove(LocalPlayer, pCmd, nullptr, bMoveData);
		Interfaces::GameMovement->ProcessMovement(LocalPlayer, bMoveData);
		Interfaces::Prediction1->FinishMove(LocalPlayer, pCmd, bMoveData);

		Interfaces::MoveHelper->SetHost(0);

		Interfaces::Globals->curtime = curtime;
		Interfaces::Globals->frametime = frametime;
		*LocalPlayer->GetPointerFlags() = iFlags;
	}
}

int kek = 0;
int autism = 0;


void SetClanTag(const char* tag, const char* name)
{

	static auto pSetClanTag = reinterpret_cast<void(__fastcall*)(const char*, const char*)>(((DWORD)Utilities::Memory::FindPattern("engine.dll", (PBYTE)"\x53\x56\x57\x8B\xDA\x8B\xF9\xFF\x15\x00\x00\x00\x00\x6A\x24\x8B\xC8\x8B\x30", "xxxxxxxxx????xxxxxx")));
	pSetClanTag(tag, name);

}
void NoClantag()
{

	SetClanTag("", "");

}

void ClanTag()
{
	static int counter = 0;

	if (!Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		SetClanTag("paperware", "paperware");

	switch (Menu::Window.MiscTab.OtherClantag.GetIndex())
	{
	case 0:
		SetClanTag("", "");

		break;
	case 1:
	{
		static int motion = 0;
		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 3;

		if (counter % 48 == 0)
			motion++;
		int value = ServerTime % 5;
		switch (value) {
		case 0:SetClanTag("paperware", "aristois"); break;
		case 1:SetClanTag("            ", "aristois"); break;
		}
		counter++;

		break;
	}
	case 2:
	{
		static int motion = 0;
		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 3;
		if (counter % 48 == 0)
			motion++;
		int value = ServerTime % 30;
		switch (value) {
		case 0:SetClanTag("WARE   PAPER", "aristois"); break;
		case 1:SetClanTag("ARE   PAPERW", "aristois"); break;
		case 2:SetClanTag("RE   PAPERWA", "aristois"); break;
		case 3:SetClanTag("E   PAPERWAR", "aristois"); break;
		case 4:SetClanTag("   PAPERWARE", "aristois"); break;
		case 5:SetClanTag("  PAPERWARE ", "aristois"); break;
		case 6:SetClanTag(" PAPERWARE  ", "aristois"); break;
		case 7:SetClanTag("PAPERWARE   ", "aristois"); break;
		case 8:SetClanTag("APERWARE    P", "aristois"); break;
		case 9:SetClanTag("PERWARE    PA", "aristois"); break;
		case 10:SetClanTag("ERWARE   PAP", "aristois"); break;
		case 11:SetClanTag("WARE   PAPER", "aristois"); break;
		}
		counter++;

		break;
	  }

	}
}

bool __stdcall CreateMoveClient_Hooked(float frametime, CUserCmd* pCmd)
{
	if (!pCmd->command_number)
		return true;

	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && pLocal)
	{

		PVOID pebp;
		__asm mov pebp, ebp;
		bool* pbSendPacket = (bool*)(*(DWORD*)pebp - 0x1C);
		bool& bSendPacket = *pbSendPacket;

		if (Menu::Window.MiscTab.OtherClantag.GetIndex() > 0)
			ClanTag();
		Vector origView = pCmd->viewangles;
		Vector viewforward, viewright, viewup, aimforward, aimright, aimup;
		Vector qAimAngles;
		qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
		AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);

		for (int i = 1; i < Interfaces::Engine->GetMaxClients(); i++) {
			IClientEntity* pBaseEntity = Interfaces::EntList->GetClientEntity(i);
			if (pBaseEntity && !pBaseEntity->IsDormant() && pBaseEntity != hackManager.pLocal()) globalsh.OldSimulationTime[i] = pBaseEntity->GetSimulationTime();
			if (pBaseEntity && !pBaseEntity->IsDormant() && pBaseEntity != hackManager.pLocal() && Menu::Window.RageBotTab.AccuracyBacktracking.GetState()) DataManager.UpdatePlayerPos();
		}

		IClientEntity* pEntity;
		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && pLocal && pLocal->IsAlive())
		{
			Hacks::MoveHacks(pCmd, bSendPacket);
			ResolverSetup::GetInst().CM(pEntity);
		}
		backtracking->legitBackTrack(pCmd, pLocal);

		if (Menu::Window.MiscTab.FakeLagEnable.GetState())
		{
			static int ticks = 0;
			static int ticks1 = 0;
			static int iTick = 0;
			static int iTick1 = 0;
			static int iTick2 = 0;
			int ticksMax = 16;
			int value = Menu::Window.MiscTab.FakeLagChoke.GetValue();
			if (Menu::Window.MiscTab.FakeLagEnable.GetState() && value > 0 && Menu::Window.MiscTab.FakeLagTyp.GetIndex() == 2)
			{
				if (ticks >= ticksMax)
				{
					bSendPacket = true;
					ticks = 0;
				}
				else
				{
					int packetsToChoke;
					if (pLocal->GetVelocity().Length() > 0.f)
					{
						packetsToChoke = (int)((128.f / Interfaces::Globals->interval_per_tick) / pLocal->GetVelocity().Length()) + 1;
						if (packetsToChoke >= 15)
							packetsToChoke = 14;
						if (packetsToChoke < value)
							packetsToChoke = value;
					}
					else
						packetsToChoke = 0;

					bSendPacket = ticks < 18 - packetsToChoke;;
				}
				ticks++;
			}

			static bool abc = false;
			if (Menu::Window.VisualsTab.NightMode.GetState())
			{
				if (!abc)
				{
					ConVar* staticdrop = Interfaces::CVar->FindVar("r_DrawSpecificStaticProp");
					SpoofedConvar* staticdrop_spoofed = new SpoofedConvar(staticdrop);
					staticdrop_spoofed->SetInt(0);

					{
						for (MaterialHandle_t i = Interfaces::MaterialSystem->FirstMaterial(); i != Interfaces::MaterialSystem->InvalidMaterial(); i = Interfaces::MaterialSystem->NextMaterial(i))
						{
							IMaterial *pMaterial = Interfaces::MaterialSystem->GetMaterial(i);

							if (!pMaterial)
								continue;

							if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))
							{
								pMaterial->ColorModulate(0.1f, 0.1f, 0.1f);
							}
							if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
							{
								pMaterial->ColorModulate(0.3f, 0.3f, 0.3f);
							}
						}
					}
				}
				abc = true;
			}
			else
			{
				for (MaterialHandle_t i = Interfaces::MaterialSystem->FirstMaterial(); i != Interfaces::MaterialSystem->InvalidMaterial(); i = Interfaces::MaterialSystem->NextMaterial(i))
				{

					IMaterial *pMaterial = Interfaces::MaterialSystem->GetMaterial(i);

					if (!pMaterial)
						continue;

					if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))
					{
						pMaterial->ColorModulate(1.0f, 1.0f, 1.0f);
					}
					if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
					{
						pMaterial->ColorModulate(1.0f, 1.0f, 1.0f);
					}
				}
				abc = false;
			}

			if (Menu::Window.VisualsTab.NightSky.GetState())
			{
				ConVar* sky = Interfaces::CVar->FindVar("sv_skyname");
				SpoofedConvar* sky_spoofed = new SpoofedConvar(sky);
				sky_spoofed->SetString("sky_csgo_night02");
			}


			qAimAngles.Init(0.0f, GetAutostrafeView().y, 0.0f);
			AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);
			qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
			AngleVectors(qAimAngles, &aimforward, &aimright, &aimup);
			Vector vForwardNorm;		Normalize(viewforward, vForwardNorm);
			Vector vRightNorm;			Normalize(viewright, vRightNorm);
			Vector vUpNorm;				Normalize(viewup, vUpNorm);

			float forward = pCmd->forwardmove;
			float right = pCmd->sidemove;
			float up = pCmd->upmove;
			if (forward > 450) forward = 450;
			if (right > 450) right = 450;
			if (up > 450) up = 450;
			if (forward < -450) forward = -450;
			if (right < -450) right = -450;
			if (up < -450) up = -450;
			pCmd->forwardmove = DotProduct(forward * vForwardNorm, aimforward) + DotProduct(right * vRightNorm, aimforward) + DotProduct(up * vUpNorm, aimforward);
			pCmd->sidemove = DotProduct(forward * vForwardNorm, aimright) + DotProduct(right * vRightNorm, aimright) + DotProduct(up * vUpNorm, aimright);
			pCmd->upmove = DotProduct(forward * vForwardNorm, aimup) + DotProduct(right * vRightNorm, aimup) + DotProduct(up * vUpNorm, aimup);


			if (Menu::Window.MiscTab.FakeLagEnable.GetState() && value > 0 && Menu::Window.MiscTab.FakeLagTyp.GetIndex() == 4)
			{
				if (!(pLocal->GetFlags() & FL_ONGROUND))
				{
					if (ticks1 >= ticksMax)
					{
						bSendPacket = true;
						ticks1 = 0;
					}
					else
					{

						int packetsToChoke;
						if (pLocal->GetVelocity().Length() > 0.f)
						{
							packetsToChoke = (int)((128.f / Interfaces::Globals->interval_per_tick) / pLocal->GetVelocity().Length()) + 1;
							if (packetsToChoke >= 15)
								packetsToChoke = 14;
							if (packetsToChoke < value)
								packetsToChoke = value;
						}
						else
							packetsToChoke = 0;

						bSendPacket = ticks1 < 18 - packetsToChoke;;
					}
					ticks1++;
				}
			}
			if (Menu::Window.MiscTab.FakeLagEnable.GetState() && value > 0 && Menu::Window.MiscTab.FakeLagTyp.GetIndex() == 1)
			{


				if (iTick < value) {
					bSendPacket = false;
					iTick++;
				}
				else {
					bSendPacket = true;
					iTick = 0;
				}
			}
			if (Menu::Window.MiscTab.FakeLagEnable.GetState() && value > 0 && Menu::Window.MiscTab.FakeLagTyp.GetIndex() == 3)
			{

				if (!(pLocal->GetFlags() & FL_ONGROUND))
				{
					if (iTick1 < value) {
						bSendPacket = false;
						iTick1++;
					}
					else {
						bSendPacket = true;
						iTick1 = 0;
					}
				}
			}
			if (Menu::Window.MiscTab.FakeLagEnable.GetState() && value > 0 && Menu::Window.MiscTab.FakeLagTyp.GetIndex() == 5)
			{

				value = LagCompBreak();
				if (iTick2 < value) {
					bSendPacket = false;
					iTick2++;
				}
				else {
					bSendPacket = true;
					iTick2 = 0;
				}

			}
		}


		static bool abc = false;

		qAimAngles.Init(0.0f, GetAutostrafeView().y, 0.0f); 
		AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);
		qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
		AngleVectors(qAimAngles, &aimforward, &aimright, &aimup);
		Vector vForwardNorm;		Normalize(viewforward, vForwardNorm);
		Vector vRightNorm;			Normalize(viewright, vRightNorm);
		Vector vUpNorm;				Normalize(viewup, vUpNorm);

		float forward = pCmd->forwardmove;
		float right = pCmd->sidemove;
		float up = pCmd->upmove;
		if (forward > 450) forward = 450;
		if (right > 450) right = 450;
		if (up > 450) up = 450;
		if (forward < -450) forward = -450;
		if (right < -450) right = -450;
		if (up < -450) up = -450;
		pCmd->forwardmove = DotProduct(forward * vForwardNorm, aimforward) + DotProduct(right * vRightNorm, aimforward) + DotProduct(up * vUpNorm, aimforward);
		pCmd->sidemove = DotProduct(forward * vForwardNorm, aimright) + DotProduct(right * vRightNorm, aimright) + DotProduct(up * vUpNorm, aimright);
		pCmd->upmove = DotProduct(forward * vForwardNorm, aimup) + DotProduct(right * vRightNorm, aimup) + DotProduct(up * vUpNorm, aimup);

		if (Menu::Window.MiscTab.OtherSafeMode.GetState())
		{
			GameUtils::NormaliseViewAngle(pCmd->viewangles);

			if (pCmd->viewangles.z != 0.0f)
			{
				pCmd->viewangles.z = 0.00;
			}

			if (pCmd->viewangles.x < -89 || pCmd->viewangles.x > 89 || pCmd->viewangles.y < -180 || pCmd->viewangles.y > 180)
			{
				Utilities::Log("Having to re-normalise!");
				GameUtils::NormaliseViewAngle(pCmd->viewangles);
				Beep(750, 800); 
				if (pCmd->viewangles.x < -89 || pCmd->viewangles.x > 89 || pCmd->viewangles.y < -180 || pCmd->viewangles.y > 180)
				{
					pCmd->viewangles = origView;
					pCmd->sidemove = right;
					pCmd->forwardmove = forward;
				}
			}
		}

		if (pCmd->viewangles.x > 90)
		{
			pCmd->forwardmove = -pCmd->forwardmove;
		}

		if (pCmd->viewangles.x < -90)
		{
			pCmd->forwardmove = -pCmd->forwardmove;
		}
		// LBY
		LBYThirdpersonAngle = Vector(pLocal->GetEyeAnglesXY()->x, pLocal->GetLowerBodyYaw(), pLocal->GetEyeAnglesXY()->z);

		if (bSendPacket == true) {
			LastAngleAA = pCmd->viewangles;
		}
		else if (bSendPacket == false) {
			LastAngleAAReal = pCmd->viewangles;
		}

		lineLBY = pLocal->GetLowerBodyYaw();
		if (bSendPacket == true) {
			lineFakeAngle = pCmd->viewangles.y;
		}
		else if (bSendPacket == false) {
			lineRealAngle = pCmd->viewangles.y;
		}

	}
	return false;
}


void __fastcall PaintTraverse_Hooked(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce)
{
	if (Menu::Window.VisualsTab.Active.GetState() && Menu::Window.VisualsTab.OtherNoScope.GetState() && strcmp("HudZoom", Interfaces::Panels->GetName(vguiPanel)) == 0)
		return;

	static unsigned int FocusOverlayPanel = 0;
	static bool FoundPanel = false;

	if (!FoundPanel)
	{

		PCHAR szPanelName = (PCHAR)Interfaces::Panels->GetName(vguiPanel);
		if (strstr(szPanelName, XorStr("MatSystemTopPanel")))
		{

			FocusOverlayPanel = vguiPanel;
			FoundPanel = true;
		}

	}
	else if (FocusOverlayPanel == vguiPanel)
	{

		if (Menu::Window.VisualsTab.LBYIdicador.GetState())
		{
			CUserCmd* cmdlist = *(CUserCmd**)((DWORD)Interfaces::pInput + 0xEC);
			CUserCmd* pCmd = cmdlist;

			IClientEntity* localplayer = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
			RECT TextSize = Render::GetTextSize(Render::Fonts::LBY, "LBY");
			RECT scrn = Render::GetViewport();
			if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
				if (pCmd->viewangles.y - *localplayer->GetLowerBodyYawTarget() >= -35 && pCmd->viewangles.y - *localplayer->GetLowerBodyYawTarget() <= 35)
					Render::Text(10, scrn.bottom - 80, Color(255, 0, 0, 255), Render::Fonts::LBY, "LBY");
				else
					Render::Text(10, scrn.bottom - 80, Color(0, 255, 95, 255), Render::Fonts::LBY, "LBY");
		}


			{

				Render::Text(1, 7, Color(255, 255, 255, 255), Render::Fonts::ESP, ("paperware framework"));

			}
			if (Menu::Window.VisualsTab.Angleinfo.GetState())
			{
				char bufferLineLBY[64];
				sprintf_s(bufferLineLBY, "LBY:  %.1f", lineLBY);
				Render::Text(50, 19, Color(255, 255, 255, 255), Render::Fonts::ESP, bufferLineLBY);

				char bufferlineRealAngle[64];
				sprintf_s(bufferlineRealAngle, "Real:  %.1f", lineRealAngle);
				Render::Text(50, 29, Color(0, 255, 15, 255), Render::Fonts::ESP, bufferlineRealAngle);

				char bufferlineFakeAngle[64];
				sprintf_s(bufferlineFakeAngle, "Fake:  %.1f", lineFakeAngle);
				Render::Text(50, 39, Color(255, 178, 100, 255), Render::Fonts::ESP, bufferlineFakeAngle);
			} 

			if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
				Hacks::DrawHacks();

			Menu::DoUIFrame();

			if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && Menu::Window.VisualsTab.OtherHitmarker.GetState())
				hitmarker::singleton()->on_paint();
	}
	oPaintTraverse(pPanels, vguiPanel, forceRepaint, allowForce);
}


void __fastcall  hkSceneEnd(void *pEcx, void *pEdx) {


	 if (Menu::Window.MiscTab.FakeAngleChams.GetState())
	{
		IClientEntity* pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
		if (pLocal)
		{
			static  IMaterial* CoveredLit = CreateMaterial(true);
			if (CoveredLit)
			{
				Vector OrigAng;
				OrigAng = pLocal->GetEyeAngles();
				pLocal->SetAngle2(Vector(0, lineFakeAngle, 0));

				bool LbyColor = false;
				float NormalColor[3] = { 1, 1, 1 };
				float lbyUpdateColor[3] = { 0, 1, 0 };
				Interfaces::RenderView->SetColorModulation(LbyColor ? lbyUpdateColor : NormalColor);
				Interfaces::ModelRender->ForcedMaterialOverride(CoveredLit);
				pLocal->draw_model(STUDIO_RENDER, 255);
				Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
				pLocal->SetAngle2(OrigAng);
			}
		}
	}
}




bool __stdcall Hooked_InPrediction()
{

	bool result;
	static InPrediction_ origFunc = (InPrediction_)Hooks::VMTPrediction.GetOriginalFunction(14);
	static DWORD *ecxVal = Interfaces::Prediction;
	result = origFunc(ecxVal);

	if (Menu::Window.VisualsTab.OtherNoVisualRecoil.GetState() && (DWORD)(_ReturnAddress()) == Offsets::Functions::dwCalcPlayerView)
	{

		IClientEntity* pLocalEntity = NULL;

		float* m_LocalViewAngles = NULL;

		__asm
		{
			MOV pLocalEntity, ESI
			MOV m_LocalViewAngles, EBX
		}

		Vector viewPunch = pLocalEntity->localPlayerExclusive()->GetViewPunchAngle();
		Vector aimPunch = pLocalEntity->localPlayerExclusive()->GetAimPunchAngle();

		m_LocalViewAngles[0] -= (viewPunch[0] + (aimPunch[0] * 2 * 0.4499999f));
		m_LocalViewAngles[1] -= (viewPunch[1] + (aimPunch[1] * 2 * 0.4499999f));
		m_LocalViewAngles[2] -= (viewPunch[2] + (aimPunch[2] * 2 * 0.4499999f));
		return true;
	}
	return result;
}


player_info_t GetInfo(int Index) {
	player_info_t Info;
	Interfaces::Engine->GetPlayerInfo(Index, &Info);
	return Info;
}

typedef void(__cdecl* MsgFn)(const char* msg, va_list);
void Msg(const char* msg, ...)
{

	if (msg == nullptr)
		return; //If no string was passed, or it was null then don't do anything
	static MsgFn fn = (MsgFn)GetProcAddress(GetModuleHandle("tier0.dll"), "Msg"); //This gets the address of export "Msg" in the dll "tier0.dll". The static keyword means it's only called once and then isn't called again (but the variable is still there)
	char buffer[989];
	va_list list; //Normal varargs stuff http://stackoverflow.com/questions/10482960/varargs-to-printf-all-arguments
	va_start(list, msg);

	vsprintf(buffer, msg, list);
	va_end(list);

	fn(buffer, list); //Calls the function, we got the address above.
}

int Kills2 = 0;
int Kills = 0;
bool RoundInfo = false;
size_t Delay = 0;
bool flipAA;
bool __fastcall Hooked_FireEventClientSide(PVOID ECX, PVOID EDX, IGameEvent *Event)
{

	if (Menu::Window.MiscTab.EnableBuyBot.GetState())
	{
		if (Menu::Window.MiscTab.BuyBot.GetIndex() == 1)
		{

			if (strcmp(Event->GetName(), "round_start") == 0)
				Interfaces::Engine->ClientCmd_Unrestricted("buy ak47; buy m4a1;");
		}
		else if (Menu::Window.MiscTab.BuyBot.GetIndex() == 2)
		{

			if (strcmp(Event->GetName(), "round_start") == 0)
				Interfaces::Engine->ClientCmd_Unrestricted("buy awp;");
		}
		else if (Menu::Window.MiscTab.BuyBot.GetIndex() == 3)
		{
			if (strcmp(Event->GetName(), "round_start") == 0) 
				Interfaces::Engine->ClientCmd_Unrestricted("buy scar20; buy g3sg1;buy elite;");
		}
	}

	if (Menu::Window.MiscTab.EnableBuyBot.GetState())
	{

		if (Menu::Window.MiscTab.BuyBotGrenades.GetIndex() == 1)
		{
			if (strcmp(Event->GetName(), "round_start") == 0)
				Interfaces::Engine->ClientCmd_Unrestricted("buy flashbang; buy flashbang; buy smokegrenade; buy hegrenade;");
		}
		else if (Menu::Window.MiscTab.BuyBotGrenades.GetIndex() == 2)
		{

			if (strcmp(Event->GetName(), "round_start") == 0)
				Interfaces::Engine->ClientCmd_Unrestricted("buy flashbang; buy smokegrenade; buy hegrenade; buy molotov; buy incgrenade;");
		}
	}

	if (Menu::Window.MiscTab.EnableBuyBot.GetState())
	{
		if (Menu::Window.MiscTab.BuyBotKevlar.GetState())
		{

			if (strcmp(Event->GetName(), "round_start") == 0)
				Interfaces::Engine->ClientCmd_Unrestricted("buy vest; buy vesthelm;");
		}
	}

	if (Menu::Window.MiscTab.EnableBuyBot.GetState())
	{
		if (Menu::Window.MiscTab.BuyBotDefuser.GetState())
		{


			if (strcmp(Event->GetName(), "round_start") == 0)
				Interfaces::Engine->ClientCmd_Unrestricted("buy defuser;");
		}

	}

	if (Menu::Window.RageBotTab.AimbotResolver.GetState() == 2)
	{

		if (!strcmp(Event->GetName(), "player_hurt"))
		{

			int deadfag = Event->GetInt("userid");
			int attackingfag = Event->GetInt("attacker");
			IClientEntity* pLocal = hackManager.pLocal();
			if (Interfaces::Engine->GetPlayerForUserID(deadfag) != Interfaces::Engine->GetLocalPlayer() && Interfaces::Engine->GetPlayerForUserID(attackingfag) == Interfaces::Engine->GetLocalPlayer())
			{
				IClientEntity* hittedplayer = (IClientEntity*)(Interfaces::Engine->GetPlayerForUserID(deadfag));
				int hit = Event->GetInt("hitgroup");
				if (hit == 1 && hittedplayer && deadfag && attackingfag)
				{
					Resolver::didhitHS = true;
					Globals::missedshots = 0;
				}
				else
				{
					Resolver::didhitHS = false;
					Globals::missedshots++;
				}
			}
		}

	}

	if (Menu::Window.RageBotTab.FlipAA.GetState())
	{

		if (!strcmp(Event->GetName(), "player_hurt"))
		{

			int deadfag = Event->GetInt("userid");
			int attackingfag = Event->GetInt("attacker");
			IClientEntity* pLocal = hackManager.pLocal();
			if (Interfaces::Engine->GetPlayerForUserID(deadfag) == Interfaces::Engine->GetLocalPlayer() && Interfaces::Engine->GetPlayerForUserID(attackingfag) != Interfaces::Engine->GetLocalPlayer())
			{

				flipAA = true;

			}
			else
			{

				flipAA = false;
			}

		}
	}
	if (Menu::Window.VisualsTab.Logs.GetState())
	{
		if (!strcmp(Event->GetName(), "item_purchase"))
		{

			int nUserID = Event->GetInt("attacker");
			int nDead = Event->GetInt("userid");
			if (nUserID || nDead)
			{
				player_info_t killed_info = GetInfo(Interfaces::Engine->GetPlayerForUserID(nDead));
				player_info_t killer_info = GetInfo(Interfaces::Engine->GetPlayerForUserID(nUserID));
				std::string before = ("[paperware] ");
				std::string one = killed_info.name;
				std::string two = (" bought ");
				std::string three = Event->GetString("weapon");
				std::string six = "\n";
				if (Menu::Window.VisualsTab.Logs.GetState())
				{

					Msg((before + one + two + three + six).c_str());

				}

			}
		}
		if (Menu::Window.VisualsTab.Logs.GetState())
		{
			if (!strcmp(Event->GetName(), "player_hurt"))
			{

				int attackerid = Event->GetInt("attacker");
				int entityid = Interfaces::Engine->GetPlayerForUserID(attackerid);
				if (entityid == Interfaces::Engine->GetLocalPlayer())
				{

					int nUserID = Event->GetInt("attacker");
					int nDead = Event->GetInt("userid");
					if (nUserID || nDead)
					{

						player_info_t killed_info = GetInfo(Interfaces::Engine->GetPlayerForUserID(nDead));
						player_info_t killer_info = GetInfo(Interfaces::Engine->GetPlayerForUserID(nUserID));
						std::string before = ("[paperware] ");
						std::string two = ("Hit ");
						std::string three = killed_info.name;
						std::string foura = " for ";
						std::string fivea = Event->GetString("dmg_health");
						std::string damage = " damage";
						std::string fourb = " (";
						std::string fiveb = Event->GetString("health");
						std::string six = " health remaining)";
						std::string newline = "\n";
						if (Menu::Window.VisualsTab.Logs.GetState())
						{
							Msg((before + two + three + foura + fivea + damage + fourb + fiveb + six + newline).c_str());
						}

					}
				}

			}

		}



	}
	return oFireEventClientSide(ECX, Event);
}


void StartPrediction(IClientEntity* LocalPlayer, CUserCmd* pCmd)
{
	static bool done = false;
	if (LocalPlayer->IsAlive() && Menu::Window.RageBotTab.AimbotEnginePred.GetState() && !done)
	{
		//Interfaces::CVar->FindVar("rate")->SetValue(1048576);
		//Interface->Cvar()->GetVTable<ICvar>()->FindVar( "viewmodel_fov" )->SetValue( 80 );
		Interfaces::CVar->FindVar("cl_interp")->SetValue(0.01f);
		Interfaces::CVar->FindVar("cl_cmdrate")->SetValue(66);
		Interfaces::CVar->FindVar("cl_updaterate")->SetValue(66);
		Interfaces::CVar->FindVar("cl_interp_all")->SetValue(0.0f);
		Interfaces::CVar->FindVar("cl_interp_ratio")->SetValue(1.0f);
		Interfaces::CVar->FindVar("cl_smooth")->SetValue(0.0f);
		Interfaces::CVar->FindVar("cl_smoothtime")->SetValue(0.01f);
		done = true;
	}
}



void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld)
{
	if (!Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;

	Color color;
	float flColor[3] = { 0.f };
	static IMaterial* CoveredLit = CreateMaterial(true);
	static IMaterial* OpenLit = CreateMaterial(false);
	static IMaterial* CoveredFlat = CreateMaterial(true, false);
	static IMaterial* OpenFlat = CreateMaterial(false, false);
	bool DontDraw = false;
	const char* ModelName = Interfaces::ModelInfo->GetModelName((model_t*)pInfo.pModel);
	IClientEntity* pModelEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(pInfo.entity_index);
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (Menu::Window.VisualsTab.Active.GetState())
	{

		if (Menu::Window.MiscTab.OtherThirdperson.GetState())
		{

			Interfaces::RenderView->SetBlend(0.75f);

		}

		int ChamsStyle = Menu::Window.VisualsTab.OptionsChams.GetIndex();
		int HandsStyle = Menu::Window.VisualsTab.OtherNoHands.GetIndex();
		if (ChamsStyle != 0 && Menu::Window.VisualsTab.FiltersPlayers.GetState() && strstr(ModelName, "models/player"))
		{
			if (pModelEntity == nullptr)
				return;

			if (pLocal && (!Menu::Window.VisualsTab.FiltersEnemiesOnly.GetState() ||
				 pModelEntity->GetTeamNum() != pLocal->GetTeamNum()))
			{
				IMaterial *covered = ChamsStyle == 1 ? CoveredLit : CoveredFlat;
				IMaterial *open = ChamsStyle == 1 ? OpenLit : OpenFlat;

				IClientEntity* pModelEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(pInfo.entity_index);
				if (pModelEntity)
				{
					if (pModelEntity == nullptr)
						return;


					IClientEntity *local = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
					if (local)
					{

						if (local == nullptr)
							return;

						if (pModelEntity->IsAlive() && pModelEntity->GetHealth() > 0 && pModelEntity->GetTeamNum() != local->GetTeamNum())
						{
							float alpha = 1.f;

							if (pModelEntity->HasGunGameImmunity())
								alpha = 0.5f;

							if (pModelEntity->GetTeamNum() == 2)
							{

								flColor[0] = Menu::Window.ColorsTab.TChamsR.GetValue() / 255.f;
								flColor[1] = Menu::Window.ColorsTab.TChamsG.GetValue() / 255.f;
								flColor[2] = Menu::Window.ColorsTab.TChamsB.GetValue() / 255.f;
							}
							else
							{

								flColor[0] = Menu::Window.ColorsTab.CTChamsR.GetValue() / 255.f;
								flColor[1] = Menu::Window.ColorsTab.CTChamsG.GetValue() / 255.f;
								flColor[2] = Menu::Window.ColorsTab.CTChamsB.GetValue() / 255.f;

							}

							if (Menu::Window.VisualsTab.ChamsVisibleOnly.GetState())
							{
								Interfaces::RenderView->SetColorModulation(flColor);
								Interfaces::RenderView->SetBlend(1);
								Interfaces::ModelRender->ForcedMaterialOverride(covered);
								oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
							}
							else
							{

								Interfaces::RenderView->SetColorModulation(flColor);
								Interfaces::RenderView->SetBlend(1);
								oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
							}

							if (pModelEntity->GetTeamNum() == 2)
							{

								flColor[0] = Menu::Window.ColorsTab.TChamsR.GetValue() / 255.f;
								flColor[1] = Menu::Window.ColorsTab.TChamsG.GetValue() / 255.f;
								flColor[2] = Menu::Window.ColorsTab.TChamsB.GetValue() / 255.f;
							}
							else
							{
								flColor[0] = Menu::Window.ColorsTab.CTChamsR.GetValue() / 255.f;
								flColor[1] = Menu::Window.ColorsTab.CTChamsG.GetValue() / 255.f;
								flColor[2] = Menu::Window.ColorsTab.CTChamsB.GetValue() / 255.f;
							}

							Interfaces::RenderView->SetColorModulation(flColor);

							Interfaces::ModelRender->ForcedMaterialOverride(open);
						}
						else
						{
							color.SetColor(255, 255, 255, 255);
							ForceMaterial(color, open);
						}
					}
				}

			}

		}

		else if (HandsStyle != 0 && strstr(ModelName, XorStr("arms")))
		{
			if (HandsStyle == 1)
			{

				DontDraw = true;
			}
			else if (HandsStyle == 2)
			{

				Interfaces::RenderView->SetBlend(0.3);

			}

			else if (HandsStyle == 3)
			{

				flColor[0] = 10.f / 255.f;
				flColor[1] = 10.f / 255.f;
				flColor[2] = 10.f / 255.f;
				Interfaces::RenderView->SetColorModulation(flColor);
				static IMaterial* wire = CreateMaterial(true, false, true);
				IMaterial *handy = wire;
				handy->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
				Interfaces::ModelRender->ForcedMaterialOverride(handy);
			}

			else if (HandsStyle == 4)
			{

				IMaterial *covered = ChamsStyle == 1 ? CoveredLit : CoveredFlat;
				IMaterial *open = ChamsStyle == 1 ? OpenLit : OpenFlat;
				if (pLocal)
				{

					if (pLocal->IsAlive())
					{
						int alpha = pLocal->HasGunGameImmunity() ? 150 : 255;

						if (pLocal->GetTeamNum() == 2)
							color.SetColor(240, 30, 35, alpha);
						else
							color.SetColor(63, 72, 205, alpha);

						ForceMaterial(color, covered);
						oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

						if (pLocal->GetTeamNum() == 2)
							color.SetColor(247, 180, 20, alpha);
						else
							color.SetColor(32, 180, 57, alpha);
					}
					else
					{

						color.SetColor(255, 255, 255, 255);
					}

					ForceMaterial(color, open);
				}

			}
			else
			{
				static int counter = 0;
				static float colors[3] = { 1.f, 0.f, 0.f };

				if (colors[counter] >= 1.0f)
				{
					colors[counter] = 1.0f;
					counter += 1;
					if (counter > 2)
						counter = 0;
				}
				else
				{

					int prev = counter - 1;
					if (prev < 0) prev = 2;
					colors[prev] -= 0.05f;
					colors[counter] += 0.05f;
				}

				Interfaces::RenderView->SetColorModulation(colors);
				Interfaces::RenderView->SetBlend(0.5);
				Interfaces::ModelRender->ForcedMaterialOverride(OpenLit);
			}
		}

	}

	if (!DontDraw)
		oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
	Interfaces::ModelRender->ForcedMaterialOverride(NULL); 
}

int RandomInt(int min, int max)
{
	return rand() % max + min;

}

std::vector<const char*> smoke_materials = {
	"particle/vistasmokev1/vistasmokev1_smokegrenade",
	"particle/vistasmokev1/vistasmokev1_emods",
	"particle/vistasmokev1/vistasmokev1_emods_impactdust",
	"particle/vistasmokev1/vistasmokev1_fire",

};


float Bolbilize(float Yaw)
{
	if (Yaw > 180)
	{
		Yaw -= (round(Yaw / 360) * 360.f);
	}
	else if (Yaw < -180)
	{
		Yaw += (round(Yaw / 360) * -360.f);
	}
	return Yaw;
}

Vector CalcAngle69(Vector dst, Vector src)
{
	Vector angles;

	double delta[3] = { (src.x - dst.x), (src.y - dst.y), (src.z - dst.z) };
	double hyp = sqrt(delta[1] * delta[1] + delta[2] * delta[2]);
	angles.x = (float)(atan(delta[2] / hyp) * 180.0 / 3.14159265);
	angles.y = (float)(atanf(delta[1] / delta[0]) * 57.295779513082f);
	angles.z = 0.0f;

	if (delta[0] >= 1.9)
	{
		angles.y += 180.0f;
	}

	return angles;
}
//Vector LastAngleAA;

namespace resolvokek
{
	int Shots;
	int missedshots;
	float RealAngle;
	float FakeAngle;
	Vector AimPoint;
	bool shouldflip;
	bool ySwitch;
	float NextTime;
	int resolvemode = 1;
	float fakeAngle;
	float OldSimulationTime[65];
	bool error;
}

#define SETANGLE 180
void  __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage)
{

	DWORD eyeangles = NetVar.GetNetVar(0xBFEA4E7B);
	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	IClientEntity* pEntity = nullptr;

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && curStage == FRAME_RENDER_START)
	{
		static bool rekt1 = false;
		if (Menu::Window.MiscTab.OtherThirdperson.GetState() && pLocal->IsAlive() && pLocal->IsScoped() == 0)
		{
			if (!rekt1)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("thirdperson");
				rekt1 = true;

			}
		}
		else if (!Menu::Window.MiscTab.OtherThirdperson.GetState())
		{
			rekt1 = false;

		}

		static bool rekt = false;
		if (!Menu::Window.MiscTab.OtherThirdperson.GetState() || pLocal->IsAlive() == 0)
		{
			if (!rekt)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("firstperson");
				rekt = true;

			}
		}
		else if (Menu::Window.MiscTab.OtherThirdperson.GetState() || pLocal->IsAlive() == 0)
		{
			rekt = false;
		}

		static bool meme = false;
		if (Menu::Window.MiscTab.OtherThirdperson.GetState() && pLocal->IsScoped() == 0)
		{
			if (!meme)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("thirdperson");
				meme = true;
			}
		}
		else if (pLocal->IsScoped())
		{

			meme = false;
		}

		static bool kek = false;
		if (Menu::Window.MiscTab.OtherThirdperson.GetState() && pLocal->IsAlive())
		{
			if (!kek)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("thirdperson");
				kek = true;
			}
		}
		else if (pLocal->IsAlive() == 0)
		{
			kek = false;
		}	if (pLocal->IsAlive() && Menu::Window.MiscTab.OtherThirdperson.GetState())
		{
			Vector thirdpersonMode;

			switch (Menu::Window.MiscTab.SeeTpangle.GetIndex())
			{
			case 0:

				thirdpersonMode = LastAngleAAReal;
				break;
			case 1:
				thirdpersonMode = LastAngleAA;
				break;
			case 2:
				thirdpersonMode = LBYThirdpersonAngle;
				break;
			}


			static bool rekt = false;
			if (!rekt)
			{
				ConVar* sv_cheats = Interfaces::CVar->FindVar("sv_cheats");
				SpoofedConvar* sv_cheats_spoofed = new SpoofedConvar(sv_cheats);
				sv_cheats_spoofed->SetInt(1);
				rekt = true;
			}


			static bool kek = false;

			if (!kek)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("thirdperson");
				kek = true;
			}

			static bool toggleThirdperson;
			static float memeTime;
			int ThirdPersonKey = Menu::Window.MiscTab.ThirdPersonKeyBind.GetKey();
			if (ThirdPersonKey >= 0 && GUI.GetKeyState(ThirdPersonKey) && abs(memeTime - Interfaces::Globals->curtime) > 0.5)
			{
				toggleThirdperson = !toggleThirdperson;
				memeTime = Interfaces::Globals->curtime;
			}


			if (toggleThirdperson)
			{
				Interfaces::pInput->m_fCameraInThirdPerson = true;
				if (*(bool*)((DWORD)Interfaces::pInput + 0xA5))
					*(Vector*)((DWORD)pLocal + 0x31C8) = thirdpersonMode;
			}
			else {
				// No Thirdperson
				static Vector vecAngles;
				Interfaces::Engine->GetViewAngles(vecAngles);
				Interfaces::pInput->m_fCameraInThirdPerson = false;
				Interfaces::pInput->m_vecCameraOffset = Vector(vecAngles.x, vecAngles.y, 0);
			}


		}
		else if (pLocal->IsAlive() == 0)
		{

			kek = false;

		}

		if (!Menu::Window.MiscTab.OtherThirdperson.GetState()) {

			// No Thirdperson
			static Vector vecAngles;
			Interfaces::Engine->GetViewAngles(vecAngles);
			Interfaces::pInput->m_fCameraInThirdPerson = false;
			Interfaces::pInput->m_vecCameraOffset = Vector(vecAngles.x, vecAngles.y, 0);
		}

		if (curStage == FRAME_RENDER_START)
		{
			for (int i = 1; i <= Interfaces::Globals->maxClients; i++)
			{
				if (i == Interfaces::Engine->GetLocalPlayer()) continue;

				IClientEntity* pCurEntity = Interfaces::EntList->GetClientEntity(i);
				if (!pCurEntity) continue;

				*(int*)((uintptr_t)pCurEntity + 0xA30) = Interfaces::Globals->framecount;
				*(int*)((uintptr_t)pCurEntity + 0xA28) = 0;
			}
			for (auto matName : smoke_materials)
			{
				IMaterial* mat = Interfaces::MaterialSystem->FindMaterial(matName, "Other textures");
			
			}
		}

	}


	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START) {
		{


		}

		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		{

			IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
			ResolverSetup::GetInst().FSN(pEntity, curStage);
			int iBayonet = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_bayonet.mdl");
			int iButterfly = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_butterfly.mdl");
			int iFlip = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_flip.mdl");
			int iGut = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_gut.mdl");
			int iKarambit = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_karam.mdl");
			int iM9Bayonet = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_m9_bay.mdl");
			int iHuntsman = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_tactical.mdl");
			int iFalchion = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_falchion_advanced.mdl");
			int iDagger = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_push.mdl");
			int iBowie = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_survival_bowie.mdl");
			int iGunGame = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_gg.mdl");

			for (int i = 0; i <= Interfaces::EntList->GetHighestEntityIndex(); i++)
			{
				IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);

				if (pEntity)
				{
					if (pEntity == nullptr)
						return;

					ULONG hOwnerEntity = *(PULONG)((DWORD)pEntity + 0x148);

					IClientEntity* pOwner = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)hOwnerEntity);

					if (pOwner)
					{

						if (pOwner == nullptr)
							return;

						if (pOwner == pLocal)
						{
							C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)pEntity;

							ClientClass *pClass = Interfaces::Client->GetAllClasses();

							if (Menu::Window.MiscTab.SkinEnable.GetState())
							{

								int Model = Menu::Window.MiscTab.KnifeModel.GetIndex();
								if (pEntity->GetClientClass()->m_ClassID == (int)CSGOClassID::CKnife)
								{
									if (Model == 0) // Bayonet
									{

										*pWeapon->ModelIndex() = iBayonet;
										*pWeapon->ViewModelIndex() = iBayonet;
										*pWeapon->WorldModelIndex() = iBayonet + 1;
										*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 500;
										*pWeapon->GetEntityQuality() = 3;
										*pWeapon->FallbackPaintKit() = atoi(Menu::Window.MiscTab.KnifeSkin.getText().c_str());
									}
									else if (Model == 1) // Bowie
									{

										*pWeapon->ModelIndex() = iBowie;
										*pWeapon->ViewModelIndex() = iBowie;
										*pWeapon->WorldModelIndex() = iBowie + 1;
										*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 514;
										*pWeapon->GetEntityQuality() = 3;
										*pWeapon->FallbackPaintKit() = atoi(Menu::Window.MiscTab.KnifeSkin.getText().c_str());
									}
									else if (Model == 2) // Butterfly 
									{

										*pWeapon->ModelIndex() = iButterfly;
										*pWeapon->ViewModelIndex() = iButterfly;
										*pWeapon->WorldModelIndex() = iButterfly + 1;
										*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 515;
										*pWeapon->GetEntityQuality() = 3;
										*pWeapon->FallbackPaintKit() = atoi(Menu::Window.MiscTab.KnifeSkin.getText().c_str());
									}
									else if (Model == 3) // Falchion
									{

										*pWeapon->ModelIndex() = iFalchion;
										*pWeapon->ViewModelIndex() = iFalchion;
										*pWeapon->WorldModelIndex() = iFalchion + 1;
										*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 512;
										*pWeapon->GetEntityQuality() = 3;
										*pWeapon->FallbackPaintKit() = atoi(Menu::Window.MiscTab.KnifeSkin.getText().c_str());
									}
									else if (Model == 4) // Flip Knife
									{

										*pWeapon->ModelIndex() = iFlip;
										*pWeapon->ViewModelIndex() = iFlip;
										*pWeapon->WorldModelIndex() = iFlip + 1;
										*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 505;
										*pWeapon->FallbackPaintKit() = atoi(Menu::Window.MiscTab.KnifeSkin.getText().c_str());
									}
									else if (Model == 5) // Gut Knife
									{

										*pWeapon->ModelIndex() = iGut;
										*pWeapon->ViewModelIndex() = iGut;
										*pWeapon->WorldModelIndex() = iGut + 1;
										*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 506;
										*pWeapon->GetEntityQuality() = 3;
										*pWeapon->FallbackPaintKit() = atoi(Menu::Window.MiscTab.KnifeSkin.getText().c_str());
									}
									else if (Model == 6) // Huntsman 
									{

										*pWeapon->ModelIndex() = iHuntsman;

										*pWeapon->ViewModelIndex() = iHuntsman;
										*pWeapon->WorldModelIndex() = iHuntsman + 1;
										*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 509;
										*pWeapon->GetEntityQuality() = 3;
										*pWeapon->FallbackPaintKit() = atoi(Menu::Window.MiscTab.KnifeSkin.getText().c_str());
									}
									else if (Model == 7) // Karambit
									{

										*pWeapon->ModelIndex() = iKarambit;
										*pWeapon->ViewModelIndex() = iKarambit;
										*pWeapon->WorldModelIndex() = iKarambit + 1;

										*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 507;
										*pWeapon->GetEntityQuality() = 3;
										*pWeapon->FallbackPaintKit() = atoi(Menu::Window.MiscTab.KnifeSkin.getText().c_str());
									}

									else if (Model == 8) // M9 Bayonet
									{

										*pWeapon->ModelIndex() = iM9Bayonet;
										*pWeapon->ViewModelIndex() = iM9Bayonet;
										*pWeapon->WorldModelIndex() = iM9Bayonet + 1;
										*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 508;
										*pWeapon->GetEntityQuality() = 3;
										*pWeapon->FallbackPaintKit() = atoi(Menu::Window.MiscTab.KnifeSkin.getText().c_str());
									}

									else if (Model == 10) // Shadow Daggers
									{
										*pWeapon->ModelIndex() = iDagger;
										*pWeapon->ViewModelIndex() = iDagger;
										*pWeapon->WorldModelIndex() = iDagger + 1;

										*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 516;
										*pWeapon->GetEntityQuality() = 3;
										*pWeapon->FallbackPaintKit() = atoi(Menu::Window.MiscTab.KnifeSkin.getText().c_str());
									}
								}

								*pWeapon->OwnerXuidLow() = 0;
								*pWeapon->OwnerXuidHigh() = 0;
								*pWeapon->FallbackWear() = 0.001f;
								*pWeapon->m_AttributeManager()->m_Item()->ItemIDHigh() = 1;
							}
						}
					}
				}

			}
		}
	}
	oFrameStageNotify(curStage);
}


void GetViewModelFOV(float& fov)
{
	IClientEntity* localplayer = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{

		if (!localplayer)
			return;


		if (Menu::Window.VisualsTab.Active.GetState())
			fov += Menu::Window.VisualsTab.OtherViewmodelFOV.GetValue();
	}
}

float __stdcall GGetViewModelFOV()
{
	float fov = Hooks::VMTClientMode.GetMethod<oGetViewModelFOV>(35)();

	GetViewModelFOV(fov);

	return fov;
}

void __fastcall Hooked_OverrideView(void* ecx, void* edx, CViewSetup* pSetup)
{

	IClientEntity* localplayer1 = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!localplayer1)
		return;

	if (!Interfaces::Engine->IsConnected())
		return;
	if (!Interfaces::Engine->IsInGame())
		return;

	if (localplayer1)
	{
		if (!localplayer1->IsScoped() && Menu::Window.VisualsTab.OtherFOV.GetValue() > 0)
			pSetup->fov += Menu::Window.VisualsTab.OtherFOV.GetValue();
	}
	currentfov = pSetup->fov;
	oOverrideView(ecx, edx, pSetup);
}


void __fastcall Hooked_RenderView(void* ecx, void* edx, CViewSetup &setup, CViewSetup &hudViewSetup, int nClearFlags, int whatToDraw)
{
	static DWORD oRenderView = Hooks::VMTRenderView.GetOriginalFunction(6);

	IClientEntity* pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	__asm
	{
		PUSH whatToDraw
		PUSH nClearFlags
		PUSH hudViewSetup
		PUSH setup
		MOV ECX, ecx
		CALL oRenderView
	}
}