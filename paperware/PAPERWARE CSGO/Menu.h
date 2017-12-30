#pragma once

#include "GUI.h"
#include "Controls.h"

class CRageBotTab : public CTab
{
public:
	void Setup();

	CLabel ActiveLabel;
	CCheckBox Active;

	CGroupBox AimbotGroup;
	CCheckBox AimbotEnable;
	CCheckBox AimbotAutoFire;
	CCheckBox AccuracyBacktracking;
	CSlider  AimbotFov;
	CCheckBox AimbotSilentAim;
	CCheckBox PVSFix;
	CCheckBox AimbotEnginePred;
	CKeyBind AimbotBaimOnKey;
	CCheckBox AimbotResolver;
	CCheckBox AutoRevolver;
	CSlider AddFakeYaw;
	CSlider CustomPitch;
	CCheckBox LBY;
	CComboBox FakeYaw;
	CComboBox MoveYaw;
	CComboBox MoveYawFake;
	CCheckBox LowerbodyFix;
	CCheckBox PreferBodyAim;
	CCheckBox AWPAtBody;
	CSlider BaimIfUnderXHealth;
	CSlider XShots;
	CCheckBox EnginePrediction;
	CCheckBox AimbotAimStep;
	CCheckBox AimbotKeyPress;
	CKeyBind  AimbotKeyBind;
	CKeyBind  AimbotStopKey;

	CGroupBox TargetGroup;
	CComboBox TargetSelection;
	CCheckBox TargetFriendlyFire;
	CComboBox TargetHitbox;
	CComboBox TargetHitscan;
	CCheckBox TargetMultipoint;
	CSlider   TargetPointscale;

	CGroupBox AccuracyGroup;
	CCheckBox AccuracyRecoil;
	CCheckBox AccuracyAutoWall;
	CSlider AccuracyMinimumDamage;
	CCheckBox AccuracyAutoStop;
	CCheckBox AccuracyAutoCrouch;
	CCheckBox AccuracyAutoScope;
	CSlider   AccuracyHitchance;
	CComboBox AccuracyResolver;

	CGroupBox AntiAimGroup;
	CCheckBox AntiAimEnable;
	CComboBox AntiAimPitch;
	CComboBox AntiAimYaw;
	CCheckBox FlipAA;
	CComboBox BreakLBY;
	CComboBox AntiAimEdge;
	CSlider AntiAimOffset;
	CCheckBox AntiAimKnife;
	CCheckBox AntiAimTarget;
};

class CLegitBotTab : public CTab
{
public:
	void Setup();

	// Master Switch
	CLabel ActiveLabel;
	CCheckBox Active;

	CGroupBox AimbotGroup;

	CCheckBox AimbotEnable;
	CCheckBox AimbotAutoFire;
	CSlider   TickModulation;
	CCheckBox AimbotBacktrack;
	CCheckBox AimbotFriendlyFire;
	CCheckBox AimbotKeyPress;
	CKeyBind  AimbotKeyBind;
	CCheckBox AimbotSmokeCheck;
	CCheckBox BackTrack;


	CGroupBox TriggerGroup;
	CCheckBox TriggerEnable;
	CCheckBox TriggerKeyPress;
	CKeyBind  TriggerKeyBind;
	CCheckBox TriggerHitChance;
	CSlider   TriggerHitChanceAmmount;
	CSlider   TriggerDelay;
	CSlider   TriggerBurst;
	CSlider   TriggerBreak;
	CCheckBox TriggerRecoil;

	CGroupBox TriggerFilterGroup;
	CCheckBox TriggerHead;
	CCheckBox TriggerChest;
	CCheckBox TriggerStomach;
	CCheckBox TriggerArms;
	CCheckBox TriggerLegs;
	CCheckBox TriggerTeammates;
	CCheckBox TriggerFlashCheck;
	CCheckBox TriggerSmokeCheck;

	CGroupBox WeaponMainGroup;
	CSlider   WeaponMainSpeed;
	CSlider   WeaponMainFoV;
	CSlider WeaponMainRecoil;
	CComboBox WeaponMainHitbox;
	CSlider WeaponMainAimtime;
	CSlider WeaoponMainStartAimtime;

	CGroupBox WeaponPistGroup;
	CSlider   WeaponPistSpeed;
	CSlider   WeaponPistFoV;
	CSlider WeaponPistRecoil;
	CComboBox WeaponPistHitbox;
	CSlider WeaponPistAimtime;
	CSlider WeaoponPistStartAimtime;

	CGroupBox WeaponSnipGroup;
	CSlider   WeaponSnipSpeed;
	CSlider   WeaponSnipFoV;
	CSlider WeaponSnipRecoil;
	CComboBox WeaponSnipHitbox;
	CSlider WeaponSnipAimtime;
	CSlider WeaoponSnipStartAimtime;

