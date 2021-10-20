/***

  Olive - Non-Linear Video Editor
  Copyright (C) 2021 Olive Team

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

#include "nodeparamviewitembase.h"

#include <QEvent>
#include <QHBoxLayout>

namespace olive {

#define super QDockWidget

NodeParamViewItemBase::NodeParamViewItemBase(Node *node, QWidget *parent) :
  super(parent),
  node_(node)
{
  // Create title bar widget
  title_bar_ = new NodeParamViewItemTitleBar(this);

  // Add title bar to widget
  this->setTitleBarWidget(title_bar_);

  connect(title_bar_, &NodeParamViewItemTitleBar::ExpandedStateChanged, this, &NodeParamViewItemBase::SetExpanded);
  connect(title_bar_, &NodeParamViewItemTitleBar::PinToggled, this, &NodeParamViewItemBase::PinToggled);

  // Use dummy QWidget to retain width when not expanded (QDockWidget seems to ignore the titlebar
  // size hints and will shrink as small as possible if the body is hidden)
  hidden_body_ = new QWidget(this);
  body_ = nullptr;

  connect(node_, &Node::LabelChanged, this, &NodeParamViewItemBase::Retranslate);

  setAutoFillBackground(true);

  Retranslate();
}

void NodeParamViewItemBase::SetBody(QWidget *body)
{
  body_ = body;

  if (IsExpanded()) {
    this->setWidget(body_);
  }
}

Node *NodeParamViewItemBase::GetNode() const
{
  return node_;
}

void NodeParamViewItemBase::changeEvent(QEvent *e)
{
  if (e->type() == QEvent::LanguageChange) {
    Retranslate();
  }

  super::changeEvent(e);
}

void NodeParamViewItemBase::moveEvent(QMoveEvent *event)
{
  super::moveEvent(event);

  emit Moved();
}

void NodeParamViewItemBase::Retranslate()
{
  node_->Retranslate();

  if (node_->GetLabel().isEmpty()) {
    title_bar_->SetText(node_->Name());
  } else {
    title_bar_->SetText(tr("%1 (%2)").arg(node_->GetLabel(), node_->Name()));
  }
}

void NodeParamViewItemBase::SetExpanded(bool e)
{
  setWidget(e ? body_ : hidden_body_);
  title_bar_->SetExpanded(e);

  emit ExpandedChanged(e);
}

void NodeParamViewItemBase::ToggleExpanded()
{
  SetExpanded(!IsExpanded());
}

NodeParamViewItemTitleBar::NodeParamViewItemTitleBar(QWidget *parent) :
  QWidget(parent),
  draw_border_(true)
{
  QHBoxLayout* layout = new QHBoxLayout(this);

  collapse_btn_ = new CollapseButton();
  connect(collapse_btn_, &QPushButton::clicked, this, &NodeParamViewItemTitleBar::ExpandedStateChanged);
  layout->addWidget(collapse_btn_);

  lbl_ = new QLabel();
  layout->addWidget(lbl_);

  // Place next buttons on the far side
  layout->addStretch();

  QPushButton* pin_btn = new QPushButton(QStringLiteral("P"));
  pin_btn->setCheckable(true);
  pin_btn->setFixedSize(pin_btn->sizeHint().height(), pin_btn->sizeHint().height());
  layout->addWidget(pin_btn);
  connect(pin_btn, &QPushButton::clicked, this, &NodeParamViewItemTitleBar::PinToggled);
}

void NodeParamViewItemTitleBar::SetExpanded(bool e)
{
  draw_border_ = e;
  collapse_btn_->setChecked(e);

  update();
}

void NodeParamViewItemTitleBar::paintEvent(QPaintEvent *event)
{
  QWidget::paintEvent(event);

  if (draw_border_) {
    QPainter p(this);

    // Draw bottom border using text color
    int bottom = height() - 1;
    p.setPen(palette().text().color());
    p.drawLine(0, bottom, width(), bottom);
  }
}

void NodeParamViewItemTitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
  QWidget::mouseDoubleClickEvent(event);

  collapse_btn_->click();
}

}
