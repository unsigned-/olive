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

#include "nodeviewedge.h"

#include <QApplication>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>

#include "common/clamp.h"
#include "common/lerp.h"
#include "nodeview.h"
#include "nodeviewscene.h"

OLIVE_NAMESPACE_ENTER

NodeViewEdge::NodeViewEdge(QGraphicsItem *parent) :
  QGraphicsPathItem(parent),
  edge_(nullptr),
  color_group_(QPalette::Active),
  color_role_(QPalette::Text)
{
  // Ensures this UI object is drawn behind other objects
  setZValue(-1);

  // Use font metrics to set edge width for basic high DPI support
  edge_width_ = QFontMetrics(QFont()).height() / 12;
}

void NodeViewEdge::SetEdge(NodeEdgePtr edge)
{
  SetConnected(true);

  // Set the new edge pointer
  edge_ = edge;

  // Re-adjust the line positioning for this new edge
  Adjust();
}

NodeEdgePtr NodeViewEdge::edge()
{
  return edge_;
}

void NodeViewEdge::Adjust()
{
  if (!edge_ || !scene()) {
    return;
  }

  // Get the UI objects of both nodes that this edge connects
  NodeViewItem* output = static_cast<NodeViewScene*>(scene())->NodeToUIObject(edge_->output()->parentNode());
  NodeViewItem* input = static_cast<NodeViewScene*>(scene())->NodeToUIObject(edge_->input()->parentNode());

  if (!output || !input) {
    return;
  }

  // Draw a line between the two
  SetPoints(output->GetParamPoint(edge_->output()), input->GetParamPoint(edge_->input()));
}

void NodeViewEdge::SetConnected(bool c)
{
  if (c) {
    color_group_ = QPalette::Active;
  } else {
    color_group_ = QPalette::Disabled;
  }

  UpdatePen();
}

void NodeViewEdge::SetHighlighted(bool e)
{
  if (e) {
    color_role_ = QPalette::Highlight;
  } else {
    color_role_ = QPalette::Text;
  }

  UpdatePen();
}

void NodeViewEdge::SetPoints(const QPointF &start, const QPointF &end)
{
  QPainterPath path;
  double half_x = lerp(start.x(), end.x(), 0.5);
  path.moveTo(start);
  path.cubicTo(QPointF(half_x, start.y()), QPointF(half_x, end.y()), end);
  setPath(path);
}

void NodeViewEdge::UpdatePen()
{
  setPen(QPen(qApp->palette().color(color_group_, color_role_), edge_width_));

  //update();
}

OLIVE_NAMESPACE_EXIT
