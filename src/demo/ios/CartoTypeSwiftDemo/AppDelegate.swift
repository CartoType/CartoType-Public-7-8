//
//  AppDelegate.swift
//  CartoTypeSwiftDemo
//
//  Copyright © 2016-2021 CartoType Ltd. All rights reserved.
//

import UIKit

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate
    {
    var window: UIWindow?
    var m_framework : CartoTypeFramework!
    var m_view_controller : ViewController!

    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?) -> Bool
        {
        // Create the CartoType framework
        let bounds = UIScreen.main.bounds
        self.window = UIWindow.init(frame: bounds)
        let scale = UIScreen.main.scale
        let width = bounds.width * scale
        let height = bounds.height * scale
        let param = CartoTypeFrameworkParam()!
        param.mapFileName = "santa-cruz"
        param.styleSheetFileName = "standard"
        param.fontFileName = "DejaVuSans"
        param.viewWidth = Int32(width)
        param.viewHeight = Int32(height)
        m_framework = CartoTypeFramework.init(param: param)!
        m_framework.license("mylicensekey")
        m_framework.loadFont("DejaVuSerif")
        m_framework.loadFont("DejaVuSans-Bold")
        m_framework.loadFont("DejaVuSerif-Italic")
        
        m_framework.setViewLimitsToMinScale(1000, maxScale: 0, panArea: nil)
                
        // Create the view controller.
        m_view_controller = ViewController.init(aFrameWork: m_framework, aBounds:bounds)
        self.window?.rootViewController = m_view_controller
        self.window?.backgroundColor = UIColor.white
        self.window?.makeKeyAndVisible()
        
        loadSettings()

        return true
        }

    func applicationWillResignActive(_ application: UIApplication)
        {
        // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
        // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
        }
    
    func loadSettings()
        {
        let defaults = UserDefaults.standard
        let view_state_string = defaults.string(forKey: "viewState")
        if (view_state_string != nil)
            {
            let view_state = CartoTypeViewState()
            view_state.read(fromXml: view_state_string)
            
            // Use the current width and height, not the saved values, in case the orientation was changed.
            let current_view_state = m_framework.getViewState()!
            view_state.widthInPixels = current_view_state.widthInPixels
            view_state.heightInPixels = current_view_state.heightInPixels
            
            m_framework.setView(view_state)
            }
        let route_string = defaults.string(forKey: "route")
        if (route_string != nil && !route_string!.isEmpty)
            {
            m_framework.readRoute(fromXml: route_string, replace: true)
            m_view_controller.updateRouteStatus()
            }
        let saved_map_objects = defaults.data(forKey: "savedMapObjects")
        if (saved_map_objects != nil)
            {
            m_framework.readMap(0, data:saved_map_objects)
            }
        }
    
    func saveSettings()
        {
        let view_state = m_framework.getViewState()
        let view_state_string = view_state?.toXml()
        let defaults = UserDefaults.standard
        defaults.set(view_state_string, forKey: "viewState")
        var route_string = ""
        if (m_framework.getRouteCount() != 0)
            {
            route_string = m_framework.writeRoute(asXmlString: m_framework.getRoute(), fileType: CartoTypeRouteFileType)
            }
        defaults.set(route_string,forKey: "route")
        let find_param = CartoTypeFindParam()!
        find_param.layers = "pushpin";
        let saved_map_objects = m_framework.saveMap(0, param: find_param)!
        if (saved_map_objects.endIndex > 1)
            {
            defaults.set(saved_map_objects,forKey: "savedMapObjects")
            }
        }

    func applicationDidEnterBackground(_ application: UIApplication)
        {
        // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
        // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
        saveSettings()
        }

    func applicationWillEnterForeground(_ application: UIApplication)
        {
        // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
        loadSettings()
        }

    func applicationDidBecomeActive(_ application: UIApplication)
        {
        // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
        }

    func applicationWillTerminate(_ application: UIApplication)
        {
        // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
        }

    }

