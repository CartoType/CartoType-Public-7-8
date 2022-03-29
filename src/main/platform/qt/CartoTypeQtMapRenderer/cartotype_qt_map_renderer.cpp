/*
cartotype_qt_map_renderer.cpp

Copyright (C) 2017 CartoType Ltd.
See www.cartotype.com for more information.
*/

#include "cartotype_qt_map_renderer.h"
#include "cartotype_vector_tile.h"

namespace CartoType
{

QOpenGLFunctions TheOpenGLFunctions;

/** Creates an OpenGL ES 2.0 map renderer for use with Qt. */
CQtMapRenderer::CQtMapRenderer(CFramework& aFramework):
    m_framework(aFramework)
    {
    }

/** Initialises the map renderer. Call this from QOpenGLWidget::initializeGL. */
void CQtMapRenderer::Init()
    {
    TheOpenGLFunctions.initializeOpenGLFunctions();
    m_vector_tile_server = CreateOpenGLESVectorTileServer(m_framework);
    }

/** Draws the map. Call this from QOpenGLWidget::paintGL. */
void CQtMapRenderer::Draw()
    {
    m_vector_tile_server->Draw();
    }

}
