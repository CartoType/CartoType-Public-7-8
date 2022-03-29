package com.cartotype.cartotypemaps;
import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.widget.Toast;

import com.cartotype.*;
import com.cartotype.Error;

@SuppressLint("ViewConstructor")
public class MainView extends MapView implements DialogInterface.OnClickListener
    {
    MainView(Context aContext,Framework aFramework)
        {
        super(aContext,aFramework);
        m_framework = aFramework;
        }

    void showError(String aText)
        {
        Toast.makeText(getContext(),"error: " + aText,Toast.LENGTH_SHORT).show();
        }

    @Override
    public void onClick(DialogInterface aDialog,int aWhich)
        {
        if (aWhich == AlertDialog.BUTTON_NEGATIVE)
            {
            m_start_x = m_cur_x;
            m_start_y = m_cur_y;
            calculateAndDisplayRoute();
            }
        else if (aWhich == AlertDialog.BUTTON_POSITIVE)
            {
            m_end_x = m_cur_x;
            m_end_y = m_cur_y;
            calculateAndDisplayRoute();
            }
        }

    @Override
    public void onLongPress(double aX,double aY)
        {
        m_cur_x = aX;
        m_cur_y = aY;
        AlertDialog.Builder builder = new AlertDialog.Builder(getContext());
        builder.setTitle("Routing");
        String summary = m_framework.getGeoCodeSummary(aX,aY,CoordType.Degree);
        builder.setMessage(summary);
        builder.setCancelable(true);
        builder.setNegativeButton("Set start of route",this);
        builder.setPositiveButton("Set end of route",this);
        AlertDialog dialog = builder.create();
        dialog.show();
        }

    void calculateAndDisplayRoute()
        {
        if ((m_start_x == 0 && m_start_y == 0) || (m_end_x == 0 && m_end_y == 0))
            return;
        int error = m_framework.startNavigation(m_start_x, m_start_y, CoordType.Degree, m_end_x, m_end_y, CoordType.Degree);
        if (error == 0)
            return;
        switch (error)
            {
            case Error.NO_ROADS_NEAR_START_OF_ROUTE: showError("no roads near start of route"); break;
            case Error.NO_ROADS_NEAR_END_OF_ROUTE: showError("no roads near end of route"); break;
            case Error.NO_ROUTE_CONNECTIVITY: showError("start and end are not connected"); break;
            default: showError("routing error, code " + error); break;
            }
        }

    void setRouteProfileType(RouteProfileType aType)
        {
        if (aType != m_route_profile_type)
            {
            m_route_profile_type = aType;
            m_framework.setMainProfile(new RouteProfile(aType));
            calculateAndDisplayRoute();
            }
        }

    void reverseRoute()
        {
        int error = m_framework.reverseRoutes();
        if (error == 0)
            {
            double x = m_start_x; m_start_x = m_end_x; m_end_x = x;
            double y = m_start_y; m_start_y = m_end_y; m_end_y = y;
            }
        }

    void deleteRoute()
        {
        m_framework.deleteRoutes();
        m_start_x = 0; m_start_y = 0;
        m_end_x = 0; m_end_y = 0;
        }

    void setRouteStart(double aX,double aY)
        {
        m_start_x = aX;
        m_start_y = aY;
        calculateAndDisplayRoute();
        }

    boolean hasDestination()
        {
        return m_end_x != 0 && m_end_y != 0;
        }

    private Framework m_framework;
    private double m_cur_x;
    private double m_cur_y;
    private double m_start_x;
    private double m_start_y;
    private double m_end_x;
    private double m_end_y;
    private RouteProfileType m_route_profile_type = RouteProfileType.Car;
    }
