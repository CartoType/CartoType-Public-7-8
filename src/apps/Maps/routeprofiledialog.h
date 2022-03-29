#ifndef ROUTEPROFILEDIALOG_H
#define ROUTEPROFILEDIALOG_H

#include <QDialog>
#include <cartotype_framework.h>

namespace Ui {
class RouteProfileDialog;
}

class RouteProfileDialog: public QDialog
    {
    Q_OBJECT

    public:
    explicit RouteProfileDialog(QWidget *aParent,const CartoType::TRouteProfile& aRouteProfile,bool aMetricUnits);
    ~RouteProfileDialog();
    const CartoType::TRouteProfile& RouteProfile();

    Ui::RouteProfileDialog* m_ui;

    private slots:
    void on_roadTypeList_currentTextChanged(const QString &aText);
    void on_speedSlider_valueChanged(int aValue);
    void on_bonusSlider_valueChanged(int aValue);
    void on_tollSlider_valueChanged(int aValue);
    void on_copyDrive_clicked();
    void on_copyCycle_clicked();
    void on_copyWalk_clicked();
    void on_copyHike_clicked();
    void on_restrictionCheckBox_toggled(bool aChecked);
    void on_onewayCheckBox_toggled(bool aChecked);
    void on_vehicleTypeList_currentIndexChanged(int aIndex);
    void on_vehicleWeightSlider_valueChanged(int aValue);
    void on_vehicleHeightSlider_valueChanged(int aValue);

    private:
    void Set(const CartoType::TRouteProfile& aRouteProfile);
    void SetRoadType();
    void SetRoadTypeLabel();
    void SetSpeedLabel();
    void SetBonusLabel();
    void SetTollLabel();
    void SetWeightLabel();
    void SetHeightLabel();
    void SetRestrictions();
    void Copy(CartoType::TRouteProfileType aType);

    CartoType::TRouteProfile m_route_profile;
    bool m_metric_units;
    };

#endif // ROUTEPROFILEDIALOG_H
