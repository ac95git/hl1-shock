#include "hud.h"
#include "cl_util.h"
#include "vgui_inventory.h"
#include "vgui_status.h"
#include "spr_fit.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <string>
#include <vector>

using namespace vgui;

// =====================================================================
// Formatting (docs/STATUS_PANEL.md, the stats column)
//
// ASCII "x" rather than a multiplication sign: the scheme fonts print a
// byte at a time, and the Skill descriptions already write "x1.5".
// =====================================================================
static std::string FormatMultiplier(float value)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "x%.2f", value);
    return buf;
}

static std::string FormatPercent(float share)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%d%%", (int)(share * 100.0f + 0.5f));
    return buf;
}

static std::string FormatPool(int current, int max, bool haveMax)
{
    char buf[24];
    if (haveMax)
        snprintf(buf, sizeof(buf), "%d / %d", current, max);
    else
        snprintf(buf, sizeof(buf), "%d / --", current);
    return buf;
}

// The key a Module's command is bound to, as "[SHIFT]", looked up every
// time so a rebind never shows a stale key.
static std::string KeyForCommand(const char* command)
{
    if (!command)
        return std::string();

    const char* key = gEngfuncs.Key_LookupBinding(command);
    std::string name = (key && key[0]) ? key : "unbound";
    for (char& c : name)
        c = (char)std::toupper((unsigned char)c);
    return "[" + name + "]";
}

// =====================================================================
// EnsureSprites
// =====================================================================
void CStatusView::EnsureSprites()
{
    for (int i = 0; i < k_NumModuleDefs; ++i)
    {
        if (m_sprites[i].hSprite != 0)
            continue;
        const char* name = k_ModuleDefs[i].spriteName;
        if (!name)
            continue;
        const int idx = gHUD.GetSpriteIndex(name);
        if (idx < 0)
            continue;
        m_sprites[i].hSprite = gHUD.GetSprite(idx);
        m_sprites[i].rc      = gHUD.GetSpriteRect(idx);
    }
}

