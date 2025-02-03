/* PlanetView.h
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef PLANETVIEW_H
#define PLANETVIEW_H

#include <QWidget>

class QLineEdit;
class QPlainTextEdit;
class QCheckBox;

class LandscapeView;
class Map;
class StellarObject;



class PlanetView : public QWidget
{
    Q_OBJECT
public:
    explicit PlanetView(Map &mapData, QWidget *parent = 0);

    void SetPlanet(StellarObject *object);
    void Reinitialize();

signals:

public slots:
    void TrueNameChanged();
    void TrueNameEdited();
    void UseDisplayNameChanged();
    void DisplayNameChanged();
    void AttributesChanged();
    void GovernmentChanged();
    void DescriptionChanged();
    void SpaceportDescriptionChanged();
    void ShipyardChanged();
    void OutfitterChanged();
    void ReputationChanged();
    void BribeChanged();
    void SecurityChanged();

    void TributeChanged();
    void TributeThresholdChanged();
    void TributeFleetNamesChanged();

private:
    static QString ToString(const std::vector<QString> &list);
    static std::vector<QString> ToList(const QString &str);


private:
    Map &mapData;
    StellarObject *object = nullptr;

    QLineEdit *trueName;
    QCheckBox *useDisplayName;
    QLineEdit *displayName;
    QLineEdit *attributes;
    QLineEdit *government;
    LandscapeView *landscape;
    QPlainTextEdit *description;
    QPlainTextEdit *spaceport;
    QLineEdit *shipyard;
    QLineEdit *outfitter;
    QLineEdit *reputation;
    QLineEdit *bribe;
    QLineEdit *security;
    QLineEdit *tribute;
    QLineEdit *tributeThreshold;
    QLineEdit *tributeFleetNames;
};

#endif // PLANETVIEW_H
