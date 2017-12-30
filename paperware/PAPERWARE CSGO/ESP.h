#pragma once

#include "Hacks.h"
extern float lineLBY;
extern float lineRealAngle;
extern float lineFakeAngle;

class CEsp : public CHack
{
public:
	void Init();
	void Draw();
	void Move(CUserCmd *pCmd, bool &bSendPacket);
private:

	IClientEntity *BombCarrier;

	struct ESPBox
	{
		int x, y, w, h;
	};

	void BulletTrace(IClientEntity * pEntity, Color color);

	void DrawPlayer(IClientEntity* pEntity, player_info_t pinfo);
	void ThreeDBox(Vector minin, Vector maxin, Vector pos, Color Col);
	Color GetPlayerColor(IClientEntity* pEntity);
	void DrawLinesAA(Color color);
	bool GetBox(IClientEntity* pEntity, ESPBox &result);
	void Corners(CEsp::ESPBox size, Color color, IClientEntity * pEntity);
	void DrawBox(ESPBox size, Color color);
	void Barrel(CEsp::ESPBox size, Color color, IClientEntity * pEntity);
	void DrawWeapon(IClientEntity * pEntity, CEsp::ESPBox size);
	void Ammo(IClientEntity * pEntity, player_info_t pinfo, CEsp::ESPBox size);
	void DrawGlow();
	void EntityGlow();
	void BacktrackingCross(IClientEntity * pEntity);
	void BombTimer(IClientEntity * pEntity, ClientClass * cClass);
	CEsp::ESPBox GetBOXX(IClientEntity * pEntity);
	void IwebzHealth(IClientEntity * pEntity, CEsp::ESPBox size);
	void DrawHealth(IClientEntity * pEntity, CEsp::ESPBox size);
	void DrawName(player_info_t pinfo, ESPBox size);
	void DrawInfo(IClientEntity* pEntity, ESPBox size);
	void DrawCross(IClientEntity* pEntity);
	void DrawSkeleton(IClientEntity* pEntity);
	void DrawMoney(IClientEntity * pEntity, CEsp::ESPBox size);
	void Armor(IClientEntity * pEntity, CEsp::ESPBox size);
	void BoxAndText(IClientEntity * entity, std::string text);
	void DrawThrowable(IClientEntity * throwable);
	void DrawDrop(IClientEntity* pEntity, ClientClass* cClass);
	void DrawBombPlanted(IClientEntity* pEntity, ClientClass* cClass);
	void DrawBomb(IClientEntity* pEntity, ClientClass* cClass);
};