// =====================================================================
// Paint
// =====================================================================
void CStatusView::Paint(CInventoryPanel* ctx,
    int x0, int y0, int areaW, int areaH,
    vgui::Font* smallFont, vgui::Font* titleFont)
{
    // Chrome only, in the suit colour (docs/STATUS_PANEL.md, "Theme").
    int sr, sg, sb, dr, dg, db, lr, lg, lb, orr, ogg, obb;
    UnpackRGB(sr, sg, sb, RGB_SUIT);
    UnpackRGB(dr, dg, db, RGB_SUIT_DIM);
    UnpackRGB(lr, lg, lb, RGB_SUIT_LIT);
    UnpackRGB(orr, ogg, obb, RGB_SUIT_OFF);

    ctx->drawSetColor(10, 10, 10, 60);
    ctx->drawFilledRect(x0, y0, x0 + areaW, y0 + areaH);
    ctx->drawSetColor(sr, sg, sb, 80);
    ctx->drawOutlinedRect(x0, y0, x0 + areaW, y0 + areaH);

    // Every label is collected and drawn after the fills and sprites
    // (docs/TECH_DEBT.md, "VGUI Draw Order").
    struct DeferredLabel { int x, y; std::string text; int r, g, b; };
    std::vector<DeferredLabel> labels;

    const int lineH = smallFont ? smallFont->getTall() : 10;

    EnsureSprites();

    // ---- The doll: five fixed Slots arranged as a body ----
    const int S = k_SlotSize, G = k_SlotGap;
    const int dollMinW = S * 3 + G * 2 + 32;
    const int dollW = std::max(dollMinW, std::min(areaW * 2 / 5, 420));
    const int cx  = x0 + dollW / 2;
    const int top = y0 + 24;

    m_slotRects[(int)ESlot::Head]     = { cx - S / 2,         top,                 S, S };
    m_slotRects[(int)ESlot::Body]     = { cx - S / 2,         top + (S + G),       S, S };
    m_slotRects[(int)ESlot::LeftArm]  = { cx - S / 2 - G - S, top + (S + G),       S, S };
    m_slotRects[(int)ESlot::RightArm] = { cx + S / 2 + G,     top + (S + G),       S, S };
    m_slotRects[(int)ESlot::Legs]     = { cx - S / 2,         top + (S + G) * 2,   S, S };

    int hoverModule = -1;
    IRect hoverRect = {};

    for (int s = 0; s < (int)ESlot::_Count; ++s)
    {
        const IRect& r = m_slotRects[s];
        const ModuleDef* def = GetModuleInSlot((ESlot)s);
        const bool bFilled = def && IsGateOpen(def->gate);

        if (!bFilled)
        {
            // Empty: a dim, unlabelled frame, and no tooltip.
            FillRGBA(r.x, r.y, r.w, r.h, 20, 20, 20, 120);
            ctx->drawSetColor(orr, ogg, obb, 170);
            ctx->drawOutlinedRect(r.x, r.y, r.x + r.w, r.y + r.h);
            continue;
        }

        FillRGBA(r.x, r.y, r.w, r.h, dr, dg, db, 120);
        ctx->drawSetColor(sr, sg, sb, 40);
        ctx->drawOutlinedRect(r.x, r.y, r.x + r.w, r.y + r.h);
        FillRGBA(r.x, r.y, r.w, 2, sr, sg, sb, 220);

        const int moduleIndex = (int)(def - k_ModuleDefs);
        const ModuleSprite& ms = m_sprites[moduleIndex];
        if (ms.hSprite != 0)
        {
            const int boxW = r.w - k_IconPad * 2;
            const int boxH = r.h - k_IconPad * 2;
            SPR_Set(ms.hSprite, sr, sg, sb);
            SPR_DrawFitted(ms.hSprite, ms.rc, r.x + k_IconPad, r.y + k_IconPad, boxW, boxH,
                           SPR_BLEND_ONE, SPR_BLEND_ONE);
        }

        if (m_iHoverX >= r.x && m_iHoverX < r.x + r.w && m_iHoverY >= r.y && m_iHoverY < r.y + r.h)
        {
            hoverModule = moduleIndex;
            hoverRect = r;
        }
    }

    // ---- The stats column ----
    const int colX = x0 + dollW + 8;
    const int colW = std::max(0, x0 + areaW - colX - 16);
    int y = y0 + 16;

    auto header = [&](const char* text)
    {
        labels.push_back({ colX, y, text, lr, lg, lb });
        FillRGBA(colX, y + lineH + 3, colW, 1, sr, sg, sb, 140);
        y += lineH + 10;
    };

    auto row = [&](const char* label, const std::string& value)
    {
        labels.push_back({ colX + 8, y, label, 205, 205, 205 });
        const int valueW = TextWidth(smallFont, value);
        labels.push_back({ colX + colW - 8 - valueW, y, value, lr, lg, lb });
        y += lineH + 5;
    };

    const bool bHave = m_stats.valid;
    const std::string none = "--";
    auto mult = [&](float v) { return bHave ? FormatMultiplier(v) : none; };
    auto pct  = [&](float v) { return bHave ? FormatPercent(v) : none; };

    header("HEALTH");
    row("Health", FormatPool(gHUD.m_Health.m_iHealth, m_stats.maxHealth, bHave));
    row("Healing", mult(m_stats.healing));
    y += 10;

    header("ARMOR");
    row("Armor", FormatPool(gHUD.m_Battery.Armor(), m_stats.maxArmor, bHave));
    row("Armor efficiency", pct(m_stats.armorEff));
    row("Explosive resistance", pct(m_stats.blastResist));
    row("Energy resistance", pct(m_stats.energyResist));
    row("Fall resistance", pct(m_stats.fallResist));
    y += 10;

    header("DAMAGE");
    row("Melee damage", mult(m_stats.melee));
    row("Bullet damage", mult(m_stats.bullet));
    row("Energy damage", mult(m_stats.energy));
    row("Explosive damage", mult(m_stats.explosive));
    y += 10;

    // The MODULES block: each line with its Module, in the doll's order --
    // head, body, right arm -- and the block hidden until one shows.
    const bool bConcealment = k_ConcealmentBuilt && IsGateOpen(EGate::NightVision);
    const bool bDash        = IsGateOpen(EGate::DashModule);
    const bool bHornet      = k_HornetReplenishBuilt && IsGateOpen(EGate::AlienModule);
    if (bConcealment || bDash || bHornet)
    {
        header("MODULES");
        if (bConcealment)
            row("Concealment", pct(m_stats.concealment));
        if (bDash)
        {
            char buf[16];
            snprintf(buf, sizeof(buf), "%.1f s", m_stats.dashRecharge);
            row("Dash recharge", bHave ? std::string(buf) : none);
        }
        if (bHornet)
            row("Hornet replenish", mult(m_stats.hornetReplenish));
    }

    // ---- Tooltip on a filled Slot: name, key, description ----
    std::string tipTitle;
    std::vector<std::string> tipLines;
    int tipX = 0, tipY = 0, tipW = 0, tipH = 0, tipBodyTop = 0;

    if (hoverModule >= 0)
    {
        const ModuleDef& def = k_ModuleDefs[hoverModule];

        constexpr int kPad = 8;
        constexpr int kTitleGap = 6;
        const int maxW = std::min(std::max(190, areaW - 16), 360);
        const int titleH = titleFont ? titleFont->getTall() : 12;

        tipTitle = def.name;

        const std::string key = KeyForCommand(def.command);
        if (!key.empty())
            tipLines.push_back(key);

        std::vector<std::string> body;
        WrapToWidth(smallFont, def.description, maxW - kPad * 2, body);
        tipLines.insert(tipLines.end(), body.begin(), body.end());

        int widest = TextWidth(titleFont, tipTitle);
        for (const std::string& line : tipLines)
            widest = std::max(widest, TextWidth(smallFont, line));

        tipW = std::min(std::max(widest + kPad * 2, 150), maxW);
        tipBodyTop = kPad + titleH + kTitleGap;
        tipH = tipBodyTop + (int)tipLines.size() * lineH + kPad;

        // To the right of the Slot, flipped rather than overhanging, and
        // kept inside the view.
        tipX = hoverRect.x + hoverRect.w + 10;
        tipY = hoverRect.y + 2;
        if (tipX + tipW > x0 + areaW)
            tipX = hoverRect.x - tipW - 10;
        if (tipX < x0)
            tipX = x0 + 4;
        if (tipY + tipH > y0 + areaH)
            tipY = y0 + areaH - tipH - 4;

        FillRGBA(tipX, tipY, tipW, tipH, 0, 0, 0, 220);
        ctx->drawSetColor(255, 170, 0, 120);
        ctx->drawOutlinedRect(tipX, tipY, tipX + tipW, tipY + tipH);
    }

    // ---- All text, last ----
    if (smallFont)
    {
        ctx->drawSetTextFont(smallFont);
        for (const DeferredLabel& lbl : labels)
        {
            ctx->drawSetTextColor(lbl.r, lbl.g, lbl.b, 0);
            ctx->drawSetTextPos(lbl.x, lbl.y);
            ctx->drawPrintText(lbl.text.c_str(), (int)lbl.text.size());
        }
    }

    if (hoverModule >= 0)
    {
        if (titleFont)
        {
            ctx->drawSetTextFont(titleFont);
            ctx->drawSetTextColor(255, 210, 80, 0);
            ctx->drawSetTextPos(tipX + 8, tipY + 8);
            ctx->drawPrintText(tipTitle.c_str(), (int)tipTitle.size());
        }

        if (smallFont)
        {
            ctx->drawSetTextFont(smallFont);
            for (int i = 0; i < (int)tipLines.size(); ++i)
            {
                // The key line in the suit's lit colour, the description grey.
                const bool bKeyLine = (i == 0 && k_ModuleDefs[hoverModule].command != nullptr);
                if (bKeyLine) ctx->drawSetTextColor(lr, lg, lb, 0);
                else          ctx->drawSetTextColor(205, 205, 205, 0);
                ctx->drawSetTextPos(tipX + 8, tipY + tipBodyTop + i * lineH);
                ctx->drawPrintText(tipLines[i].c_str(), (int)tipLines[i].size());
            }
        }
    }

    ctx->drawSetTextPos(0, 0);
}