	CGroupBox WeaponMpGroup;
	CSlider   WeaponMpSpeed;
	CSlider   WeaponMpFoV;
	CSlider WeaponMpRecoil;
	CComboBox WeaponMpHitbox;
	CSlider WeaponMpAimtime;
	CSlider WeaoponMpStartAimtime;

	CGroupBox WeaponShotgunGroup;
	CSlider   WeaponShotgunSpeed;
	CSlider   WeaponShotgunFoV;
	CSlider WeaponShotgunRecoil;
	CComboBox WeaponShotgunHitbox;
	CSlider WeaponShotgunAimtime;
	CSlider WeaoponShotgunStartAimtime;

	CGroupBox WeaponMGGroup;
	CSlider   WeaponMGSpeed;
	CSlider   WeaponMGFoV;
	CSlider WeaponMGRecoil;
	CComboBox WeaponMGHitbox;
	CSlider WeaponMGAimtime;
	CSlider WeaoponMGStartAimtime;
};

class CVisualTab : public CTab
{
public:
	void Setup();

	CLabel ActiveLabel;
	CCheckBox Active;
	CCheckBox pLocalOpacity;
	CGroupBox OptionsGroup;
	CGroupBox InfoGroup;
	CGroupBox RemovalsGroup;
	CGroupBox ChamsGroup;
	CGroupBox BoxGroup;
	CGroupBox EffectGroup;
	CComboBox OptionsBox;
	CCheckBox OptionsName;
	CCheckBox Normalcrosshair;
	CCheckBox HitmarkerSound;
	CCheckBox dot;
	CCheckBox Angleinfo;
	CCheckBox Logs;
	CCheckBox OtherHitmarker;
	CCheckBox OptionsWeapon;
	CComboBox OptionHealthEnable;
	CCheckBox OptionsInfo;
	CCheckBox LBYIdicador;
	CCheckBox ResolverInfo;
	CCheckBox BulletTrace;
	CCheckBox Barrels;
	CSlider BulletTraceLength;
	CCheckBox AALines;
	CCheckBox ChamsVisibleOnly;
	CCheckBox Watermark;
	CCheckBox OtherGrenadeTrajectory;
	CCheckBox otherbulletimpact;
	CCheckBox WelcomeMessage;
	CCheckBox GrenadePrediction;
	CCheckBox LocalPlayerESP;
	CCheckBox CanHit;
	CCheckBox OptionsHelmet;
	CCheckBox NightMode;
	CCheckBox OptionsKit;
	CCheckBox SpecList;
	CCheckBox IsScoped;
	CCheckBox CompRank;
	CCheckBox HasDefuser;
	CCheckBox GrenadeTrace;
	CCheckBox Distance;
	CCheckBox DiscoLights;
	CCheckBox Money;
	CCheckBox OptionsDefuse;
	CComboBox OptionsChams;
	CCheckBox OptionsArmor;
	CSlider  GlowZ;
	CCheckBox OptionsSkeleton;
	CCheckBox BacktrackingLol;
	CCheckBox OptionsAimSpot;
	CCheckBox OtherNoScope;
	CCheckBox OptionsCompRank;

	CGroupBox FiltersGroup;
	CCheckBox FiltersAll;
	CCheckBox FiltersPlayers;
	CCheckBox FiltersSelf;
	CCheckBox FiltersEnemiesOnly;
	CCheckBox FiltersWeapons;
	CCheckBox FiltersNades;
	CCheckBox FiltersC4;
	CGroupBox GlowGroup;
	CGroupBox RenderGroup;
	CGroupBox OtherGroup;
	CComboBox XHairs;
	CCheckBox OptionsGlow;
	CCheckBox EntityGlow;
	CCheckBox NightSky;
	CCheckBox OtherRadar;
	CCheckBox OtherNoVisualRecoil;
	CCheckBox OtherNoSky; 
	CCheckBox OtherNoFlash; 
	CCheckBox OtherNoSmoke;
	CCheckBox OtherAsusWalls;
	CComboBox OtherNoHands;
	CSlider OtherViewmodelFOV;
	CSlider OtherFOV;
};

class CColorTab : public CTab
{
public:
	void Setup();

	CGroupBox RGBGroup;
	CSlider   MenuR;
	CSlider   MenuG;
	CSlider  MenuB;

	CGroupBox CTVisibleGroup;
	CComboBox ColorsPicker;
	CGroupBox CTBoxNotVisGroup;
	CGroupBox ColorsGroup;
	CSlider CTColorVisR;
	CSlider CTColorVisG;
	CSlider CTColorVisB;

