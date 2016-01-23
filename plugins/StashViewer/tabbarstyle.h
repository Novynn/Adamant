#ifndef TABBARSTYLE
#define TABBARSTYLE

#include "QtWidgets/5.5.0/QtWidgets/private/qfusionstyle_p.h"

class TabBarStyle : public QFusionStyle {
    Q_OBJECT
public:
    TabBarStyle() : QFusionStyle() {}

protected:
    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const {
        if (metric == PM_TabBarBaseHeight || metric == PM_TabBarBaseOverlap) {
            return 0;
        }
        return QFusionStyle::pixelMetric(metric, option, widget);
    }
};

#endif // TABBARSTYLE

