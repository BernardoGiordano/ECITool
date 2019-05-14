/*
 *   This file is part of PKSM
 *   Copyright (C) 2016-2019 Bernardo Giordano, Admiral Fish, piepie62
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   Additional Terms 7.b and 7.c of GPLv3 apply to this file:
 *       * Requiring preservation of specified reasonable legal notices or
 *         author attributions in that material or in the Appropriate Legal
 *         Notices displayed by works containing it.
 *       * Prohibiting misrepresentation of the origin of that material,
 *         or requiring that modified versions of such material be marked in
 *         reasonable ways as different from the original version.
 */

#include "TextParse.hpp"
#include <algorithm>

namespace TextParse
{
    void Text::addWord(std::pair<std::vector<Glyph>, float>&& word, float maxWidth)
    {
        if (maxWidth > 0.0f && lineWidths.back() + word.second > maxWidth)
        {
            lineWidths.push_back(word.second);
            for (auto& glyph : word.first)
            {
                glyph.line = lineWidths.size();
            }
        }
        else
        {
            for (auto& glyph : word.first)
            {
                glyph.xPos += lineWidths.back();
                glyph.line = lineWidths.size();
            }
            lineWidths.back() += word.second;
        }
        glyphs.insert(glyphs.end(), word.first.begin(), word.first.end());
    }

    TextBuf::TextBuf(size_t maxGlyphs, const std::vector<C2D_Font>& fonts) : fonts(fonts), maxGlyphs(maxGlyphs), currentGlyphs(0)
    {
        for (auto& font : this->fonts)
        {
            TGLP_s* glyphInfo = C2D_FontGetInfo(font)->tglp;
            std::vector<C3D_Tex> fontSheets(glyphInfo->nSheets);
            for (size_t i = 0; i < fontSheets.size(); i++)
            {
                fontSheets[i].data   = glyphInfo->sheetData + glyphInfo->sheetSize * i;
                fontSheets[i].fmt    = (GPU_TEXCOLOR)glyphInfo->sheetFmt;
                fontSheets[i].size   = glyphInfo->sheetSize;
                fontSheets[i].width  = glyphInfo->sheetWidth;
                fontSheets[i].height = glyphInfo->sheetHeight;
                fontSheets[i].param  = GPU_TEXTURE_MAG_FILTER(GPU_LINEAR) | GPU_TEXTURE_MIN_FILTER(GPU_LINEAR) |
                                      GPU_TEXTURE_WRAP_S(GPU_CLAMP_TO_BORDER) | GPU_TEXTURE_WRAP_T(GPU_CLAMP_TO_BORDER);
                fontSheets[i].border   = 0xFFFFFFFF;
                fontSheets[i].lodParam = 0;
            }
            glyphSheets.emplace(font, std::move(fontSheets));
        }
    }

    void TextBuf::addFont(C2D_Font font)
    {
        fonts.emplace_back(font);
        TGLP_s* glyphInfo = C2D_FontGetInfo(font)->tglp;
        std::vector<C3D_Tex> fontSheets(glyphInfo->nSheets);
        for (size_t i = 0; i < fontSheets.size(); i++)
        {
            fontSheets[i].data   = glyphInfo->sheetData + glyphInfo->sheetSize * i;
            fontSheets[i].fmt    = (GPU_TEXCOLOR)glyphInfo->sheetFmt;
            fontSheets[i].size   = glyphInfo->sheetSize;
            fontSheets[i].width  = glyphInfo->sheetWidth;
            fontSheets[i].height = glyphInfo->sheetHeight;
            fontSheets[i].param  = GPU_TEXTURE_MAG_FILTER(GPU_LINEAR) | GPU_TEXTURE_MIN_FILTER(GPU_LINEAR) | GPU_TEXTURE_WRAP_S(GPU_CLAMP_TO_BORDER) |
                                  GPU_TEXTURE_WRAP_T(GPU_CLAMP_TO_BORDER);
            fontSheets[i].border   = 0xFFFFFFFF;
            fontSheets[i].lodParam = 0;
        }
        glyphSheets.emplace(font, std::move(fontSheets));
    }

    void TextBuf::clear()
    {
        if (currentGlyphs >= maxGlyphs)
        {
            clearUnconditional();
        }
    }

    void TextBuf::clearUnconditional() { parsedText.clear(); }

    bool TextBuf::fontHasChar(const C2D_Font& font, u32 codepoint)
    {
        if (C2D_FontGlyphIndexFromCodePoint(font, codepoint) == C2D_FontGetInfo(font)->alterCharIndex)
        {
            return false;
        }
        return true;
    }

    C2D_Font TextBuf::fontForCodepoint(u32 codepoint)
    {
        if (codepoint != ' ')
        {
            for (size_t font = 0; font < fonts.size(); font++)
            {
                if (TextBuf::fontHasChar(fonts[font], codepoint))
                {
                    return fonts[font];
                }
            }
        }
        // Not found, so use the system font
        return nullptr;
    }

