#include "Interfaces.h"
#include "Utilities.h"

typedef void* (__cdecl* CreateInterface_t)(const char*, int*);
typedef void* (*CreateInterfaceFn)(const char *pName, int *pReturnCode);

CreateInterface_t EngineFactory = NULL; 
CreateInterface_t ClientFactory = NULL;
CreateInterface_t VGUISurfaceFactory = NULL;
CreateInterface_t VGUI2Factory = NULL; 
CreateInterface_t MatFactory = NULL;
CreateInterface_t PhysFactory = NULL;
CreateInterface_t StdFactory = NULL;
CreateInterface_t InputSystemPointer = NULL;

void Interfaces::peahjhekjwfheuwfhwe()
{
	float pJunkcode = 3124627894;
	pJunkcode = 2805328324;
	if (pJunkcode = 3018218242)
		pJunkcode = 190625368;
	pJunkcode = 1926828759;
	pJunkcode = 1969310409;
	if (pJunkcode = 55511256);
	pJunkcode = 2208616422;
	pJunkcode = 267127615;
	if (pJunkcode = 544617010)
		pJunkcode = 1199016478;
	pJunkcode = 1238214469;
	pJunkcode = 148525131;
	if (pJunkcode = 2461119952);
	pJunkcode = 1090719164;
	pJunkcode = 358717599;
	if (pJunkcode = 720528302)
		pJunkcode = 1924824739;
	pJunkcode = 2699121005;
	pJunkcode = 132813409;
	if (pJunkcode = 200751103);
	pJunkcode = 479417445;
	pJunkcode = 487920801;

}

void Interfaces::dwieohfeiwufhiwuef()
{
	float pJunkcode = 1736714165;
	pJunkcode = 2391124576;
	if (pJunkcode = 221358764)
		pJunkcode = 2959723259;
	pJunkcode = 2628617278;
	pJunkcode = 751911067;
	if (pJunkcode = 107151360);
	pJunkcode = 156461693;
	pJunkcode = 105184025;
	if (pJunkcode = 2069520698)
		pJunkcode = 2442315859;
	pJunkcode = 868023721;
	pJunkcode = 2101788;
	if (pJunkcode = 100185478);
	pJunkcode = 12801494;
	pJunkcode = 1974330226;
	if (pJunkcode = 2696423603)
		pJunkcode = 635829082;
	pJunkcode = 408531965;
	pJunkcode = 2263612380;
	if (pJunkcode = 1340825036);
	pJunkcode = 2209418151;
	pJunkcode = 3211610417;

}

