/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2016 Matus & Michal Tomlein

 Leaklog is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public Licence
 as published by the Free Software Foundation; either version 2
 of the Licence, or (at your option) any later version.

 Leaklog is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public Licence for more details.

 You should have received a copy of the GNU General Public Licence
 along with Leaklog; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
********************************************************************/

#ifndef MTCOLOURCOMBOBOX_H
#define MTCOLOURCOMBOBOX_H

#include <QComboBox>

class MTColourComboBox : public QComboBox
{
    Q_OBJECT

public:
    MTColourComboBox(QWidget *parent = 0):
    QComboBox(parent) {
        QStringList colours;
        colours << "aliceblue"
        << "antiquewhite"
        << "aqua"
        << "aquamarine"
        << "azure"
        << "beige"
        << "bisque"
        << "black"
        << "blanchedalmond"
        << "blue"
        << "blueviolet"
        << "brown"
        << "burlywood"
        << "cadetblue"
        << "chartreuse"
        << "chocolate"
        << "coral"
        << "cornflowerblue"
        << "cornsilk"
        << "crimson"
        << "cyan"
        << "darkblue"
        << "darkcyan"
        << "darkgoldenrod"
        << "darkgreen"
        << "darkgrey"
        << "darkkhaki"
        << "darkmagenta"
        << "darkolivegreen"
        << "darkorange"
        << "darkorchid"
        << "darkred"
        << "darksalmon"
        << "darkseagreen"
        << "darkslateblue"
        << "darkslategrey"
        << "darkturquoise"
        << "darkviolet"
        << "deeppink"
        << "deepskyblue"
        << "dimgrey"
        << "dodgerblue"
        << "firebrick"
        << "floralwhite"
        << "forestgreen"
        << "fuchsia"
        << "gainsboro"
        << "ghostwhite"
        << "gold"
        << "goldenrod"
        << "grey"
        << "green"
        << "greenyellow"
        << "honeydew"
        << "hotpink"
        << "indianred"
        << "indigo"
        << "ivory"
        << "khaki"
        << "lavender"
        << "lavenderblush"
        << "lawngreen"
        << "lemonchiffon"
        << "lightblue"
        << "lightcoral"
        << "lightcyan"
        << "lightgoldenrodyellow"
        << "lightgreen"
        << "lightgrey"
        << "lightpink"
        << "lightsalmon"
        << "lightseagreen"
        << "lightskyblue"
        << "lightslategrey"
        << "lightsteelblue"
        << "lightyellow"
        << "lime"
        << "limegreen"
        << "linen"
        << "magenta"
        << "maroon"
        << "mediumaquamarine"
        << "mediumblue"
        << "mediumorchid"
        << "mediumpurple"
        << "mediumseagreen"
        << "mediumslateblue"
        << "mediumspringgreen"
        << "mediumturquoise"
        << "mediumvioletred"
        << "midnightblue"
        << "mintcream"
        << "mistyrose"
        << "moccasin"
        << "navajowhite"
        << "navy"
        << "oldlace"
        << "olive"
        << "olivedrab"
        << "orange"
        << "orangered"
        << "orchid"
        << "palegoldenrod"
        << "palegreen"
        << "paleturquoise"
        << "palevioletred"
        << "papayawhip"
        << "peachpuff"
        << "peru"
        << "pink"
        << "plum"
        << "powderblue"
        << "purple"
        << "red"
        << "rosybrown"
        << "royalblue"
        << "saddlebrown"
        << "salmon"
        << "sandybrown"
        << "seagreen"
        << "seashell"
        << "sienna"
        << "silver"
        << "skyblue"
        << "slateblue"
        << "slategrey"
        << "snow"
        << "springgreen"
        << "steelblue"
        << "tan"
        << "teal"
        << "thistle"
        << "tomato"
        << "turquoise"
        << "violet"
        << "wheat"
        << "white"
        << "whitesmoke"
        << "yellow"
        << "yellowgreen";
        this->addItem(QString());
        for (int i = 0; i < colours.count(); ++i) {
            QPixmap pm(16, 16);
            pm.fill(QColor(colours.at(i)));
            this->addItem(QIcon(pm), colours.at(i));
        }
    };
};

#endif // MTCOLOURCOMBOBOX_H
