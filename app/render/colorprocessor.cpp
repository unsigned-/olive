/***

  Olive - Non-Linear Video Editor
  Copyright (C) 2019 Olive Team

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

***/

#include "colorprocessor.h"

#include "common/define.h"
#include "colormanager.h"

OLIVE_NAMESPACE_ENTER

ColorProcessor::ColorProcessor(ColorManager *config, const QString& source_space, const QString& dest_space)
{
  processor_ = config->GetConfig()->getProcessor(source_space.toUtf8(),
                                                dest_space.toUtf8());
}

ColorProcessor::ColorProcessor(ColorManager *config,
                               QString source_space,
                               QString display,
                               QString view,
                               const QString& look,
                               Direction direction)
{
  if (source_space.isEmpty()) {
    source_space = config->GetDefaultInputColorSpace();
  }

  if (display.isEmpty()) {
    display = config->GetDefaultDisplay();
  }

  if (view.isEmpty()) {
    view = config->GetDefaultView(display);
  }

  // Get current display stats
  OCIO::DisplayTransformRcPtr transform = OCIO::DisplayTransform::Create();
  transform->setInputColorSpaceName(source_space.toUtf8());
  transform->setDisplay(display.toUtf8());
  transform->setView(view.toUtf8());

  if (!look.isEmpty()) {
    transform->setLooksOverride(look.toUtf8());
    transform->setLooksOverrideEnabled(true);
  }

  OCIO::TransformDirection dir = (direction == kInverse) ? OCIO::TRANSFORM_DIR_INVERSE : OCIO::TRANSFORM_DIR_FORWARD;

  processor_ = config->GetConfig()->getProcessor(transform, dir);
}

void ColorProcessor::ConvertFrame(Frame *f)
{
  OCIO::PackedImageDesc img(reinterpret_cast<float*>(f->data()), f->width(), f->height(), PixelFormat::ChannelCount(f->format()));

  processor_->apply(img);
}

Color ColorProcessor::ConvertColor(Color in)
{
  processor_->applyRGBA(in.data());
  return in;
}

ColorProcessorPtr ColorProcessor::Create(ColorManager *config, const QString& source_space, const QString& dest_space)
{
  return std::make_shared<ColorProcessor>(config, source_space, dest_space);
}

ColorProcessorPtr ColorProcessor::Create(ColorManager *config, const QString &source_space, const QString &display, const QString &view, const QString &look, Direction direction)
{
  return std::make_shared<ColorProcessor>(config, source_space, display, view, look, direction);
}

OCIO::ConstProcessorRcPtr ColorProcessor::GetProcessor()
{
  return processor_;
}

void ColorProcessor::ConvertFrame(FramePtr f)
{
  ConvertFrame(f.get());
}

OLIVE_NAMESPACE_EXIT
