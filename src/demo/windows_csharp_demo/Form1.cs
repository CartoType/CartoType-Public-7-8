using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace CartoTypeDemo
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();

            // Uncomment the next line to add a status bar at the bottom of the display; however, it hides the copyright notice at present.
            // this.Controls.Add(m_status_bar); 

            string map_file = Application.StartupPath + "/../../../../../map/isle_of_wight.ctm1";
            string style_file = Application.StartupPath + "/../../../../../style/standard.ctstyle";
            string font_path = Application.StartupPath + "/../../../../../font/";
            if (!System.IO.File.Exists(map_file)) // if we're running in an ordinary source tree, not an SDK
            {
                map_file = Application.StartupPath + "/../../../../../../../map/isle_of_wight.ctm1";
                style_file = Application.StartupPath + "/../../../../../../../style/standard.ctstyle";
                font_path = Application.StartupPath + "/../../../../../../../font/";
            }
            m_framework = new CartoType.Framework(map_file,
                                                  style_file,
                                                  font_path + "DejaVuSans.ttf",
                                                  this.ClientSize.Width,
                                                  this.ClientSize.Height);
            m_framework.LoadFont(font_path + "DejaVuSans-Bold.ttf");
            m_framework.LoadFont(font_path + "DejaVuSerif.ttf");
            m_framework.LoadFont(font_path + "DejaVuSerif-Italic.ttf");
            m_framework.SetResolutionDpi(144);
            m_framework.SetFollowMode(CartoType.FollowMode.LocationHeading);
            m_framework.EnableLegend(m_draw_legend);
            Text = m_framework.DataSetName();

            m_map_renderer = new CartoType.MapRenderer(m_framework, Handle);
            m_graphics_acceleration = m_map_renderer.Valid();

            // Uncomment the next line to create a legend (map key).
            // CreateLegend();

            // Sample code to insert a green pushpin.
            // String s = CartoType.Util.SetAttribute("", "_color", "green");
            // long id = 0;
            // double lon = -1.289708;
            // double lat = 50.655351;
            // m_framework.InsertPointMapObject(0, "pushpin", lon, lat, CartoType.CoordType.Degree, s, 0,ref id, false);
        }

        private CartoType.Framework m_framework;
        private CartoType.MapRenderer m_map_renderer;
        private bool m_draw_legend = false;
        private bool m_graphics_acceleration = false;
        private bool m_map_drag_enabled;
        private int m_map_drag_offset_x;
        private int m_map_drag_offset_y;
        private int m_map_drag_anchor_x;
        private int m_map_drag_anchor_y;
        private Graphics m_map_drag_graphics;
        private CartoType.Point m_last_point = new CartoType.Point();
        private CartoType.Turn m_first_turn = new CartoType.Turn();
        private CartoType.Turn m_second_turn = new CartoType.Turn();
        private StatusBar m_status_bar = new StatusBar();

        private void Form1_Paint(object sender, PaintEventArgs e)
        {
            if (m_graphics_acceleration)
                m_map_renderer.Draw();

            if (!m_graphics_acceleration)
                Draw(e.Graphics);
        }

        private void Draw(Graphics aGraphics)
        {
            Text = m_framework.DataSetName() + " 1:" + (int)m_framework.ScaleDenominator();
            
            if (m_map_drag_enabled)
            {
                if (m_map_drag_offset_x > 0)
                    aGraphics.FillRectangle(Brushes.White, 0, 0, m_map_drag_offset_x, this.ClientRectangle.Height);
                else if (m_map_drag_offset_x < 0)
                    aGraphics.FillRectangle(Brushes.White, this.ClientRectangle.Width + m_map_drag_offset_x, 0, -m_map_drag_offset_x, this.ClientRectangle.Height);
                if (m_map_drag_offset_y > 0)
                    aGraphics.FillRectangle(Brushes.White, 0, 0, this.ClientRectangle.Width, m_map_drag_offset_y);
                else if (m_map_drag_offset_y < 0)
                    aGraphics.FillRectangle(Brushes.White, 0, this.ClientRectangle.Height + m_map_drag_offset_y, this.ClientRectangle.Width, -m_map_drag_offset_y);
            }

            aGraphics.DrawImageUnscaled(m_framework.MapBitmap(), m_map_drag_offset_x, m_map_drag_offset_y);
        }

        protected override void OnPaintBackground(PaintEventArgs e)
        {
            // do nothing: the whole window is drawn by Form1_Paint
        }

        private void Form1_KeyPress(object sender, KeyPressEventArgs e)
        {
            switch (e.KeyChar)
            {
                // Press 'i' to zoom in.
                case 'i':
                    m_framework.Zoom(2);
                    Invalidate();
                    break;

                // Press 'o' to zoom out.
                case 'o':
                    m_framework.Zoom(0.5);
                    Invalidate();
                    break;

                // Press 'r' to rotate right.
                case 'r':
                    m_framework.Rotate(10);
                    Invalidate();
                    break;

                // Press 'l' to rotate left.
                case 'l':
                    m_framework.Rotate(-10);
                    Invalidate();
                    break;

                // Press 'p' to toggle perspective mode.
                case 'p':
                    m_framework.SetPerspective(!m_framework.Perspective());
                    Invalidate();
                    break;
            }
        }

        protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
        {
            switch (keyData)
            {
                case Keys.Left:
                    m_framework.Pan(-50,0);
                    Invalidate();
                    return true;
                case Keys.Right:
                    m_framework.Pan(50,0);
                    Invalidate();
                    return true;
                case Keys.Up:
                    m_framework.Pan(0,-50);
                    Invalidate();
                    return true;
                case Keys.Down:
                    m_framework.Pan(0,50);
                    Invalidate();
                    return true;
            }
            return false;
        }

        private void Form1_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                m_map_drag_enabled = true;
                m_map_drag_anchor_x = e.X;
                m_map_drag_anchor_y = e.Y;

                if (!m_graphics_acceleration)
                    m_map_drag_graphics = CreateGraphics();
            }
        }

        private void Navigate(int aValidity,double aTime,double aLong,double aLat,double aSpeed,double aBearing, double aHeight)
        {
            m_framework.Navigate(aValidity,aTime,aLong,aLat,aSpeed,aBearing,aHeight);
            m_framework.GetFirstTurn(m_first_turn);
            m_framework.GetSecondTurn(m_second_turn);
            String message = "";

            switch (m_framework.GetNavigationState())
            {
                case CartoType.NavigationState.None:
                case CartoType.NavigationState.NoPosition:
                    break;

                case CartoType.NavigationState.Turn:
                    message = m_first_turn.TurnCommand() + " after " + (int)m_first_turn.m_distance + "m";
 
                    if (!m_second_turn.m_continue)
                        message += " then " + m_second_turn.TurnCommand() + " after " + (int)m_second_turn.m_distance + "m";
                    break;

                case CartoType.NavigationState.OffRoute:
                    message = "off route";
                    break;

                case CartoType.NavigationState.ReRouteNeeded:
                    message = "calculating a new route";
                    break;

                case CartoType.NavigationState.ReRouteDone:
                    message = "new route calculated";
                    break;

                case CartoType.NavigationState.TurnRound:
                    message = "turn round at the next safe and legal opportunity";
                    break;

                case CartoType.NavigationState.Arrival:
                    message = "arriving after " + (int)m_first_turn.m_distance + "m";
                    break;
            }

            m_status_bar.Text = message;
        }

        private void Form1_MouseUp(object sender, MouseEventArgs e)
        {

            if (e.Button == MouseButtons.Left)
            {
                if (m_graphics_acceleration)
                {
                    m_map_drag_enabled = false;
                    m_map_drag_offset_x = 0;
                    m_map_drag_offset_y = 0;
                }
                else
                {
                    m_map_drag_enabled = false;
                    m_map_drag_offset_x = e.X - m_map_drag_anchor_x;
                    m_map_drag_offset_y = e.Y - m_map_drag_anchor_y;
                    Draw(m_map_drag_graphics);
                    if (m_map_drag_graphics != null)
                        m_map_drag_graphics.Dispose();
                    m_map_drag_graphics = null;
                    m_framework.Pan(-m_map_drag_offset_x, -m_map_drag_offset_y);
                    // Simulate a navigation position if there was no drag.
                    //if (m_map_drag_offset_x == 0 && m_map_drag_offset_y == 0)
                    //{
                    //    double time = (double)DateTime.Now.Ticks / 10000000.0;
                    //    int validity = (int)CartoType.ValidityFlag.Position | (int)CartoType.ValidityFlag.Time;
                    //    double[] coord = { e.X, e.Y };
                    //    m_framework.ConvertCoords(coord,CartoType.CoordType.Screen,CartoType.CoordType.Degree);
                    //    Navigate(validity, time, coord[0], coord[1], 0, 0, 0);
                    //}

                    m_map_drag_offset_x = 0;
                    m_map_drag_offset_y = 0;
                    Invalidate();
                }

            }
            
            // Right-click calculates a route between the last point and this point.
            else if (e.Button == MouseButtons.Right)
            {
                var p = new CartoType.Point();
                p.X = e.X;
                p.Y = e.Y;
                
                m_framework.ConvertPoint(p,CartoType.CoordType.Screen,CartoType.CoordType.Degree);
                if (m_last_point.X != 0 && m_last_point.Y != 0)
                {
                    // Delete the previous route, which consists of object IDs 0, 1, and 2.
                    m_framework.StartNavigation(m_last_point.X, m_last_point.Y, CartoType.CoordType.Degree,p.X,p.Y,CartoType.CoordType.Degree);
                    
                    Invalidate();
                }
                m_last_point.X = p.X;
                m_last_point.Y = p.Y;
            }
        }

        private void Form1_MouseMove(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                m_map_drag_offset_x = e.X - m_map_drag_anchor_x;
                m_map_drag_offset_y = e.Y - m_map_drag_anchor_y;

                if (m_graphics_acceleration)
                {
                    m_framework.Pan(-m_map_drag_offset_x, -m_map_drag_offset_y);
                    m_map_drag_offset_x = 0;
                    m_map_drag_offset_y = 0;
                    m_map_drag_anchor_x = e.X;
                    m_map_drag_anchor_y = e.Y;
                    Invalidate();
                }
                else
                    Draw(m_map_drag_graphics);
            }
        }

        private void Form1_MouseWheel(object sender, MouseEventArgs e)
        {
            int zoom_count = e.Delta / 120;
            double zoom = Math.Sqrt(2);
            if (zoom_count == 0)
                zoom_count = e.Delta >= 0 ? 1 : -1;
            zoom = Math.Pow(zoom,zoom_count);
            if (ClientRectangle.Contains(e.Location))
                m_framework.ZoomAt(zoom, e.X, e.Y, CartoType.CoordType.Screen);
            else
                m_framework.Zoom(zoom);
            Invalidate();
        }

        private void Form1_ClientSizeChanged(object sender, EventArgs e)
        {
            if (!this.ClientSize.IsEmpty && m_framework != null)
            {
                m_framework.Resize(this.ClientSize.Width, this.ClientSize.Height);
                Invalidate();
            }
        }

        private void CreateLegend()
        {
            CartoType.Legend legend = new CartoType.Legend(m_framework);
            CartoType.Color text_color = new CartoType.Color(90,90,90);
            legend.SetTextColor(text_color);
            legend.SetAlignment(CartoType.Align.Center);
            String dataset_name = m_framework.DataSetName();
            dataset_name = m_framework.SetCase(dataset_name, CartoType.LetterCase.Title);
            legend.SetFontSize(10, "pt");
            legend.AddTextLine(dataset_name);

            legend.SetAlignment(CartoType.Align.Right);
            legend.SetFontSize(6, "pt");

            String s = "";
            CartoType.Util.SetAttribute(s, "ref", "M4");
            legend.AddMapObjectLine(CartoType.MapObjectType.Line, "road/major", null, (int)CartoType.RoadType.Motorway, s, "motorway");
            CartoType.Util.SetAttribute(s, "ref", "A40");
            legend.AddMapObjectLine(CartoType.MapObjectType.Line, "road/major", null, (int)CartoType.RoadType.TrunkRoad, s, "trunk road");
            CartoType.Util.SetAttribute(s, "ref", "A414");
            legend.AddMapObjectLine(CartoType.MapObjectType.Line, "road/major", null, (int)CartoType.RoadType.PrimaryRoad, s, "primary road");
            CartoType.Util.SetAttribute(s, "ref", "B4009");
            legend.AddMapObjectLine(CartoType.MapObjectType.Line, "road/mid", null, (int)CartoType.RoadType.SecondaryRoad, s, "secondary road");
            legend.AddMapObjectLine(CartoType.MapObjectType.Line, "road/mid", null, (int)CartoType.RoadType.TertiaryRoad, "High Street", "tertiary road");
            legend.AddMapObjectLine(CartoType.MapObjectType.Line, "path", "cyc", 0, "", "cycleway");
            legend.AddMapObjectLine(CartoType.MapObjectType.Line, "path", "bri", 0, "", "bridle path");
            legend.AddMapObjectLine(CartoType.MapObjectType.Line, "path", "foo", 0, "", "footpath");
            legend.AddMapObjectLine(CartoType.MapObjectType.Polygon, "land/major", "for", 0, "Ashridge", "forest or wood");
            legend.AddMapObjectLine(CartoType.MapObjectType.Polygon, "land/minor", "par", 0, "Green Park", "park, golf course or common");
            legend.AddMapObjectLine(CartoType.MapObjectType.Polygon, "land/minor", "gra", 0, "", "grassland");
            legend.AddMapObjectLine(CartoType.MapObjectType.Polygon, "land/minor", "orc", 0, "", "orchard, vineyard, etc.");
            legend.AddMapObjectLine(CartoType.MapObjectType.Polygon, "land/minor", "cmr", 0, "", "commercial or industrial");
            legend.AddMapObjectLine(CartoType.MapObjectType.Polygon, "land/minor", "cns", 0, "", "construction, quarry, landfill, etc.");
            legend.AddMapObjectLine(CartoType.MapObjectType.Point, "amenity/minor", "stn", 0, "Berkhamsted", "station");

            // Uncomment these lines to create a scale bar as part of the legend.
            // legend.SetAlignment(CartoType.Align.Center);
            // legend.AddScaleLine(true);

            CartoType.Color border_color = CartoType.Color.KGray;
            legend.SetBorder(border_color, 1, 4, "pt");
            CartoType.Color background_color = new CartoType.Color(255,255,255,224);
            legend.SetBackgroundColor(background_color);
            m_framework.SetLegend(legend, 1, "in", CartoType.NoticePosition.TopRight);
        }

    }
}
