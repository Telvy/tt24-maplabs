//============================== MK 2022 ======================================//
//
// Purpose: Aldebaran crosshair and interaction text.
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "c_baseplayer.h"

#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/Panel.h>

ConVar aldb_crosshair("aldb_crosshair", "1", FCVAR_ARCHIVE);
ConVar aldb_crosshair_onlynamed("aldb_crosshair_onlynamed", "0", FCVAR_ARCHIVE);
ConVar aldb_crosshair_static("aldb_crosshair_static", "0", FCVAR_ARCHIVE);
ConVar aldb_crosshair_hidetext("aldb_crosshair_hidetext", "0", FCVAR_ARCHIVE);
ConVar aldb_crosshair_pulse("aldb_crosshair_pulse", "1", FCVAR_ARCHIVE);
ConVar aldb_crosshair_pulsescale("aldb_crosshair_pulsescale", "0.3", FCVAR_ARCHIVE);


class CHudALDBCrosshair : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudALDBCrosshair, vgui::Panel);

public:
	CHudALDBCrosshair(const char *pElementName);
	bool	ShouldDraw();
	void	LevelInit();
private:
	void	Paint(void);
	void	ApplySchemeSettings(vgui::IScheme *pScheme);
	void	DrawAldbCrosshair(int CenterOffsetX, int CenterOffsetY, int baserotation, bool inverted, bool counterclockwise);

	float	m_fLastRotationTime;
	float	m_fRotpercent;
	float	m_fPulserot;

	CHudTexture	*m_pCrosshairAldb;	// Crosshair image.

	CPanelAnimationVar(vgui::HFont, m_hText1Font, "Text1Font", "Default");
	CPanelAnimationVar(Color, m_Text1Color, "Text1Color", "255 255 255 255");
	CPanelAnimationVar(vgui::HFont, m_hText2Font, "Text2Font", "Default");
	CPanelAnimationVar(Color, m_Text2Color, "Text2Color", "255 255 255 255");
	CPanelAnimationVar(Color, m_CrosshairColor, "CrosshairColor", "255 255 255 255");
	CPanelAnimationVar(int, m_iText1Offset, "Text1Offset", "50");
	CPanelAnimationVar(int, m_iText2Offset, "Text2Offset", "50");
};

DECLARE_HUDELEMENT(CHudALDBCrosshair);


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudALDBCrosshair::CHudALDBCrosshair(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudALDBCrosshair")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_pCrosshairAldb = 0;
	m_fLastRotationTime = 0;
	m_fRotpercent = 0;
	m_fPulserot = 0;
	SetProportional(false);
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_CROSSHAIR);
}

void CHudALDBCrosshair::LevelInit()
{
	CHudElement::LevelInit();
	m_fLastRotationTime = 0;
	m_fRotpercent = 0;
	m_fPulserot = 0;
}

bool CHudALDBCrosshair::ShouldDraw()
{
	if (!aldb_crosshair.GetBool())
	{
		return false;
	}

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
	{
		return false;
	}

	return (CHudElement::ShouldDraw());
}

