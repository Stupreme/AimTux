#include "hooker.h"

IBaseClientDLL* client = nullptr;
ISurface* surface = nullptr;
IVPanel* panel = nullptr;
IEngineClient* engine = nullptr;
IClientEntityList* entitylist = nullptr;
IVDebugOverlay* debugOverlay = nullptr;
IVModelInfo* modelInfo = nullptr;
IVModelRender* modelRender = nullptr;
IClientMode* clientMode = nullptr;
IEngineTrace* trace = nullptr;
IInputSystem* input = nullptr;
IMaterialSystem* material = nullptr;
ICvar* cvar = nullptr;
CGlobalVars* globalvars = nullptr;
CEffects* effects = nullptr;
IGameEventManager2* gameevents = nullptr;
IPhysicsSurfaceProps* physics = nullptr;
CViewRender* viewrender = nullptr;


VMT* panel_vmt = nullptr;
VMT* client_vmt = nullptr;
VMT* modelRender_vmt = nullptr;
VMT* clientMode_vmt = nullptr;
VMT* gameEvents_vmt = nullptr;
VMT* viewRender_vmt = nullptr;

GlowObjectManagerFn GlowObjectManager;
MsgFunc_ServerRankRevealAllFn MsgFunc_ServerRankRevealAll;
SendClanTagFn SendClanTag;

void Hooker::HookInterfaces()
{
	client = GetInterface<IBaseClientDLL>("./csgo/bin/linux64/client_client.so", CLIENT_DLL_INTERFACE_VERSION);
	engine = GetInterface<IEngineClient>("./bin/linux64/engine_client.so", VENGINE_CLIENT_INTERFACE_VERSION);
	entitylist = GetInterface<IClientEntityList>("./csgo/bin/linux64/client_client.so", VCLIENTENTITYLIST_INTERFACE_VERSION);
	surface = GetInterface<ISurface>("./bin/linux64/vguimatsurface_client.so", SURFACE_INTERFACE_VERSION);
	panel = GetInterface<IVPanel>("./bin/linux64/vgui2_client.so", PANEL_INTERFACE_VERSION);
	debugOverlay = GetInterface<IVDebugOverlay>("./bin/linux64/engine_client.so", DEBUG_OVERLAY_VERSION);
	modelInfo = GetInterface<IVModelInfo>("./bin/linux64/engine_client.so", VMODELINFO_CLIENT_INTERFACE_VERSION);
	modelRender = GetInterface<IVModelRender>("./bin/linux64/engine_client.so", VMODELRENDER_CLIENT_INTERFACE_VERSION);
	trace = GetInterface<IEngineTrace>("./bin/linux64/engine_client.so", ENGINETRACE_CLIENT_INTERFACE_VERSION);
	input = GetInterface<IInputSystem>("./bin/linux64/inputsystem_client.so", INPUTSYSTEM_CLIENT_INTERFACE_VERSION);
	material = GetInterface<IMaterialSystem>("./bin/linux64/materialsystem_client.so", MATERIALSYSTEM_CLIENT_INTERFACE_VERSION);
	cvar = GetInterface<ICvar>("./bin/linux64/libvstdlib_client.so", ENGINECVAR_INTERFACE_VERSION);
	effects = GetInterface<CEffects>("./bin/linux64/engine_client.so", ENGINEEFFECTS_INTERFACE_VERSION);
	gameevents = GetInterface<IGameEventManager2>("./bin/linux64/engine_client.so", GAMEEVENTSMANAGER2_INTERFACE_VERSION);
	physics = GetInterface<IPhysicsSurfaceProps>("./bin/linux64/vphysics_client.so", PHYSICS_CLIENT_INTERFACE_VERSION);
}

void Hooker::HookVMethods()
{
	panel_vmt = new VMT(panel);
	client_vmt = new VMT(client);
	modelRender_vmt = new VMT(modelRender);
	gameEvents_vmt = new VMT(gameevents);
	viewRender_vmt = new VMT(viewrender);
}

std::unordered_map<const char*, uintptr_t> GetProcessLibraries() {
	std::unordered_map<const char*, uintptr_t> modules;

	dl_iterate_phdr([](struct dl_phdr_info* info, size_t size, void* data) {
		reinterpret_cast<std::unordered_map<const char*, uintptr_t>*>(data)->insert({ info->dlpi_name, reinterpret_cast<uintptr_t>(info->dlpi_addr + info->dlpi_phdr[0].p_vaddr) });
		return 0;
	}, &modules);

	return modules;
}

uintptr_t GetLibraryAddress(const char* moduleName)
{
	std::unordered_map<const char*, uintptr_t> modules = GetProcessLibraries();

	for (auto module : modules)
		if (strcasestr(module.first, moduleName))
			return module.second;

	return 0;
}

void Hooker::HookIClientMode()
{
	uintptr_t init_address = FindPattern(GetLibraryAddress("client_client.so"), 0xFFFFFFFFF, (unsigned char*) CCSMODEMANAGER_INIT_SIGNATURE, CCSMODEMANAGER_INIT_MASK);

	if (!init_address)
		return;

	uint32_t offset = *reinterpret_cast<uint32_t*>(init_address + 3);
	clientMode = reinterpret_cast<IClientMode*>(init_address + offset + 7);
	
	clientMode_vmt = new VMT(clientMode);
}

void Hooker::HookGlobalVars()
{
	uintptr_t hudupdate = reinterpret_cast<uintptr_t>(getvtable(client)[11]);
	globalvars = *reinterpret_cast<CGlobalVars**>(GetAbsoluteAddress(hudupdate + 13, 3, 7));
}

void Hooker::HookGlowManager()
{
	uintptr_t instruction_addr = FindPattern(GetLibraryAddress("client_client.so"), 0xFFFFFFFFF, (unsigned char*) GLOWOBJECT_SIGNATURE, GLOWOBJECT_MASK);

	GlowObjectManager = reinterpret_cast<GlowObjectManagerFn>(GetAbsoluteAddress(instruction_addr, 1, 5));
}

void Hooker::HookRankReveal()
{
	uintptr_t func_address = FindPattern(GetLibraryAddress("client_client.so"), 0xFFFFFFFFF, (unsigned char*) MSGFUNC_SERVERRANKREVEALALL_SIGNATURE, MSGFUNC_SERVERRANKREVEALALL_MASK);

	MsgFunc_ServerRankRevealAll = reinterpret_cast<MsgFunc_ServerRankRevealAllFn>(func_address);
}

void Hooker::HookSendClanTag()
{
	uintptr_t func_address = FindPattern(GetLibraryAddress("engine_client.so"), 0xFFFFFFFFF, (unsigned char*) SENDCLANTAG_SIGNATURE, SENDCLANTAG_MASK);

	SendClanTag = reinterpret_cast<SendClanTagFn>(func_address);
}

void Hooker::HookViewRender()
{
	uintptr_t func_address = FindPattern(GetLibraryAddress("client_client.so"), 0xFFFFFFFFF, (unsigned char*) VIEWRENDER_SIGNATURE, VIEWRENDER_MASK);

	viewrender = reinterpret_cast<CViewRender*>(GetAbsoluteAddress(func_address + 14, 3, 7));
}