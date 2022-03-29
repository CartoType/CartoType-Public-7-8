Imports System.IO

Partial Public Class Form1
    Inherits Form

    Public Sub New()
        InitializeComponent()

        Dim map_file As String = Application.StartupPath & "/../../../../map/isle_of_wight.ctm1"
        Dim style_file As String = Application.StartupPath & "/../../../../style/standard.ctstyle"
        Dim font_path As String = Application.StartupPath & "/../../../../font/"

        If Not File.Exists(map_file) Then ' if we're running in an ordinary source tree, not an SDK
            map_file = Application.StartupPath & "/../../../../../../map/isle_of_wight.ctm1"
            style_file = Application.StartupPath & "/../../../../../../style/standard.ctstyle"
            font_path = Application.StartupPath & "/../../../../../../font/"
        End If

        m_framework = New CartoType.Framework(map_file, style_file, font_path & "DejaVuSans.ttf", ClientSize.Width, ClientSize.Height)
        m_framework.LoadFont(font_path & "DejaVuSans-Bold.ttf")
        m_framework.LoadFont(font_path & "DejaVuSerif.ttf")
        m_framework.LoadFont(font_path & "DejaVuSerif-Italic.ttf")
        m_framework.SetResolutionDpi(144)
        m_framework.SetFollowMode(CartoType.FollowMode.LocationHeading)
        m_framework.EnableLegend(m_draw_legend)
        Text = m_framework.DataSetName()
        m_map_renderer = New CartoType.MapRenderer(m_framework, Handle)
        m_graphics_acceleration = m_map_renderer.Valid()

        ' Uncomment the next line to create and display a legend (map key).
        ' CreateLegend()

        ' Sample code to insert a green pushpin.
        ' Dim s As String = CartoType.Util.SetAttribute("", "_color", "green")
        ' Dim id As Long = 0
        ' Dim lon As Double = -1.289708
        ' Dim lat As Double = 50.655351
        ' m_framework.InsertPointMapObject(0, "pushpin", lon, lat, CartoType.CoordType.Degree, s, 0, id, False)

    End Sub

    Private m_framework As CartoType.Framework
    Private m_map_renderer As CartoType.MapRenderer
    Private m_draw_legend As Boolean = False
    Private m_graphics_acceleration As Boolean = False
    Private m_map_drag_enabled As Boolean
    Private m_map_drag_offset_x As Integer
    Private m_map_drag_offset_y As Integer
    Private m_map_drag_anchor_x As Integer
    Private m_map_drag_anchor_y As Integer
    Private m_map_drag_graphics As Graphics
    Private m_last_point As CartoType.Point = New CartoType.Point()
    Private m_first_turn As CartoType.Turn = New CartoType.Turn()
    Private m_second_turn As CartoType.Turn = New CartoType.Turn()
    Private m_status_bar As StatusBar = New StatusBar()

    Private Sub Form1_Paint(ByVal sender As Object, ByVal e As PaintEventArgs)
        If m_graphics_acceleration Then m_map_renderer.Draw()
        If Not m_graphics_acceleration Then Draw(e.Graphics)
    End Sub

    Private Sub Draw(ByVal aGraphics As Graphics)
        Text = m_framework.DataSetName() & " 1:" + CInt(m_framework.ScaleDenominator())

        If m_map_drag_enabled Then

            If m_map_drag_offset_x > 0 Then
                aGraphics.FillRectangle(Brushes.White, 0, 0, m_map_drag_offset_x, ClientRectangle.Height)
            ElseIf m_map_drag_offset_x < 0 Then
                aGraphics.FillRectangle(Brushes.White, ClientRectangle.Width + m_map_drag_offset_x, 0, -m_map_drag_offset_x, ClientRectangle.Height)
            End If

            If m_map_drag_offset_y > 0 Then
                aGraphics.FillRectangle(Brushes.White, 0, 0, ClientRectangle.Width, m_map_drag_offset_y)
            ElseIf m_map_drag_offset_y < 0 Then
                aGraphics.FillRectangle(Brushes.White, 0, ClientRectangle.Height + m_map_drag_offset_y, ClientRectangle.Width, -m_map_drag_offset_y)
            End If
        End If

        aGraphics.DrawImageUnscaled(m_framework.MapBitmap(), m_map_drag_offset_x, m_map_drag_offset_y)
    End Sub

    Protected Overrides Sub OnPaintBackground(ByVal e As PaintEventArgs)
        ' do nothing: the whole window is drawn by Form1_Paint
    End Sub

    Private Sub Form1_KeyPress(ByVal sender As Object, ByVal e As KeyPressEventArgs) Handles MyBase.KeyPress
        Select Case e.KeyChar
                            ' Press 'i' to zoom in.
            Case "i"c
                m_framework.Zoom(2)
                Invalidate()

                ' Press 'o' to zoom out.
            Case "o"c
                m_framework.Zoom(0.5)
                Invalidate()

                ' Press 'r' to rotate right.
            Case "r"c
                m_framework.Rotate(10)
                Invalidate()

                ' Press 'l' to rotate left.
            Case "l"c
                m_framework.Rotate(-10)
                Invalidate()

                ' Press 'p' to toggle perspective mode.
            Case "p"c
                m_framework.SetPerspective(Not m_framework.Perspective())
                Invalidate()
        End Select
    End Sub

    Protected Overrides Function ProcessCmdKey(ByRef msg As Message, ByVal keyData As Keys) As Boolean
        Select Case keyData
            Case Keys.Left
                m_framework.Pan(-50, 0)
                Invalidate()
                Return True
            Case Keys.Right
                m_framework.Pan(50, 0)
                Invalidate()
                Return True
            Case Keys.Up
                m_framework.Pan(0, -50)
                Invalidate()
                Return True
            Case Keys.Down
                m_framework.Pan(0, 50)
                Invalidate()
                Return True
        End Select

        Return False
    End Function

    Private Sub Form1_MouseDown(ByVal sender As Object, ByVal e As MouseEventArgs) Handles MyBase.MouseDown
        If e.Button = MouseButtons.Left Then
            m_map_drag_enabled = True
            m_map_drag_anchor_x = e.X
            m_map_drag_anchor_y = e.Y
            If Not m_graphics_acceleration Then m_map_drag_graphics = CreateGraphics()
        End If
    End Sub

    Private Sub Navigate(ByVal aValidity As Integer, ByVal aTime As Double, ByVal aLong As Double, ByVal aLat As Double, ByVal aSpeed As Double, ByVal aBearing As Double, ByVal aHeight As Double)
        m_framework.Navigate(aValidity, aTime, aLong, aLat, aSpeed, aBearing, aHeight)
        m_framework.GetFirstTurn(m_first_turn)
        m_framework.GetSecondTurn(m_second_turn)
        Dim message As String = ""

        Select Case m_framework.GetNavigationState()
            Case CartoType.NavigationState.None, CartoType.NavigationState.NoPosition
            Case CartoType.NavigationState.Turn
                message = m_first_turn.TurnCommand() & " after " + CInt(m_first_turn.m_distance) & "m"
                If Not m_second_turn.m_continue Then message += " then " & m_second_turn.TurnCommand() & " after " + CInt(m_second_turn.m_distance) & "m"
            Case CartoType.NavigationState.OffRoute
                message = "off route"
            Case CartoType.NavigationState.ReRouteNeeded
                message = "calculating a new route"
            Case CartoType.NavigationState.ReRouteDone
                message = "new route calculated"
            Case CartoType.NavigationState.TurnRound
                message = "turn round at the next safe and legal opportunity"
            Case CartoType.NavigationState.Arrival
                message = "arriving after " & CInt(m_first_turn.m_distance) & "m"
        End Select

        m_status_bar.Text = message
    End Sub

    Private Sub Form1_MouseUp(ByVal sender As Object, ByVal e As MouseEventArgs) Handles MyBase.MouseUp
        If e.Button = MouseButtons.Left Then
            If m_graphics_acceleration Then
                m_map_drag_enabled = False
                m_map_drag_offset_x = 0
                m_map_drag_offset_y = 0
            Else
                m_map_drag_enabled = False
                m_map_drag_offset_x = e.X - m_map_drag_anchor_x
                m_map_drag_offset_y = e.Y - m_map_drag_anchor_y
                Draw(m_map_drag_graphics)
                If m_map_drag_graphics IsNot Nothing Then m_map_drag_graphics.Dispose()
                m_map_drag_graphics = Nothing
                m_framework.Pan(-m_map_drag_offset_x, -m_map_drag_offset_y)
                m_map_drag_offset_x = 0
                m_map_drag_offset_y = 0
                Invalidate()
            End If

            ' Right-click calculates a route between the last point and this point.
        ElseIf e.Button = MouseButtons.Right Then
            Dim p = New CartoType.Point()
            p.X = e.X
            p.Y = e.Y
            m_framework.ConvertPoint(p, CartoType.CoordType.Screen, CartoType.CoordType.Degree)

            If m_last_point.X <> 0 AndAlso m_last_point.Y <> 0 Then
                m_framework.StartNavigation(m_last_point.X, m_last_point.Y, CartoType.CoordType.Degree, p.X, p.Y, CartoType.CoordType.Degree)
                Invalidate()
            End If

            m_last_point.X = p.X
            m_last_point.Y = p.Y
        End If
    End Sub

    Private Sub Form1_MouseMove(ByVal sender As Object, ByVal e As MouseEventArgs) Handles MyBase.MouseMove
        If e.Button = MouseButtons.Left Then
            m_map_drag_offset_x = e.X - m_map_drag_anchor_x
            m_map_drag_offset_y = e.Y - m_map_drag_anchor_y

            If m_graphics_acceleration Then
                m_framework.Pan(-m_map_drag_offset_x, -m_map_drag_offset_y)
                m_map_drag_offset_x = 0
                m_map_drag_offset_y = 0
                m_map_drag_anchor_x = e.X
                m_map_drag_anchor_y = e.Y
                Invalidate()
            Else
                Draw(m_map_drag_graphics)
            End If
        End If
    End Sub

    Private Sub Form1_MouseWheel(ByVal sender As Object, ByVal e As MouseEventArgs) Handles MyBase.MouseWheel
        Dim zoom_count As Integer = e.Delta / 120
        Dim zoom As Double = Math.Sqrt(2)
        If zoom_count = 0 Then zoom_count = If(e.Delta >= 0, 1, -1)
        zoom = Math.Pow(zoom, zoom_count)

        If ClientRectangle.Contains(e.Location) Then
            m_framework.ZoomAt(zoom, e.X, e.Y, CartoType.CoordType.Screen)
        Else
            m_framework.Zoom(zoom)
        End If

        Invalidate()
    End Sub

    Private Sub Form1_ClientSizeChanged(ByVal sender As Object, ByVal e As EventArgs) Handles MyBase.ClientSizeChanged
        If Not ClientSize.IsEmpty AndAlso m_framework IsNot Nothing Then
            m_framework.Resize(ClientSize.Width, ClientSize.Height)
            Invalidate()
        End If
    End Sub

    Private Sub CreateLegend()
        Dim legend As CartoType.Legend = New CartoType.Legend(m_framework)
        Dim text_color As CartoType.Color = New CartoType.Color(90, 90, 90)
        legend.SetTextColor(text_color)
        legend.SetAlignment(CartoType.Align.Center)
        Dim dataset_name As String = m_framework.DataSetName()
        dataset_name = m_framework.SetCase(dataset_name, CartoType.LetterCase.Title)
        legend.SetFontSize(10, "pt")
        legend.AddTextLine(dataset_name)
        legend.SetAlignment(CartoType.Align.Right)
        legend.SetFontSize(6, "pt")
        Dim s As String = ""
        CartoType.Util.SetAttribute(s, "ref", "M4")
        legend.AddMapObjectLine(CartoType.MapObjectType.Line, "road/major", Nothing, CInt(CartoType.RoadType.Motorway), s, "motorway")
        CartoType.Util.SetAttribute(s, "ref", "A40")
        legend.AddMapObjectLine(CartoType.MapObjectType.Line, "road/major", Nothing, CInt(CartoType.RoadType.TrunkRoad), s, "trunk road")
        CartoType.Util.SetAttribute(s, "ref", "A414")
        legend.AddMapObjectLine(CartoType.MapObjectType.Line, "road/major", Nothing, CInt(CartoType.RoadType.PrimaryRoad), s, "primary road")
        CartoType.Util.SetAttribute(s, "ref", "B4009")
        legend.AddMapObjectLine(CartoType.MapObjectType.Line, "road/mid", Nothing, CInt(CartoType.RoadType.SecondaryRoad), s, "secondary road")
        legend.AddMapObjectLine(CartoType.MapObjectType.Line, "road/mid", Nothing, CInt(CartoType.RoadType.TertiaryRoad), "High Street", "tertiary road")
        legend.AddMapObjectLine(CartoType.MapObjectType.Line, "path", "cyc", 0, "", "cycleway")
        legend.AddMapObjectLine(CartoType.MapObjectType.Line, "path", "bri", 0, "", "bridle path")
        legend.AddMapObjectLine(CartoType.MapObjectType.Line, "path", "foo", 0, "", "footpath")
        legend.AddMapObjectLine(CartoType.MapObjectType.Polygon, "land/major", "for", 0, "Ashridge", "forest or wood")
        legend.AddMapObjectLine(CartoType.MapObjectType.Polygon, "land/minor", "par", 0, "Green Park", "park, golf course or common")
        legend.AddMapObjectLine(CartoType.MapObjectType.Polygon, "land/minor", "gra", 0, "", "grassland")
        legend.AddMapObjectLine(CartoType.MapObjectType.Polygon, "land/minor", "orc", 0, "", "orchard, vineyard, etc.")
        legend.AddMapObjectLine(CartoType.MapObjectType.Polygon, "land/minor", "cmr", 0, "", "commercial or industrial")
        legend.AddMapObjectLine(CartoType.MapObjectType.Polygon, "land/minor", "cns", 0, "", "construction, quarry, landfill, etc.")
        legend.AddMapObjectLine(CartoType.MapObjectType.Point, "amenity/minor", "stn", 0, "Berkhamsted", "station")

        ' Uncomment these lines to create a scale bar as part of the legend.
        ' legend.SetAlignment(CartoType.Align.Center)
        ' legend.AddScaleLine(True)

        Dim border_color As CartoType.Color = CartoType.Color.KGray
        legend.SetBorder(border_color, 1, 4, "pt")
        Dim background_color As CartoType.Color = New CartoType.Color(255, 255, 255, 224)
        legend.SetBackgroundColor(background_color)
        m_framework.SetLegend(legend, 1, "in", CartoType.NoticePosition.TopRight)
    End Sub

End Class
