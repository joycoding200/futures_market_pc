#pragma once
#include <QPoint>
#include <QPainter>

struct Crosshair {
    bool   visible = false;
    QPoint pos;

    void draw(QPainter* painter, const QRect& chartRect) const;
    void moveTo(const QPoint& p, const QRect& chartRect);
    void hide() { visible = false; }
};