void Interfaces::Initialise()
{
	peahjhekjwfheuwfhwe();
	EngineFactory = (CreateInterface_t)GetProcAddress((HMODULE)Offsets::Modules::Engine, "CreateInterface");
	ClientFactory = (CreateInterface_t)GetProcAddress((HMODULE)Offsets::Modules::Client, "CreateInterface");
	VGUI2Factory = (CreateInterface_t)GetProcAddress((HMODULE)Offsets::Modules::VGUI2, "CreateInterface");
	VGUISurfaceFactory = (CreateInterface_t)GetProcAddress((HMODULE)Offsets::Modules::VGUISurface, "CreateInterface");
	MatFactory = (CreateInterface_t)GetProcAddress((HMODULE)Offsets::Modules::Material, "CreateInterface");
	PhysFactory = (CreateInterface_t)GetProcAddress((HMODULE)Offsets::Modules::VPhysics, "CreateInterface");
	StdFactory = (CreateInterface_t)GetProcAddress((HMODULE)Offsets::Modules::Stdlib, "CreateInterface");
	peahjhekjwfheuwfhwe();
	dwieohfeiwufhiwuef();
	char* CHLClientInterfaceName = (char*)Utilities::Memory::FindTextPattern("client.dll", "VClient0");
	char* VGUI2PanelsInterfaceName = (char*)Utilities::Memory::FindTextPattern("vgui2.dll", "VGUI_Panel0");
	char* VGUISurfaceInterfaceName = (char*)Utilities::Memory::FindTextPattern("vguimatsurface.dll", "VGUI_Surface0");
	char* EntityListInterfaceName = (char*)Utilities::Memory::FindTextPattern("client.dll", "VClientEntityList0");
	char* EngineDebugThingInterface = (char*)Utilities::Memory::FindTextPattern("engine.dll", "VDebugOverlay0");
	char* EngineClientInterfaceName = (char*)Utilities::Memory::FindTextPattern("engine.dll","VEngineClient0");
	char* ClientPredictionInterface = (char*)Utilities::Memory::FindTextPattern("client.dll", "VClientPrediction0");
	char* MatSystemInterfaceName = (char*)Utilities::Memory::FindTextPattern("materialsystem.dll", "VMaterialSystem0");
	char* EngineRenderViewInterface = (char*)Utilities::Memory::FindTextPattern("engine.dll", "VEngineRenderView0");
	char* EngineModelRenderInterface = (char*)Utilities::Memory::FindTextPattern("engine.dll", "VEngineModel0");
	char* EngineModelInfoInterface = (char*)Utilities::Memory::FindTextPattern("engine.dll", "VModelInfoClient0");
	char* EngineTraceInterfaceName = (char*)Utilities::Memory::FindTextPattern("engine.dll", "EngineTraceClient0");
	char* PhysPropsInterfaces = (char*)Utilities::Memory::FindTextPattern("client.dll", "VPhysicsSurfaceProps0");
	char* VEngineCvarName = (char*)Utilities::Memory::FindTextPattern("engine.dll", "VEngineCvar00");
	InputSystemPointer = (CreateInterface_t)GetProcAddress((HMODULE)Utilities::Memory::WaitOnModuleHandle("inputsystem.dll"), "CreateInterface");
	InputSystem = (IInputSystem*)InputSystemPointer("InputSystemVersion001", NULL);
	peahjhekjwfheuwfhwe();
	dwieohfeiwufhiwuef();
	Client = (IBaseClientDLL*)ClientFactory(CHLClientInterfaceName, NULL);
	Engine = (IVEngineClient*)EngineFactory(EngineClientInterfaceName, NULL);
	Panels = (IPanel*)VGUI2Factory(VGUI2PanelsInterfaceName, NULL);
	Surface = (ISurface*)VGUISurfaceFactory(VGUISurfaceInterfaceName, NULL);
	EntList = (IClientEntityList*)ClientFactory(EntityListInterfaceName, NULL);
	DebugOverlay = (IVDebugOverlay*)EngineFactory(EngineDebugThingInterface, NULL);
	Prediction = (DWORD*)ClientFactory(ClientPredictionInterface, NULL);
	MaterialSystem = (CMaterialSystem*)MatFactory(MatSystemInterfaceName, NULL);
	RenderView = (CVRenderView*)EngineFactory(EngineRenderViewInterface, NULL);
	ModelRender = (IVModelRender*)EngineFactory(EngineModelRenderInterface, NULL);
	ModelInfo = (CModelInfo*)EngineFactory(EngineModelInfoInterface, NULL);
	Trace = (IEngineTrace*)EngineFactory(EngineTraceInterfaceName, NULL);
	PhysProps = (IPhysicsSurfaceProps*)PhysFactory(PhysPropsInterfaces, NULL);
	CVar = (ICVar*)StdFactory(VEngineCvarName, NULL);
	EventManager = (IGameEventManager2*)EngineFactory("GAMEEVENTSMANAGER002", NULL);
	ClientMode = **(IClientModeShared***)((*(uintptr_t**)Client)[10] + 0x5);
	Globals = **reinterpret_cast< CGlobalVarsBase*** >((*reinterpret_cast< DWORD** >(Client))[0] + 0x1B);
	PDWORD pdwClientVMT = *(PDWORD*)Client;
	pInput = *(CInput**)((*(DWORD**)Client)[15] + 0x1);
	dwieohfeiwufhiwuef();
}

namespace Interfaces
{

	IInputSystem* InputSystem;
	IBaseClientDLL* Client;
	IVEngineClient* Engine;
	IPanel* Panels;
	IClientEntityList* EntList;
	ISurface* Surface;
	IVDebugOverlay* DebugOverlay;
	IClientModeShared *ClientMode;
	CGlobalVarsBase *Globals;
	DWORD *Prediction;
	CMaterialSystem* MaterialSystem;
	CVRenderView* RenderView;
	IVModelRender* ModelRender;
	CModelInfo* ModelInfo;
	IEngineTrace* Trace;
	IPhysicsSurfaceProps* PhysProps;
	ICVar *CVar;
	CInput* pInput;
	IGameEventManager2 *EventManager;
	HANDLE __FNTHANDLE;
	IMoveHelper* MoveHelper;
	IPrediction *Prediction1;
	IGameMovement* GameMovement;
};