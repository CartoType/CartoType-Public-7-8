#include <QMessageBox>
#include "routeprofiledialog.h"
#include "ui_routeprofiledialog.h"
#include <cartotype_navigation.h>

static const char* RoadTypeName[CartoType::KArcRoadTypeCount] =
    {
    "motorway",
    "motorway link",
    "trunk road",
    "trunk road link",
    "primary road",
    "primary road link",
    "secondary road",
    "secondary road link",
    "tertiary road",
    "unclassified road",
    "residential road",
    "track",
    "service road",
    "pedestrian road",
    "vehicular ferry",
    "passenger ferry",
    "living street",
    "cycleway",
    "path",
    "footway",
    "bridleway",
    "steps",
    "unknown road type",
    "unpaved road",
    "other road type 0",
    "other road type 1",
    "other road type 2",
    "other road type 3",
    "other road type 4",
    "other road type 5",
    "other road type 6",
    "other road type 7"
    };

static const std::pair<const char*,uint32_t> VehicleType[] =
    {
    { "walk", CartoType::KArcPedestrianAccessFlag },
    { "bicycle", CartoType::KArcBicycleAccessFlag | CartoType::KArcWrongWayFlag },
    { "motor cycle", CartoType::KArcMotorCycleAccessFlag | CartoType::KArcWrongWayFlag },
    { "private car", CartoType::KArcCarAccessFlag | CartoType::KArcWrongWayFlag },
    { "high-occupancy vehicle", CartoType::KArcHighOccupancyAccessFlag | CartoType::KArcWrongWayFlag },
    { "light goods vehicle", CartoType::KArcLightGoodsAccessFlag | CartoType::KArcWrongWayFlag },
    { "heavy goods vehicle", CartoType::KArcHeavyGoodsAccessFlag | CartoType::KArcWrongWayFlag },
    { "bus", CartoType::KArcBusAccessFlag | CartoType::KArcWrongWayFlag },
    { "taxi", CartoType::KArcTaxiAccessFlag | CartoType::KArcWrongWayFlag },
    { "emergency vehicle", CartoType::KArcEmergencyAccessFlag | CartoType::KArcWrongWayFlag },
    { "vehicle with hazardous material", CartoType::KArcOtherAccessFlag | CartoType::KArcWrongWayFlag }
    };

RouteProfileDialog::RouteProfileDialog(QWidget* aParent,const CartoType::TRouteProfile& aRouteProfile,bool aMetricUnits):
    QDialog(aParent),
    m_ui(new Ui::RouteProfileDialog),
    m_metric_units(aMetricUnits)
    {
    m_ui->setupUi(this);
    for (auto p: RoadTypeName)
        m_ui->roadTypeList->addItem(p);
    for (auto p: VehicleType)
        m_ui->vehicleTypeList->addItem(p.first);
    Set(aRouteProfile);
    }

RouteProfileDialog::~RouteProfileDialog()
    {
    delete m_ui;
    }

void RouteProfileDialog::Set(const CartoType::TRouteProfile& aRouteProfile)
    {
    m_route_profile = aRouteProfile;
    m_ui->tollSlider->setValue((1.0 - m_route_profile.iTollPenalty) * 100);

    uint32_t v = aRouteProfile.iVehicleType.iAccessFlags;
    int index = 0;
    if (v & CartoType::KArcPedestrianAccessFlag)
        index = 0;
    else if (v & CartoType::KArcBicycleAccessFlag)
        index = 1;
    else if (v & CartoType::KArcMotorCycleAccessFlag)
        index = 2;
    else if (v & CartoType::KArcCarAccessFlag)
        index = 3;
    else if (v & CartoType::KArcHighOccupancyAccessFlag)
        index = 4;
    else if (v & CartoType::KArcLightGoodsAccessFlag)
        index = 5;
    else if (v & CartoType::KArcHeavyGoodsAccessFlag)
        index = 6;
    else if (v & CartoType::KArcBusAccessFlag)
        index = 7;
    else if (v & CartoType::KArcTaxiAccessFlag)
        index = 8;
    else if (v & CartoType::KArcEmergencyAccessFlag)
        index = 9;
    else if (v & CartoType::KArcOtherAccessFlag)
        index = 10;
    m_ui->vehicleTypeList->setCurrentIndex(index);

    QString s;
    s.setNum(aRouteProfile.iTurnTime);
    m_ui->turnTime->setText(s);
    s.setNum(aRouteProfile.iUTurnTime);
    m_ui->uTurnTime->setText(s);
    s.setNum(aRouteProfile.iCrossTrafficTurnTime);
    m_ui->crossTrafficTurnTime->setText(s);
    s.setNum(aRouteProfile.iTrafficLightTime);
    m_ui->trafficLightTime->setText(s);
    m_ui->vehicleWeightSlider->setValue((int)(aRouteProfile.iVehicleType.iWeight * 10));
    m_ui->vehicleHeightSlider->setValue((int)(aRouteProfile.iVehicleType.iHeight * 100));

    SetRoadType();
    SetTollLabel();
    SetWeightLabel();
    SetHeightLabel();
    }