    std::pair<std::vector<Glyph>, float> TextBuf::parseWord(const std::string& str, size_t& offset)
    {
        std::vector<Glyph> ret;
        float width  = 0.0f;
        ssize_t iMod = 0;
        for (; offset < str.size(); offset += iMod)
        {
            u32 chr;
            iMod = decode_utf8(&chr, (u8*)str.data() + offset);
            if (iMod == -1)
            {
                chr  = 0xFFFD;
                iMod = 1;
            }
            if (iMod == '\0' || iMod == ' ' || iMod == '\n')
            {
                offset++;
                return {ret, width};
            }
            C2D_Font font = fontForCodepoint(chr);
            fontGlyphPos_s glyphPos;
            C2D_FontCalcGlyphPos(font, &glyphPos, C2D_FontGlyphIndexFromCodePoint(font, chr), 0, 1.0f, 1.0f);
            if (glyphPos.width > 0.0f)
            {
                ret.emplace_back(Tex3DS_SubTexture{static_cast<u16>(ceilf(glyphPos.width)), (u16)C2D_FontGetInfo(font)->tglp->cellHeight,
                                     glyphPos.texcoord.left, glyphPos.texcoord.top, glyphPos.texcoord.right, glyphPos.texcoord.bottom},
                    &glyphSheets[font][glyphPos.sheetIndex], font, 0, width + glyphPos.xOffset, glyphPos.width);
            }
            width += glyphPos.xAdvance;
        }
        // SHOULD never happen
        return {ret, width};
    }

    std::shared_ptr<Text> TextBuf::parse(const std::string& str, float maxWidth)
    {
        auto it = parsedText.find(str);
        if (it != parsedText.end())
        {
            return it->second;
        }
        else
        {
            std::shared_ptr<Text> tmp = std::make_shared<Text>();
            tmp->glyphs.reserve(str.size());
            tmp->lineWidths.push_back(0.0f);
            size_t offset = 0;
            do
            {
                auto word = parseWord(str, offset);
                tmp->addWord(std::move(word), maxWidth);
            } while (offset < str.size());

            tmp->maxLineWidth = *std::max_element(tmp->lineWidths.begin(), tmp->lineWidths.end());

            auto ret = parsedText.emplace(str, std::move(tmp));
            return ret.first->second;
            // ssize_t iMod = 0;
            // for (size_t i = 0; i < str.size(); i += iMod)
            // {
            //     u32 chr;
            //     iMod = decode_utf8(&chr, (u8*)str.data() + i);
            //     if (iMod == -1)
            //     {
            //         chr  = 0xFFFD;
            //         iMod = 1;
            //     }
            //     if (chr == u'\n')
            //     {
            //         tmp->maxLineWidth = std::max(tmp->lineWidths[tmp->lines], tmp->maxLineWidth);
            //         tmp->lines++;
            //         tmp->lineWidths.push_back(0.0f);
            //         continue;
            //     }
            //     C2D_Font font = fontForCodepoint(chr);
            //     fontGlyphPos_s glyphPos;
            //     C2D_FontCalcGlyphPos(font, &glyphPos, C2D_FontGlyphIndexFromCodePoint(font, chr), 0, 1.0f, 1.0f);
            //     if (glyphPos.width > 0.0f)
            //     {
            //         if (maxWidth != 0.0f && tmp->lineWidths[tmp->lines] + glyphPos.xAdvance > maxWidth)
            //         {
            //             tmp->maxLineWidth = std::max(tmp->lineWidths[tmp->lines], tmp->maxLineWidth);
            //             tmp->lines++;
            //             tmp->lineWidths.push_back(0.0f);
            //         }
            //         tmp->glyphs.emplace_back(Tex3DS_SubTexture{static_cast<u16>(ceilf(glyphPos.width)),
            //         (u16)C2D_FontGetInfo(font)->tglp->cellHeight,
            //                                      glyphPos.texcoord.left, glyphPos.texcoord.top, glyphPos.texcoord.right, glyphPos.texcoord.bottom},
            //             &glyphSheets[font][glyphPos.sheetIndex], font, tmp->lines, tmp->lineWidths[tmp->lines] + glyphPos.xOffset, glyphPos.width);
            //         currentGlyphs++;
            //     }
            //     tmp->lineWidths[tmp->lines] += glyphPos.xAdvance;
            // }
        }
    }

    void ScreenText::addText(std::shared_ptr<Text> text, float x, float y, float scaleX, float scaleY, TextPosX textPos, u32 color)
    {
        if (!text || text->glyphs.empty())
            return;

        for (auto& glyph : text->glyphs)
        {
            float glyphX = x + scaleX * glyph.xPos;
            switch (textPos)
            {
                case TextPosX::LEFT:
                    break;
                case TextPosX::CENTER:
                    glyphX -= scaleX * text->lineWidths[glyph.line] / 2;
                    break;
                case TextPosX::RIGHT:
                    glyphX -= scaleX * text->lineWidths[glyph.line];
                    break;
            }
            float glyphY = y + scaleY * (C2D_FontGetInfo(glyph.font)->lineFeed * glyph.line - C2D_FontGetInfo(glyph.font)->tglp->baselinePos);
            glyphs.emplace_back(glyph, glyphX, glyphY, scaleX, scaleY, color);
        }
    }

    void ScreenText::optimize()
    {
        std::sort(glyphs.begin(), glyphs.end(), [](const PositionedGlyph& g1, const PositionedGlyph& g2) {
            if (g1.glyph.font != g2.glyph.font)
            {
                // These are arbitrarily ordered
                return g1.glyph.font < g2.glyph.font;
            }
            else
            {
                // Also arbitrary, but keeps those of the same type together
                return g1.glyph.tex < g2.glyph.tex;
            }
        });
    }

    void ScreenText::draw() const
    {
        C2D_ImageTint tint;
        for (auto& glyph : glyphs)
        {
            C2D_PlainImageTint(&tint, glyph.color, 1.0f);
            C2D_DrawImageAt({glyph.glyph.tex, &glyph.glyph.subtex}, glyph.x, glyph.y, 0.5f, &tint, glyph.scaleX, glyph.scaleY);
        }
    }

    void ScreenText::clear() { glyphs.clear(); }
}