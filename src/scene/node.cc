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

#include "node.hh"

#include <algorithm>

namespace hojy::scene {

Node::Node(Node *parent, int x, int y, int width, int height) : parent_(parent), renderer_(parent->renderer_), x_(x), y_(y), width_(width), height_(height) {
    if (parent_) { parent_->add(this); }
}

Node::~Node() {
    if (parent_) { parent_->remove(this); }
    removeAllChildren();
}

void Node::add(Node *child) {
    children_.push_back(child);
}

void Node::remove(Node *child) {
    children_.erase(std::remove(children_.begin(), children_.end(), child), children_.end());
}

void Node::doRender() {
    render();
    for (auto *node : children_) {
        node->doRender();
    }
}

void Node::doHandleKeyInput(Node::Key key) {
    if (children_.empty()) {
        handleKeyInput(key);
        return;
    }
    children_.back()->handleKeyInput(key);
}

void Node::doTextInput(const std::wstring &str) {
    if (children_.empty()) {
        handleTextInput(str);
        return;
    }
    children_.back()->handleTextInput(str);
}

void Node::removeAllChildren() {
    for (auto *n: children_) {
        n->parent_ = nullptr;
        delete n;
    }
    children_.clear();
}

}