void CHudALDBCrosshair::DrawAldbCrosshair(int CenterOffsetX, int CenterOffsetY, int baserotation, bool inverted, bool counterclockwise)
{
	int xCenter = ScreenWidth() / 2;
	int yCenter = ScreenHeight() / 2;

	int width, height;
	float xMod, yMod;

	//Crosshair pulse
	float boostrange;

	if (aldb_crosshair_pulse.GetBool() && !aldb_crosshair_static.GetBool())
	{
		m_fPulserot += (gpGlobals->curtime - m_fLastRotationTime) * 250; //Reused delta, but it seems to work.

		if (m_fRotpercent != 1)
			m_fPulserot = 0;

		if (m_fPulserot > 180)
			m_fPulserot = 0;

		boostrange = (1 + aldb_crosshair_pulsescale.GetFloat()*sin(DEG2RAD(m_fPulserot)));

		if (!inverted)
			boostrange = 1;
	}
	else
	{
		boostrange = 1;
	}

	int xCenterFinal = xCenter + CenterOffsetX * boostrange;
	int yCenterFinal = yCenter + CenterOffsetY * boostrange;

	vgui::surface()->DrawSetTexture(m_pCrosshairAldb->textureId);
	vgui::surface()->DrawGetTextureSize(m_pCrosshairAldb->textureId, width, height);

	vgui::Vertex_t vert[4];

	Vector2D uv11(0, 0);
	Vector2D uv12(0, 1);
	Vector2D uv21(1, 0);
	Vector2D uv22(1, 1);

	xMod = width;
	yMod = height;

	xMod /= 2;
	yMod /= 2;

	vert[0].Init(Vector2D(xCenterFinal + xMod, yCenterFinal + yMod), uv22);
	vert[1].Init(Vector2D(xCenterFinal - xMod, yCenterFinal + yMod), uv12);
	vert[2].Init(Vector2D(xCenterFinal - xMod, yCenterFinal - yMod), uv11);
	vert[3].Init(Vector2D(xCenterFinal + xMod, yCenterFinal - yMod), uv21);

	// It takes 1 second to reach 1 from 0 and vice versa, so multiply this to speed up interpolation.
	if (inverted)
	{
		m_fRotpercent += (gpGlobals->curtime - m_fLastRotationTime) * 4;
	}
	else
	{
		m_fRotpercent -= (gpGlobals->curtime - m_fLastRotationTime) * 4;
	}
	m_fLastRotationTime = gpGlobals->curtime;

	m_fRotpercent = clamp(m_fRotpercent, 0.0f, 1.0f);

	float flFinalAngle;
	float finalrot = 180 * m_fRotpercent;

	//If we want static crosshair.
	if (aldb_crosshair_static.GetBool())
	{
		finalrot = 0;
	}

	if (counterclockwise)
		flFinalAngle = DEG2RAD(baserotation - finalrot);
	else
		flFinalAngle = DEG2RAD(baserotation + finalrot);

	float mid_x = vert[2].m_Position.x + 0.5 * width;
	float mid_y = vert[2].m_Position.y + 0.5 * height;

	float flCosA = cos(flFinalAngle);
	float flSinA = sin(flFinalAngle);

	//Polygon rotation
	for (int i = 0; i<4; i++)
	{
		Vector2D result;

		vert[i].m_Position.x -= mid_x;
		vert[i].m_Position.y -= mid_y;

		result.x = (vert[i].m_Position.x * flCosA - vert[i].m_Position.y * flSinA) + mid_x;
		result.y = (vert[i].m_Position.x * flSinA + vert[i].m_Position.y * flCosA) + mid_y;

		vert[i].m_Position = result;
	}

#if 0
	//Color pulse
	Color	clr;
	clr = m_CrosshairColor;
	int r, g, b, a;
	clr.GetColor(r, g, b, a);

	int alphadelta = 255 - a;

	clr.SetColor(r, g, b, a + alphadelta*sin(DEG2RAD(m_fPulserot)));

	vgui::surface()->DrawSetColor(clr);
#endif

	vgui::surface()->DrawSetColor(m_CrosshairColor);
	vgui::surface()->DrawTexturedPolygon(4, vert);
}

void CHudALDBCrosshair::Paint(void)
{
	if (!m_pCrosshairAldb)
		return;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	int xCenter = ScreenWidth() / 2;
	int yCenter = ScreenHeight() / 2;

	bool entfound;
	if (pPlayer->m_bALDBOnUseEntity)
		entfound = true;
	else
		entfound = false;

	if (aldb_crosshair_onlynamed.GetBool())
	{
		if (pPlayer->m_iszALDBUseEntityUse[0] == '\0' && pPlayer->m_iszALDBUseEntityName[0] == '\0')
			entfound = false;
	}

	//Draw crosshair
	DrawAldbCrosshair(-20, -20, 180, entfound, true);  //Upper left
	DrawAldbCrosshair(20, -20, 270, entfound, false);	//Upper right
	DrawAldbCrosshair(20, 20, 0, entfound, true);		//Bottom right
	DrawAldbCrosshair(-20, 20, 90, entfound, false);	//Bottom left

	//Draw text

	if (aldb_crosshair_hidetext.GetBool())
		return;

	if (!entfound)
		return;

	wchar_t text[128];
	int wide, tall;
	//Ent use text (text1)
	g_pVGuiLocalize->ConvertANSIToUnicode(pPlayer->m_iszALDBUseEntityUse, text, sizeof(text));
	if (text[0])
	{
		vgui::surface()->GetTextSize(m_hText1Font, text, wide, tall);
		vgui::surface()->DrawSetTextFont(m_hText1Font);
		vgui::surface()->DrawSetTextColor(m_Text1Color);
		vgui::surface()->DrawSetTextPos((xCenter - wide*0.5), yCenter + m_iText1Offset);
		vgui::surface()->DrawPrintText(text, wcslen(text));
	}
	//Ent name text (text2)
	g_pVGuiLocalize->ConvertANSIToUnicode(pPlayer->m_iszALDBUseEntityName, text, sizeof(text));
	if (text[0])
	{
		//int tallold = tall;
		vgui::surface()->GetTextSize(m_hText2Font, text, wide, tall);
		vgui::surface()->DrawSetTextFont(m_hText2Font);
		vgui::surface()->DrawSetTextColor(m_Text2Color);
		vgui::surface()->DrawSetTextPos((xCenter - wide*0.5), yCenter + tall + m_iText2Offset);
		vgui::surface()->DrawPrintText(text, wcslen(text));
	}
}

void CHudALDBCrosshair::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	m_pCrosshairAldb = gHUD.GetIcon("aldb_crosshair");
	SetProportional(false);
	SetPaintBackgroundEnabled(false);
	SetSize(ScreenWidth(), ScreenHeight());
}