const CartoType::TRouteProfile& RouteProfileDialog::RouteProfile()
    {
    m_route_profile.iVehicleType.iAccessFlags = VehicleType[m_ui->vehicleTypeList->currentIndex()].second;
    // iSpeed and iBonus are kept up to date: no need to copy them.
    // iRestrictionOverride is kept up to date: no need to copy it.
    m_route_profile.iTurnTime = m_ui->turnTime->text().toInt();
    m_route_profile.iUTurnTime = m_ui->uTurnTime->text().toInt();
    m_route_profile.iCrossTrafficTurnTime = m_ui->crossTrafficTurnTime->text().toInt();
    m_route_profile.iTrafficLightTime = m_ui->trafficLightTime->text().toInt();
    // iShortest is not currently handled.
    // iTollPenalty is kept up to date: no need to copy it.

    return m_route_profile;
    }

void RouteProfileDialog::SetRestrictions()
    {
    uint32_t v = VehicleType[m_ui->vehicleTypeList->currentIndex()].second;
    int road_type_index = m_ui->roadTypeList->currentRow();
    if (road_type_index < 0 || road_type_index >= m_route_profile.iRestrictionOverride.size())
        {
        m_ui->restrictionCheckBox->setEnabled(false);
        m_ui->restrictionCheckBox->setChecked(false);
        m_ui->onewayCheckBox->setEnabled(false);
        m_ui->onewayCheckBox->setChecked(false);
        return;
        }

    if (v & ~CartoType::KArcWrongWayFlag)
        {
        m_ui->restrictionCheckBox->setEnabled(true);
        if (m_route_profile.iRestrictionOverride[road_type_index] & v & ~CartoType::KArcWrongWayFlag)
            m_ui->restrictionCheckBox->setChecked(false);
        else
            m_ui->restrictionCheckBox->setChecked(true);
        }
    else
        {
        m_ui->restrictionCheckBox->setEnabled(false);
        m_ui->restrictionCheckBox->setChecked(false);
        }

    if (v & CartoType::KArcWrongWayFlag)
        {
        m_ui->onewayCheckBox->setEnabled(true);
        if (m_route_profile.iRestrictionOverride[road_type_index] & CartoType::KArcWrongWayFlag)
            m_ui->onewayCheckBox->setChecked(false);
        else
            m_ui->onewayCheckBox->setChecked(true);
        }
    else
        {
        m_ui->onewayCheckBox->setEnabled(false);
        m_ui->onewayCheckBox->setChecked(false);
        }
    }

void RouteProfileDialog::SetRoadType()
    {
    int index = m_ui->roadTypeList->currentRow();
    if (index < 0)
        return;

    m_ui->speedSlider->setValue(m_route_profile.iSpeed[index]);
    m_ui->bonusSlider->setValue(m_route_profile.iBonus[index]);

    SetRestrictions();
    SetRoadTypeLabel();
    SetSpeedLabel();
    SetBonusLabel();
    }

void RouteProfileDialog::SetRoadTypeLabel()
    {
    if (!m_ui->roadTypeList->currentItem())
        return;

    QString s(m_ui->roadTypeList->currentItem()->text());
    int index = m_ui->roadTypeList->currentRow();
    double total = m_route_profile.iSpeed[index] + m_route_profile.iBonus[index];
    if (total < 0)
        s += ": forbid (negative weighting)";
    else if (total == 0)
        s += ": avoid (zero weighting)";
    else
        {
        QString n;
        if (!m_metric_units)
            total /= 1.609344;
        n.setNum(CartoType::Round(total));
        s += ": total weighting = " + n;
        s += m_metric_units ? " kph" : " mph";
        }
    m_ui->roadTypeLabel->setText(s);
    }

void RouteProfileDialog::SetSpeedLabel()
    {
    double speed = m_ui->speedSlider->value();
    if (!m_metric_units)
        speed /= 1.609344; // convert to mph
    QString s = "expected speed: ";
    QString n; n.setNum(CartoType::Round(speed));
    s += n;
    s += m_metric_units ? " kph" : " mph";
    m_ui->speedLabel->setText(s);
    }

void RouteProfileDialog::SetBonusLabel()
    {
    double bonus = m_ui->bonusSlider->value();
    if (!m_metric_units)
        bonus /= 1.609344; // convert to mph
    QString s = "bonus: ";
    QString n; n.setNum(CartoType::Round(bonus));
    s += n;
    s += m_metric_units ? " kph" : " mph";
    m_ui->bonusLabel->setText(s);
    }

