#include "Crosshair.h"

void Crosshair::draw(QPainter* painter, const QRect& chartRect) const {
    if (!visible) return;
    painter->save();
    QPen pen(QColor(255, 255, 255, 180), 1, Qt::DotLine);
    painter->setPen(pen);
    painter->drawLine(chartRect.left(), pos.y(), chartRect.right(), pos.y());
    painter->drawLine(pos.x(), chartRect.top(), pos.x(), chartRect.bottom());
    painter->restore();
}

void Crosshair::moveTo(const QPoint& p, const QRect& chartRect) {
    pos = p;
    visible = true;
}
