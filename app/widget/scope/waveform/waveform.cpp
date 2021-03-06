
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

#include "waveform.h"

#include "node/node.h"
#include "render/backend/opengl/openglrenderfunctions.h"

OLIVE_NAMESPACE_ENTER

WaveformScope::WaveformScope(QWidget* parent) :
  QOpenGLWidget(parent),
  texture_(nullptr)
{

}

void WaveformScope::SetTexture(OpenGLTexture *texture)
{
  texture_ = texture;

  update();
}

void WaveformScope::initializeGL()
{
  pipeline_ = OpenGLShader::Create();
  pipeline_->create();
  pipeline_->addShaderFromSourceCode(QOpenGLShader::Vertex, OpenGLShader::CodeDefaultVertex());
  pipeline_->addShaderFromSourceCode(QOpenGLShader::Fragment, Node::ReadFileAsString(":/shaders/rgbwaveform.frag"));
  pipeline_->link();
}

void WaveformScope::paintGL()
{
  context()->functions()->glClearColor(0, 0, 0, 0);
  context()->functions()->glClear(GL_COLOR_BUFFER_BIT);

  if (!pipeline_ || !texture_) {
    return;
  }

  pipeline_->bind();
  pipeline_->setUniformValue("ove_resolution", texture_->width(), texture_->height());

  // The general size of a pixel
  pipeline_->setUniformValue("threshold", 2.0f / static_cast<float>(height()));

  pipeline_->release();

  texture_->Bind();

  OpenGLRenderFunctions::Blit(pipeline_);

  texture_->Release();
}

void WaveformScope::CleanUp()
{
  pipeline_ = nullptr;
  texture_ = nullptr;
}

OLIVE_NAMESPACE_EXIT
