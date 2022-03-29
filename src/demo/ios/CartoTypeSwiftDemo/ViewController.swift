//
//  ViewController.swift
//  CartoTypeSwiftDemo
//
//  Copyright © 2016-2021 CartoType Ltd. All rights reserved.
//

import UIKit
import GLKit
import CoreLocation

class ViewController: CartoTypeViewController, UISearchBarDelegate, CLLocationManagerDelegate
    {
    var m_framework: CartoTypeFramework!
    var m_ui_scale: Double = 1
    var m_route_start_in_degrees = CartoTypePoint(x:0, y:0)
    var m_route_end_in_degrees = CartoTypePoint(x:0, y:0)
    var m_last_point_pressed_in_degrees = CartoTypePoint(x:0, y:0)
    var m_route_profile_type = CarProfile
    var m_search_bar: UISearchBar!
    var m_restore_search_bar: Bool = false
    var m_toolbar: UIToolbar!
    var m_find_button = UIBarButtonItem(title: "Find", style: UIBarButtonItem.Style.plain, target: self, action: #selector(onFindButton))
    var m_view_button = UIBarButtonItem(title: "View", style: UIBarButtonItem.Style.plain, target: self, action: #selector(onViewButton))
    var m_route_button = UIBarButtonItem(title: "Route", style: UIBarButtonItem.Style.plain, target: self, action: #selector(onRouteButton))
    var m_navigate_button = UIBarButtonItem(title: "Start", style: UIBarButtonItem.Style.plain, target: self, action: #selector(onNavigateButton))
    var m_help_button = UIBarButtonItem(title: "Help", style:UIBarButtonItem.Style.plain, target: self, action: #selector(onHelpButton))
    var m_pushpin_id: UInt64 = 0
    var m_navigating: Bool = false
    var m_show_location: Bool = false
    var m_location_manager: CLLocationManager!
    var m_location: CLLocation!
    var m_has_scale_bar: Bool = true
    var m_metric_units: Bool = true
    var m_find_text: String = ""
    var m_found_item = CartoTypeMapObject()
    var m_found_item_id: UInt64 = 0
    var m_ignore_symbols: Bool = true
    var m_fuzzy: Bool = false
    
    required init?(coder aDecoder: NSCoder)
        {
        super.init(coder: aDecoder)
        }
    
    override init(nibName nibNameOrNil: String?, bundle nibBundleOrNil: Bundle?)
        {
        super.init(nibName: nibNameOrNil, bundle: nibBundleOrNil)
        }

    init(aFrameWork: CartoTypeFramework!, aBounds: CGRect)
        {
        super.init(_:aFrameWork, bounds:aBounds)
        
        m_ui_scale = Double(view.contentScaleFactor)
        print("scale = \(m_ui_scale)")

        m_framework = aFrameWork
        self.becomeFirstResponder()
            
        // Set up location services.
        m_location_manager = CLLocationManager.init()
        m_location_manager.delegate = self
        m_location_manager.desiredAccuracy = kCLLocationAccuracyBest
        
        if (CLLocationManager.authorizationStatus() != CLAuthorizationStatus.authorizedAlways &&
            CLLocationManager.authorizationStatus() != CLAuthorizationStatus.authorizedWhenInUse)
            {
            m_location_manager.requestWhenInUseAuthorization()
            }
        }
        
    func createUI()
        {
        // Create a toolbar.
        if (m_toolbar == nil)
            {
            updateRouteStatus()
            m_toolbar = UIToolbar.init()
            let toolbar_items = [ m_find_button, m_view_button, m_route_button, m_navigate_button, m_help_button ]
            m_toolbar.setItems(toolbar_items, animated: true)
            m_toolbar.setBackgroundImage(UIImage(),forToolbarPosition: .any,barMetrics: .default)
            m_toolbar.setShadowImage(UIImage(), forToolbarPosition: .any)
            view.addSubview(m_toolbar!)
            }
            
        // Create a search bar.
        if (m_search_bar == nil)
            {
            m_search_bar = UISearchBar.init()
            m_search_bar.delegate = self
            // Show cancel button.
            m_search_bar.showsCancelButton = true
            
            // Set placeholder
            m_search_bar.placeholder = "Find a place"
            
            // Add the search bar to the view.
            view.addSubview(m_search_bar!)
            }
        
        var insets = UIEdgeInsets()
        if #available(iOS 11.0,*)
            {
            insets = view.safeAreaInsets
            }
        else
            {
            insets.top = topLayoutGuide.length
            insets.bottom = bottomLayoutGuide.length
            }
        let safe_area = view.bounds.inset(by: insets)
        m_toolbar.frame = CGRect(x:0, y:safe_area.maxY - 30, width:view.bounds.size.width, height: 30)
        m_search_bar.frame = CGRect(x:0, y:safe_area.minY, width:view.bounds.size.width, height:40)
            
        if (m_framework != nil)
            {
            createScaleBarAndTurnInstructions()
            // Set the vehicle location to a quarter of the way up the display.
            m_framework.setVehiclePosOffsetX(0, andY: 0.25)
            }
        }
        
    func createScaleBarAndTurnInstructions()
        {
        var insets = UIEdgeInsets()
        if #available(iOS 11.0,*)
            {
            insets = view.safeAreaInsets
            }
        else
            {
            insets.top = topLayoutGuide.length
            insets.bottom = bottomLayoutGuide.length
            }
        let scale_bar_pos = CartoTypeExtendedNoticePosition(NoticePositionBottomLeft, xInset: 12 * m_ui_scale, xUnit: "px", yInset: (Double(insets.bottom) + 34) * m_ui_scale, yUnit: "px")
        m_framework.setScaleBar(m_metric_units,width: Double(view.bounds.size.width - 24) * m_ui_scale,unit: "px",extendedPosition: scale_bar_pos)
        let turn_instr_pos = CartoTypeExtendedNoticePosition(NoticePositionTopLeft, xInset: 12 * m_ui_scale, xUnit: "px", yInset: (Double(insets.top) + 12) * m_ui_scale, yUnit: "px")
        m_framework.setTurnInstructions(m_metric_units, abbreviate: true, width: Double(view.bounds.size.width - 54) * m_ui_scale, widthUnit: "px", extendedPosition: turn_instr_pos, textSize: 12, textSizeUnit: "pt")
        }
        
    override func viewDidLayoutSubviews()
        {
        createUI()
        }
    
    override func didReceiveMemoryWarning()
        {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
        }
    
    override func onTap(_ aPointInDegrees: CartoTypePoint)
        {
        }
    
    override func onLongPress(_ aPointInDegrees: CartoTypePoint)
        {
        m_last_point_pressed_in_degrees = aPointInDegrees
        let p = m_framework.convert(m_last_point_pressed_in_degrees, from: DegreeCoordType, to: DisplayCoordType).point
        
        // Find nearby objects.
        let object_array = NSMutableArray.init()
        let pixel_mm = m_framework.getResolutionDpi() / 25.4
        m_framework.find(inDisplay: object_array, maxItems: 10, point: p, radius: ceil(2 * pixel_mm))
        
        // See if we have a pushpin.
        m_pushpin_id = 0
        for (cur_object) in object_array
            {
            if ((cur_object as! CartoTypeMapObject).getLayerName().isEqual("pushpin"))
                {
                m_pushpin_id = (cur_object as! CartoTypeMapObject).getId()
                
                // HACK
                
                
                break
                }
            }
            
        // Create the menu.
        let menu = UIMenuController.shared
        var pushpin_menu_item : UIMenuItem?
        if (m_pushpin_id != 0)
            {
            pushpin_menu_item = UIMenuItem.init(title: "Delete pin", action: #selector(deletePushPin))
            }
        else
            {
            pushpin_menu_item = UIMenuItem.init(title: "Insert pin", action: #selector(insertPushPin))
            }
        menu.menuItems = [
                         pushpin_menu_item!,
                         UIMenuItem.init(title: "Start here", action: #selector(ViewController.setRouteStart)),
                         UIMenuItem.init(title: "End here", action: #selector(ViewController.setRouteEnd)),
                         ]
        menu.setTargetRect(CGRect(x:p.x / m_ui_scale,y:p.y / m_ui_scale,width:1,height:1), in: view)
        menu.setMenuVisible(true, animated: true)
        }
    
    override var canBecomeFirstResponder: Bool { return true }
    
    func showError(_ aText : String)
        {
        let alert = UIAlertController(title: "Error", message: aText, preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "OK", style: .default, handler:
            { _ in
            }))
        present(alert, animated: true, completion: nil)
        }
    
    func calculateAndDisplayRoute()
        {
        if ((m_route_start_in_degrees.x == 0 && m_route_start_in_degrees.y == 0) || (m_route_end_in_degrees.x == 0 && m_route_end_in_degrees.y == 0))
            {
            return
            }
        let cs = CartoTypeRouteCoordSet()
        cs.coordType = DegreeCoordType
        let start = CartoTypeRoutePoint()
        start.point = m_route_start_in_degrees
        let end = CartoTypeRoutePoint()
        end.point = m_route_end_in_degrees
        cs.append(start)
        cs.append(end)
        let error = m_framework.startNavigation(cs)
        let errorcode = CartoTypeResultCode(rawValue: UInt32(error))
        m_navigate_button.isEnabled = false
        switch (errorcode)
            {
            case CTErrorNone:
                stopNavigating()
                m_show_location = false
                m_navigate_button.isEnabled = true
                break
            case CTErrorNoRoadsNearStartOfRoute: showError("no roads near start of route"); break
            case CTErrorNoRoadsNearEndOfRoute: showError("no roads near end of route"); break
            case CTErrorNoRouteConnectivity: showError("start and end are not connected"); break
            default: showError("routing error, code \(error)"); break
            }
        }
        
    func updateRouteStatus()
        {
        let have_route = m_framework.getRouteCount() != 0
        m_navigate_button.isEnabled = have_route
        if (m_navigating)
            {
            m_navigate_button.title = "End"
            }
        else
            {
            m_navigate_button.title = "Start"
            }
        if (have_route)
            {
            let route = m_framework.getRoute()!
            let start1 = route.point(0, pointIndex: 0)
            let end1 = route.point(0, pointIndex: route.pointCount(route.contourCount() - 1) - 1)
            let start2 = CartoTypePoint(x:Double(start1.x), y:Double(start1.y))
            let end2 = CartoTypePoint(x:Double(end1.x), y:Double(end1.y))
            m_route_start_in_degrees = m_framework.convert(start2, from: MapCoordType, to: DegreeCoordType).point
            m_route_end_in_degrees = m_framework.convert(end2, from: MapCoordType, to: DegreeCoordType).point
            let profile = m_framework.getProfile(0)!
            switch (profile.name)
                {
                case "car" : m_route_profile_type = CarProfile; break
                case "cycle" : m_route_profile_type = BicycleProfile; break
                case "walk" : m_route_profile_type = WalkingProfile; break
                case "hike" : m_route_profile_type = HikingProfile; break
                default: m_route_profile_type = CarProfile
                }
            }
        }
    
    func setRouteProfileType(_ aType : CartoTypeRouteProfileType)
        {
        if (aType != m_route_profile_type)
            {
            m_route_profile_type = aType
            m_framework.setMainProfileType(aType)
            calculateAndDisplayRoute()
            }
        }
        
    func chooseMapObject(_ aMapObjectArray : NSMutableArray)
        {
        let alert = UIAlertController(title: "Choose a place", message: "", preferredStyle: .alert)
        for object in aMapObjectArray
            {
            let map_object = object as! CartoTypeMapObject
            let address_object = CartoTypeAddress()
            m_framework.getAddress(address_object, mapObject: map_object)
            let address = address_object.toString(false)
            alert.addAction(UIAlertAction(title: address, style: .default, handler:
                { _ in
                let id_and_result : CartoTypeIdAndResult = self.m_framework.insertCopy(of: map_object, map: 0, layerName: "found", radius: 20, radiusCoordType: MapMeterCoordType, id: self.m_found_item_id, replace: true)
                self.m_found_item_id = id_and_result.objectId
                self.m_found_item = map_object
                self.m_framework.setViewObject(self.m_found_item, margin: Int32(8 * self.m_ui_scale), minScale: 1000)
                }))
            }
        alert.addAction(UIAlertAction(title: "Cancel", style: .default, handler:
            { _ in
            }))

        present(alert, animated: true, completion: nil)
        UILabel.appearance(whenContainedInInstancesOf:[UIAlertController.self]).numberOfLines = 2
        UILabel.appearance(whenContainedInInstancesOf:[UIAlertController.self]).lineBreakMode = .byWordWrapping
        }
        
    func find(_ aPlace : String)
        {
        if (!aPlace.isEmpty)
            {
            m_find_text = aPlace
            m_search_bar.text = aPlace
            var match_method = StringMatchFoldCaseFlag.rawValue | StringMatchIgnoreWhitespaceFlag.rawValue | StringMatchFoldAccentsFlag.rawValue
            if (m_ignore_symbols)
                {
                match_method |= StringMatchIgnoreSymbolsFlag.rawValue
                }
            if (m_fuzzy)
                {
                match_method |= StringMatchFuzzyFlag.rawValue
                }
            let find_param = CartoTypeFindParam()!
            find_param.text = aPlace
            find_param.stringMatchMethod = CartoTypeStringMatchMethod(match_method)
            find_param.maxObjectCount = 20
            let map_object_array = NSMutableArray()
            m_framework.find(map_object_array, with: find_param)
            if (map_object_array.count != 0)
                {
                chooseMapObject(map_object_array)
                }
            else
                {
                showError("place '\(aPlace)' not found")
                }
            }
        }
        
    func find()
        {
        let alert = UIAlertController(title: "Find a place", message: "", preferredStyle: .alert)
        alert.addTextField()
        alert.addAction(UIAlertAction(title: "OK", style: .default, handler:
            { _ in
            let text_field = alert.textFields![0]
            self.presentedViewController?.dismiss(animated: true, completion: nil)
            let place = text_field.text ?? ""
            self.find(place)
            }))
        alert.addAction(UIAlertAction(title: "Cancel", style: .default, handler:
            { _ in
            }))
            
        alert.textFields![0].text = m_find_text
        present(alert, animated: true, completion: nil)
        }
        
    func findAddress()
        {
        let alert = UIAlertController(title: "Find an address", message: "", preferredStyle: .alert)
        alert.addTextField()
        alert.addTextField()
        alert.addTextField()
        let building_field = alert.textFields![0]
        building_field.placeholder = "building name or number"
        let street_field = alert.textFields![1]
        street_field.placeholder = "street"
        let locality_field = alert.textFields![2]
        locality_field.placeholder = "locality"
        alert.addAction(UIAlertAction(title: "OK", style: .default, handler:
            { _ in
            let address = CartoTypeAddress()
            address.building = building_field.text ?? ""
            address.street = street_field.text ?? ""
            address.locality = locality_field.text ?? ""
            if (!address.building.isEmpty || !address.street.isEmpty || !address.locality.isEmpty)
                {
                let map_object_array = NSMutableArray()
                self.m_framework.find(map_object_array, maxItems: 20, address: address, fuzzy: self.m_fuzzy)
                if (map_object_array.count != 0)
                    {
                    self.chooseMapObject(map_object_array)
                    }
                else
                    {
                    self.showError("address not found")
                    }
                }
            }))
        alert.addAction(UIAlertAction(title: "Cancel", style: .default, handler:
            { _ in
            }))
        
        present(alert, animated: true, completion: nil)
        }
        
    @objc func onFindButton()
        {
        let alert = UIAlertController(title: "Find", message: "Find a place or an address, or set search options.", preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "Find...", style: .default, handler:
            { _ in
            self.presentedViewController?.dismiss(animated: true, completion: nil)
            self.find()
            }))
        alert.addAction(UIAlertAction(title: "Find Address...", style: .default, handler:
            { _ in
            self.presentedViewController?.dismiss(animated: true, completion: nil)
            self.findAddress()
            }))
        alert.addAction(UIAlertAction(title: m_ignore_symbols ? "Don't Ignore Symbols" : "Ignore Symbols", style: .default, handler:
            { _ in
            self.m_ignore_symbols = !self.m_ignore_symbols
            }))
        alert.addAction(UIAlertAction(title: m_fuzzy ? "Don't Use Fuzzy Matching" : "Use Fuzzy Matching", style: .default, handler:
            { _ in
            self.m_fuzzy = !self.m_fuzzy
            }))
        alert.addAction(UIAlertAction(title: "Cancel", style: .default, handler:
            { _ in
            }))

        present(alert, animated: true, completion: nil)
        }
    
    @objc func onRouteButton()
        {
        var cur_type : String
        switch m_route_profile_type
            {
            case CarProfile: cur_type = "Car"; break
            case BicycleProfile: cur_type = "Bike"; break
            case WalkingProfile: cur_type = "Walk"; break
            case HikingProfile: cur_type = "Hike"; break
            default: cur_type = "Unknown"; break
            }
        
        let alert = UIAlertController(title: "Route options", message: "The current profile is '\(cur_type)'.", preferredStyle: .alert)
        
        if (m_framework.getRouteCount() != 0)
            {
            alert.addAction(UIAlertAction(title: "Reverse Route", style: .default, handler:
                { _ in
                let temp = self.m_route_start_in_degrees
                self.m_route_start_in_degrees = self.m_route_end_in_degrees
                self.m_route_end_in_degrees = temp
                self.calculateAndDisplayRoute()
                }))
            alert.addAction(UIAlertAction(title: "Delete Route", style: .default, handler:
                { _ in
                self.stopNavigating()
                self.m_framework.deleteRoutes()
                self.m_navigate_button.isEnabled = false
                }))
            }
           
        if (m_route_profile_type != CarProfile)
            {
            alert.addAction(UIAlertAction(title: "Car", style: .default, handler:
                { _ in
                self.setRouteProfileType(CarProfile)
                }))
            }
        if (m_route_profile_type != BicycleProfile)
            {
            alert.addAction(UIAlertAction(title: "Bike", style: .default, handler:
                { _ in
                self.setRouteProfileType(BicycleProfile)
                }))
            }
        if (m_route_profile_type != WalkingProfile)
            {
            alert.addAction(UIAlertAction(title: "Walk", style: .default, handler:
                { _ in
                self.setRouteProfileType(WalkingProfile)
                }))
            }
        if (m_route_profile_type != HikingProfile)
            {
            alert.addAction(UIAlertAction(title: "Hike", style: .default, handler:
                { _ in
                self.setRouteProfileType(HikingProfile)
                }))
            }
        alert.addAction(UIAlertAction(title: "Cancel", style: .default, handler:
            { _ in
            }))
            
        present(alert, animated: true, completion: nil)
        }

    @objc func onViewButton()
        {
        let alert = UIAlertController(title: "View options", message: "Choose the appearance of the map.", preferredStyle: .alert)
        
        if (m_show_location)
            {
            alert.addAction(UIAlertAction(title: "Stop Showing Location", style: .default, handler:
                { _ in
                self.stopShowingLocation()
                }))
            }
        else if (!m_navigating && m_framework.getRouteCount() == 0)
            {
            alert.addAction(UIAlertAction(title: "Show Location", style: .default, handler:
                { _ in
                self.startShowingLocation()
                }))
            }
        if (m_framework.tracking())
            {
            let length = m_framework.distance(toString: m_framework.trackLengthInMeters(), metricUnits: m_metric_units, abbreviate: true)!;
            let message = "Stop Tracking (track length = \(length))"
            alert.addAction(UIAlertAction(title: message, style: .default, handler:
                { _ in
                self.m_framework.endTracking()
                self.m_framework.deleteTrack()
                self.startOrStopUpdatingLocation()
                }))
            }
        else
            {
            alert.addAction(UIAlertAction(title: "Start Tracking", style: .default, handler:
                { _ in
                self.m_framework.startTracking()
                self.m_framework.displayTrack(true)
                self.startOrStopUpdatingLocation()
                }))
            }
        if (m_framework.getRotation() != 0)
            {
            alert.addAction(UIAlertAction(title: "North Up", style: .default, handler:
                { _ in
                let a = self.m_framework.getAnimateTransitions()
                self.m_framework.setAnimateTransitions(true)
                self.m_framework.setRotation(0)
                self.m_framework.setAnimateTransitions(a)
                }))
            }
        alert.addAction(UIAlertAction(title: self.m_search_bar.isHidden ? "Show Search Bar" : "Hide Search Bar", style: .default, handler:
            { _ in
            self.m_search_bar.isHidden = !self.m_search_bar.isHidden
            self.m_restore_search_bar = false
            }))
        alert.addAction(UIAlertAction(title: m_has_scale_bar ? "Hide Scale Bar" : "Show Scale Bar", style: .default, handler:
            { _ in
            self.m_has_scale_bar = !self.m_has_scale_bar
            self.m_framework.enableScaleBar(self.m_has_scale_bar)
            }))
        alert.addAction(UIAlertAction(title: m_metric_units ? "Non-Metric Units" : "Metric Units", style: .default, handler:
            { _ in
            self.m_metric_units = !self.m_metric_units
            self.m_framework.setLocale(self.m_metric_units ? "en_xx" : "en")
            self.createScaleBarAndTurnInstructions()
            }))
        alert.addAction(UIAlertAction(title: self.m_framework.getPerspective() ? "Perspective Off" : "Perspective On", style: .default, handler:
            { _ in
            self.m_framework.setPerspective(!self.m_framework.getPerspective())
            }))
        if (m_framework.getPerspective())
            {
            alert.addAction(UIAlertAction(title: self.m_framework.getDraw3DBuildings() ? "No 3D Buildings" : "3D Buildings", style: .default, handler:
                { _ in
                self.m_framework.setDraw3DBuildings(!self.m_framework.getDraw3DBuildings())
                }))
            }
        alert.addAction(UIAlertAction(title: self.m_framework.getNightMode() ? "Day Mode" : "Night Mode", style: .default, handler:
            { _ in
            self.m_framework.setNightMode(!self.m_framework.getNightMode())
            }))
        alert.addAction(UIAlertAction(title: "Cancel", style: .default, handler:
            { _ in
            }))

        present(alert, animated: true, completion: nil)
        }
        
    func startOrStopUpdatingLocation()
        {
        if (m_navigating || m_show_location || m_framework.tracking())
            {
            m_location_manager.startUpdatingLocation()
            UIApplication.shared.isIdleTimerDisabled = true  // stop screen going to sleep
            }
        else
            {
            m_location_manager.stopUpdatingLocation()
            UIApplication.shared.isIdleTimerDisabled = false
            }
        }
                
    func startShowingLocation()
        {
        if (m_framework.getRouteCount() == 0)
            {
            m_framework.setFollowMode(FollowModeLocation)
            m_framework.enableLayer("route-vector")
            m_show_location = true
            startOrStopUpdatingLocation()
            }
        }
        
    func stopShowingLocation()
        {
        m_framework.disableLayer("route-vector")
        m_show_location = false
        startOrStopUpdatingLocation()
        }
        
    func startNavigating()
        {
        if (m_framework.getRouteCount() != 0)
            {
            let alert = UIAlertController(title: "NAVIGATION IS FOR TESTING ONLY AND NOT INTENDED FOR ACTUAL ROUTE GUIDANCE", message: "Press OK to confirm.", preferredStyle: .alert)
            alert.addAction(UIAlertAction(title: "OK", style: .default, handler:
                { _ in
                self.m_navigate_button.title = "End"
                self.m_restore_search_bar = !self.m_search_bar.isHidden
                self.m_search_bar.isHidden = true
                self.m_framework.enableTurnInstructions(true)
                self.m_framework.setFollowMode(FollowModeLocationHeadingZoom)
                self.m_framework.enableLayer("route-vector")
                self.m_navigating = true
                self.m_show_location = false
                self.startOrStopUpdatingLocation()
                }))
            alert.addAction(UIAlertAction(title: "Cancel", style: .default, handler:
                { _ in
                }))
            present(alert, animated: true, completion: nil)
            }
        }
        
    func stopNavigating()
        {
        m_navigate_button.title = "Start"
        if (m_restore_search_bar)
            {
            m_search_bar.isHidden = false
            }
        m_restore_search_bar = false
        m_framework.enableTurnInstructions(false)
        m_framework.disableLayer("route-vector")
        m_navigating = false
        startOrStopUpdatingLocation()
        }

    @objc func onNavigateButton()
        {
        if (!m_navigating)
            {
            startNavigating()
            }
        else
            {
            stopNavigating()
            }
        }
        
    @objc func onHelpButton()
        {
        let v = CartoTypeFramework.version() + "." + CartoTypeFramework.build()
        let m = m_framework.mapMetaData(0)!
        let s = "This application demonstrates the CartoType mapping and routing library. See cartotype.com for more information about creating maps and using CartoType in your application.\n\nCreated using CartoType version \(v).\n\nMap created using CartoType \(m.cartoTypeVersionMajor).\(m.cartoTypeVersionMinor).\(m.cartoTypeBuild). Projection: \(m.projectionName!)"
        let alert = UIAlertController(title: "About CartoType Maps", message: s, preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "OK", style: .default, handler:
            { _ in
            }))
        present(alert, animated: true, completion: nil)
        }
    
    @objc func setRouteStart()
        {
        m_route_start_in_degrees = m_last_point_pressed_in_degrees
        calculateAndDisplayRoute()
        }
    
    @objc func setRouteEnd()
        {
        m_route_end_in_degrees = m_last_point_pressed_in_degrees
        calculateAndDisplayRoute()
        }
    
    @objc func insertPushPin()
        {
        let a = CartoTypeAddress()
        m_framework.getAddress(a, point: m_last_point_pressed_in_degrees, coordType: DegreeCoordType)
        let p : CartoTypeMapObjectParam = CartoTypeMapObjectParam.init(type: PointMapObjectType, andLayer: "pushpin", andCoordType: DegreeCoordType)
        p.appendX(m_last_point_pressed_in_degrees.x, andY: m_last_point_pressed_in_degrees.y)
        p.mapHandle = 0
        p.stringAttrib = a.toString(false)
        m_framework.insertMapObject(p)
        m_pushpin_id = p.objectId
        }
    
    @objc func deletePushPin()
        {
        m_framework.deleteObjects(fromMap: 0, fromID: m_pushpin_id, toID: m_pushpin_id, withCondition: nil)
        m_pushpin_id = 0
        }
        
    func searchBar(_ searchBar: UISearchBar, textDidChange aSearchText: String)
        {
        }

    func searchBarCancelButtonClicked(_ aSearchBar: UISearchBar)
        {
        view.endEditing(true)
        }

    func searchBarSearchButtonClicked(_ aSearchBar: UISearchBar)
        {
        view.endEditing(true)
        find(aSearchBar.text ?? "")
        }
    
    func locationManager(_ manager: CLLocationManager, didFailWithError error: Error)
        {
        let alert_controller = UIAlertController.init(title: "Error", message: "could not get your location", preferredStyle: UIAlertController.Style.alert)
        present(alert_controller, animated: true, completion: nil)
        }
    
    func locationManager(_ manager: CLLocationManager, didUpdateLocations aLocations: [CLLocation])
        {
        let new_location = aLocations.last
        if (new_location == nil)
            {
            return
            }
        m_location = new_location!
        var nav_data = CartoTypeNavigationData.init(validity: KTimeValid, time: m_location.timestamp.timeIntervalSinceReferenceDate, longitude: 0, latitude: 0, speed: 0, course: 0, height: 0)
        if (m_location.horizontalAccuracy > 0 && m_location.horizontalAccuracy <= 100)
            {
            nav_data.validity |= KPositionValid
            nav_data.latitude = m_location.coordinate.latitude
            nav_data.longitude = m_location.coordinate.longitude
            }
        if (m_location.course >= 0)
            {
            nav_data.validity |= KCourseValid
            nav_data.course = m_location.course
            }
        if (m_location.speed >= 0)
            {
            nav_data.validity |= KSpeedValid
            nav_data.speed = m_location.speed * 3.6 // convert from metres per second to kilometres per hour
            }
        if (m_location.verticalAccuracy >= 0 && m_location.verticalAccuracy <= 100)
            {
            nav_data.validity |= KHeightValid
            nav_data.height = m_location.altitude
            }
        m_framework.navigate(&nav_data)
        }
        
    }
