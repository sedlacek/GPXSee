#include <QPainter>
#include "config.h"
#include "ll.h"
#include "misc.h"
#include "scaleitem.h"


#define SCALE_WIDTH  132
#define SCALE_HEIGHT 5
#define SEGMENTS 3

#define PADDING 4


ScaleItem::ScaleItem(QGraphicsItem *parent) : QGraphicsItem(parent)
{
	_units = Metric;
	_zoom = ZOOM_MIN;
	_lat = 0;

#ifndef Q_OS_MAC
	setCacheMode(QGraphicsItem::DeviceCoordinateCache);
#endif // Q_OS_MAC
}

void ScaleItem::updateBoundingRect()
{
	QFont font;
	font.setPixelSize(FONT_SIZE);
	font.setFamily(FONT_FAMILY);
	QFontMetrics fm(font);
	QRect ss, es, us;

	ss = fm.tightBoundingRect(QString::number(0));
	es = fm.tightBoundingRect(QString::number(_length * SEGMENTS));
	us = fm.tightBoundingRect(units());

	_boundingRect = QRectF(-ss.width()/2, 0, _width * SEGMENTS + ss.width()/2
	  + qMax(us.width() + PADDING, es.width()/2) + 1, SCALE_HEIGHT + PADDING
	  + ss.height() + 2*fm.descent());
}

void ScaleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	QFont font;
	font.setPixelSize(FONT_SIZE);
	font.setFamily(FONT_FAMILY);
	QFontMetrics fm(font);
	QRect br;
	bool aa;


	if ((aa = painter->testRenderHint(QPainter::Antialiasing)))
		painter->setRenderHint(QPainter::Antialiasing, false);

	painter->setFont(font);

	for (int i = 0; i <= SEGMENTS; i++) {
		QString label = QString::number(_length * i);
		br = fm.tightBoundingRect(label);
		painter->drawText(_width * i - br.width()/2, br.height(), label);
	}
	painter->drawText(_width * SEGMENTS + PADDING, SCALE_HEIGHT + PADDING
	  + br.height() + fm.descent(), units());

	painter->drawRect(QRectF(0, br.height() + PADDING, SEGMENTS * _width,
	  SCALE_HEIGHT));
	for (int i = 0; i < SEGMENTS; i += 2)
		painter->fillRect(QRectF(i * _width, br.height() + PADDING, _width,
		  SCALE_HEIGHT), Qt::black);

/*
	painter->setPen(Qt::red);
	painter->drawRect(boundingRect());
*/

	if (aa)
		painter->setRenderHint(QPainter::Antialiasing, true);
}

QString ScaleItem::units() const
{
	if (_units == Imperial)
		return _scale ? qApp->translate("ScaleItem", "mi")
		  : qApp->translate("ScaleItem", "ft");
	else
		return _scale ? qApp->translate("ScaleItem", "km")
		  : qApp->translate("ScaleItem", "m");
}

void ScaleItem::computeScale()
{
	qreal res = zoom2resolution(_zoom, _lat);

	if (_units == Imperial) {
		_length = niceNum((res * M2FT * SCALE_WIDTH) / SEGMENTS, 1);
		if (_length >= MIINFT) {
			_length = niceNum((res * M2FT * FT2MI * SCALE_WIDTH) / SEGMENTS, 1);
			_width = (_length / (res * M2FT * FT2MI));
			_scale = true;
		} else {
			_width = (_length / (res * M2FT));
			_scale = false;
		}
	} else {
		_length = niceNum((res * SCALE_WIDTH) / SEGMENTS, 1);
		if (_length >= KMINM) {
			_length *= M2KM;
			_width = (_length / (res * M2KM));
			_scale = true;
		} else {
			_width = (_length / res);
			_scale = false;
		}
	}
}

void ScaleItem::setZoom(int z, qreal lat)
{
	prepareGeometryChange();
	_zoom = z;
	_lat = lat;
	computeScale();
	updateBoundingRect();
	update();
}

void ScaleItem::setZoom(int z)
{
	prepareGeometryChange();
	_zoom = z;
	computeScale();
	updateBoundingRect();
	update();
}

void ScaleItem::setUnits(enum Units units)
{
	prepareGeometryChange();
	_units = units;
	computeScale();
	updateBoundingRect();
	update();
}
