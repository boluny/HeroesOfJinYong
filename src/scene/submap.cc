/*
 * Heroes of Jin Yong.
 * A reimplementation of the DOS game `The legend of Jin Yong Heroes`.
 * Copyright (C) 2021, Soar Qin<soarchin@gmail.com>

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "submap.hh"

#include "texture.hh"
#include "data/grpdata.hh"
#include "mem/savedata.hh"

namespace hojy::scene {

SubMap::SubMap(Renderer *renderer, std::uint32_t width, std::uint32_t height, std::int16_t id): Map(renderer, width, height), subMapId_(id) {
    drawingTerrainTex2_ = Texture::createAsTarget(renderer_, 2048, 2048);
    drawingTerrainTex2_->enableBlendMode(true);

    mapWidth_ = mem::SubMapWidth;
    mapHeight_ = mem::SubMapHeight;
    char idxstr[8], grpstr[8];
    snprintf(idxstr, 8, "SDX%03d", id);
    snprintf(grpstr, 8, "SMP%03d", id);
    auto &submapData = data::grpData.lazyLoad(idxstr, grpstr);
    auto sz = submapData.size();
    for (size_t i = 0; i < sz; ++i) {
        textureMgr.loadFromRLE(i, submapData[i]);
    }

    {
        auto *tex = textureMgr[0];
        cellWidth_ = tex->width();
        cellHeight_ = tex->height();
        offsetX_ = tex->originX();
        offsetY_ = tex->originY();
    }
    int cellDiffX = cellWidth_ / 2;
    int cellDiffY = cellHeight_ / 2;
    texWidth_ = (mapWidth_ + mapHeight_) * cellDiffX;
    texHeight_ = (mapWidth_ + mapHeight_) * cellDiffY;

    auto size = mapWidth_ * mapHeight_;
    cellInfo_.resize(size);

    auto &layers = mem::currSave.subMapLayerInfo[id]->data;
    auto &events = mem::currSave.subMapEventInfo[id]->events;
    int x = (mapHeight_ - 1) * cellDiffX + offsetX_;
    int y = offsetY_;
    int pos = 0;
    for (int j = mapHeight_; j; --j) {
        int tx = x, ty = y;
        for (int i = mapWidth_; i; --i, ++pos, tx += cellDiffX, ty += cellDiffY) {
            auto &ci = cellInfo_[pos];
            auto texId = layers[0][pos] >> 1;
            ci.isWater = texId >= 179 && texId <= 181 || texId == 261 || texId == 511 || texId >= 662 && texId <= 665 || texId == 674;
            ci.earth = textureMgr[texId];
            texId = layers[1][pos] >> 1;
            if (texId) {
                ci.building = textureMgr[texId];
            }
            texId = layers[2][pos] >> 1;
            if (texId) {
                ci.decoration = textureMgr[texId];
            }
            auto ev = layers[3][pos];
            if (ev >= 0) {
                texId = events[ev].currTex >> 1;
                if (texId) {
                    ci.event = textureMgr[texId];
                }
            }
            ci.buildingDeltaY = layers[4][pos];
            ci.decorationDeltaY = layers[5][pos];
        }
        x -= cellDiffX; y += cellDiffY;
    }

    currX_ = 19, currY_ = 20;
    resetTime();
    updateMainCharTexture();
}

SubMap::~SubMap() {
    delete drawingTerrainTex2_;
}

void SubMap::render() {
    Map::render();

    if (moveDirty_) {
        moveDirty_ = false;
        int cellDiffX = cellWidth_ / 2;
        int cellDiffY = cellHeight_ / 2;
        int curX = currX_, curY = currY_;
        int nx = int(width_) / 2 + cellWidth_;
        int ny = int(height_) / 2 + cellHeight_;
        int wcount = nx * 2 / cellWidth_;
        int hcount = ny * 2 / cellDiffY;
        int cx, cy, tx, ty;
        int delta = -mapWidth_ + 1;

        renderer_->setTargetTexture(drawingTerrainTex_);
        renderer_->setClipRect(0, 0, 2048, 2048);
        renderer_->fill(0, 0, 0, 0);
        cx = (nx / cellDiffX + ny / cellDiffY) / 2;
        cy = (ny / cellDiffY - nx / cellDiffX) / 2;
        tx = int(width_) / 2 - (cx - cy) * cellDiffX;
        ty = int(height_) / 2 - (cx + cy) * cellDiffY;
        cx = curX - cx; cy = curY - cy;
        for (int j = hcount; j; --j) {
            int x = cx, y = cy;
            int dx = tx;
            int offset = y * mapWidth_ + x;
            for (int i = wcount; i; --i, dx += cellWidth_, offset += delta, ++x, --y) {
                if (x < 0 || x >= mem::SubMapWidth || y < 0 || y >= mem::SubMapHeight) {
                    continue;
                }
                auto &ci = cellInfo_[offset];
                auto h = ci.buildingDeltaY;
                if (h == 0) {
                    renderer_->renderTexture(ci.earth, dx, ty);
                }
            }
            if (j % 2) {
                ++cx;
                tx += cellDiffX;
                ty += cellDiffY;
            } else {
                ++cy;
                tx -= cellDiffX;
                ty += cellDiffY;
            }
        }

        cx = (nx / cellDiffX + ny / cellDiffY) / 2;
        cy = (ny / cellDiffY - nx / cellDiffX) / 2;
        tx = int(width_) / 2 - (cx - cy) * cellDiffX;
        ty = int(height_) / 2 - (cx + cy) * cellDiffY;
        cx = curX - cx; cy = curY - cy;
        for (int j = hcount; j; --j) {
            int x = cx, y = cy;
            int dx = tx;
            int offset = y * mapWidth_ + x;
            for (int i = wcount; i; --i, dx += cellWidth_, offset += delta, ++x, --y) {
                if (x < 0 || x >= mem::SubMapWidth || y < 0 || y >= mem::SubMapHeight) {
                    continue;
                }
                auto &ci = cellInfo_[offset];
                auto h = ci.buildingDeltaY;
                if (h > 0) {
                    renderer_->renderTexture(ci.earth, dx, ty);
                }
                if (ci.building) {
                    renderer_->renderTexture(ci.building, dx, ty - h);
                }
                if (x == curX && y == curY) {
                    renderer_->setTargetTexture(drawingTerrainTex2_);
                    renderer_->setClipRect(0, 0, 2048, 2048);
                    renderer_->fill(0, 0, 0, 0);
                    charHeight_ = h;
                }
                if (ci.event) {
                    renderer_->renderTexture(ci.event, dx, ty - h);
                }
                if (ci.decoration) {
                    renderer_->renderTexture(ci.decoration, dx, ty - ci.decorationDeltaY);
                }
            }
            if (j % 2) {
                ++cx;
                tx += cellDiffX;
                ty += cellDiffY;
            } else {
                ++cy;
                tx -= cellDiffX;
                ty += cellDiffY;
            }
        }
        renderer_->setTargetTexture(nullptr);
        renderer_->unsetClipRect();
    }

    renderer_->fill(0, 0, 0, 0);
    renderer_->renderTexture(drawingTerrainTex_, 0, 0, width_, height_);
    renderChar(charHeight_);
    renderer_->renderTexture(drawingTerrainTex2_, 0, 0, width_, height_);
}

bool SubMap::tryMove(int x, int y) {
    auto pos = y * mapWidth_ + x;
    auto &ci = cellInfo_[pos];
    if (ci.building || ci.isWater) {
        return true;
    }
    auto &layers = mem::currSave.subMapLayerInfo[subMapId_]->data;
    auto &events = mem::currSave.subMapEventInfo[subMapId_]->events;
    auto ev = layers[3][pos];
    if (ev >= 0 && events[ev].blocked) {
        return true;
    }

    currX_ = x;
    currY_ = y;
    moveDirty_ = true;
    currFrame_ = currFrame_ % 6 + 1;
    return true;
}

void SubMap::updateMainCharTexture() {
    if (resting_) {
        mainCharTex_ = textureMgr[2501 + int(direction_) * 7];
        return;
    }
    mainCharTex_ = textureMgr[2501 + int(direction_) * 7 + currFrame_];
}

}