void RouteProfileDialog::SetTollLabel()
    {
    QString s;
    int value = m_ui->tollSlider->value();
    if (value == 100)
        s = "toll road preference: equal to other roads";
    else if (value == 0)
        s = "toll road preference: forbid";
    else
        {
        QString n;
        n.setNum(value);
        s = "toll road preference: " + n + "%";
        }
    m_ui->tollLabel->setText(s);
    }

void RouteProfileDialog::SetWeightLabel()
    {
    QString s;
    int value = m_ui->vehicleWeightSlider->value();
    if (value == 0)
        s = "vehicle weight not specified";
    else
        {
        QString n;
        n.setNum(double(value) / 10);
        s = "vehicle weight: " + n + "t";
        }
    m_ui->weightLabel->setText(s);
    }

void RouteProfileDialog::SetHeightLabel()
    {
    QString s;
    int value = m_ui->vehicleHeightSlider->value();
    if (value == 0)
        s = "vehicle height not specified";
    else
        {
        QString n;
        n.setNum(double(value) / 100);
        s = "vehicle height: " + n + "m";
        }
    m_ui->heightLabel->setText(s);
    }

void RouteProfileDialog::Copy(CartoType::TRouteProfileType aType)
    {
    QString s;
    switch (aType)
        {
        case CartoType::TRouteProfileType::Car: default: s = "Copy driving route profile"; break;
        case CartoType::TRouteProfileType::Cycle: s = "Copy cycling route profile"; break;
        case CartoType::TRouteProfileType::Walk: s = "Copy walking route profile"; break;
        case CartoType::TRouteProfileType::Hike: s = "Copy hiking route profile"; break;
        }

    QMessageBox message_box(QMessageBox::Question,s,"Overwrite the current profile?",
                            QMessageBox::Yes | QMessageBox::Cancel);
    QMessageBox::StandardButton button = (QMessageBox::StandardButton)message_box.exec();
    if (button == QMessageBox::Yes)
        {
        CartoType::TRouteProfile p(aType);
        Set(p);
        }
    }

void RouteProfileDialog::on_roadTypeList_currentTextChanged(const QString& /*aText*/)
    {
    SetRoadType();
    }

void RouteProfileDialog::on_speedSlider_valueChanged(int aValue)
    {
    m_route_profile.iSpeed[m_ui->roadTypeList->currentRow()] = aValue;
    SetRoadTypeLabel();
    SetSpeedLabel();
    }

void RouteProfileDialog::on_bonusSlider_valueChanged(int aValue)
    {
    m_route_profile.iBonus[m_ui->roadTypeList->currentRow()] = aValue;
    SetRoadTypeLabel();
    SetBonusLabel();
    }

void RouteProfileDialog::on_tollSlider_valueChanged(int aValue)
    {
    m_route_profile.iTollPenalty = 1.0 - aValue / 100.0;
    SetTollLabel();
    }

void RouteProfileDialog::on_copyDrive_clicked()
    {
    Copy(CartoType::TRouteProfileType::Car);
    }

void RouteProfileDialog::on_copyCycle_clicked()
    {
    Copy(CartoType::TRouteProfileType::Cycle);
    }

void RouteProfileDialog::on_copyWalk_clicked()
    {
    Copy(CartoType::TRouteProfileType::Walk);
    }

void RouteProfileDialog::on_copyHike_clicked()
    {
    Copy(CartoType::TRouteProfileType::Hike);
    }

void RouteProfileDialog::on_restrictionCheckBox_toggled(bool aChecked)
    {
    uint32_t v = VehicleType[m_ui->vehicleTypeList->currentIndex()].second & ~CartoType::KArcWrongWayFlag;
    if (v)
        {
        int index = m_ui->roadTypeList->currentRow();
        if (aChecked)
            m_route_profile.iRestrictionOverride[index] &= ~CartoType::KArcAllVehicles;
        else
            m_route_profile.iRestrictionOverride[index] |= CartoType::KArcAllVehicles;
        }
    }

void RouteProfileDialog::on_onewayCheckBox_toggled(bool aChecked)
    {
    uint32_t v = VehicleType[m_ui->vehicleTypeList->currentIndex()].second & CartoType::KArcWrongWayFlag;
    if (v)
        {
        int index = m_ui->roadTypeList->currentRow();
        if (aChecked)
            m_route_profile.iRestrictionOverride[index] &= ~CartoType::KArcWrongWayFlag;
        else
            m_route_profile.iRestrictionOverride[index] |= CartoType::KArcWrongWayFlag;
        }
    }

void RouteProfileDialog::on_vehicleTypeList_currentIndexChanged(int /*aIndex*/)
    {
    SetRestrictions();
    }

void RouteProfileDialog::on_vehicleWeightSlider_valueChanged(int aValue)
    {
    m_route_profile.iVehicleType.iWeight = (double)aValue / 10;
    SetWeightLabel();
    }

void RouteProfileDialog::on_vehicleHeightSlider_valueChanged(int aValue)
    {
    m_route_profile.iVehicleType.iHeight = (double)aValue / 100;
    SetHeightLabel();
    }
