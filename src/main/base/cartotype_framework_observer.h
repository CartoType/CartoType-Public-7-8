/*
cartotype_framework_observer.h
Copyright (C) 2021 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_FRAMEWORK_OBSERVER_H__
#define CARTOTYPE_FRAMEWORK_OBSERVER_H__

#include <cartotype_navigation.h>

namespace CartoType
{

/**
An observer interface which receives notifications
of changes to the framework state. It is intended for the use
of higher-level GUI objects which need to update their display
after framework state has been changed programmatically. For example,
if a route is created, dynamic map objects need to be redrawn.
*/
class MFrameworkObserver: public MNavigatorObserver
    {
    public:
    virtual ~MFrameworkObserver() { }

    /**
    This virtual function is called when the map view changes,
    which can be caused by panning, zooming, rotation, moving to a
    new location, enabling or disabling a layer, or resizing the map.
    */
    virtual void OnViewChange() { }
    
    /**
    This virtual function is called when the map data changes,
    which can be caused by loading a new map, unloading a map,
    or enabling or disabling a map.
    */
    virtual void OnMainDataChange() { }
    
    /**
    This virtual function is called when the dynamic data changes,
    which can be caused by creating or deleting a route, or
    inserting or deleting a pushpin or other dynamic map object.
    */
    virtual void OnDynamicDataChange() { }

    /** This virtual function is called when the style sheet, style sheet variables, or blend style is changed. */
    virtual void OnStyleChange() { }

    /** This virtual function is called when layers are enabled or disabled, or the drawing of 3D buildings is enabled or disabled. */
    virtual void OnLayerChange() { }

    /** This virtual function is called when the notices such as the legend, scale bar and copyright notice are changed, enabled or disabled. */
    virtual void OnNoticeChange() { }
    };

}

#endif
