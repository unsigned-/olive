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

#include "colormanager.h"

#include <QFloat16>

#include "common/define.h"
#include "config/config.h"
#include "core.h"

OLIVE_NAMESPACE_ENTER

ColorManager::ColorManager()
{
  // Ensures config is set to something
  config_ = OCIO::GetCurrentConfig();

  // Default reference space is scene linear
  reference_space_ = OCIO::ROLE_SCENE_LINEAR;
}

OCIO::ConstConfigRcPtr ColorManager::GetConfig() const
{
  return config_;
}

void ColorManager::SetConfig(const QString &filename)
{
  OCIO::ConstConfigRcPtr cfg;

  if (filename.isEmpty()) {
    cfg = OCIO::Config::CreateFromEnv();
  } else {
    cfg = OCIO::Config::CreateFromFile(filename.toUtf8());
  }

  SetConfig(cfg);
}

void ColorManager::SetConfig(OCIO::ConstConfigRcPtr config)
{
  config_ = config;

  emit ConfigChanged();
}

void ColorManager::DisassociateAlpha(FramePtr f)
{
  AssociateAlphaPixFmtFilter(kDisassociate, f);
}

void ColorManager::AssociateAlpha(FramePtr f)
{
  AssociateAlphaPixFmtFilter(kAssociate, f);
}

void ColorManager::ReassociateAlpha(FramePtr f)
{
  AssociateAlphaPixFmtFilter(kReassociate, f);
}

QStringList ColorManager::ListAvailableDisplays()
{
  QStringList displays;

  int number_of_displays = config_->getNumDisplays();

  for (int i=0;i<number_of_displays;i++) {
    displays.append(config_->getDisplay(i));
  }

  return displays;
}

QString ColorManager::GetDefaultDisplay()
{
  return config_->getDefaultDisplay();
}

QStringList ColorManager::ListAvailableViews(QString display)
{
  QStringList views;

  int number_of_views = config_->getNumViews(display.toUtf8());

  for (int i=0;i<number_of_views;i++) {
    views.append(config_->getView(display.toUtf8(), i));
  }

  return views;
}

QString ColorManager::GetDefaultView(const QString &display)
{
  return config_->getDefaultView(display.toUtf8());
}

QStringList ColorManager::ListAvailableLooks()
{
  QStringList looks;

  int number_of_looks = config_->getNumLooks();

  for (int i=0;i<number_of_looks;i++) {
    looks.append(config_->getLookNameByIndex(i));
  }

  return looks;
}

QStringList ColorManager::ListAvailableInputColorspaces()
{
  return ListAvailableInputColorspaces(config_);
}

const QString &ColorManager::GetDefaultInputColorSpace() const
{
  return default_input_color_space_;
}

void ColorManager::SetDefaultInputColorSpace(const QString &s)
{
  default_input_color_space_ = s;
}

const QString &ColorManager::GetReferenceColorSpace() const
{
  return reference_space_;
}

void ColorManager::SetReferenceColorSpace(const QString &s)
{
  reference_space_ = s;
}

QStringList ColorManager::ListAvailableInputColorspaces(OCIO::ConstConfigRcPtr config)
{
  QStringList spaces;

  int number_of_colorspaces = config->getNumColorSpaces();

  for (int i=0;i<number_of_colorspaces;i++) {
    spaces.append(config->getColorSpaceNameByIndex(i));
  }

  return spaces;
}

ColorManager::OCIOMethod ColorManager::GetOCIOMethodForMode(RenderMode::Mode mode)
{
  return static_cast<OCIOMethod>(Core::GetPreferenceForRenderMode(mode, QStringLiteral("OCIOMethod")).toInt());
}

void ColorManager::SetOCIOMethodForMode(RenderMode::Mode mode, ColorManager::OCIOMethod method)
{
  Core::SetPreferenceForRenderMode(mode, QStringLiteral("OCIOMethod"), method);
}

void ColorManager::AssociateAlphaPixFmtFilter(ColorManager::AlphaAction action, FramePtr f)
{
  if (!PixelFormat::FormatHasAlphaChannel(f->format())) {
    // This frame has no alpha channel, do nothing
    return;
  }

  int pixel_count = f->width() * f->height() * kRGBAChannels;

  switch (static_cast<PixelFormat::Format>(f->format())) {
  case PixelFormat::PIX_FMT_INVALID:
  case PixelFormat::PIX_FMT_COUNT:
    qWarning() << "Alpha association functions received an invalid pixel format";
    break;
  case PixelFormat::PIX_FMT_RGB8:
  case PixelFormat::PIX_FMT_RGBA8:
  case PixelFormat::PIX_FMT_RGB16U:
  case PixelFormat::PIX_FMT_RGBA16U:
    qWarning() << "Alpha association functions only works on float-based pixel formats at this time";
    break;
  case PixelFormat::PIX_FMT_RGB16F:
  case PixelFormat::PIX_FMT_RGBA16F:
  {
    AssociateAlphaInternal<qfloat16>(action, reinterpret_cast<qfloat16*>(f->data()), pixel_count);
    break;
  }
  case PixelFormat::PIX_FMT_RGB32F:
  case PixelFormat::PIX_FMT_RGBA32F:
  {
    AssociateAlphaInternal<float>(action, reinterpret_cast<float*>(f->data()), pixel_count);
    break;
  }
  }
}

template<typename T>
void ColorManager::AssociateAlphaInternal(ColorManager::AlphaAction action, T *data, int pix_count)
{
  for (int i=0;i<pix_count;i+=kRGBAChannels) {
    T alpha = data[i+kRGBChannels];

    if (action == kAssociate || alpha > 0) {
      for (int j=0;j<kRGBChannels;j++) {
        if (action == kDisassociate) {
          data[i+j] /= alpha;
        } else {
          data[i+j] *= alpha;
        }
      }
    }
  }
}

OLIVE_NAMESPACE_EXIT