	CSlider TChams;
	CSlider CTChams;
	CSlider CTChamsR;
	CSlider CTChamsG;
	CSlider CTChamsB;
	CSlider TChamsR;
	CSlider TChamsB;
	CSlider TChamsG;
	CSlider ColorMenuAccentR;
	CSlider ColorMenuAccentG;
	CSlider ColorMenuAccentB;

	CSlider CTColorNoVisR;
	CSlider CTColorNoVisG;
	CSlider CTColorNoVisB;
	CGroupBox TVisibleBoxGroup;
	CGroupBox Tboxnotvisiblegroup;
	CGroupBox GlowGroup;
	CLabel ColorSpacer;

	CSlider TColorVisR;
	CSlider TColorVisG;
	CSlider TColorVisB;

	CSlider TColorNoVisR;
	CSlider TColorNoVisG;
	CSlider TColorNoVisB;

	CSlider GlowR;
	CSlider GlowG;
	CSlider GlowB;
	CGroupBox CTCHamsGroup;
	CGroupBox TChamsGroup;
	CGroupBox MenuColorGroup;
	CComboBox ConfigBox;
	CGroupBox ConfigGroup;
	CGroupBox Settings;
	CButton SettingsSave;
	CButton SettingsLoad;
};

class CSettingsTab : public CTab
{
public:
	void Setup();

	CGroupBox wpnhitchanceGroup;
	CCheckBox WeaponCheck;
	CSlider  scoutChance;
	CSlider  AWPChance;
	CSlider  AutoChance;
	CSlider  RifleChance;
	CSlider  MPChance;
	CSlider  PistolChance;
	CSlider  R8Chance;

	CGroupBox wpnmindmgGroup;
	CSlider  scoutmindmg;
	CSlider  AWPmindmg;
	CSlider  Automindmg;
	CSlider  Riflemindmg;
	CSlider  MPmindmg;
	CSlider  Pistolmindmg;
	CSlider  R8mindmg;

};

class CMiscTab : public CTab
{
public:
	void Setup();

	CGroupBox OtherGroup;
	CCheckBox OtherAutoJump;
	CCheckBox OtherEdgeJump;
	CComboBox OtherAutoStrafe;
	CCheckBox OtherSafeMode;
	CCheckBox ChatSpam;
	CCheckBox NoName;
	CCheckBox Namespam;
	CCheckBox OtherCircle;
	CKeyBind OtherCirclekey;
	CTextField CustomClantag;
	CCheckBox Impacts;
	CCheckBox CheatsByPass;
	CCheckBox AutoAccept;
	CCheckBox BuyBotKevlar;
	CCheckBox MoonWalk;
	CCheckBox BuyBotDefuser;
	CComboBox BuyBotGrenades;
	CSlider CircleAmount;
	CKeyBind FakeWalk;
	CComboBox FakeLagTyp;
	CSlider ClanTagSpeed;
	CComboBox NameChanger;
	CCheckBox AutoPistol;
	CCheckBox ClanTag;
	CCheckBox TrashTalk;
	CKeyBind  OtherAirStuck;
	CCheckBox OtherSpectators;
	CCheckBox OtherThirdperson;
	CKeyBind  ThirdPersonKeyBind;
	CCheckBox FakeAngleChams;
	CComboBox SeeTpangle;
	CCheckBox OtherThirdpersonFake;
	CComboBox OtherClantag;
	CTextField customclan;
	CCheckBox EnableBuyBot;
	CComboBox BuyBot;
	CCheckBox DisablePostProcess;
	CGroupBox BuyBotGroup;
	CSlider WalkBotDistance;
	CSlider WalkBotEdge;
	CCheckBox WalkBot;
	CGroupBox WalkBotGroup;
	CGroupBox FakeLagGroup;
	CCheckBox FakeLagEnable;
	CSlider   FakeLagChoke;
	CComboBox KnifeModel;
	CCheckBox SkinEnable;
	CTextField KnifeSkin;
	CGroupBox KnifeGroup;
	CButton KnifeApply;
};

class aristoispejawindow : public CWindow
{
public:
	void Setup();

	CRageBotTab RageBotTab;
	CLegitBotTab LegitBotTab;
	CVisualTab VisualsTab;
	CMiscTab MiscTab;
	CSettingsTab SettingsTab;
	CColorTab ColorsTab;

	CButton SaveButton1;
	CButton LoadButton1;
	CButton SaveButton;
	CButton LoadButton;
	CButton UnloadButton;
	CButton PanicButton;
	CButton ThirdButton;
	CButton ByPass;
	CButton FirstButton;
	CComboBox ConfigBox;
};

namespace Menu
{
	void SetupMenu();
	void DoUIFrame();

	extern aristoispejawindow Window;
};