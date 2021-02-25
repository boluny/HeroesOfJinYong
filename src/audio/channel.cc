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

#include "channel.hh"

#include "resampler.hh"

#include <util/file.hh>

namespace hojy::audio {

Channel::Channel(Mixer *mixer, std::string_view filename): sampleRateOut_(mixer->sampleRate()) {
    util::File::getFileContent(filename, data_);
}

Channel::Channel(Mixer *mixer, const void *data, size_t size): sampleRateOut_(mixer->sampleRate()) {
    data_.resize(size);
    memcpy(data_.data(), data, size);
}

size_t Channel::readData(void *data, size_t size) {
    if (resampler_) {
        return resampler_->read(data, size);
    }
    return readPCMData(data, size);
}

void Channel::start() {
    if (sampleRateIn_ != sampleRateOut_ || typeIn_ != typeOut_) {
        resampler_ = std::make_unique<Resampler>(channels_, sampleRateIn_, sampleRateOut_, typeIn_, typeOut_);
    }
}

}
