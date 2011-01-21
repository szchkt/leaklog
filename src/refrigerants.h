#ifndef REFRIGERANTS_H
#define REFRIGERANTS_H

#include <QMap>
#include <QString>

class Refrigerants
{
public:
    Refrigerants();
    inline double pressureToTemperature(const QString & c, double r) {
        return dict_pressureToTemperature.value(c).value(r, -273.15);
    };
    inline QMap<QString, QMap<double, double> > * pressureToTemperature() {
        return &dict_pressureToTemperature;
    };
private:
    QMap<QString, QMap<double, double> > dict_pressureToTemperature;
};

#endif // REFRIGERANTS_H
