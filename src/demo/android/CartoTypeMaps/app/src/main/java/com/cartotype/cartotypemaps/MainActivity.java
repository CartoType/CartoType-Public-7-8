package com.cartotype.cartotypemaps;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.arch.core.util.Function;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.location.*;
import android.os.Bundle;
import android.os.Environment;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Toast;

import com.cartotype.*;

import java.io.File;
import java.io.FileOutputStream;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class MainActivity extends AppCompatActivity implements LocationListener
    {
    @Override
    protected void onCreate(Bundle savedInstanceState)
        {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        File map_file, font_file, style_file;

        try
            {
            map_file = getCopyOfFile("isle_of_wight.ctm1", "isle_of_wight.ctm1");
            font_file = getCopyOfFile("DejaVuSans.ttf", "DejaVuSans.ttf");
            style_file = getCopyOfFile("standard.ctstyle", "standard.ctstyle");
            }
        catch (Exception e)
            {
            Log.d("CartoType","failed to open map, font or style sheet file");
            return;
            }

        m_style_filename = style_file.getAbsolutePath();
        m_font_filename = font_file.getAbsolutePath();
        createMap(map_file.getAbsolutePath());

        m_location_manager = (LocationManager)getSystemService(LOCATION_SERVICE);
        }

    @Override
    public void onLocationChanged(@NonNull Location loc)
        {
        if (!hasRoute() && !m_tracking)
            m_view.setRouteStart(loc.getLongitude(), loc.getLatitude());
        m_framework.navigate(Framework.TIME_VALID | Framework.POSITION_VALID,
                        loc.getElapsedRealtimeNanos() / 1000000000.0, loc.getLongitude(), loc.getLatitude(),0,0,0);
        if (!hasRoute() && !m_tracking)
            stopNavigating();
        }

    @Override
    public void onStatusChanged(String provider, int status, Bundle extras)
        {
        }

    @Override
    public void onProviderEnabled(String provider)
        {
        }

    @Override
    public void onProviderDisabled(String provider)
        {
        }

    @Override
    public boolean onCreateOptionsMenu(Menu aMenu)
        {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main,aMenu);
        return true;
        }

    @Override
    public boolean onPrepareOptionsMenu(Menu aMenu)
        {
        if (aMenu != null)
            {
            aMenu.findItem(R.id.find_ignore_symbols).setChecked(m_ignore_symbols);
            aMenu.findItem(R.id.find_fuzzy).setChecked(m_fuzzy);

            boolean has_route = hasRoute();
            MenuItem track = aMenu.findItem(R.id.view_track_location);
            track.setEnabled(!has_route);
            track.setChecked(m_tracking);

            aMenu.findItem(R.id.view_scale).setChecked(m_has_scale_bar);
            aMenu.findItem(R.id.view_north_up).setEnabled(m_framework != null && m_framework.getRotation() != 0);
            aMenu.findItem(R.id.view_perspective).setChecked(m_framework != null && m_framework.getPerspective());
            aMenu.findItem(R.id.view_metric).setChecked(m_metric_units);
            aMenu.findItem(R.id.view_3d_buildings).setChecked(m_framework != null && m_framework.getDraw3DBuildings());
            aMenu.findItem(R.id.view_night_mode).setChecked(m_framework != null && m_framework.getNightMode());

            MenuItem nav = aMenu.findItem(R.id.route_navigate);
            nav.setEnabled(has_route);
            nav.setChecked(has_route && m_navigating);

            aMenu.findItem(R.id.route_from_here).setEnabled(m_view.hasDestination());
            aMenu.findItem(R.id.route_reverse).setEnabled(has_route);
            aMenu.findItem(R.id.route_delete).setEnabled(has_route);
            }
        return super.onPrepareOptionsMenu(aMenu);
        }

    @Override
    public boolean onOptionsItemSelected(MenuItem aItem)
        {
        int id = aItem.getItemId();
        switch (id)
            {
            case R.id.file_open_builtin:
                chooseFile(getDir("localAssets",Context.MODE_PRIVATE).getPath(),false);
                return true;
            case R.id.file_open_app:
                chooseFile(getExternalFilesDir(null).getPath(),true);
                return true;
            case R.id.file_open_doc:
                chooseFile(Environment.getExternalStorageDirectory().getPath() + "/Documents/CartoType/maps",true);
                return true;

            case R.id.find_find:
                findPlace();
                return true;
            case R.id.find_address:
                findAddress();
                return true;
            case R.id.find_ignore_symbols:
                m_ignore_symbols = !aItem.isChecked();
                return true;
            case R.id.find_fuzzy:
                m_fuzzy = !aItem.isChecked();
                return true;

            case R.id.view_track_location:
                if (m_navigating)
                    stopNavigating();
                else
                    {
                    m_framework.setFollowMode(Framework.FOLLOW_MODE_LOCATION);
                    m_tracking = true;
                    startNavigating();
                    }
                return true;

            case R.id.view_scale:
                setScaleBar(!aItem.isChecked());
                return true;
            case R.id.view_north_up:
                m_framework.setAnimateTransitions(true);
                m_framework.setRotation(0);
                m_framework.setAnimateTransitions(false);
                return true;
            case R.id.view_perspective:
                m_framework.setPerspective(!aItem.isChecked());
                return true;
            case R.id.view_metric:
                setMetricUnits(!aItem.isChecked());
                return true;
            case R.id.view_3d_buildings:
                m_framework.setDraw3DBuildings(!aItem.isChecked());
                return true;
            case R.id.view_night_mode:
                m_framework.setNightMode(!aItem.isChecked());
                return true;

            case R.id.route_navigate:
                if (m_navigating)
                    stopNavigating();
                else
                    {
                    m_framework.setFollowMode(Framework.FOLLOW_MODE_LOCATION_HEADING_ZOOM);
                    startNavigating();
                    }
                return true;
            case R.id.route_from_here:
                stopNavigating();
                m_framework.setFollowMode(Framework.FOLLOW_MODE_LOCATION_HEADING_ZOOM);
                startNavigating();
                return true;
            case R.id.route_reverse:
                m_view.reverseRoute();
                stopNavigating();
                return true;
            case R.id.route_delete:
                m_view.deleteRoute();
                stopNavigating();
                return true;
            case R.id.route_drive:
                aItem.setChecked(true);
                m_view.setRouteProfileType(RouteProfileType.Car);
                return true;
            case R.id.route_cycle:
                aItem.setChecked(true);
                m_view.setRouteProfileType(RouteProfileType.Cycle);
                return true;
            case R.id.route_walk:
                aItem.setChecked(true);
                m_view.setRouteProfileType(RouteProfileType.Walk);
                return true;
            case R.id.route_hike:
                aItem.setChecked(true);
                m_view.setRouteProfileType(RouteProfileType.Hike);
                return true;

            case R.id.help_about:
                showAboutMessage();
                return true;
            }
        return super.onOptionsItemSelected(aItem);
        }

    @Override
    public void onConfigurationChanged(Configuration aNewConfig)
        {
        super.onConfigurationChanged(aNewConfig);
        createScaleBarAndTurnInstructions();
        }

    private void showAboutMessage()
        {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("About CartoType Maps");
        String message = "This application demonstrates the CartoType mapping and routing library. See cartotype.com for more information about creating maps and using CartoType in your application.";
        message += " \n\nCreated using CartoType " + Util.version() + "." + Util.build() + ".";
        builder.setMessage(message);
        builder.setPositiveButton("OK", new DialogInterface.OnClickListener()
            {
            @Override
            public void onClick(DialogInterface dialogInterface, int i)
                {
                }
            });
        builder.show();
        }

    private void warn(String aTitle,String aText)
        {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(aTitle);
        builder.setMessage(aText);
        builder.setPositiveButton("OK", new DialogInterface.OnClickListener()
            {
            @Override
            public void onClick(DialogInterface dialogInterface, int i)
                {
                }
            });
        builder.show();
        }

    private void confirm(String aTitle, String aText, final Function<Void,Void> aFunction)
        {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(aTitle);
        builder.setMessage(aText);
        AlertDialog dialog = builder.create();
        builder.setPositiveButton("OK", new DialogInterface.OnClickListener()
            {
            @Override
            public void onClick(DialogInterface dialogInterface, int i)
                {
                aFunction.apply(null);
                }
            });
        builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener()
            {
            @Override
            public void onClick(DialogInterface dialogInterface, int i)
                {
                }
            });
        builder.show();
        }

    private void createMap(String aMapFilename)
        {
        m_view = null;
        m_framework = null;

        m_map_filename = aMapFilename;
        try
            {
            m_framework = new Framework(this, m_map_filename, m_style_filename, m_font_filename, 256, 256);
            }
        catch (RuntimeException e)
            {
            Log.d("CartoType",e.getMessage());
            finishAffinity();
            }

        // License the framework. Replace 'mylicensekey' with a valid license key if you are a licensee.
        m_framework.license("mylicensekey");

        // Start loading navigation data in a background thread.
        m_framework.loadNavigationData();

        // Load extra fonts.
        try
            {
            String font1 = getCopyOfFile("DejaVuSans-Bold.ttf", "DejaVuSans-Bold.ttf").getAbsolutePath();
            m_framework.loadFont(font1);
            String font2 = getCopyOfFile("DejaVuSerif.ttf", "DejaVuSerif.ttf").getAbsolutePath();
            m_framework.loadFont(font2);
            String font3 = getCopyOfFile("DejaVuSerif-Italic.ttf", "DejaVuSerif-Italic.ttf").getAbsolutePath();
            m_framework.loadFont(font3);
            }
        catch (Exception e)
            {
            Log.d("CartoType","failed to open font file");
            }

        // Set the vehicle position to a quarter of the way up the display.
        m_framework.setVehiclePosOffset(0,0.25);

        m_view = new MainView(this,m_framework);
        setContentView(m_view);
        // Make it impossible to pan outside the map or zoom in closer than the scale 1:1000.
        m_framework.setViewLimits(1000,0,null);
        createScaleBarAndTurnInstructions();
        }

    void chooseFile(final String aDirectory,boolean aPermissionNeeded)
        {
        if (aDirectory == null)
            {
            Toast.makeText(this, "no directory found",Toast.LENGTH_LONG).show();
            return;
            }

        if (aPermissionNeeded)
            {
            int permission = ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE);
            if (permission != PackageManager.PERMISSION_GRANTED)
                {
                warn("Storage Permission Needed", "To read a file from external storage, add the Storage permission to this app.");
                return;
                }
            }

        File file = new File(aDirectory);
        if (!file.isDirectory())
            {
            Toast.makeText(this, "'" + aDirectory + "' is not a directory",Toast.LENGTH_LONG).show();
            return;
            }
        final String[] file_array = file.list(new FilenameFilter()
            {
            @Override
            public boolean accept(File dir, String name)
                {
                return name.toLowerCase().endsWith(".ctm1");
                }
            });
        if (file_array == null || file_array.length == 0)
            {
            Toast.makeText(this, "'" + aDirectory + "' contains no CTM1 files",Toast.LENGTH_LONG).show();
            return;
            }
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Open a Map");
        builder.setItems(file_array, new DialogInterface.OnClickListener()
            {
            @Override
            public void onClick(DialogInterface aDialog,int aWhich)
                {
                createMap(aDirectory + "/" + file_array[aWhich]);
                }
            });
        builder.show();
        }

    void chooseMapObject(final MapObject[] aMapObjectArray,String aDialogTitle)
        {
        m_found_item = null;
        if (aMapObjectArray != null && aMapObjectArray.length > 0)
            {
            AlertDialog.Builder choose_place_builder = new AlertDialog.Builder(this);
            choose_place_builder.setTitle(aDialogTitle);
            final String[] item_array = new String[aMapObjectArray.length];
            for (int i = 0; i < aMapObjectArray.length; i++)
                item_array[i] = aMapObjectArray[i].getGeoCodeSummary();
            choose_place_builder.setItems(item_array,new DialogInterface.OnClickListener()
                {
                @Override
                public void onClick(DialogInterface aDialog,int aWhich)
                    {
                    m_found_item = aMapObjectArray[aWhich];
                    if (m_found_item != null)
                        {
                        m_framework.insertCopyOfMapObject(0,"found",m_found_item,20,CoordType.MapMeter,m_found_item_id,true);
                        m_found_item_id = m_framework.getLastObjectId();
                        m_framework.setView(m_found_item, 16, 1000);
                        }
                    }
                });
            choose_place_builder.show();
            }
        }

    void choosePlace()
        {
        if (m_place_to_find != null)
            {
            int match_method = Framework.FOLD_CASE_STRING_MATCH_METHOD | Framework.IGNORE_WHITESPACE_STRING_MATCH_FLAG | Framework.FOLD_ACCENTS_STRING_MATCH_FLAG;
            if (m_ignore_symbols)
                match_method |= Framework.IGNORE_SYMBOLS_STRING_MATCH_FLAG;
            if (m_fuzzy)
                match_method |= Framework.FUZZY_STRING_MATCH_FLAG;
            final MapObject[] map_object_array = m_framework.find(16,m_place_to_find,match_method);
            if (map_object_array == null || map_object_array.length == 0)
                Toast.makeText(this, "'" + m_place_to_find + "' not found",Toast.LENGTH_SHORT).show();
            else
                chooseMapObject(map_object_array,"Choose a place");
            }
        }

    void findPlace()
        {
        AlertDialog.Builder find_place_builder = new AlertDialog.Builder(this);
        find_place_builder.setTitle("Find a place");
        final EditText input = new EditText(this);
        if (m_place_to_find != null)
            {
            input.setText(m_place_to_find);
            input.setSelection(0,m_place_to_find.length());
            }
        find_place_builder.setView(input);
        find_place_builder.setPositiveButton("OK",new DialogInterface.OnClickListener()
            {
            @Override
            public void onClick(DialogInterface aDialog,int aWhich)
                {
                m_place_to_find = input.getText().toString();
                choosePlace();
                }
            });
        find_place_builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener()
            {
            @Override
            public void onClick(DialogInterface aDialog,int aWhich)
                {
                }
            });

        find_place_builder.show();
        }

    void chooseAddress(com.cartotype.Address aAddress)
        {
        MapObject[] address_array = m_framework.findAddress(16,aAddress,m_fuzzy);
        if (address_array == null || address_array.length == 0)
            Toast.makeText(this, "address not found",Toast.LENGTH_SHORT).show();
        else
            chooseMapObject(address_array,"Choose an address");
        }

    void findAddress()
        {
        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);

        final EditText building = new EditText(this);
        building.setHint("building name or number");
        layout.addView(building);

        final EditText street = new EditText(this);
        street.setHint("street");
        layout.addView(street);

        final EditText locality = new EditText(this);
        locality.setHint("locality");
        layout.addView(locality);

        AlertDialog.Builder find_address_builder = new AlertDialog.Builder(this);
        find_address_builder.setTitle("Find an address");
        find_address_builder.setView(layout);

        final com.cartotype.Address address = new com.cartotype.Address();

        find_address_builder.setPositiveButton("OK",new DialogInterface.OnClickListener()
            {
            @Override
            public void onClick(DialogInterface aDialog,int aWhich)
                {
                address.iBuilding = building.getText().toString();
                address.iStreet = street.getText().toString();
                address.iLocality = locality.getText().toString();
                chooseAddress(address);
                }
            });

        find_address_builder.setNegativeButton("Cancel",new DialogInterface.OnClickListener()
            {
            @Override
            public void onClick(DialogInterface aDialog,int aWhich)
                {
                }
            });

        find_address_builder.show();
        }

    private File getCopyOfFile(String aAssetPath, String aAssetFile) throws IOException
        {
        File dir = getDir("localAssets",Context.MODE_PRIVATE);
        File f = new File(dir,aAssetFile);

        // Copy the file if it doesn't exist.
        if (!f.exists())
            {
            InputStream is = getAssets().open(aAssetPath);
            OutputStream os = new FileOutputStream(f);
            byte[] buffer = new byte[102400];
            for (; ; )
                {
                int length = is.read(buffer);
                if (length <= 0)
                    break;
                os.write(buffer, 0, length);
                }
            os.flush();
            os.close();
            is.close();
            }

        return f;
        }

    boolean hasRoute()
        {
        return m_framework != null && m_framework.getRouteCount() != 0;
        }

    void startNavigating()
        {
        final int permission = ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION);
        if (permission != PackageManager.PERMISSION_GRANTED)
            {
            warn("Location Permission Needed","To enable navigation, add the Location permission to this app.");
            return;
            }

        confirm("NAVIGATION IS FOR TESTING ONLY AND NOT INTENDED FOR ACTUAL ROUTE GUIDANCE", "Press OK to confirm.", new Function<Void,Void>()
            {
            @Override
            public Void apply(Void input) { startNavigatingHelper(); return null; }
            });
        }

    void startNavigatingHelper()
        {
        @SuppressLint("MissingPermission")
        final int permission = ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION);
        if (permission != PackageManager.PERMISSION_GRANTED)
            {
            warn("Location Permission Needed", "To enable navigation, add the Location permission to this app.");
            return;
            }
        m_location_manager.requestLocationUpdates(LocationManager.GPS_PROVIDER, 1000, 5.0f, this);
        m_framework.enableTurnInstructions(true);
        m_framework.enableLayer("route-vector",true);
        m_navigating = true;
        }

    void stopNavigating()
        {
        m_navigating = false;
        m_tracking = false;
        m_location_manager.removeUpdates(this);
        m_framework.enableTurnInstructions(false);
        m_framework.enableLayer("route-vector",false);
        }

    void setMetricUnits(boolean aEnable)
        {
        if (aEnable != m_metric_units)
            {
            m_metric_units = aEnable;
            m_framework.setLocale(m_metric_units ? "en_xx" : "en");
            createScaleBarAndTurnInstructions();
            m_framework.enableScaleBar(m_has_scale_bar);
            }
        }

    void createScaleBarAndTurnInstructions()
        {
        DisplayMetrics metrics = Resources.getSystem().getDisplayMetrics();
        double width_in_inches = (double)metrics.widthPixels / (double)metrics.xdpi;
        width_in_inches *= 0.8;
        if (width_in_inches > 3.0)
            width_in_inches = 3.0;
        m_framework.setScaleBar(m_metric_units,width_in_inches,"in",NoticePosition.BottomLeft);
        m_framework.setTurnInstructions(m_metric_units,false, width_in_inches, "in", NoticePosition.TopLeft, 14, "pt");
        }

    void setScaleBar(boolean aEnable)
        {
        if (aEnable != m_has_scale_bar)
            {
            m_has_scale_bar = aEnable;
            m_framework.enableScaleBar(aEnable);
            }
        }

    private Framework m_framework;
    private String m_map_filename;
    private String m_font_filename;
    private String m_style_filename;
    private MainView m_view;
    private boolean m_has_scale_bar = true;
    private boolean m_metric_units = true;
    private String m_place_to_find;
    private MapObject m_found_item;
    private long m_found_item_id;
    private boolean m_ignore_symbols = true;
    private boolean m_fuzzy = false;
    private LocationManager m_location_manager;
    private boolean m_navigating = false;
    private boolean m_tracking = false;
    }
